#include "integerVariable.h"
#include "intAdverb.h"

integerVariable::integerVariable()
{
    //ctor
}

integerVariable::~integerVariable()
{
    //dtor
}

void integerVariable::encodeToAdverb(intAdverb * theAdverb) {
    theAdverb->theType=intVarType;
    theAdverb->theValue=this->theValue;
}
