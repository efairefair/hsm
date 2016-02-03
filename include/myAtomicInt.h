#ifndef MYATOMICINT_H
#define MYATOMICINT_H

#include <atomic>
#include <ostream>

class myAtomicInt {
    public:
        myAtomicInt();
        myAtomicInt(long long int);
        myAtomicInt(const myAtomicInt &);
        myAtomicInt& operator=( const myAtomicInt& rhs );
        //std::ostream& operator<<(const myAtomicInt& obj);

    private:
        std::atomic<long long int> data;
};


#endif // MYATOMICINT_H
