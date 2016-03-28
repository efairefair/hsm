#ifndef STATE_H
#define STATE_H

#include <string>

#include "hsmTypes.h"
#include "forwardLink.h"
#include "subMachine.h"
#include "rtFrame.h"
#include "edge.h"


class state {
    friend class edge;
    friend class rtFrame;
    friend class rtZone;
    friend class subMachine;
    public:
        state * addChildState(std::string);             // used to create any states below the start state
        edge inboundEdge;
        bool setForwardLink(forwardLinkType,subMachine *, bool);
        bool setPredicate(predType );
        bool setCodeFragment(codeType,bool inNewThread=false);
        std::string name;                               // useful for debugging
    private:
        state(subMachine * owner);                      // used only to create the start state
        state(state *,std::string);                     // create state below another state
        ~state();
        unsigned numChildStates;                        // the number of children below this state
        std::list<forwardLink *> links;
        predType predicate;                             // pointer to bool function
        codeType codeFragment;                          // pointer to void function
        bool codeFragmentInNewThread;                   // flag to indicate whether to run code fragment sync or async
        subMachine * mySubMachine;                      // necessary when addChildState adds another state
        state * parentState;
};

#endif // STATE_H
