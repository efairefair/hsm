#ifndef RTFRAME_H
#define RTFRAME_H

#include <map>
#include <string>
#include <atomic>

#include "hsmTypes.h"

class rtZone;
class state;

class rtFrame
{
    public:

        ~rtFrame();
        rtFrame(rtZone *, subMachine * );               // create frame to execute subMachine

        bool frameIsDormant;
        state * currentState;
        rtZone * myZone;
        subMachine * mySubMachine;

        absolutePathType absolutePath;                  // the frames location in the runtime tree
        std::string pathStr();                          // converts absolute path to a string
        variable siblingOrder();                        // the last number in the absolutePath
        soType numSiblings();

        rtFrame * parentFrame;
        std::atomic<uint_least32_t> numChildFrames;     // the number of active + dormant child frames
        std::atomic<uint_least32_t> nextChild;          // the siblingOrder of the next child of this frame

        std::map<std::string,variable> instantiations;

        state * evaluateNextStates();                   // return first possible transition or 0 if none
        void makeTransition(state * nextState);         // make transition to nextState (presumably found via evaluateNextStates() above)

        msg * myMsg;                                    // for incoming or outgoing messages from this frame (only one at a time is possible)
        void encodeMsg();
        void decodeMsg();


    protected:
    private:

};

#endif // RTFRAME_H
