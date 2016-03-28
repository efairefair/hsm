#include <iostream>
#include <assert.h>

#include "hsmTypes.h"

#include "subMachine.h"
#include "state.h"
//#include "myAtomicInt.h"

subMachine::subMachine(hsm * theMachine)    // use to create the Initial subMachine
{
    name="Initial";
    myHsm=theMachine;
    parentSubMachine=0;
    assert(myHsm->subMachineTable.size()==0);
    myHsm->subMachineTable.insert(std::pair<subMachine *,subMachine *> ((subMachine *) 0, this));
    startState = new state(this);
    //ctor
}

subMachine::subMachine(subMachine * parent,std::string newName) { //use to create other subMachines besides the Initial subMachine
    name=newName;
    myHsm=parent->myHsm;
    parentSubMachine=parent;
    assert(myHsm->subMachineTable.size()>0);
    myHsm->subMachineTable.insert(std::pair<subMachine *,subMachine *>(parent,this));
}

subMachine * subMachine::addSubMachine(std::string newName)  // use to create all other subMachines
{
    assert(myHsm->subMachineTable.size()>0);
    // ensure subMachine with this name doesn't already exist
    for (std::multimap<subMachine *, subMachine *>::iterator i = myHsm->subMachineTable.begin();i!=myHsm->subMachineTable.end();i++) {
        assert(i->second->name!=newName);
    }
    subMachine * newChild = new subMachine(this,newName);
    state * newInitialState = new state(newChild);
    newChild->stateTable.insert(std::pair<state *,state *> ((state *)0, newInitialState));
    return newChild;
    //ctor
}

void subMachine::addVariable(std::string newName, variable * initialValue, hsmType newType)
{
    // this is called during building of the hsm only - not during execution
    // ensure variable of this name not previously declared
    // clone a copy of the initialValue
    assert(declarations.find(newName)==declarations.end());
    std::pair<hsmType,variable *> newRHS(newType,initialValue->clone());
    declarations.insert(std::pair<std::string,std::pair<hsmType, variable *>>(newName,newRHS));
    return;
}

subMachine::~subMachine()
{
    //dtor
    for(std::multimap<state *,state *>::iterator i=stateTable.begin();i!=stateTable.end();i++)
        delete i->second;
    for(std::map<std::string, std::pair<hsmType,variable *>>::iterator j=declarations.begin();j!=declarations.end();j++)
        delete j->second.second;
}

