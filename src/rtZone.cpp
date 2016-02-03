#include "rtZone.h"
#include "rtFrame.h"

#include <iterator>
#include <list>
#include <map>
#include <assert.h>
#include <algorithm>
#include <unistd.h>

#include "hsmTypes.h"
#include "subMachine.h"
#include "state.h"
#include "variable.h"

//
// note: the rtZone class includes an ofstream member, and ofstreams cannot be copied.  Thus no copy or assignment operators are defined for rtZone
//

void rtZone::operator()() {
    //
    // thread entry point for any zone
    //
    // first create an eyecatcher -- a unique string for the zone.  Use the path of the initial frame in the zone.  Note: initial zone will have zero-length eyecatcher.
    // this is used for several purposes, most notably as part of the per-zone log file name.

    eyeCatcher=activeFrames.begin()->second->pathStr();

    // open a log file for this zone only-- useful because sharing an ostream between zones/threads is not practical due to resulting interleaved output
    //erfc logFile.open("/tmp/{"+eyeCatcher+"}",std::ios::out);
    //erfc logFile<<"zone starting"<<std::endl;

    executeActivePhase();       // make state transitions for the frame(s) in the zone until no active frames remain
                                // note: dormant frames may still remain afterwards

    myTree->forwardLinksEncountered    += forwardLinksEncountered;  // for you performance and accounting types
    myTree->newFramesCreated           += newFramesCreated;
    myTree->oldFramesDeleted           += oldFramesDeleted;
    myTree->oldFramesReused            += oldFramesReused;
    myTree->predicatesEvaluatedFalse   += predicatesEvaluatedFalse;
    myTree->predicatesEvaluatedTrue    += predicatesEvaluatedTrue;
    myTree->totalSlept                 += totalSlept;
    myTree->transitionCount            += transitionCount;

    //erfc logFile<<"operator(): zone done, locking self"<<std::endl;
    zoneLock.lock();
    //erfc logFile<<"operator(): zone done, self locked"<<std::endl;

    // make a sweep with the zone locked to delete any remaining dormant frames with no children
    unsigned int dormantFramesFreed=0;
    while (dormantFramesFreed>0) { // while loop always loops at least once
        dormantFramesFreed=0;
        for(auto i=dormantFrames.begin();i!=dormantFrames.end();i++) {
            //erfc logFile<<"operator(): considering: {"<<i->second->pathStr()<<"}"<<std::endl;
            if (i->second->numChildFrames==0) {
                //erfc logFile<<"operator(): considering: {"<<i->second->pathStr()<<"} has no childFrames"<<std::endl;
                dormantFramesFreed++;
                dormantFrames.erase(i->second->absolutePath);   // caution: this is deleting from the map being iterated
                if (i->second->parentFrame != 0) {
                    //erfc logFile<<"operator():  C: {"<<i->second->parentFrame->pathStr()<<"} numChildFrames: "<<i->second->parentFrame->numChildFrames<<std::endl;
                }
                freeFrames.insert(std::pair<std::string,rtFrame *>(i->second->mySubMachine->name,i->second));
                break;
            }
            //erfc else {
            //erfc     logFile<<"operator(): considering: {"<<i->second->pathStr()<<"} has childFrames"<<std::endl;
            //erfc }
        }
    }
    //erfc logFile<<"operator(): marking zone ("<<eyeCatcher<<") inactive"<<std::endl;
    zoneIsInactive=true;
    if (superiorZone!=0) {
        //erfc logFile<<"operator(): decrementing superior zone ("<<superiorZone->eyeCatcher<<") numActiveChildZones"<<std::endl;
        superiorZone->zoneLock.lock();
        superiorZone->numActiveInferiorZones--;
        superiorZone->zoneLock.unlock();
        //erfc logFile<<"operator(): done decrementing superior zone ("<<superiorZone->eyeCatcher<<") numActiveChildZones"<<std::endl;
    }
    //erfc logFile<<"operator(): zone done, unlocking self"<<std::endl;
    zoneLock.unlock();
    //erfc logFile<<"zone done, self lock released, deleting"<<std::endl;
    //erfc logFile<<"operator(): zone done, tryDeleting"<<std::endl;
    // note: don't do anything with "this" after tryDelete() call (this will likely get deleted)

    rtTree * originalTree=myTree;
    if (tryDelete(this) ) {
         originalTree->wait_c.notify_all();   // if we deleted the top zone... we are done!
    }
    return; // note: this is where the thread associated with the zone exits
}

