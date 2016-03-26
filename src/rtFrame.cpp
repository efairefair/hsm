#include "rtFrame.h"

#include <string>
#include <iostream>
#include <assert.h>
#include <thread>
#include <utility>
#include <atomic>
#include <map>

#include "hsmTypes.h"
#include "state.h"
#include "rtZone.h"
#include "variable.h"
#include "pdu.h"

#include "subMachine.h"

rtFrame::rtFrame(rtZone * theZone, subMachine * target)  {            // note: rtZone includes a free list - hope you checked there first before calling this constructor
    myZone=theZone;
    mySubMachine=target;
    currentState=target->stateTable.find( (state *) 0)->second;       // initialize currentState to the state with no parent - it's the Start state by definition
    parentFrame=0;                                                    // set parentFrame to zero - the caller must update this later
    thePdu=0;
    frameIsDormant=false;

    depth=0;
    numChildFrames=0;
    nextChild=0;
    absolutePath.clear();                                             // initialize to empty vector - caller will update
    siblingOrder=0;                                                   // caller will also update

    // instantiate (clone) the declared variables from the target subMachine
    for (auto i = target->declarations.begin(); i!=target->declarations.end(); i++) {           // reminder: declarations is map<string,pair<hsmType,variable *>>
        instantiations.insert(std::pair<std::string,variable *>(i->first, i->second.second->clone()));
    }
}

rtFrame::~rtFrame()
{
    assert((numChildFrames==0) && frameIsDormant);//dtor
}

std::string rtFrame::pathStr() {
    // print the frames absolute path as a string of dot-separated numbers
    std::string rv;
    bool pastFirst=false;

    for (auto i=absolutePath.begin();i!=absolutePath.end();i++) {
        if (pastFirst)
            rv=rv.append('.' + std::to_string(*i));
        else {
            rv=rv.append(      std::to_string(*i));
            pastFirst=true;
        }
    }
    return rv;
}

state * rtFrame::evaluateNextStates() {

    // this function evaluates some or all all possible nextStates for this frame, and return either:
    //      - a pointer to a nextState that is ready (i.e. whose predicate evaluates true), or
    //      - 0 if no possible transitions

    // get the potential next states into a range
    std::pair<std::multimap<state *, state *>::iterator,std::multimap<state *, state *>::iterator> possibleNextStates=mySubMachine->stateTable.equal_range(currentState);    // find child states of the current state

    // ensure currentState is not pointing to a final state (see makeTransition step 5 to know why this should never happen)
    assert(possibleNextStates.first != possibleNextStates.second);

    // check predicates (stop if one evalutes true)
    std::multimap<state *, state *>::iterator candidateNextState;
    for (candidateNextState=possibleNextStates.first;candidateNextState!=possibleNextStates.second;candidateNextState++) {
        //erfc myZone->logFile<<"evaluateNextStates: {" << pathStr() << "}  subMachine " << mySubMachine->name <<  " in state " << currentState->name << " checking predicate for state "<<candidateNextState->second->name<<std::endl;
        if  (
                ((candidateNextState->second->predicate==0) || (candidateNextState->second->predicate(this)))
                &&
                (
                    ( candidateNextState->second->inboundEdge.sign!=hsmRxEdge) ||
                    ((candidateNextState->second->inboundEdge.sign==hsmRxEdge) &&
                        ((thePdu != 0) && (thePdu->verb == candidateNextState->second->inboundEdge.verb))
                    )
                )
            ) {
            //erfc myZone->logFile<<"evaluateNextStates: predicate evaluated true, returning"<<std::endl;
            myZone->predicatesEvaluatedTrue++;
            return candidateNextState->second;
        }
        else {
            myZone->predicatesEvaluatedFalse++;
            //erfc myZone->logFile<<"evaluateNextStates: predicate evaluated false"<<std::endl;
        }
    }
    //erfc myZone->logFile<<"evaluateNextStates: no next state ready"<<std::endl;
    return 0;
}

