#include "edge.h"

#include <assert.h>
#include <tuple>

#include "hsmTypes.h"
#include "state.h"
#include "variable.h"

void edge::setVerb(std::string newVerb, hsmEdgeType newSign) {
    // ensure a verb was not previously set
    assert (verb.size()==0);
    // ensure verb was not previously set in another state within this subMachine
    for(auto i = myState->mySubMachine->stateTable.begin();i!=myState->mySubMachine->stateTable.end();i++) {
        assert(i->second->inboundEdge.verb !=newVerb);
    }
    sign=newSign;
    verb=newVerb;
    return;
}

void edge::addParameter(std::string newParameterName) {
    // parameterName names a variable declared in either:
    //      a) the subMachine containing this edge, or
    //      b) a subMachine above the subMachine containing this edge.
    // Note the same variable name can be declared in multiple subMachines, so:
    // Find the "nearest" declaration, starting at the subMachine containing this edge, and working up towards Initial subMachine
    unsigned int heightAbove=0;
    subMachine * i = myState->mySubMachine;
    while (i!= (subMachine *) 0) {
        if (i->declarations.find(newParameterName) != i->declarations.end() )
            break;
        i=i->parentSubMachine;
        heightAbove++;
    }
    assert(i!=(subMachine *) 0); // ensure we found a sub-machine containing the variable
    assert(i->declarations.find(newParameterName) != i->declarations.end());    // ensure we found the variable

    // this was kinda hard to read so I broke it into three statements
    pListEntry newParameter(newParameterName,heightAbove);
    this->pList.push_back( newParameter );
}

