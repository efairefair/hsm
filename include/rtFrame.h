#ifndef RTFRAME_H
#define RTFRAME_H

#include <map>
#include <string>
#include <atomic>

#include "hsmTypes.h"
#include "variable.h"

class rtZone;
class state;

class rtFrame
{
    friend class rtZone;
    public:
        hsmString pathStr();                // converts absolute path to a string
                                            // I would make the following const but I re-use rtFrame objects when possible (and these values change)
        rtFrame * parentFrame;              // the parent frame in the runtime tree
        hsmUint         depth;              // the depth of this frame in the runtime tree
        hsmAtomicUint   numChildFrames;     // the number of active + dormant child frames
        hsmUint         nextChild;          // the siblingOrder of the next child of this frame
        hsmUint         siblingOrder;
        std::map<std::string,variable *> instantiations;  // variable instantiations.

    private:

        state * evaluateNextStates();                   // return first possible transition or 0 if none
        void makeTransition(state * nextState);         // make transition to nextState (presumably found via evaluateNextStates() above)
        state * currentState;
        rtZone * myZone;
        subMachine * mySubMachine;
        bool frameIsDormant;
        pdu * thePdu;                                    // for incoming or outgoing messages from this frame (only one at a time is possible)
        void encodeMsg();
        void decodeMsg();
        ~rtFrame();
        rtFrame(rtZone *, subMachine * );               // create frame to execute subMachine
        hsmPath absolutePath;                               // the frames location in the runtime tree
};

#endif // RTFRAME_H
