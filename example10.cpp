#include <iostream>
#include <utility>
#include <queue>

#include "hsmTypes.h"
#include "hsm.h"
#include "subMachine.h"
#include "rtFrame.h"
#include "state.h"
#include "variable.h"

using namespace std;

// declare predicates and code fragments as global functions
void code_Initial_Start(rtFrame * here) {

    unsigned int & i = ( (hsmVariable<unsigned int> *) here->instantiations["i"])->theValue;

    srand(time(NULL));
    i=rand()%500000;
    std::cout<<"random number:"<<i<<std::endl;
}

bool pred_Z_notEqual(rtFrame * here) {
    hsmUint & siblingOrder = here->siblingOrder;
    unsigned int & i = ( (hsmVariable<unsigned int> *) here->parentFrame->instantiations["i"])->theValue;
    return (siblingOrder<i);
}
bool pred_Z_equal(rtFrame * here) {
    hsmUint & siblingOrder = here->siblingOrder;
    unsigned int & i = ( (hsmVariable<unsigned int> *) here->parentFrame->instantiations["i"])->theValue;
    return (siblingOrder==i);
}

void code_Z_notEqual(rtFrame * here) {
    hsmUint & siblingOrder = here->siblingOrder;
    std::queue<hsmUint> & q  = ( (hsmVariable<std::queue<hsmUint>> *) here->parentFrame->instantiations["q"])->theValue;
    q.push(siblingOrder);
}

void code_Z_equal(rtFrame * here) {
    std::queue<hsmUint> & q  = ( (hsmVariable<std::queue<hsmUint>> *) here->parentFrame->instantiations["q"])->theValue;
    std::cout<<"done, q.size(): "<<q.size()<< std::endl;
}

int main() {

    hsm * myMachine = new hsm("example10");

    // create sub-machines and their variables

    myMachine->initialSubMachine->declarations["i"] = new hsmVariable<unsigned int>(0) ;
    myMachine->initialSubMachine->declarations["q"] = new hsmVariable<std::queue<hsmUint>>;

    subMachine * sm_Z = myMachine->initialSubMachine->addSubMachine("Z");

    // create states (and, implied, edges), forward links, predicates, and code fragments
    // note: Initial submachine, and all Start states, are created implicitly
    state * st_initial_growVert  = myMachine->initialSubMachine->startState->addChildState("growVert");
    st_initial_growVert->setForwardLink(vertical,sm_Z,false);
    myMachine->initialSubMachine->startState->setCodeFragment(code_Initial_Start);

    state * st_Z_notEqual = sm_Z->startState->addChildState("notEqual");
    st_Z_notEqual->setForwardLink(horizontal,sm_Z,false);
    st_Z_notEqual->setPredicate(pred_Z_notEqual);
    st_Z_notEqual->setCodeFragment(code_Z_notEqual);

    state * st_Z_equal  = sm_Z->startState->addChildState("equal");
    st_Z_equal->setPredicate(pred_Z_equal);
    st_Z_equal->setCodeFragment(code_Z_equal);


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
