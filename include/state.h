#ifndef STATE_H
#define STATE_H

#include "forwardLink.h"
#include "subMachine.h"
#include "rtFrame.h"
#include "edge.h"

#include <string>

#include "hsmTypes.h"

class state
{
    public:
        state(subMachine * owner);                      // used only to create the start state
        state(state *,std::string);                     // create state below another state
        state * addChildState(std::string);             // used to create any states below the start state
        ~state();

        std::string name;
        unsigned numChildStates;                        // the number of children below this state
        edge inboundEdge;
        std::list<forwardLink *> links;

        predType predicate;
        codeType codeFragment;
        bool codeFragmentInNewThread;

        subMachine * mySubMachine;                      // necessary when addChildState adds another state
        state * parentState;

        bool setForwardLink(forwardLinkType,subMachine *, bool);
        bool setPredicate(predType );
        bool setCodeFragment(codeType,bool inNewThread=false);

    protected:
    private:
};

#endif // STATE_H
