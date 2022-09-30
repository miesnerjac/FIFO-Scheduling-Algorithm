#include "process.h"
#include "ioModule.h"
#include "processMgmt.h"

#include <chrono>
#include <thread>


int main(int argc, char* argv[])
{
    // single thread processor
    // it's either processing something or it's not
    bool processorAvailable;

    // vector of processes, processes will appear here when they are created by the ProcessMgmt object
    list<Process> processList;

    //queue of processes, the next process to run will be at the front()
    list<Process> nextToRunQueue;

    // this will orchestrate process creation in our system, it will add processes to
    // processList when they are created and ready to be run/managed
    ProcessManagement processMgmt(processList);

    // this is where interrupts will appear when the ioModule detects that an IO operation is complete
    list<IOInterrupt> interrupts;

    // this manages io operations and will raise interrupts to signal io completion
    IOModule ioModule(interrupts);

    list<Process>::iterator iter = processList.begin(); //iterator to keep track of currently running process
    list<Process>::iterator otherIter = processList.begin(); //separate iterator for indexing purposes
    bool allDone(false); //bool that is set to true when all processes are done
    bool admittedProc(false); //used to only allow one step action at a time

    long time;
    long sleepDuration = 50; //easier to read outputting
    string file;
    stringstream ss;

    enum stepActionEnum {noAct, admitNewProc, handleInterrupt, beginRun, continueRun, ioRequest, complete} stepAction;

    // Do not touch
    switch(argc)
    {
        case 1:
            file = "./procList.txt";  // default input file
            break;
        case 2:
            file = argv[1];         // file given from command line
            break;
        case 3:
            file = argv[1];         // file given
            ss.str(argv[2]);        // sleep duration given
            ss >> sleepDuration;
            break;
        default:
            cerr << "incorrect number of command line arguments" << endl;
            cout << "usage: " << argv[0] << " [file] [sleepDuration]" << endl;
            return 1;
            break;
    }

    processMgmt.readProcessFile(file);

    //initialization of values
    time = 0;
    processorAvailable = true;

    //keep running the loop until all processes have been added or all have not run to completion
    while(processMgmt.moreProcessesComing() || !allDone)
    {
        //Update current time step
        ++time;

        //let new processes in if there are any
        processMgmt.activateProcesses(time);

        //update the status for any active IO requests
        ioModule.ioProcessing(time);

        //init the stepAction, update below
        stepAction = noAct;

        if(!processorAvailable){ //if processor is running a process
            if(iter->ioEvents.size() > 0 && iter->ioEvents.front().time == iter->processorTime){ //if the running process has an IO event scheduled
                stepAction = ioRequest; //action for this iteration will be to handle IO
                iter->state = blocked; //set the process state to blocked
                ioModule.submitIORequest(time, iter->ioEvents.front(), *iter); //submit IO request
                iter->ioEvents.pop_front(); //remove the IO request
                processorAvailable = true; //processor is not available
            }
            else if(iter->reqProcessorTime == iter->processorTime){ //if the required time is reached
                iter->state = done; //set process state to done
                stepAction = complete; //action for this iteration is complete
                processorAvailable = true;
            }
            else{
                stepAction = continueRun;
                iter->state = processing;
                iter->processorTime += 1; //incr run time
                processorAvailable = false;
            }
        }
        else{ //if the processor is available
            admittedProc = false;
            //searches to see if any processes are in the new state, ends when one is found
            for(otherIter = processList.begin(); otherIter != processList.end() && !admittedProc; otherIter++){
                if(otherIter->state == newArrival){ //if there is a new arrival
                    otherIter->state = ready; //set the state to ready
                    nextToRunQueue.push_back(*otherIter); //sets the process to run last in the current queue
                    stepAction = admitNewProc; //action for this iteration is to admit
                    admittedProc = true;
                }
            } //end of searching for new arrivals, if one was found: admittedProc = true

            if(!admittedProc){ //if did not admit a new process,
                if(interrupts.size() > 0){ //look for interrupts to handle
                    for(otherIter = processList.begin(); otherIter != processList.end(); otherIter++){
                        if(otherIter->id == interrupts.front().procID) break;
                        //when found, break to preserve iterator's position in the list
                    }
                    nextToRunQueue.push_back(*otherIter); //sets the process to run last in the current queue
                    interrupts.pop_front(); //remove the interrupt that just came back
                    otherIter->state = ready;
                    stepAction = handleInterrupt;
                }
                else{ //if did not find new process AND did not find interrupt to handle: run new process
                    if(nextToRunQueue.size() > 0){ //if there are still ready processes to run
                        for(iter = processList.begin(); iter != processList.end(); iter++){ //find the next ready process
                            if(iter->id == nextToRunQueue.front().id){
                                nextToRunQueue.pop_front(); //remove the process from the queue
                                iter->state = processing; //sets state
                                stepAction = beginRun;
                                processorAvailable = false;
                                iter->processorTime += 1; //incr run time
                                break; //break to preserve iter's position in the list
                            }
                        }
                    }
                    else{ //if there are no ready processes to run (only waiting on IO requests to come back)
                        stepAction = noAct; //do nothing
                    }
                }
            }
        }

        allDone = true; //initializes list to be finished
        for(otherIter = processList.begin(); otherIter != processList.end(); otherIter++){ //goes thru list to see if their states are done
            if(otherIter->state != done) allDone = false; //if one is not done then keep looping
        }

        // Outputs current processes and their states to the console
        cout << setw(5) << time << "\t";

        switch(stepAction)
        {
            case admitNewProc:
              cout << "[  admit]\t";
              break;
            case handleInterrupt:
              cout << "[ inrtpt]\t";
              break;
            case beginRun:
              cout << "[  begin]\t";
              break;
            case continueRun:
              cout << "[contRun]\t";
              break;
            case ioRequest:
              cout << "[  ioReq]\t";
              break;
            case complete:
              cout << "[ finish]\t";
              break;
            case noAct:
              cout << "[*noAct*]\t";
              break;
        }

        printProcessStates(processList);

        this_thread::sleep_for(chrono::milliseconds(sleepDuration));
    } //end of while loop

    return 0;
}
