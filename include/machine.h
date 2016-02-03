#ifndef MACHINE_H
#define MACHINE_H

#include <map>
#include <string>

#include "rtTree.h"

class subMachine;

class machine
{
    public:
        machine();
        ~machine();
        subMachine * initial;
        bool isExecuting;
        bool execute();
    protected:
    private:
        std::map<std::string, subMachine *> subMachineTable;
};

#endif // MACHINE_H
