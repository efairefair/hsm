#ifndef RTZONE_H
#define RTZONE_H

#include <string>
#include <list>
#include <condition_variable>
#include <fstream>
#include <atomic>
#include <map>
#include <utility>
#include <mutex>

#include "hsmTypes.h"
#include "pdu.h"

class rtZone
{
    public:

        rtZone(rtTree *);                                       // constructor
        rtZone(const rtZone&);
        ~rtZone();
        bool tryDelete(rtZone * deletingZone);                 // for recursive removal of the zone from the tree
                                                                // note: deletingZone is for logging purposes
        std::mutex zoneLock;
        std::atomic<uint_least32_t> numActiveInferiorZones;     // the number of active zones "below" this zone
        bool zoneIsInactive;                                    // flag to indicate when a zones thread exits.  Note the zone may still have dormant frames.
                                                                // this bool is protected by zoneLock

        std::string eyeCatcher;                                 // unique string for each zone; set when zone starts executing
        //erfc std::ofstream logFile;

        rtTree * myTree;                                        // the rtTree I am part of
        rtZone * superiorZone;                                  // the zone "above me"


        std::map<hsmPath, rtFrame *> activeFrames;     // indexed by absolute path
        std::map<hsmPath, rtFrame *> dormantFrames;    // indexed by absolute path
        std::multimap<std::string, rtFrame *> freeFrames;       // indexed by subMachineName that created the frame originally
                                                                // it behooves subMachines to re-use their own frames since the variables
                                                                // don't need to be re-created (malloced) if so.

        std::multimap<std::pair<std::string, std::string>, pdu *> freeMsgs;  // indexed by subMachine name, verb

        std::pair<rtFrame *, state *> evaluateFrames();   // find a frame that can transition, and it's next state
                                                                // if first is NULL, no transition is possible
                                                                // if first is not NULL, transition is possible and second is state to advance to

        void operator()();                                      // entry point to "execute" the zone.  called by std::thread constructor
        void executeActivePhase();                              // called by operator()

        rtFrame * makeFrame(rtZone *, subMachine *);            // to reuse a freed or construct a new frame

        long long unsigned int forwardLinksEncountered;         // for you accounting types
        long long unsigned int oldFramesDeleted;
        long long unsigned int newFramesCreated;
        long long unsigned int oldFramesReused;
        long long unsigned int totalSlept;
        long long unsigned int transitionCount;
        long long unsigned int predicatesEvaluatedTrue;
        long long unsigned int predicatesEvaluatedFalse;
        void freeZone();

    protected:
    private:
};

#endif // RTZONE_H
