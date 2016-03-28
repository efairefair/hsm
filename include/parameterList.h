#ifndef PARAMETERLIST_H
#define PARAMETERLIST_H

#include <string>

#include "hsmTypes.h"

class pListEntry {
    friend class edge;
    friend class rtFrame;
    private:
        pListEntry(std::string newName, uint16_t heightAbove) : theName(newName), theHeight(heightAbove) { };
        const std::string theName;
        const uint16_t theHeight;
};

#endif // PARAMETERLIST_H
