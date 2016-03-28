#include <iostream>
#include <assert.h>

#include "state.h"
#include "edge.h"

state::state(subMachine * owner) // use only to create the Start state
{
    //ctor
    assert(owner->stateTable.size()==0);
    name="Start";
    mySubMachine=owner;
    parentState=0;
    predicate=0;
    codeFragment=0;
    codeFragmentInNewThread=false;
    owner->stateTable.insert(std::pair<state *,state *> ((state *) 0,this));
    owner->startState=this;
    numChildStates=0;
    inboundEdge.myState=this;
}

state::state(state * theParent=(state *) 0, std::string newStateName="Start")  // use to create other states besides the Start state
{
    assert(theParent->mySubMachine->stateTable.size()>0);
    for (std::multimap<state *,state *>::iterator i = theParent->mySubMachine->stateTable.begin();i!=theParent->mySubMachine->stateTable.end();i++)
        assert(i->second->name != newStateName);
    name=newStateName;
    mySubMachine=theParent->mySubMachine;
    parentState=theParent;
    predicate=0;
    codeFragment=0;
    codeFragmentInNewThread=false;
    mySubMachine->stateTable.insert(std::pair<state *,state *>(theParent,this));
    numChildStates=0;
    theParent->numChildStates++;
    inboundEdge.myState=this;
    return;
}

state * state::addChildState(std::string newName) {
    assert(newName!="Start");   // "Start" is a reserved name
    // ensure no state was previously added with this name
    for(std::multimap<state *,state *>::iterator i=mySubMachine->stateTable.begin();i!=mySubMachine->stateTable.end();i++) {
        assert(i->second->name!=newName);
    }
    state * newState = new state(this,newName);
    return newState;
}

bool state::setForwardLink(forwardLinkType newDirection,subMachine * target, bool newThread) {

    // ensure not a horizontal link in the Initial sub-machine
    assert(!(mySubMachine->name=="Initial" && newDirection==horizontal));   // initial subMachine cannot contain horizontal links
    assert(name!="Start");                                                  // start state cannot contain forward links

    // insure no forward link with similar direction exists already here or above here in the subMachine's state hierarchy
    int numPresent=0;
    state * checkState=this;  // check from this state up towards the Start state
    while ((checkState!=0) && (numPresent==0)) {
        for (std::list<forwardLink *>::iterator i=links.begin();i!=links.end();i++) {
            if ((*i)->direction==newDirection) {
                numPresent++;
                break;
            }
        }
        checkState=checkState->parentState;
    }
    // ensure valid to put this link here
    assert(numPresent==0);  // cannot set multiple links in the same direction

    // todo:  check that target subMachine is in scope
    forwardLink * newLink = new forwardLink(target,newDirection,newThread);
    links.push_back(newLink);
    return true;
}

bool state::setPredicate(predType newPredicate) {
    // ensure no predicate already present
    assert(this->predicate==0);
    this->predicate=newPredicate;
    return true;
}

bool state::setCodeFragment(codeType newCode,bool separateThread) {
    // ensure no code fragment already present
    assert(this->codeFragment==0);
    codeFragment=newCode;
    codeFragmentInNewThread=separateThread;
    return true;
}

state::~state()
{
    //dtor
}

