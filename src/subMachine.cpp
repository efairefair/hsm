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

subMachine::subMachine(subMachine * parent,std::string newName) {
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

bool subMachine::addVariable(std::string newName, hsmVarType newType)
{
    // ensure variable not previously declared
    assert(declarations.find(newName)==declarations.end());
    // ensure listInt type (this is the only hsmType that has no initialValue provided (e.g. signature has only a name and type)
    assert(newType==hsmListIntType);
    variable varToAdd(newType);
    declarations.insert(std::pair<std::string,variable>(newName,varToAdd));
    return true;
}


bool subMachine::addVariable(std::string newName, hsmVarType newType, hsmInt initialValue)
{
    // ensure variable not previously declared
    assert(declarations.find(newName)==declarations.end());
    variable varToAdd(newType,initialValue);
    declarations.insert(std::pair<std::string,variable>(newName,varToAdd));
    return true;
}

bool subMachine::addVariable(std::string newName, hsmVarType newType, hsmBool initialValue)
{
    // ensure variable not previously declared
    assert(declarations.find(newName)==declarations.end());
    declarations.insert(std::pair<std::string,variable>(newName,variable(newType,initialValue)));
    return true;
}

bool subMachine::addVariable(std::string newName, hsmVarType newType, hsmString initialValue)
{
    // ensure variable not previously declared
    assert(declarations.find(newName)==declarations.end());
    declarations.insert(std::pair<std::string,variable>(newName,variable(newType,initialValue)));
    return true;
}

bool subMachine::addVariable(std::string newName, hsmVarType newType, hsmListInt initialValue)
{
    // ensure variable not previously declared
    assert(declarations.find(newName)==declarations.end());
    declarations.insert(std::pair<std::string,variable>(newName,variable(newType,initialValue)));
    return true;
}

subMachine::~subMachine()
{
    //dtor
    for(std::multimap<state *,state *>::iterator i=stateTable.begin();i!=stateTable.end();i++)
        delete i->second;
    stateTable.clear();
}