bool rtZone::tryDelete(rtZone * deletingZone) {

    // try to delete this zone (and, if deleted, recursively try to delete the parent too; return TRUE if top-most zone is deleted (I.E. the hsm is done executing!)
    // original zone performing the delete is passed in as deletingZone

    //erfc deletingZone->logFile<<"tryDelete: "<<"("<<eyeCatcher<<")"<<" locking self zone"<<std::endl;
    zoneLock.lock();
    //erfc deletingZone->logFile<<"tryDelete: "<<"("<<eyeCatcher<<")"<<" locked self zone"<<std::endl;
    //
    // handle case where zone is not dormant
    //
    if ((!zoneIsInactive)) {
        // this zone is not dormant -- its thread is still accessing frames -- we can't really do anything to it
        //erfc deletingZone->logFile<<"tryDelete: "<<"("<<eyeCatcher<<")"<<" unlocking self zone (not dormant)"<<std::endl;
        zoneLock.unlock();
        //erfc deletingZone->logFile<<"tryDelete: "<<"("<<eyeCatcher<<")"<<" unlocked self zone (not dormant) and returning false"<<std::endl;
        return false;
    }


    //
    // zone is locked and inactive -- its thread is not accessing frames any longer
    // sweep through and remove any dormant frames with no child frames
    // this MT activity is mutexed via zoneLock
    //
    size_t dormantFramesFreed;
    while (dormantFramesFreed>0) { // while loop - at least one iteration
        dormantFramesFreed=0;
        //erfc deletingZone->logFile<<"tryDelete: "<<"("<<eyeCatcher<<")"<<" before sweep through dormant frames (active/dormant/free: "<<activeFrames.size()<<"/"<<dormantFrames.size()<<"/"<<freeFrames.size()<<")"<<std::endl;
        for(auto i=dormantFrames.begin();i!=dormantFrames.end();i++) {
            if (i->second->numChildFrames==0) {
                dormantFramesFreed++;
                freeFrames.insert(std::pair<std::string,rtFrame *>(i->second->mySubMachine->name,i->second));

                if (i->second->parentFrame != 0) {
                    i->second->parentFrame->numChildFrames--;
                    //erfc deletingZone->logFile<<"tryDelete: C: {"<<i->second->parentFrame->pathStr()<<"} numChildFrames: "<<i->second->parentFrame->numChildFrames<<std::endl;
                }

                dormantFrames.erase(i->second->absolutePath); // careful: erasing from map we're iterating.
                break;
            }
        }
        //erfc deletingZone->logFile<<"tryDelete: "<<"("<<eyeCatcher<<")"<<" after sweep through dormant frames (active/dormant/free: "<<activeFrames.size()<<"/"<<dormantFrames.size()<<"/"<<freeFrames.size()<<")"<<std::endl;
    }
    //erfc deletingZone->logFile<<"tryDelete: "<<"("<<eyeCatcher<<")"<<" done sweeping up (active/dormant/free: "<<activeFrames.size()<<"/"<<dormantFrames.size()<<"/"<<freeFrames.size()<<")"<<std::endl;

    //
    // zone is locked and inactive, check if root zone with no dormant frames remaining (if yes, return true to indicate "hsm execution complete")
    //
    //erfc deletingZone->logFile<<"tryDelete: "<<"("<<eyeCatcher<<")"<<" check if hsm done"<<std::endl;
    if ((dormantFrames.size()==0) && (superiorZone==0)) {
        //erfc deletingZone->logFile<<"tryDelete: "<<"("<<eyeCatcher<<")"<<" topmost zone, unlocking self, deleting self, and returning true"<<std::endl;
        zoneLock.unlock();
        freeZone();
        return true;
    }

    //
    // zone is locked and inactive, handle case where dormant frames remain
    //
    //erfc deletingZone->logFile<<"tryDelete: "<<"("<<eyeCatcher<<")"<<" hsm not done, check if any dormant frames"<<std::endl;
    if (dormantFrames.size()>0) {
        //erfc deletingZone->logFile<<"tryDelete: "<<"("<<eyeCatcher<<")"<<" dormant frames detected, returning false"<<std::endl;
        zoneLock.unlock();
        return false;
    }

    //
    // zone is locked, inactive, and no dormant frames remaining
    //
    //erfc deletingZone->logFile<<"tryDelete: "<<"("<<eyeCatcher<<")"<<" no dormant frames, assert superior zone present and tryDelete the superior"<<std::endl;
    assert (superiorZone!=0);  // we checked for this case earlier

    //erfc deletingZone->logFile<<"tryDelete: "<<"("<<eyeCatcher<<")"<<" tryDelete superior zone"<<std::endl;
    bool rv=superiorZone->tryDelete(deletingZone);
    //erfc deletingZone->logFile<<"tryDelete: "<<"("<<eyeCatcher<<")"<<" tryDelete of superior zone completed, unlocking self, deleting self, and returning rv: "<<rv<<" from tryDelete of superior"<<std::endl;
    zoneLock.unlock();
    freeZone();
    return rv;
}

