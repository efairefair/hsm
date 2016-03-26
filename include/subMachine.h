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
    public:
        subMachine(hsm *);                                // to create the Initial (top-most) sub-machine of hsm
        subMachine(subMachine * parent,std::string name); // to create all other sub-machines
        ~subMachine();

        std::string name;
        hsm * myHsm;
        subMachine * parentSubMachine;

        std::multimap<state *, state *> stateTable;                         // key=value; key is parent-of value; if key=0 it's the Start state
        std::map<std::string, std::pair<hsmType,variable *>>declarations;   // variable declarations for this subMachine

        subMachine * addSubMachine(std::string);

        void addVariable(std::string, variable *, hsmType);

        state * startState;

    protected:
    private:
};

#endif // SUBMACHINE_H
