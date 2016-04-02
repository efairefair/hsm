#include <iostream>
#include <utility>

#include "hsmTypes.h"
#include "hsm.h"
#include "subMachine.h"
#include "rtFrame.h"
#include "state.h"
#include "variable.h"

using namespace std;

// declare predicates and code fragments as global functions
void code_Initial_Start(rtFrame * here) {
    int & a  = ( (hsmVariable<int> *) here->instantiations["a"])->theValue;

    srand(time(NULL));
    a=rand()%1000;
    std::cout<<"random number:"<<a<<std::endl;
}

void code_Z_Start(rtFrame * here) {
    //std::cout<<"%% " << "{" << here->pathStr() << "}" << " Z starting, numSiblings: " << here->numSiblings() << ", threadId: "<< std::hex << pthread_self()<<std::dec<<std::endl;
}

bool pred_Z_nonZero(rtFrame * here) {
    int & a  = ( (hsmVariable<int> *) here->parentFrame->instantiations["a"])->theValue;

    return (a>0);
}

void code_Z_nonZero(rtFrame * here) {
    int & a  = ( (hsmVariable<int> *) here->parentFrame->instantiations["a"])->theValue;
    int & b  = ( (hsmVariable<int> *) here->instantiations["b"])->theValue;

    a--;
    b=rand()%9999;
    if ((a%100)==0)
        std::cout<<"%% a decremented to: "<<a<<", "<<"b: "<<b<<std::endl;
}

bool pred_Z_zero(rtFrame * here) {
    int & a  = ( (hsmVariable<int> *) here->parentFrame->instantiations["a"])->theValue;

    return (a==0);
}

bool pred_Z_growHoriz(rtFrame * here) {
    hsmAtomicUint & numSiblings = here->parentFrame->numChildFrames;
    return (numSiblings<60);
}

bool pred_Z_even(rtFrame * here) {
    int & b  = ( (hsmVariable<int> *) here->instantiations["b"])->theValue;

    return ((b%2) == 0);
}

void code_Z_even(rtFrame * here) {
    //std::cout<<"%% {" << here->pathStr() << "} b is even, launching X"<<std::endl;
}

bool pred_Z_odd(rtFrame * here) {
    int & b  = ( (hsmVariable<int> *) here->instantiations["b"])->theValue;

    return ((b%2) == 1);
}

void code_Z_odd(rtFrame * here) {
    //std::cout<<"%% {" << here->pathStr() << "} b is odd, launching Y"<<std::endl;
}

bool pred_X_lessThan(rtFrame * here) {
    int & b  = ( (hsmVariable<int> *) here->parentFrame->instantiations["b"])->theValue;
    hsmUint & siblingOrder = here->siblingOrder;
    //std::cout<<"{"<<here->pathStr()<<"}"<<"pred_X_lessThan:"<<here->siblingOrder()<<","<<here->parentFrame->instantiations["b"]<<std::endl;
    return (siblingOrder < b);
}

bool pred_X_equal(rtFrame * here)   {
    int & b  = ( (hsmVariable<int> *) here->parentFrame->instantiations["b"])->theValue;
    hsmUint & siblingOrder = here->siblingOrder;
    return (siblingOrder==b);
}

void code_X_equal(rtFrame * here) {
    //std::cout<<"%% exiting X thread:"<<pthread_self()<<std::endl;
}

bool pred_Y_nonZero(rtFrame * here) {
    hsmUint & siblingOrder = here->siblingOrder;
    int & b  = ( (hsmVariable<int> *) here->parentFrame->instantiations["b"])->theValue;

    return (siblingOrder==b);
}
bool pred_Y_zero(rtFrame * here)    {
    hsmUint & siblingOrder = here->siblingOrder;
    int & b  = ( (hsmVariable<int> *) here->parentFrame->instantiations["b"])->theValue;

    return (siblingOrder<b);
}

void code_Y_nonZero(rtFrame * here) {
    //std::cout<<"%% exiting Y thread:"<<pthread_self()<<std::endl;
}

