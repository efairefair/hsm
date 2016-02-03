#include "machine.h"
#include "subMachine.h"
#include "state.h"
#include "rtTree.h"

machine::machine()
{
    //ctor
    isExecuting=false;
    initial = new subMachine(0); // null pointer for inital subMachine only
}

bool machine::execute()
{
    rtTree * tree;

    if (isExecuting==false) {
        isExecuting=true;
        tree=new rtTree(this);
        return true;
    }
    else {
        return false;
    }
}
machine::~machine()
{
    //dtor
}
