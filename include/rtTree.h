#ifndef RTTREE_H
#define RTTREE_H

#include <list>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <time.h>
#include <atomic>

#include "hsmTypes.h"

class rtTree
{
    public:
        rtTree( hsm * );                                            // create tree to execute hsm in
        ~rtTree();
        hsm * myHsm;                                                // the hsm being executed in this tree
        rtZone * topZone;
        std::mutex wait_m;
        std::condition_variable wait_c;
        void wait();
        std::list<rtZone *> freeZones;                              // where zones go when they're deleted
        std::atomic<uint_least32_t> forwardLinksEncountered;        // for you accounting types
        std::atomic<uint_least32_t> oldFramesDeleted;
        std::atomic<uint_least32_t> newFramesCreated;
        std::atomic<uint_least32_t> oldFramesReused;
        std::atomic<uint_least32_t> totalSlept;
        std::atomic<uint_least32_t> transitionCount;
        std::atomic<uint_least32_t> predicatesEvaluatedTrue;
        std::atomic<uint_least32_t> predicatesEvaluatedFalse;
        std::atomic<uint_least32_t> newZonesCreated;
        std::atomic<uint_least32_t> oldZonesReused;
        time_t startTime;
        rtZone * makeZone();
        std::mutex treeLock;
    protected:
    private:

};

#endif // RTTREE_H
