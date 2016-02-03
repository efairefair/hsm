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
#include "myAtomicInt.h"


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

        std::multimap<state *, state *> stateTable;                     // key is parent; if key=0 it's the Start state
        std::map<std::string, variable> declarations;                   // variable declarations

        subMachine * addSubMachine(std::string);

        bool addVariable(std::string, hsmVarType);
        bool addVariable(std::string, hsmVarType,hsmInt);
        bool addVariable(std::string, hsmVarType,hsmBool);
        bool addVariable(std::string, hsmVarType,hsmString);
        bool addVariable(std::string, hsmVarType,hsmListInt);
        state * startState;

    protected:
    private:
};

#endif // SUBMACHINE_H
