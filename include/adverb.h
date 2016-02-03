#ifndef ADVERB_H
#define ADVERB_H

#include "hsmTypes.h"

class adverb
{
    public:
        adverb();
        virtual ~adverb();
        hsmVarType theType;
    protected:
    private:
        union {
            int64_t i;
            bool b;
            std::string s;
            std::list<int64_t> li;
        };
};

#endif // ADVERB_H
