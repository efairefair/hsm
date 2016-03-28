#ifndef EDGE_H
#define EDGE_H

#include <string>

#include "hsm.h"
#include "parameterList.h"

class edge
{
    friend class rtFrame;
    friend class state;
    public:
        void setVerb(std::string, hsmEdgeType);     // set verb and sign
        void addParameter(std::string);
    private:
        hsmEdgeType sign = hsmInternalEdge;  // default: internal
        std::string verb;                        // default: empty string
        std::vector<pListEntry> pList;           // default: empty vector
        state * myState;

};

#endif // EDGE_H
