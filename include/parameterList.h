#ifndef PARAMETERLIST_H
#define PARAMETERLIST_H

#include <string>

#include "hsmTypes.h"

class pListEntry {
    friend class edge;
    friend class rtFrame;
    private:
        pListEntry(std::string newName, hsmType newType, uint16_t heightAbove) : theName(newName), theType(newType), theHeight(heightAbove) { };
        const std::string theName;
        const hsmType theType;
        const uint16_t theHeight;
};

#endif // PARAMETERLIST_H
