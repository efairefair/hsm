#ifndef MSG_H
#define MSG_H

#include <utility>
#include <string>
#include <vector>

#include "hsmTypes.h"
#include "variable.h"

class msg
{
    public:
        std::string verb;
        std::vector<variable> payload;                       // list of  [variable name, variable type, levels above], [variable name, variable type, levels above]
        msg();
        ~msg();
    protected:
    private:
};

#endif // MSG_H
