#include "edge.h"

#include <assert.h>
#include <tuple>

#include "hsmTypes.h"
#include "state.h"
#include "variable.h"

edge::edge()
{
    sign=hsmInternalEdge;
}

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

void edge::addParameter(std::string parameterName) {

    // parameterName always names a variable at or above the subMachine containing this edge
    unsigned int heightAbove=0;
    subMachine * i = myState->mySubMachine;

    // find the height above us in the hierarchy of subMachines where the variable named parameterName is declared
    while (i!= (subMachine *) 0) {
        if (i->declarations.find(parameterName) != i->declarations.end() )
            break;
        i=i->parentSubMachine;
        heightAbove++;
    }

    // ensure variable with that name and type is found
    assert(i!=(subMachine *) 0);
    parameters.push_back( pListEntryType(parameterName, i->declarations[parameterName].theType, heightAbove) );
}

edge::~edge()
{
    //dtor
}
