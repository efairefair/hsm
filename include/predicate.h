#ifndef PREDICATE_H
#define PREDICATE_H

#include "rtFrame.h"

class predicate
{
    public:
        predicate();
        ~predicate();
        bool evaluate();
    protected:
    private:
        bool (* ptf) (rtFrame *);
};

#endif // PREDICATE_H
