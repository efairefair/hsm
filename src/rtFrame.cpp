#include "rtFrame.h"

#include <string>
#include <iostream>
#include <assert.h>
#include <thread>
#include <utility>

#include "hsmTypes.h"
#include "state.h"
#include "rtZone.h"
#include "variable.h"
#include "msg.h"

#include "subMachine.h"

rtFrame::rtFrame(rtZone * theZone, subMachine * target)  {
    myZone=theZone;
    mySubMachine=target;
    currentState=target->stateTable.find( (state *) 0)->second;
    nextChild=0;
    numChildFrames=0;
    parentFrame=0;
    myMsg=0;
    frameIsDormant=false;

    // instantiate (copy) the declared variables in target subMachine
    for (auto i = target->declarations.begin(); i!=target->declarations.end(); i++) {
            instantiations[i->first] = target->declarations[i->first];
    }
}

rtFrame::~rtFrame()
{
    assert((numChildFrames==0) && frameIsDormant);//dtor
}

std::string rtFrame::pathStr() {
    // print the absolute path as a string of dot-separated numbers
    std::string rv;
    bool pastFirst=false;
    for (auto i = absolutePath.begin();i!=absolutePath.end();i++) {
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

    // this function should evaluate the possible nextStates for the currentState of this frame, and return either:
    //      - a pointer to a nextState that is ready (i.e. whose predicate evaluates true), or
    //      - 0 if no possible transitions

    // get the potential next states into a range
    std::pair<std::multimap<state *, state *>::iterator,std::multimap<state *, state *>::iterator> possibleNextStates=mySubMachine->stateTable.equal_range(currentState);    // find child states of the current state

    // ensure currentState is not pointing to a final state (see makeTransition step 5 to know why this should never happen)
    assert(possibleNextStates.first != possibleNextStates.second);

    // check each predicate until one evalutes true
    std::multimap<state *, state *>::iterator candidateNextState;
    for (candidateNextState=possibleNextStates.first;candidateNextState!=possibleNextStates.second;candidateNextState++) {
        //erfc  myZone->logFile<<"evaluateNextStates: {" << pathStr() << "}  subMachine " << mySubMachine->name <<  " in state " << currentState->name << " checking predicate for state "<<candidateNextState->second->name<<std::endl;
        // understanding this boolean expression here is the key to understanding state transitions...
        if  (
                ((candidateNextState->second->predicate==0) || (candidateNextState->second->predicate(this)))
                &&
                (
                    ( candidateNextState->second->inboundEdge.sign!=hsmRxEdge) ||
                    ((candidateNextState->second->inboundEdge.sign==hsmRxEdge) &&
                        ((myMsg != 0) && (myMsg->verb == candidateNextState->second->inboundEdge.verb))
                    )
                )
            ) {
            //erfc  myZone->logFile<<"evaluateNextStates: predicate evaluated true, returning"<<std::endl;
            myZone->predicatesEvaluatedTrue++;
            return candidateNextState->second;
        }
        else {
            myZone->predicatesEvaluatedFalse++;
            //erfc  myZone->logFile<<"evaluateNextStates: predicate evaluated false"<<std::endl;
        }
    }
    //erfc  myZone->logFile<<"evaluateNextStates: no next state ready"<<std::endl;
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
    //erfc  myZone->logFile<< "makeTransition: {" << pathStr() << "} in subMachine: " << mySubMachine->name <<  " state: " << currentState->name << " >>>>>>>>>>>> " << nextState->name <<std::endl;
    myZone->transitionCount++;
    currentState=nextState;

    //////////////////////////////////////////////////////////////////
    //
    // step 2 of 5 - process the inbound or outbound message, if any
    //
    if (currentState->inboundEdge.sign!=hsmInternalEdge) {
        //erfc  myZone->logFile<< "makeTransition: {" << pathStr() << "} in subMachine: " << mySubMachine->name <<  " state: " << currentState->name << ": processing message" <<std::endl;
        if (currentState->inboundEdge.sign==hsmRxEdge) {  // receive a message
            //erfc myZone->logFile<<"makeTransition: rx message"<<std::endl;
            assert((myMsg!=0) && (myMsg->payload.size()==currentState->inboundEdge.parameters.size()));
            assert(myMsg->verb==currentState->inboundEdge.verb);
            decodeMsg();
        }
        else {                                  // transmit a message
            //erfc myZone->logFile<<"makeTransition: tx message"<<std::endl;
            assert(myMsg==0);
            encodeMsg();
        }
    }
    else {
        myZone->logFile<< "makeTransition: {" << pathStr() << "} in subMachine: " << mySubMachine->name <<  " state: " << currentState->name << ": no message to process" <<std::endl;
    }
    //////////////////////////////////////////////////////////////////
    //
    // step 3 of 5 - process forward links if any
    //
    assert (currentState->links.size()<3);
    //std::list<forwardLink *>::iterator theLink;
    for(auto theLink=currentState->links.begin();theLink!=currentState->links.end();theLink++) {
        //erfc myZone->logFile<<"makeTransition: processing forward link"<<std::endl;
        myZone->forwardLinksEncountered++;

        //erfc  std::string logString;
        //erfc  if ((*theLink)->direction==horizontal) logString="horizontal";else logString="vertical";
        //erfc  myZone->logFile<<"makeTransition: {" << pathStr() << "} in subMachine: " << mySubMachine->name <<  " state: " << currentState->name << " processing "<<logString<<" forward link to: " << (*theLink)->targetSubMachine->name << std::endl;

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

        if ((*theLink)->direction==vertical) {
            //erfc myZone->logFile<<"makeTransition: setup new frame for vertical link"<<std::endl;
            newFrame->parentFrame=this;
            numChildFrames++;
            //erfc myZone->logFile<<"makeTransition: {"<<pathStr()<<"} V: {"<<pathStr()<<"} numChildFrames: "<<numChildFrames<<std::endl;
            newFrame->absolutePath=absolutePath;            // copy operation
            newFrame->absolutePath.push_back(nextChild++);
        }
        else {
            //erfc myZone->logFile<<"makeTransition: setup new frame for horizontal link"<<std::endl;
            newFrame->parentFrame=parentFrame;
            parentFrame->numChildFrames++;
            //erfc myZone->logFile<<"makeTransition: {"<<pathStr()<<"} H: {"<<parentFrame->pathStr()<<"} numChildFrames: "<<parentFrame->numChildFrames<<std::endl;
            assert(absolutePath.size()>0);
            newFrame->absolutePath=absolutePath;            // copy operation
            newFrame->absolutePath.pop_back();
            newFrame->absolutePath.push_back(parentFrame->nextChild++);
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

        rtFrame * candidateForDeletion=this;
        while ((candidateForDeletion!=0) && (candidateForDeletion->myZone==myZone) && (candidateForDeletion->frameIsDormant) && (candidateForDeletion->numChildFrames==0)) {
            myZone->freeFrames.insert(std::pair<std::string,rtFrame *>(candidateForDeletion->mySubMachine->name,candidateForDeletion));
            myZone->dormantFrames.erase(candidateForDeletion->absolutePath);
            if (candidateForDeletion->parentFrame!=0) {
                candidateForDeletion->parentFrame->numChildFrames--;
            }
            candidateForDeletion=candidateForDeletion->parentFrame;
        }
    }
    return;
}
variable rtFrame::siblingOrder() {
    variable rv;
    assert(absolutePath.size()>0);
    rv.theType=hsmUIntType;
    rv.u=absolutePath.back();
    return rv;
}

soType rtFrame::numSiblings() {
    if (parentFrame==0)
        return 0;
    else
        return parentFrame->numChildFrames-1;
}

void rtFrame::decodeMsg() {
    // reminder: msg has a verb and vector of variables
    auto oneVariable=myMsg->payload.begin();                             // traverses message payload (reminder: payload is a vector of pair<type,pointer to data> )
    for(auto oneParameter=currentState->inboundEdge.parameters.begin();  // traverses edge parameters
                                                                         // (reminder: edge parameters are tuple of <name, type, heightAbove>)
        oneParameter!=currentState->inboundEdge.parameters.end();
        oneParameter++) {
        // ensure the data types match
        assert(oneVariable->theType==std::get<1>(*oneParameter));
        // find the target frame where the adverb will be stored
        rtFrame * targetFrame=this;
        for(unsigned int i=0;i<std::get<2>(*oneParameter);i++) {
            assert (targetFrame->parentFrame!=0);
            targetFrame=parentFrame;
        }

        targetFrame->instantiations[std::get<0>(*oneParameter)]=(*oneVariable);
        oneVariable++;
    }
    std::pair<std::string, std::string> ix = std::pair<std::string, std::string>(mySubMachine->name, currentState->inboundEdge.verb);
    myZone->freeMsgs.insert(std::pair<std::pair<std::string, std::string>,msg*>(ix,myMsg));
    myMsg=(msg *) 0;
}

void rtFrame::encodeMsg() {
    auto i=myZone->freeMsgs.find(std::pair<std::string, std::string>(this->mySubMachine->name, currentState->inboundEdge.verb));
    if (i==myZone->freeMsgs.end())
        myMsg = new msg();
    else {
        myMsg = i->second;
        myZone->freeMsgs.erase(i);
    }

    myMsg->verb = currentState->inboundEdge.verb;
    myMsg->payload.clear();

    for(auto oneParameter=currentState->inboundEdge.parameters.begin();  // traverses edge parameters (reminder: edge parameters are tuple of <string, type, heightAbove>)
        oneParameter!=currentState->inboundEdge.parameters.end();
        oneParameter++) {

        // find the target frame where the adverb will be stored
        rtFrame * targetFrame=this;
        for(unsigned int j=0;j<std::get<2>(*oneParameter);j++) {
            assert (targetFrame->parentFrame!=0);
            targetFrame=parentFrame;
        }

        // ensure the data types match from the parameters and targetFrame variable
        assert(std::get<1>(*oneParameter)==targetFrame->instantiations[std::get<0>(*oneParameter)].theType);
        myMsg->payload.push_back(targetFrame->instantiations[std::get<0>(*oneParameter)]);

    }
}
