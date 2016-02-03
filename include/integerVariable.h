#ifndef INTEGERVARIABLE_H
#define INTEGERVARIABLE_H

#include <variable.h>


class integerVariable : public variable
{
    public:
        integerVariable();
        ~integerVariable();
        void encodeToAdverb(adverb *);
        void decodeFromAdverb(adverb *);
        void set(int);
    protected:
    private:
        int theValue;
};

#endif // INTEGERVARIABLE_H