void rtZone::freeZone() {
    myTree->treeLock.lock();
    myTree->freeZones.push_back(this);
    myTree->treeLock.unlock();
}

rtFrame * rtZone::makeFrame(rtZone * theZone, subMachine * targetSubMachine)  {

    rtFrame * newFrame;
    auto i = freeFrames.find(targetSubMachine->name);       // find a previous frame that instantiated the same targetSubMachine
                                                            // note: this allows us to avoid allocating variables every time
    if (i!=freeFrames.end()) {
        oldFramesReused++;
        newFrame=i->second;
        freeFrames.erase(i);
        newFrame->myZone=theZone;
        newFrame->mySubMachine=targetSubMachine;
        newFrame->currentState=targetSubMachine->stateTable.find( (state *) 0)->second;         // initialize current-state to Start state of targetSubMachine
        newFrame->nextChild=0;
        newFrame->numChildFrames=0;
        newFrame->parentFrame=0;
        newFrame->frameIsDormant=false;
        assert(newFrame->myMsg==0);
    }
    else {
        newFrame = new rtFrame(theZone,targetSubMachine);
        newFramesCreated++;
    }
    // instantiate or re-initialize the variables
    for(std::map<std::string,variable>::iterator j = targetSubMachine->declarations.begin();j!=targetSubMachine->declarations.end();j++) {
        newFrame->instantiations[j->first]=j->second;   // re-initialize the variables
    }

    return newFrame;
}

rtZone::rtZone(rtTree * theTree) {
    zoneIsInactive=false;
    myTree=theTree;
    forwardLinksEncountered=0;
    oldFramesDeleted=0;
    newFramesCreated=0;
    oldFramesReused=0;
    totalSlept=0;
    numActiveInferiorZones=0;
    superiorZone=0;
    transitionCount=0;
}

rtZone::~rtZone()
{
    assert(dormantFrames.size()==0);
    assert(activeFrames.size()==0);
    assert(zoneIsInactive);
    for (auto i=freeMsgs.begin();i!=freeMsgs.end();i++) {
        delete i->second;
    }
    //dtor
}

std::pair<rtFrame *, state *> rtZone::evaluateFrames() {

    state * potentialNextState;
    size_t iterationCount=0;
    // consider active frames first
    for (auto j=activeFrames.begin();j!=activeFrames.end();j++) {
        //erfc logFile << "evaluateAllFrames: evaluating {" << j->second->pathStr() << "}"<<std::endl;
        potentialNextState = j->second->evaluateNextStates();
        if (potentialNextState!=0 ) {
            //erfc  logFile << "evaluateAllFrames: frame {" << j->second->pathStr() << "} ready for transition"<<std::endl;
            return std::pair<rtFrame *, state *> (j->second,potentialNextState);
        }
        //erfc  else {
            //erfc  logFile << "evaluateAllFrames: frame {" << j->second->pathStr() << "} NOT ready for transition"<<std::endl;
        //erfc  }
        iterationCount++;
    }
    // consider dormant frames second
    for (auto j=dormantFrames.begin();j!=dormantFrames.end();j++) {
        if (j->second->numChildFrames==0) {
            return std::pair<rtFrame *, state *>( (rtFrame *) j->second, (state *) 0);
        }
    }

    // nothing to do
    //erfc  logFile << "evaluateAllFrames: no frame ready for transition"<<std::endl;
    return std::pair<rtFrame *, state *>( (rtFrame *) 0, (state *) 0);
}