void rtFrame::makeTransition(state * nextState) {

    assert((parentFrame==0 && absolutePath.size()==0) || (parentFrame !=0 && absolutePath.size()>0));

    //////////////////////////////////////////////////////////////////
    //
    // Transition this frame to nextState in five easy steps.
    //

    //////////////////////////////////////////////////////////////////
    //
    // step 1 of 5 - advance the current state pointer
    //
    //erfc myZone->logFile<< "makeTransition: {" << pathStr() << "} in subMachine: " << mySubMachine->name <<  " state: " << currentState->name << " >>>>>>>>>>>> " << nextState->name <<std::endl;
    myZone->transitionCount++;
    currentState=nextState;

    //////////////////////////////////////////////////////////////////
    //
    // step 2 of 5 - process the inbound or outbound message, if any
    //
    if (currentState->inboundEdge.sign!=hsmInternalEdge) {
        //erfc myZone->logFile<< "makeTransition: {" << pathStr() << "} in subMachine: " << mySubMachine->name <<  " state: " << currentState->name << ": processing message" <<std::endl;
        if (currentState->inboundEdge.sign==hsmRxEdge) {  // receive a message
            //erfc myZone->logFile<<"makeTransition: rx message"<<std::endl;
            assert((thePdu!=0) && (thePdu->payload.size()==currentState->inboundEdge.pList.size()));
            assert(thePdu->verb==currentState->inboundEdge.verb);
            decodeMsg();
        }
        else {                                  // transmit a message
            //erfc myZone->logFile<<"makeTransition: tx message"<<std::endl;
            assert(thePdu==0);
            encodeMsg();
        }
    }
    //erfc else {
        //erfc myZone->logFile<< "makeTransition: {" << pathStr() << "} in subMachine: " << mySubMachine->name <<  " state: " << currentState->name << ": no message to process" <<std::endl;
    //erfc }
    //////////////////////////////////////////////////////////////////
    //
    // step 3 of 5 - process forward links if any
    //
    assert (currentState->links.size()<3);

    for(auto theLink=currentState->links.begin();theLink!=currentState->links.end();theLink++) {
        //erfc myZone->logFile<<"makeTransition: processing forward link"<<std::endl;
        myZone->forwardLinksEncountered++;

        //erfc std::string logString;
        //erfc if ((*theLink)->direction==horizontal) logString="horizontal";else logString="vertical";
        //erfc myZone->logFile<<"makeTransition: {" << pathStr() << "} in subMachine: " << mySubMachine->name <<  " state: " << currentState->name << " processing "<<logString<<" forward link to: " << (*theLink)->targetSubMachine->name << std::endl;

        rtZone * newFramesZone; // there will surely be a new frame created, but the new frame may go into our zone or a new zone (depending on newThread flag)

        if ((*theLink)->newThread) {
            //erfc myZone->logFile<<"makeTransition: target subMachine to run in a new thread"<<std::endl;
            newFramesZone = myZone->myTree->makeZone();

            if ((*theLink)->direction==vertical) {
                //erfc myZone->logFile<<"makeTransition: setup new zone for vertical link"<<std::endl;
                newFramesZone->superiorZone=this->myZone;
                myZone->numActiveInferiorZones++;
            }
            else {
                //erfc myZone->logFile<<"makeTransition: setup new zone for horizontal link"<<std::endl;
                newFramesZone->superiorZone=this->parentFrame->myZone;
                parentFrame->myZone->numActiveInferiorZones++;
            }
        }
        else {
            //erfc myZone->logFile<<"makeTransition: target subMachine to run in the same thread"<<std::endl;
            newFramesZone=myZone;
        }

        rtFrame * newFrame = myZone->makeFrame(newFramesZone,(*theLink)->targetSubMachine);
        newFrame->numChildFrames=0;
        newFrame->nextChild=0;

        if ((*theLink)->direction==vertical) {
            //erfc myZone->logFile<<"makeTransition: setup new frame for vertical link"<<std::endl;
            newFrame->parentFrame=this;
            newFrame->depth=depth+1;
            numChildFrames++;
            newFrame->absolutePath=absolutePath;
            newFrame->absolutePath.push_back(0);
            newFrame->siblingOrder=0;
            nextChild++;
            assert((newFrame->depth==(depth+1)) && (newFrame->absolutePath.size()==(absolutePath.size()+1)));
        }
        else {
            //erfc myZone->logFile<<"makeTransition: setup new frame for horizontal link"<<std::endl;
            newFrame->parentFrame=parentFrame;
            newFrame->depth=depth;
            parentFrame->numChildFrames++;
            newFrame->absolutePath=parentFrame->absolutePath;
            newFrame->absolutePath.push_back(parentFrame->nextChild++);
            newFrame->siblingOrder=siblingOrder+1;
            parentFrame->nextChild++;
            assert((newFrame->depth==depth) && (newFrame->absolutePath.size()==absolutePath.size()));
        }

        if ((*theLink)->newThread){
            //erfc myZone->logFile<<"makeTransition: adding new frame to new zone and launching new detached thread"<<std::endl;
            assert(newFramesZone->activeFrames.find(newFrame->absolutePath)==newFramesZone->activeFrames.end());
            newFramesZone->activeFrames[newFrame->absolutePath]=newFrame;
            std::thread t = std::thread( (std::reference_wrapper<rtZone>) *newFramesZone );
            t.detach();
        }
        else {
            //erfc myZone->logFile<<"makeTransition: adding new frame to this same zone"<<std::endl;
            assert(myZone->activeFrames.find(newFrame->absolutePath)==myZone->activeFrames.end());
            myZone->activeFrames[newFrame->absolutePath]=newFrame;
            newFrame->myZone=myZone;

            if (newFrame->currentState->codeFragment != 0) {
                //erfc  myZone->logFile<<"makeTransition: {" << pathStr() << "}" <<\
                //erfc  " in state [" << currentState->mySubMachine->name <<  "." << currentState->name << "]" <<\
                //erfc  " running same-thread start state code fragment for subMachine:" << newFrame->mySubMachine->name << \
                //erfc  " on behalf of {"<<newFrame->pathStr()<<"}"<<std::endl;
                newFrame->currentState->codeFragment(newFrame);
            }
        }
    }
    //////////////////////////////////////////////////////////////////
    //
    // step 4 of 5 - run the code fragment of the new state
    //
    if (currentState->codeFragment!=0) {
        //erfc  myZone->logFile<<"makeTransition: {" << pathStr() << "} in state [" << currentState->mySubMachine->name <<  "." << currentState->name << "] running code fragment"<<std::endl;
        if (currentState->codeFragmentInNewThread) {
            //erfc myZone->logFile<<"makeTransition: launching new detached thread for code fragment"<<std::endl;
            std::thread t = std::thread( currentState->codeFragment,this);
            t.detach();
        }
        else {
            //erfc myZone->logFile<<"makeTransition: running code fragment in same thread"<<std::endl;
            currentState->codeFragment(this);
        }
    }
    //////////////////////////////////////////////////////////////////
    //
    // step 5 of 5 - handle transition into a final state
    //
    if (currentState->numChildStates==0) {
        //erfc myZone->logFile<<"makeTransition: {" << pathStr() << "} in subMachine " << mySubMachine->name <<  " state " << currentState->name << " has reached a final state" << std::endl;
        frameIsDormant=true;
        myZone->activeFrames.erase(absolutePath);
        myZone->dormantFrames[absolutePath]=this;

        rtFrame * candidate=this; // find candidates for deletion - start with this frame and work up towards the root frame.  Limited to just this zone, just dormant frames with no children

        while ((candidate!=0) && (candidate->myZone==myZone) && (candidate->frameIsDormant) && (candidate->numChildFrames==0)) {

            // get references to some candidate data (remember: horizontal links are not allowed in the root frame)

            myZone->freeFrames.insert(std::pair<std::string,rtFrame *>(candidate->mySubMachine->name,candidate));
            myZone->dormantFrames.erase(candidate->absolutePath);

            if (candidate->parentFrame!=0) {
                candidate->parentFrame->numChildFrames--;
            }
            candidate=candidate->parentFrame;
        }
    }
    return;
}

