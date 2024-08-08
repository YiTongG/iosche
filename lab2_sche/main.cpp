
//
// Created by yitong on 12/03/2024.
//
using namespace std;
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <getopt.h>
#include <cstdlib>
#include <memory>

#include "DESLayer.h"
#include "scheduler.h"
#include "log.h"
using namespace std;
vector<int> randvals;


size_t ofs = 0;
int maxprio=4;
int quantum = 10000;
//
int myrandom(int burst) {
    int val = 1 + (randvals[ofs] % burst);
    ofs = (ofs + 1) % randvals.size();
    return val;
}

deque<Process*> readInputFile(const string& inputFile) {
    deque<Process*> processes;
    ifstream inFile(inputFile);
    if (!inFile) {
        cerr << "Unable to open input file: " << inputFile << endl;
        exit(EXIT_FAILURE);
    }
    int pid = 0;  //
    string line;
    while (getline(inFile, line)) {
        istringstream iss(line);
        int at, tc, cb, io;
        iss >> at >> tc >> cb >> io;
        processes.push_back(new Process(pid++, at, tc, cb, io, myrandom(maxprio)));
    }
    inFile.close();
    return processes;
}

void readRandFile(const string& randFile) {
    ifstream randIn(randFile);
    int count;
    randIn >> count;
    randvals.resize(count);

    for (int i = 0; i < count; ++i) {
        randIn >> randvals[i];
    }

    randIn.close();
}

