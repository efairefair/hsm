#include "edge.h"

#include <assert.h>
#include <tuple>

#include "hsmTypes.h"
#include "state.h"
#include "variable.h"

void edge::setVerb(std::string newVerb, hsmEdgeSignType newSign) {
    // ensure a verb is not already present
    assert (verb.size()==0);
    // ensure verb being set was not previously set elsewhere in this subMachine
    for(auto i = myState->mySubMachine->stateTable.begin();i!=myState->mySubMachine->stateTable.end();i++) {
        assert(i->second->inboundEdge.verb !=newVerb);
    }
    sign=newSign;
    verb=newVerb;
    return;
}

void edge::addParameter(std::string newParameterName) {
    // parameterName names a variable declared
    //      a) in the subMachine containing this edge, or
    //      b) in a subMachine above it in the hierarchy of subMachines
    // Note the same name can be declared in multiple subMachines;
    // so find the "nearest" declaration, starting at the subMachine containing this edge and working up
    unsigned int heightAbove=0;
    subMachine * i = myState->mySubMachine;
    while (i!= (subMachine *) 0) {
        if (i->declarations.find(newParameterName) != i->declarations.end() )
            break;
        i=i->parentSubMachine;
        heightAbove++;
    }
    assert(i!=(subMachine *) 0); // make sure we found the name
    assert(i->declarations.find(newParameterName) != i->declarations.end());

    std::pair<hsmType,variable *> theDeclaration = i->declarations[newParameterName];
    pListEntry newParameter(newParameterName,theDeclaration.first,heightAbove);
    this->pList.push_back( newParameter );
}