int main() {

    hsm * myMachine = new hsm("example2");

    // create sub-machines
    subMachine * sm_Z = myMachine->initialSubMachine->addSubMachine("Z");
    subMachine * sm_X = sm_Z->addSubMachine("X");
    subMachine * sm_Y = sm_Z->addSubMachine("Y");

    // declare variables

    myMachine->initialSubMachine->declarations["a"] = new hsmVariable<int>(0) ;
    //myMachine->initialSubMachine->addVariable("a",zero,hsmIntType);

    sm_Z->declarations ["b"] =  new hsmVariable<int>(0);
    //sm_Z->addVariable("b",zero, hsmIntType);
    //sm_X->addVariable("c",hsmIntType,(hsmInt)0);
    //sm_Y->addVariable("d",hsmIntType,(hsmInt)0);


    // create states and edges
    state * st_initial_growVert  = myMachine->initialSubMachine->startState->addChildState("growVert");

    st_initial_growVert->inboundEdge.addParameter("a");

    state * st_Z_nonZero = sm_Z->startState->addChildState("nonZero");
    state * st_Z_zero  = sm_Z->startState->addChildState("zero");
    state * st_Z_growHoriz = st_Z_nonZero->addChildState("growHoriz");
    state * st_Z_even = st_Z_growHoriz->addChildState("even");
    state * st_Z_odd = st_Z_growHoriz->addChildState("odd");

    state * st_X_lessThan = sm_X->startState->addChildState("lessThan");
    state * st_X_equal = sm_X->startState->addChildState("equal");
    state * st_X_growHoriz = st_X_lessThan->addChildState("growHoriz");

    state * st_Y_zero =  sm_Y->startState->addChildState("zero");
    state * st_Y_nonZero = sm_Y->startState->addChildState("nonZero");
    state * st_Y_growHoriz = st_Y_zero->addChildState("growHoriz");


    // set forward links
    st_initial_growVert->setForwardLink(vertical,sm_Z,true);

    st_Z_growHoriz->setForwardLink(horizontal,sm_Z,true);
    st_Z_even->setForwardLink(vertical,sm_X,false);
    st_Z_odd->setForwardLink(vertical,sm_Y,false);

    st_X_growHoriz->setForwardLink(horizontal,sm_X,false);

    st_Y_growHoriz->setForwardLink(horizontal,sm_Y,false);

    // set predicates
    st_Z_nonZero->setPredicate(pred_Z_nonZero);
    st_Z_zero->setPredicate(pred_Z_zero);
    st_Z_growHoriz->setPredicate(pred_Z_growHoriz);
    st_Z_even->setPredicate(pred_Z_even);
    st_Z_odd->setPredicate(pred_Z_odd);

    st_X_lessThan->setPredicate(pred_X_lessThan);
    st_X_equal->setPredicate(pred_X_equal);

    st_Y_nonZero->setPredicate(pred_Y_nonZero);
    st_Y_zero->setPredicate(pred_Y_zero);

    // set codeFragments
    myMachine->initialSubMachine->startState->setCodeFragment(code_Initial_Start);

    sm_Z->startState->setCodeFragment(code_Z_Start);
    st_Z_nonZero->setCodeFragment(code_Z_nonZero);
    st_Z_even->setCodeFragment(code_Z_even);
    st_Z_odd->setCodeFragment(code_Z_odd);

    st_X_equal->setCodeFragment(code_X_equal);

    st_Y_nonZero->setCodeFragment(code_Y_nonZero);


    // execute
    rtTree * myTree = myMachine->execute();
    std::cout << "waiting for execution to complete" << std::endl;
    myTree->wait();
    std::cout << "execution completed" << std::endl;
    time_t duration = time(NULL) - myTree->startTime;
    std::cout << "duration: " << duration << std::endl;
    cout \
        << " forwardLinksEncountered:  "    << myTree->forwardLinksEncountered              << std::endl \
        << " oldFramesDeleted:         "    << myTree->oldFramesDeleted                     << std::endl \
        << " newFramesCreated:         "    << myTree->newFramesCreated                     << std::endl \
        << " oldFramesReused:          "    << myTree->oldFramesReused                      << std::endl \
        << " totalSlept:               "    << myTree->totalSlept                           << std::endl \
        << " transitionCount:          "    << myTree->transitionCount                      << std::endl \
        << " predicatesEvaluatedTrue:  "    << myTree->predicatesEvaluatedTrue              << std::endl \
        << " predicatesEvaluatedFalse: "    << myTree->predicatesEvaluatedFalse             << std::endl \
        << " newZonesCreated:          "    << myTree->newZonesCreated                      << std::endl \
        << " oldZonesReused:           "    << myTree->oldZonesReused                       << std::endl \
                                                                                            << std::endl;
    if (duration>0) cout \
        << " forwardLinksEncountered:  "    << myTree->forwardLinksEncountered/duration     << std::endl \
        << " oldFramesDeleted:         "    << myTree->oldFramesDeleted/duration            << std::endl \
        << " newFramesCreated:         "    << myTree->newFramesCreated/duration            << std::endl \
        << " oldFramesReused:          "    << myTree->oldFramesReused/duration             << std::endl \
        << " transitionCount:          "    << myTree->transitionCount/duration             << std::endl \
        << " predicatesEvaluatedTrue:  "    << myTree->predicatesEvaluatedTrue/duration     << std::endl \
        << " predicatesEvaluatedFalse: "    << myTree->predicatesEvaluatedFalse/duration    << std::endl \
        << " newZonesCreated:          "    << myTree->newZonesCreated/duration             << std::endl \
        << " oldZonesReused:           "    << myTree->oldZonesReused/duration              << std::endl;

    cout << "Hello world!" << endl;
    return 0;
}