int main(int argc, char* argv[]) {
    string schedulerSpec;
    int opt;
    string scheType;
    bool verbose = false;

    while ((opt = getopt(argc, argv, "vtep:s:")) != -1) {
        switch (opt) {
            case 'v':
                verbose = true;
                break;
            case 's':
                schedulerSpec = optarg;
                break;
            default:
                exit(EXIT_FAILURE);
        }
    }
    Scheduler* scheduler;
    if (schedulerSpec[0] == 'F') {
        scheduler = new FCFS_Scheduler();
        scheType = "FCFS";
    } else if (schedulerSpec[0] == 'L') {
        scheduler = new LCFS_Scheduler();
        scheType = "LCFS";

    } else if (schedulerSpec[0] == 'S') {
        scheduler = new SRTF_Scheduler();
        scheType = "SRTF";
    }  else if (schedulerSpec[0] == 'R') {
        sscanf(schedulerSpec.c_str(), "R%d", &quantum);
        scheduler = new RR_Scheduler();
        scheType = "RR";// + to_string(quantum);
    } else if (schedulerSpec[0] == 'P') {
        if (schedulerSpec.find(':') != string::npos) {
            sscanf(schedulerSpec.c_str(), "P%d:%d", &quantum, &maxprio);
        } else {
            sscanf(schedulerSpec.c_str(), "P%d", &quantum);
        }

        scheduler = new PRIO_Scheduler(maxprio);
        scheType = "PRIO";
    } else if (schedulerSpec[0] == 'E') {
        if (schedulerSpec.find(':') != string::npos) {
            sscanf(schedulerSpec.c_str(), "E%d:%d", &quantum, &maxprio);
        } else {
            sscanf(schedulerSpec.c_str(), "E%d", &quantum);
        }
        scheduler = new PREPRIO_Scheduler(maxprio);
        scheType = "PREPRIO";//;
    } else {
        cerr << "Unknown scheduler type" << endl;
        exit(EXIT_FAILURE);
    }

    string inputFile;
    string randFile;
    if (optind < argc) {
        inputFile = argv[optind++];
    }
    if (optind < argc) {
        randFile = argv[optind++];
    }
    readRandFile(randFile);
    auto processes = readInputFile(inputFile);


    DESLayer des;
    for (auto& proc : processes) {
        des.addEvent(new Event(proc->AT, proc,STATE_CREATED,STATE_READY, TRANS_TO_READY));
    }



    Event* evt;
    Process* CURRENT_RUNNING_PROCESS = nullptr; // this is the currently running process

    int last_event_time = 0;
    int ioLast = 0;//the time of last io finish
    int lastCpuBurst = 0;
    int curTime;
    int timeInPrevState;
    int io_burst;
    int cpu_burst;
    int realTime;
    bool CALL_SCHEDULER = false;
    double io_utilization = 0; //io utilization
    double cpu_utilization = 0; //cpu utilization
    string oldState,newState;
    while((evt = des.getNextEvent())){
        Process *proc = evt->evtProcess; //
        curTime = evt->timestamp;
        timeInPrevState = curTime - proc->ready_ts;
        Transition trans = evt->transition;
        switch(trans) { //
            case TRANS_TO_READY:
                oldState = (proc->state == STATE_BLOCK) ? "BLOCK" : "CREATE";
                newState = "READY";
                    if(CURRENT_RUNNING_PROCESS == nullptr || scheType !="PREPRIO"){
                        proc->ready_ts = curTime;
                        if (proc->state == STATE_CREATED) {
                            proc->state = STATE_READY;
                            CALL_SCHEDULER = (CURRENT_RUNNING_PROCESS == nullptr && curTime >= lastCpuBurst);

                        } else if (proc->state == STATE_BLOCK) {
                            proc->state = STATE_READY;
                            proc->dynamic_prio = proc->PRIO;
                            CALL_SCHEDULER = (CURRENT_RUNNING_PROCESS == nullptr && curTime >= lastCpuBurst);
                        } else {
                            proc->state = STATE_READY;
                            CALL_SCHEDULER = true;
                        }
                        scheduler->addProcess(proc);
                    }else {
                        if (proc->state == STATE_BLOCK || proc->state == STATE_CREATED) {
                            proc->dynamic_prio = proc->PRIO - 1;
                        }
                        Event *currentEvent = des.findEventByProcess(CURRENT_RUNNING_PROCESS);
                        int eventTimeStamp = (currentEvent != nullptr) ? currentEvent->timestamp : 0;
                        //use priority not testpreempt
                        if (proc->dynamic_prio > CURRENT_RUNNING_PROCESS->dynamic_prio) {


                            if (scheduler->testPreempt(proc, eventTimeStamp) ||
                                (eventTimeStamp != curTime && eventTimeStamp > 0)) {
                                int timeDiff = curTime - CURRENT_RUNNING_PROCESS->ready_ts;
                                if (currentEvent != nullptr) {
                                    CURRENT_RUNNING_PROCESS->remainingCB = CURRENT_RUNNING_PROCESS->prev_remaincb - timeDiff;
                                    CURRENT_RUNNING_PROCESS->cpuBurst = CURRENT_RUNNING_PROCESS->prev_cb - timeDiff;
                                }
                                des.removeEvent(currentEvent);
                                auto *newEvt = new Event(curTime, CURRENT_RUNNING_PROCESS, STATE_RUN, STATE_READY,
                                                         TRANS_TO_PREEMPT);
                                des.addEvent(newEvt);

                                CURRENT_RUNNING_PROCESS = nullptr;
                            }
                        }
                        proc->ready_ts = curTime;
                        scheduler->addProcess(proc);
                    }
                    if(verbose) logEvent(curTime, proc->PID, timeInPrevState, oldState, newState);

                break;


                case TRANS_TO_RUN:
                    oldState = "READY";
                newState = "RUN";
                proc->state = STATE_RUN;
                proc->ready_ts = curTime;
                proc->CW += timeInPrevState;

                if (proc->run_ts == -1)
                    proc->run_ts = curTime;

                if(proc->cpuBurst == 0){
                    proc->cpuBurst = myrandom(proc->CB);
                    if(proc->cpuBurst > proc->remainingCB) proc->cpuBurst = proc->remainingCB;
                }

                cpu_burst = std::min(quantum, proc->cpuBurst);
                if(scheduler->testPreempt(proc,quantum)) {
                    realTime = curTime + cpu_burst;
                }else {
                    realTime = curTime + proc->cpuBurst;
                }

                proc->prev_cb = proc->cpuBurst;//for eprio
                proc->prev_remaincb = proc->remainingCB;
                if(!scheduler->testPreempt(proc,quantum)){ //no preempt
                    proc->remainingCB -= proc->cpuBurst;
                    proc->cpuBurst = 0;
                    if(proc->remainingCB > 0){
                        des.addEvent(new Event{realTime,proc,STATE_RUN,STATE_BLOCK,TRANS_TO_BLOCK});

                    } else {
                        proc->state = STATE_DONE;
                        curTime = realTime;
                        proc->TT = realTime - proc->AT;
                        des.addEvent(new Event(curTime,proc,STATE_RUN,STATE_DONE,TRANS_TO_DONE));
                    }
                } else {
                    //preempt
                    proc->remainingCB -= cpu_burst;
                    proc->cpuBurst -= cpu_burst;
                    if (proc->remainingCB <= 0) {
                        proc->TT = realTime - proc->AT;
                        des.addEvent(new Event(realTime, proc, STATE_RUN, STATE_DONE, TRANS_TO_DONE));
                    } else {
                        if (scheType != "PREPRIO") {
                            des.addEvent(new Event(realTime, proc, STATE_RUN, STATE_READY, TRANS_TO_READY));

                            CURRENT_RUNNING_PROCESS = nullptr;
                        }else {
                            des.addEvent(new Event(realTime, proc, STATE_RUN, STATE_READY, TRANS_TO_PREEMPT));

                        }
                        lastCpuBurst = realTime;

                    }

                }
                if(verbose)  logEvent(curTime, proc->PID, timeInPrevState, oldState, newState);

                break;

            case TRANS_TO_BLOCK:
                oldState = "RUN";
                newState = "BLOCK";
                //create an event for when process becomes READY again CALL_SCHEDULER = true;
                proc->ready_ts = curTime;
                proc->run_ts = -1;
                proc->state = STATE_BLOCK;
                io_burst = myrandom(proc->IO);
                proc->IT += io_burst;
                des.addEvent(new Event(curTime +io_burst,proc, STATE_BLOCK,STATE_READY,TRANS_TO_READY));
                {
                    int endIO = curTime + io_burst;
                    if (curTime > ioLast) {
                        io_utilization += io_burst;
                        ioLast = endIO;

                    } else if (endIO > ioLast) {
                        io_utilization += endIO - ioLast;
                        ioLast = endIO;

                    }
                }
                CURRENT_RUNNING_PROCESS = nullptr;
                CALL_SCHEDULER = true;
                if(verbose) logEvent(curTime, proc->PID, timeInPrevState, oldState, newState);

                break;

            case TRANS_TO_PREEMPT:
                oldState = "RUN";
                newState = "READY";

                proc->ready_ts = curTime;
                proc->run_ts = -1;
                proc->state = STATE_READY; //keep
                scheduler->addProcess(proc);
                CALL_SCHEDULER = true;
                CURRENT_RUNNING_PROCESS = nullptr;
                if(verbose)logEvent(curTime, proc->PID, timeInPrevState, oldState, newState);

                break;
            case TRANS_TO_DONE:
                oldState = "RUN";
                newState = "DONE";
                proc->ready_ts = curTime;
                proc->state = STATE_DONE;
                proc->FT=curTime;
                last_event_time = curTime;
                CALL_SCHEDULER = true;
                CURRENT_RUNNING_PROCESS = nullptr;
                if(verbose) logEvent(curTime, proc->PID, timeInPrevState, oldState, newState);

                break;

        }
        delete evt;
        evt = nullptr;

        if(CALL_SCHEDULER) {
            CALL_SCHEDULER = false;
            if (des.getNextEventTime() != curTime && CURRENT_RUNNING_PROCESS == nullptr) {
                CURRENT_RUNNING_PROCESS = scheduler->getNextProcess();
                if (CURRENT_RUNNING_PROCESS != nullptr){
                    des.addEvent(new Event(curTime, CURRENT_RUNNING_PROCESS, STATE_READY, STATE_RUN, TRANS_TO_RUN));
                }
            }
        }
    }

    //for summary output
    int sumTT = 0;
    int sumCW = 0;
    if(scheType=="FCFS" || scheType == "LCFS" || scheType == "SRTF") {
        cout<<scheType<<endl;
    }else{
        cout<<scheType<<" "<<quantum<<endl;

    }
    size_t Procsize = processes.size();
    for(int i = 0;i<Procsize;i++){
        printf("%04d: %4d %4d %4d %4d %1d | %5d %5d %5d %5d\n",
               processes[i]->PID,
               processes[i]->AT,
               processes[i]->TC,
               processes[i]->CB,
               processes[i]->IO,
               processes[i]->PRIO,
               processes[i]->FT,
               processes[i]->TT,
               processes[i]->IT,
               processes[i]->CW);
        cpu_utilization += processes[i]->TC;
        sumTT += processes[i]->TT;
        sumCW += processes[i]->CW;
    }

    cpu_utilization = cpu_utilization * 100 / last_event_time;
    io_utilization = io_utilization * 100 / last_event_time;
    printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n", last_event_time, cpu_utilization, io_utilization,  double (sumTT ) / (int)Procsize, double (sumCW) / (int)Procsize, double ((int)Procsize * 100) / last_event_time);

    return 0;
}