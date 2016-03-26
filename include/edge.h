#ifndef EDGE_H
#define EDGE_H

#include <string>

#include "hsm.h"
#include "parameterList.h"

class edge
{
    public:

        hsmEdgeSignType sign = hsmInternalEdge;  // default: internal
        std::string verb;                        // default: empty string
        std::vector<pListEntry> pList;           // default: empty vector

        state * myState;

        void setVerb(std::string, hsmEdgeSignType);     // set verb and sign
        void addParameter(std::string);
    protected:
    private:
};

#endif // EDGE_H
