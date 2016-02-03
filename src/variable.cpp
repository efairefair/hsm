#include "variable.h"

#include <assert.h>
#include <new>

////////////////////////////////////////////////////////////////
//
// a class for hsm storage.  Note variables are strongly typed.
//

//
// non-class declarations
//

hsmInt operator%(const hsmInt &a, const variable &b) {
    assert(b.theType==hsmIntType || b.theType==hsmUIntType);
    if (b.theType==hsmIntType)
        return a%b.i;
    else
        return a%b.u;
}

bool operator<(const hsmInt a, variable const &b) {
    assert(b.theType==hsmIntType || b.theType==hsmUIntType);
    if (b.theType==hsmIntType)
        return a<b.i;
    else
        return a<b.u;
}

bool operator<(const size_t a, variable const &b) {
    assert(b.theType==hsmIntType || b.theType==hsmUIntType);
    if (b.theType==hsmIntType)
        return a<b.i;
    else
        return a<b.u;
}

std::ostream & operator<<(std::ostream & theStream, const variable & rhs) {
    assert(rhs.theType==hsmBoolType||rhs.theType==hsmIntType||rhs.theType==hsmUIntType||rhs.theType==hsmStringType);

    if      (rhs.theType==hsmBoolType)  {if (rhs.b) theStream<<"true"; else theStream<<"false";return theStream;}
    else if (rhs.theType==hsmIntType)   {theStream<<std::to_string(rhs.i);return theStream;}
    else if (rhs.theType==hsmUIntType)  {theStream<<std::to_string(rhs.u);return theStream;}
    else                                {theStream<<rhs.s;return theStream;}
}

//
// class declarations
//

variable::variable()
{
    //ctor
}
variable::~variable()
{
    if      (theType==hsmStringType)    {s.std::string::~string();}
    else if (theType==hsmListIntType)   {li.~list<hsmInt>();}
}

variable::variable(const variable& rhs) {
    this->theType=rhs.theType;
    if      (theType==hsmBoolType)      {b=rhs.b;}
    else if (theType==hsmIntType)       {i=rhs.i;}
    else if (theType==hsmUIntType)      {u=rhs.u;}
    else if (theType==hsmStringType)    {new(&s) std::string(rhs.s);}
    else if (theType==hsmListIntType)   {new(&li)std::list<hsmInt> (rhs.li);}
}

variable & variable::operator=(const variable & rhs) {
    this->theType = rhs.theType;
    if      (theType==hsmBoolType)      {b =rhs.b; return *this;}
    else if (theType==hsmIntType)       {i =rhs.i; return *this;}
    else if (theType==hsmUIntType)      {u =rhs.u; return *this;}
    else if (theType==hsmStringType)    {new(&s) std::string(rhs.s); return *this;}
    else                                {new(&li) std::list<hsmInt> (rhs.li); return *this;}
}

variable::variable(hsmVarType newType, hsmInt newValue){
    assert(newType==hsmIntType);
    theType=newType;
    i=newValue;
}
variable::variable(hsmVarType newType, hsmUInt newValue){
    assert(newType==hsmUIntType);
    theType=newType;
    u=newValue;
}

variable::variable(hsmVarType newType, hsmString newValue){
    assert(newType==hsmStringType);
    theType=newType;
    s=newValue;
}
variable::variable(hsmVarType newType, hsmBool newValue){
    assert(newType==hsmBoolType);
    theType=newType;
    b=newValue;
}
variable::variable(hsmVarType newType, hsmListInt newValue){
    assert(newType==hsmListIntType);
    theType=newType;
    li=newValue;
}

variable::variable(hsmVarType newType){
    // only listInt types can be created without some form of initialization
    assert(newType==hsmListIntType);
    theType=newType;
    new(&li) std::list<hsmInt>();
}

variable variable::operator+(const variable & rhs) const {
    variable rv;
    assert(((theType==hsmIntType) && (rhs.theType==hsmIntType))||((theType==hsmUIntType) && (rhs.theType==hsmUIntType)));
    rv.theType=hsmIntType;
    if (theType==hsmIntType)    rv.i=this->i+rhs.i;
    else                        rv.u=this->u+rhs.u;
    return rv;
}

variable variable::operator*(const variable & rhs) const {
    variable rv;
    assert((theType==hsmIntType && rhs.theType==hsmIntType)||(theType==hsmUIntType && rhs.theType==hsmUIntType));
    rv.theType=theType;
    if (theType==hsmIntType)    rv.i=this->i*rhs.i;
    else                        rv.u=this->u*rhs.u;
    return rv;
}

variable variable::operator+(const int & rhs) const {
    variable rv;
    assert(theType==hsmIntType||theType==hsmUIntType);
    rv.theType=hsmIntType;
    if (theType==hsmIntType) rv.i=this->i+rhs;
    else rv.u=this->u+rhs;
    return rv;
}

variable variable::operator*(const int & rhs) const {
    variable rv;
    assert(theType==hsmIntType||theType==hsmUIntType);
    rv.theType=hsmIntType;
    if (theType==hsmIntType) rv.i=this->i*rhs;
    else rv.u=this->u*rhs;
    return rv;
}

