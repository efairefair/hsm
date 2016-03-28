#ifndef FORWARDLINK_H
#define FORWARDLINK_H

#include "hsmTypes.h"

class forwardLink {
    friend class rtFrame;
    friend class state;
    private:
        forwardLink(subMachine * newTarget, forwardLinkType newDirection, bool newThreadRequested) : targetSubMachine(newTarget), direction(newDirection), newThread(newThreadRequested){};
        subMachine * targetSubMachine;
        forwardLinkType direction;
        bool newThread;
};

#endif // FORWARDLINK_H
