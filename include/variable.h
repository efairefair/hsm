#ifndef VARIABLE_H
#define VARIABLE_H

#include <list>
#include <string>
#include <ostream>

#include "hsmTypes.h"

hsmInt operator%(const hsmInt &a, const variable &b);
bool operator<(const size_t, variable const &);

std::ostream & operator<<(std::ostream &, const variable &);

class variable
{
    public:
        variable();
        variable(const variable&);

        variable(hsmVarType, hsmInt);
        variable(hsmVarType, hsmUInt);
        variable(hsmVarType, hsmString);
        variable(hsmVarType, hsmBool);
        variable(hsmVarType, hsmListInt);
        variable(hsmVarType);

        ~variable();

        variable & operator=(const variable &);
        variable & operator=(const hsmInt &);
        variable & operator=(const hsmBool &);
        variable & operator=(const std::string &);
        variable & operator=(const std::list<hsmInt>&);

        variable operator+(const variable &) const;
        variable operator+(const int &) const;

        variable operator*(const variable &) const;
        variable operator*(const int &) const;

        variable operator%(const int &) const;

        bool operator>=(const hsmInt &);
        bool operator>=(const variable &);
        bool operator>=(const hsmUInt &);

        bool operator<(const hsmInt &);
        bool operator<(const hsmUInt &);
        bool operator<(const variable &);

        bool operator>(const variable &);
        bool operator>(const hsmInt &);
        bool operator>(const hsmUInt &);


        bool operator==(const variable &);
        bool operator==(const hsmUInt &);
        bool operator==(const hsmInt &);
        bool operator==(const hsmBool &);

        variable & operator--(int);
        variable & operator++(int);

        //void initialize();
        void push_back(hsmInt);
        void push_back(variable);
        void pop_front();
        hsmInt front();
        hsmInt back();
        size_t size();
        hsmVarType theType;     // discriminant -- identifies which member in the union below is used for this variable
        union {
            hsmInt i;
            hsmUInt u;
            hsmBool b;
            hsmString s;
            hsmListInt li;
        };

    protected:
    private:

};

#endif // VARIABLE_H
