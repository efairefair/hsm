#ifndef HSM_H
#define HSM_H

#include <map>
#include <string>
#include <cstdint>

#include "hsmTypes.h"
#include "subMachine.h"
#include "rtTree.h"

class subMachine;
class rtTree;

class hsm
{
    friend class subMachine;
    public:
        hsm(std::string);
        ~hsm();
        rtTree * execute();
        subMachine * const initialSubMachine;
    protected:
    private:
        std::string name;
        std::multimap<subMachine *, subMachine *> subMachineTable;    // parent-child; first is parent; second is child; if first=0 then second is Initial
};

#endif // HSM_H
