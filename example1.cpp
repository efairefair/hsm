#include <iostream>
#include <string>
#include <limits>

#include "hsmTypes.h"
#include "hsm.h"
#include "rtTree.h"
#include "state.h"
#include "variable.h"


using namespace std;

//////////////////////////////////////////////////////////////
//
// beginning of example HSM
//
//////////////////////////////////////////////////////////////
//
// define code fragments and predicates
//
void code_Initial_start(rtFrame * here) {
    hsmUint & randNum = ( (hsmVariable<hsmUint> *) here->instantiations["randNum"] )->theValue;
    hsmUint & answer = ( (hsmVariable<hsmUint> *) here->instantiations["answer"] )->theValue;

    srand (time(NULL));
    randNum=(rand()%10)+2;
    std::cout<<"randNum: "<<randNum<<std::endl;
    answer=randNum;
}

bool predicate_Initial_ckOdd(rtFrame * here) {
    hsmUint & randNum = ( (hsmVariable<hsmUint> *) here->instantiations["randNum"] )->theValue;
    return ((randNum%2) == 1);
}

bool predicate_Initial_ckEven(rtFrame * here) {
    hsmUint & randNum = ( (hsmVariable<hsmUint> *) here->instantiations["randNum"] )->theValue;
    return ((randNum%2)==0);
}

bool predicate_factorial_calc(rtFrame * here) {
    hsmUint & randNum = ( (hsmVariable<hsmUint> *) here->parentFrame->instantiations["randNum"] )->theValue;

    return (randNum > 1);
}

void code_factorial_calc(rtFrame * here) {
    hsmUint & randNum = ( (hsmVariable<hsmUint> *) here->parentFrame->instantiations["randNum"] )->theValue;
    hsmUint & answer = ( (hsmVariable<hsmUint> *) here->parentFrame->instantiations["answer"] )->theValue;

    randNum--;
    answer = answer*randNum;
}

bool predicate_factorial_last(rtFrame * here) {
    hsmUint & randNum = ( (hsmVariable<hsmUint> *) here->parentFrame->instantiations["randNum"] )->theValue;
    return (randNum==1);
}

void code_factorial_last(rtFrame * here) {
    hsmUint & answer = ( (hsmVariable<hsmUint> *) here->parentFrame->instantiations["answer"] )->theValue;
    std::cout<<"factorial is: "<<answer<<std::endl;
}

void code_fibonacci_Start(rtFrame * here) {
    hsmUint & randNum    =  ( (hsmVariable<hsmUint> *) here->parentFrame->  instantiations["randNum"]       )->getReference();
    hsmPath & fibSeq     =  ( (hsmVariable<hsmPath> *) here->               instantiations["fibSeq"]        )->getReference();
    hsmUint & nextToLast =  ( (hsmVariable<hsmUint> *) here->               instantiations["nextToLast"]    )->getReference();
    hsmUint & temp       =  ( (hsmVariable<hsmUint> *) here->               instantiations["temp"]          )->getReference();

    fibSeq.push_back(0);
    fibSeq.push_back(1);
    std::cout<<"after initialization, fibSeq.size(): "<<fibSeq.size()<<", randNum: "<<randNum<<std::endl;
    nextToLast = 0;

    while (
        fibSeq.size() < randNum
        ) {
        temp = fibSeq.back();
        //std::cout<<"push "<<(temp+nextToLast)<<"(percent of limit: "<< 100.0*((1.0*(temp+nextToLast))/std::numeric_limits<hsmUint>::max() )<<", length: "<<to_string(temp+nextToLast).length()<<")";
        fibSeq.push_back(temp+nextToLast);
        //std::cout<<", fibSeq.size()="<<fibSeq.size()<<std::endl;
        nextToLast=temp;
    }

}

bool predicate_fibonacci_done(rtFrame * here) {
    hsmUint & randNum =     ( (hsmVariable<hsmUint> *) here->parentFrame->  instantiations["randNum"] )->theValue;
    hsmPath & fibSeq =      ( (hsmVariable<hsmPath> *) here->               instantiations["fibSeq"] )->theValue;

    return (fibSeq.size()==randNum);
}

void code_fibonacci_done(rtFrame * here) {

    hsmPath & fibSeq = ( (hsmVariable<hsmPath> *) here->instantiations["fibSeq"] )->theValue;

    for (size_t counter=0;counter<fibSeq.size();counter++)
        std::cout<<counter<<": "<<fibSeq[counter]<<std::endl;
}

int main()
{
    hsm * myMachine = new hsm("example1");

    //////////////////////////////////////////////////////////////
    //
    // create sub-machines
    //
    subMachine * sm_factorial = myMachine->initialSubMachine->addSubMachine("factorial");
    subMachine * sm_fibonacci = myMachine->initialSubMachine->addSubMachine("fibonacci");



    //////////////////////////////////////////////////////////////
    //
    // create variables initialized to the above constants
    //
    myMachine->initialSubMachine->declarations["randNum"]   =   new hsmVariable<unsigned int>(0);
    myMachine->initialSubMachine->declarations["answer"]    =   new hsmVariable<unsigned int>(0);

    sm_fibonacci->declarations["fibSeq"]    =   new hsmVariable<hsmPath>;
    sm_fibonacci->declarations["nextToLast"]=   new hsmVariable<unsigned int>(0);
    sm_fibonacci->declarations["temp"]      =   new hsmVariable<unsigned int>(0);


    //////////////////////////////////////////////////////////////
    //
    // create states (and, implicitly, edges)
    //
    state * st_initial_ckOdd  = myMachine->initialSubMachine->startState->addChildState("ckOdd");
    state * st_initial_ckEven = myMachine->initialSubMachine->startState->addChildState("ckEven");
    state * st_factorial_calc = sm_factorial->startState->addChildState("calc");
    state * st_factorial_step = st_factorial_calc->addChildState("step");
    state * st_factorial_last = sm_factorial->startState->addChildState("last");
    state * st_fibonacci_done = sm_fibonacci->startState->addChildState("done");

    //////////////////////////////////////////////////////////////
    //
    // set forward links on some of the states
    //
    st_initial_ckOdd->setForwardLink(vertical,sm_factorial,false);
    st_initial_ckEven->setForwardLink(vertical,sm_fibonacci,true);
    st_factorial_step->setForwardLink(horizontal,sm_factorial,false);

    //////////////////////////////////////////////////////////////
    //
    // set predicates on some of the states (default: return true;)
    //
    st_initial_ckOdd->setPredicate(predicate_Initial_ckOdd);
    st_initial_ckEven->setPredicate(predicate_Initial_ckEven);
    st_factorial_calc->setPredicate(predicate_factorial_calc);
    st_factorial_last->setPredicate(predicate_factorial_last);
    st_fibonacci_done->setPredicate(predicate_fibonacci_done);

    //////////////////////////////////////////////////////////////
    //
    // set codeFragments on some of the states (default: return;)
    //
    myMachine->initialSubMachine->startState->setCodeFragment(code_Initial_start);
    st_factorial_calc->setCodeFragment(code_factorial_calc);
    st_factorial_last->setCodeFragment(code_factorial_last);
    sm_fibonacci->startState->setCodeFragment(code_fibonacci_Start,true);
    st_fibonacci_done->setCodeFragment(code_fibonacci_done);

    //////////////////////////////////////////////////////////////
    //
    // start execution of the hsm, save a handle to executing instance
    //
    rtTree * myTree = myMachine->execute();

    //////////////////////////////////////////////////////////////
    //
    // wait for completion of the hsm
    //

    myTree->wait();
    return 0;
}
