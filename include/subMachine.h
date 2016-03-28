#ifndef SUBMACHINE_H
#define SUBMACHINE_H

#include <map>
#include <string>
#include <list>
#include <atomic>

#include "hsmTypes.h"
#include "hsm.h"
#include "forwardLink.h"
#include "hsm.h"
#include "variable.h"

class state;
class hsm;

class subMachine
{
    friend class hsm;
    friend class edge;
    friend class rtFrame;
    friend class rtZone;
    friend class state;
    public:
        subMachine * addSubMachine(std::string);
        void addVariable(std::string, variable *, hsmType);
        state * startState;
    private:
        subMachine(hsm *);                                // to create the Initial (top-most) sub-machine of hsm
        subMachine(subMachine * parent,std::string name); // to create all other sub-machines
        ~subMachine();
        std::string name;
        hsm * myHsm;
        subMachine * parentSubMachine;
        std::multimap<state *, state *> stateTable;                         // key=value; key is parent-of value; if key=0 it's the Start state
        std::map<std::string, std::pair<hsmType,variable *>>declarations;   // variable declarations for this subMachine
};

#endif // SUBMACHINE_H
