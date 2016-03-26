#ifndef HSM_H
#define HSM_H

#include "hsmTypes.h"

#include <map>
#include <string>
#include <cstdint>

#include "subMachine.h"
#include "rtTree.h"



class subMachine;
class rtTree;

class hsm
{
    public:
        //typedef std::list<unsigned int> absolutePathType;

        hsm(std::string);
        ~hsm();
        std::string name;
        std::multimap<subMachine *, subMachine *> subMachineTable;    // parent-child; first is parent; second is child; if first=0 then second is Initial
        rtTree * execute();
        subMachine * initialSubMachine;
    protected:
    private:

};

#endif // HSM_H
