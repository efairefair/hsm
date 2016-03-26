#include "forwardLink.h"

forwardLink::forwardLink(subMachine * newTarget, forwardLinkType newDirection,bool newThreadRequested)
{
    //ctor
    this->targetSubMachine=newTarget;
    this->newThread=newThreadRequested;
    this->direction=newDirection;
}
