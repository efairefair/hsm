#include <iostream>

#include "rtTree.h"
#include "rtZone.h"
#include "subMachine.h"
#include "assert.h"


rtZone * rtTree::makeZone() {
    rtZone * rv;
    treeLock.lock();
    if (freeZones.size()>0) {
        rv = freeZones.front();
        freeZones.pop_front();
        oldZonesReused++;
    }
    else {
        rv = new rtZone(this);
        newZonesCreated++;
    }
    treeLock.unlock();
    return rv;
}

rtTree::rtTree(hsm * myMachine)
{
    //ctor

    myHsm=myMachine;
    forwardLinksEncountered=0;
    oldFramesDeleted=0;
    newFramesCreated=0;
    oldFramesReused=0;
    totalSlept=0;
    transitionCount=0;
    predicatesEvaluatedTrue=0;
    predicatesEvaluatedFalse=0;
    newZonesCreated=0;
    oldZonesReused=0;
}

rtTree::~rtTree()
{
    //dtor
    std::cout << "rtTree deleted"<<std::endl;
}

void rtTree::wait() {

    std::unique_lock<std::mutex> lk(wait_m);
    wait_c.wait(lk);
    wait_m.unlock();
    wait_c.notify_one();
    return;
}
