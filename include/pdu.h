#ifndef PDU_H
#define PDU_H
#include <string>
#include <utility>
#include "hsmTypes.h"
#include "variable.h"

class pdu {
    friend class rtFrame;
    private:
        std::string verb;
        std::vector<variable *> payload;                       // ordered list of [variable type, levels above] tuples
};

#endif // PDU_H