void rtZone::executeActivePhase() {

    //erfc  logFile << "executeActivePhase: begin active phase"<<std::endl;

    assert(activeFrames.size()==1);                            // zone always starts with exactly one frame

    rtFrame * initialFrame = activeFrames.begin()->second;     // run initial frame's Start state code fragment if present

    if (initialFrame->currentState->codeFragment != 0) {
        if (initialFrame->currentState->codeFragmentInNewThread) {
            //erfc  logFile<<"executeActivePhase: frame {" << initialFrame->pathStr() << "} in state [" << initialFrame->mySubMachine->name <<  "." << initialFrame->currentState->name << "] running start state code fragment in new thread"<<std::endl;
            std::thread t(initialFrame->currentState->codeFragment,initialFrame);
            t.detach();
        }
        else {
            //erfc  logFile<<"executeActivePhase: frame {" << initialFrame->pathStr() << "} in state [" << initialFrame->mySubMachine->name <<  "." << initialFrame->currentState->name << "] running start state code fragment in same thread"<<std::endl;
            initialFrame->currentState->codeFragment(initialFrame);
        }
    }
    //erfc  else {
    //erfc      logFile<<"executeActivePhase: frame {" << initialFrame->pathStr() << "} in state [" << initialFrame->mySubMachine->name <<  "." << initialFrame->currentState->name << "] no start state code fragment"<<std::endl;
    //erfc  }

    // initialize sleep timer - this code uses periodic sleeps to avoid tight cpu loops when none of the frames in the zone aren't transitioning; sleep time bound by min and max
    unsigned int minSleepTime(10), maxSleepTime(minSleepTime*100), sleepTime(minSleepTime);

    while (activeFrames.size()>0) {
        //erfc  logFile << "executeActivePhase: looping, active(dormant/free): " << activeFrames.size() << "(" << dormantFrames.size() << "/"<<freeFrames.size()<<") deleted: " << freeFrames.size() << ", transitionCount: " << transitionCount << ", newFramesCreated: " << newFramesCreated << ", oldFramesReused: " << oldFramesReused << ", forwardLinksEncountered: " << forwardLinksEncountered << ", oldFramesDeleted: " << oldFramesDeleted << ", totalSlept: " << totalSlept << std::endl;
        //
        // find frame and target state ready to transition
        // returns two pointers to a frame and state
        // possible return values:
        // 1) &frame,   &state:   an actual state change.  this frame needs to transition to this state
        // 2) &frame,   NULL:     a dormant frame that has no children.  this frame needs to be moved from dormant to free
        // 3) NULL,     NULL:     nothing to do.
        //
        std::pair<rtFrame *, state *> readyToTransition = evaluateFrames();

        if (readyToTransition.first != (rtFrame *) 0 ) {    // there is a frame ready to transition, reset timer
            //erfc logFile<<"executeActivePhase: resetting sleepTime"<< std::endl;
            sleepTime=minSleepTime;
        }
        else {                                              // there is no frame ready to transition, sleep some, double the sleep timer, and restart while loop
            //erfc logFile<<"executeActivePhase: sleeping for: "<< sleepTime<< " milliseconds; totalSleepTime: "<<totalSlept<<std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
            totalSlept+=sleepTime;
            sleepTime=std::min(sleepTime*2, maxSleepTime);
            continue;
        }

        // check if a dormant frame needs to be freed
        if ((readyToTransition.first!=0) && (readyToTransition.second==0)) {
            dormantFrames.erase(readyToTransition.first->absolutePath);
            freeFrames.insert(std::pair<std::string,rtFrame *>(readyToTransition.first->mySubMachine->name,readyToTransition.first));
            continue;
        }
        //erfc logFile<<"executeActivePhase: {"<<readyToTransition.first->pathStr()<<"} make transition to state: "<<readyToTransition.second->name<< std::endl;

        readyToTransition.first->makeTransition(readyToTransition.second);  // note: this call may mark first as dormant or free it (and it might get re-used) so don't use readyToTransition after this call

        //erfc logFile<<"executeActivePhase: finished transition"<< std::endl;
    }
    //erfc  logFile << "executeActivePhase: end active phase"<<std::endl;
}


