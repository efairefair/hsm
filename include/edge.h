#ifndef EDGE_H
#define EDGE_H

#include "msg.h"
#include "hsm.h"

class edge
{
    public:
        edge();
        ~edge();

        hsmEdgeSignType sign;   // default: internal
        std::string verb;       // default: empty string
        pListType parameters;   // default: empty vector

        state * myState;

        void setVerb(std::string, hsmEdgeSignType);        // set verb and sign
        void addParameter(std::string);                 // variable name
    protected:
    private:
};

#endif // EDGE_H