void rtFrame::decodeMsg() {
    // retrieve an incoming pdu - move the data from the pdu to final destinations at or above this frame
    // reminder 1: the incoming pdu is stored in the rtFrame
    // reminder 2: the destinations are determined by the edge
    assert(thePdu!=0);
    assert(thePdu->payload.size()==currentState->inboundEdge.pList.size());
    assert(thePdu->verb==currentState->inboundEdge.verb);

    variable * varbind;

    for(auto i=currentState->inboundEdge.pList.begin();i!=currentState->inboundEdge.pList.end();i++) {
        varbind = *(thePdu->payload.begin());
        assert(depth >= i->theHeight);                              // ensure the height does not exceed the depth
        rtFrame * dstFrame = this;                                    // find the rtFrame where it goes
        for(hsmUint j=0;j<i->theHeight;j++)
            dstFrame=parentFrame;
        *(dstFrame->instantiations[i->theName])=*varbind;
        varbind++;
    }
    std::pair<std::string, std::string> ix(mySubMachine->name, currentState->inboundEdge.verb);
    myZone->freeMsgs.emplace(std::make_pair(std::make_pair(mySubMachine->name, currentState->inboundEdge.verb),thePdu));
    thePdu=0;
}

void rtFrame::encodeMsg() {

    assert(thePdu==0);
    size_t varbindNumber=0;

    //////////////////////////////////////
    //
    // try to allocate a previously freed PDU
    // note: free list key is "subMachine name", "verb"
    //

    auto i=myZone->freeMsgs.find(std::pair<std::string, std::string>(this->mySubMachine->name, currentState->inboundEdge.verb));

    if (i==myZone->freeMsgs.end()) { // if no previously freed PDU is available, create a new one, and initialize it's payload
        thePdu = new pdu();
        // initialize the payload of the new pdu using declarations
        thePdu->payload.resize(currentState->inboundEdge.pList.size());
        for (auto j = currentState->inboundEdge.pList.begin();
            j != currentState->inboundEdge.pList.end();
            j++) {
                std::string varToClone=j->theName;
                subMachine * theSrcSubMachine = currentState->mySubMachine;
                for (auto k=0;k<j->theHeight;k++)
                    theSrcSubMachine=theSrcSubMachine->parentSubMachine;
                variable * theSrcVariable=theSrcSubMachine->declarations[varToClone].second;
                thePdu->payload[varbindNumber]=theSrcVariable->clone();
            }
        varbindNumber++;
        }
    else {
        thePdu = i->second;
        myZone->freeMsgs.erase(i);
        thePdu->payload.clear();
    }

    thePdu->verb=currentState->inboundEdge.verb;


    varbindNumber=0;
    for(auto        p= currentState->inboundEdge.pList.begin();  // traverses parameters (reminder: edge parameters are tuple of <string, type, heightAbove>)
                    p!=currentState->inboundEdge.pList.end();
                    p++) {
        assert(depth >= p->theHeight);
        // find the target frame where this part of the message will come from
        rtFrame * srcFrame=this;
        for(unsigned int j=0;j<p->theHeight;j++)
            srcFrame=parentFrame;

        /////////////////////////////////////////////
        //
        // copy the variable described in this pListEntry to the payload
        //

        *(thePdu->payload[varbindNumber]) = *(srcFrame->instantiations[p->theName]);
        varbindNumber++;
    }
}
