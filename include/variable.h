#ifndef VARIABLE_H
#define VARIABLE_H

#include <list>
#include <string>
#include <ostream>

#include "hsmTypes.h"

class variable {
    public:
    virtual ~variable() {};
    virtual bool operator==(variable &) = 0;
    virtual bool operator<(variable &) = 0;
    virtual bool operator<=(variable &) = 0;
    virtual bool operator>(variable &) = 0;
    virtual bool operator>=(variable &) = 0;
    virtual bool operator!=(variable &) = 0;
    virtual variable & operator=(const variable &) = 0;
    virtual variable * clone() = 0;
};

template <class T> class hsmVariable : public variable {
    public:
        ~hsmVariable() {};
        hsmVariable() {};
        hsmVariable(T theInitialData) : theValue(theInitialData) {};
        bool operator==(variable & rhs);
        bool operator<(variable & rhs);
        bool operator<=(variable & rhs);
        bool operator>(variable & rhs);
        bool operator>=(variable & rhs);
        bool operator!=(variable & rhs);
        hsmVariable & operator=(const variable &) {return *this;};
        hsmVariable & operator=(const T &);
        T getValue() {return theValue;};
        variable * clone();
        T & getReference() {return theValue;};
        T theValue;
};

template <class T> variable * hsmVariable<T>::clone() {
    hsmVariable<T> * junk = new hsmVariable<T>(*this);
    //return new hsmVariable<T>(*this);
    return junk;
}
template <class T> bool hsmVariable<T>::operator==(variable & rhs) {
    return this->theValue == ( (hsmVariable<T> &) rhs).theValue;
}
template <class T> bool hsmVariable<T>::operator<(variable & rhs) {
    return this->theValue <  ( (hsmVariable<T> &) rhs).theValue;
}
template <class T> bool hsmVariable<T>::operator<=(variable & rhs) {
    return this->theValue <= ( (hsmVariable<T> &) rhs).theValue;
}
template <class T> bool hsmVariable<T>::operator>(variable & rhs) {
    return this->theValue >  ( (hsmVariable<T> &) rhs).theValue;
}
template <class T> bool hsmVariable<T>::operator>=(variable & rhs) {
    return this->theValue >= ( (hsmVariable<T> &) rhs).theValue;
}
template <class T> bool hsmVariable<T>::operator!=(variable & rhs) {
    return this->theValue != ( (hsmVariable<T> &) rhs).theValue;
}

#endif // VARIABLE_H