variable variable::operator%(const int & rhs) const {
    variable rv;
    assert(theType==hsmIntType||theType==hsmUIntType);
    rv.theType=hsmIntType;
    if (theType==hsmIntType) rv.i=this->i%rhs;
    else rv.u=this->u%rhs;
    return rv;
}

variable & variable::operator--(int) {
    assert(theType==hsmIntType);
    i--;
    return *this;
}

variable & variable::operator++(int) {
    assert(theType==hsmIntType);
    i++;
    return *this;
}


variable & variable::operator=(const hsmInt & rhs) {
    assert(theType==hsmIntType);
    i =rhs;
    return *this;
}
variable & variable::operator=(const hsmBool & rhs) {
    assert(theType==hsmBoolType);
    b =rhs;
    return *this;
}
variable & variable::operator=(const hsmString & rhs) {
    assert(theType==hsmStringType);
    s =rhs;
    return *this;
}

variable & variable::operator=(const hsmListInt & rhs) {
    assert(theType==hsmListIntType);
    li =rhs;
    return *this;
}

bool variable::operator<(const hsmInt & rhs) {
    assert(theType==hsmIntType);
    return i<rhs;
}

bool variable::operator>(const hsmInt & rhs) {
    assert(theType==hsmIntType);
    return i>rhs;
}

bool variable::operator>=(const hsmInt & rhs) {
    assert(theType==hsmIntType);
    return i>=rhs;
}


bool variable::operator==(const hsmInt & rhs) {
    assert(theType==hsmIntType);
    return i==rhs;
}

bool variable::operator==(const hsmBool & rhs) {
    assert(theType==hsmBoolType);
    return b==rhs;
}

bool variable::operator<(const hsmUInt & rhs) {
    assert(theType==hsmUIntType);
    return i<rhs;
}

bool variable::operator>(const hsmUInt & rhs) {
    assert(theType==hsmUIntType);
    return i>rhs;
}
bool variable::operator>=(const hsmUInt & rhs) {
    assert(theType==hsmUIntType);
    return i>=rhs;
}
bool variable::operator==(const hsmUInt & rhs) {
    assert(theType==hsmUIntType);
    return i==rhs;
}


bool variable::operator<(const variable & rhs) {
    assert((theType==hsmIntType||theType==hsmUIntType) && (rhs.theType==hsmUIntType||rhs.theType==hsmIntType));
    if      (theType==hsmIntType && rhs.theType==hsmIntType) return i<rhs.i;
    else if (theType==hsmIntType && rhs.theType==hsmUIntType) return i<rhs.u;
    else if (theType==hsmUIntType && rhs.theType==hsmUIntType) return u<rhs.u;
    else return u<rhs.i;
}

bool variable::operator>(const variable & rhs) {
    assert((theType==hsmIntType||theType==hsmUIntType) && (rhs.theType==hsmUIntType||rhs.theType==hsmIntType));
    if      (theType==hsmIntType && rhs.theType==hsmIntType) return i>rhs.i;
    else if (theType==hsmIntType && rhs.theType==hsmUIntType) return i>rhs.u;
    else if (theType==hsmUIntType && rhs.theType==hsmUIntType) return u>rhs.u;
    else return u>rhs.i;
}

bool variable::operator>=(const variable & rhs) {
    assert((theType==hsmIntType||theType==hsmUIntType) && (rhs.theType==hsmUIntType||rhs.theType==hsmIntType));
    if      (theType==hsmIntType && rhs.theType==hsmIntType) return i>=rhs.i;
    else if (theType==hsmIntType && rhs.theType==hsmUIntType) return i>=rhs.u;
    else if (theType==hsmUIntType && rhs.theType==hsmUIntType) return u>=rhs.u;
    else return u>=rhs.i;
}
bool variable::operator==(const variable & rhs) {
    assert((theType==rhs.theType)||(theType==hsmIntType||theType==hsmUIntType) && (rhs.theType==hsmUIntType||rhs.theType==hsmIntType));
    if      (theType==hsmIntType && rhs.theType==hsmIntType) return i==rhs.i;
    else if (theType==hsmIntType && rhs.theType==hsmUIntType) return i==rhs.u;
    else if (theType==hsmUIntType && rhs.theType==hsmUIntType) return u==rhs.u;
    else if (theType==hsmUIntType && rhs.theType==hsmIntType) return u==rhs.i;
    else if (theType==hsmBoolType) return b==rhs.b;
    else return s==rhs.s;
}

void variable::push_back(hsmInt newEntry) {
    // only listInt types can use this
    assert (theType==hsmListIntType);
    li.push_back(newEntry);
}
void variable::push_back(variable newEntry) {
    // only listInt can use this, and only ints can be pushed onto a listInt
    assert (theType==hsmListIntType);
    assert (newEntry.theType==hsmIntType);
    li.push_back(newEntry.i);
}

void variable::pop_front() {
    assert (theType=hsmListIntType);
    li.pop_front();
};

hsmInt variable::front() {
    assert (theType=hsmListIntType);
    return li.front();
};

hsmInt variable::back() {
    assert (theType=hsmListIntType);
    return li.back();
};

size_t variable::size() {
        assert (theType=hsmListIntType);
        return li.size();
}
