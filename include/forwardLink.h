#ifndef FORWARDLINK_H
#define FORWARDLINK_H

#include "hsmTypes.h"

class forwardLink
{
    public:
        forwardLink(subMachine *, forwardLinkType,bool);
        ~forwardLink();
        subMachine * targetSubMachine;
        forwardLinkType direction;
        bool newThread;
    protected:
    private:
};

#endif // FORWARDLINK_H
