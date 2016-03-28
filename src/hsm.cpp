#include "hsm.h"
#include "subMachine.h"
#include "state.h"
#include "rtTree.h"
#include "rtZone.h"
#include "rtFrame.h"

#include <thread>
#include <iostream>
#include <atomic>
#include <assert.h>
#include <condition_variable>

#include <assert.h>
#include <unistd.h>
#include <functional>

#include "rtZone.h"

//
// hsm -- the hierarchial state machine
//

hsm::hsm(std::string newName) : initialSubMachine(new subMachine(this)) {
    name=newName;
    //initialSubMachine = new subMachine(this);
    subMachineTable.insert(std::pair<subMachine *, subMachine *> ( (subMachine *) 0,initialSubMachine));
}

hsm::~hsm()
{
    for(auto i=subMachineTable.begin();i!=subMachineTable.end();i++)
        delete i->second;
}

rtTree * hsm::execute() {

    // executes the hsm asyncronously in a new and detached thread
    // note: caller should use hsm::wait to wait for the hsm to finish executing to ensure this new thread and it's child threads have exited

    //erfc std::cout<<"executing machine"<<std::endl;

    ////////////////////////////////////////////////
    //
    // construct the initial objects required: one rtTree containing exactly one rtFrame (i.e. exactly one node, the root), and exactly one zone to contain that frame
    //

    rtTree * myTree = new rtTree(this);                                         // returned to caller, caller should delete after execution completes
    rtZone * topZone = myTree->makeZone();
    rtFrame * topFrame = topZone->makeFrame(topZone,initialSubMachine);

    topZone->activeFrames[{}]= topFrame;

    time(&(myTree->startTime));                                                 // in case you want to know how long the hsm has been executing

    std::thread t(std::thread( (std::reference_wrapper<rtZone>)*topZone ));     // call the rtZone operator() in a new thread to execute the top zone
    t.detach();

    return myTree;
}
