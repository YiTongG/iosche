//
// Created by yitong on 2024/5/8.
//

#ifndef IOSCHE_SIMULATOR_H
#define IOSCHE_SIMULATOR_H
#include <memory>
#include <vector>
#include <list>
#include "IORequest.h"
#include "scheduler.h"
#include "debug.h"

class Simulation {

private:
    unique_ptr<Scheduler> scheduler;


    double avg_turnaround = 0;
    double avg_waittime = 0;
    int max_waittime = 0;
    int currentTime = 0;
    int total_time = 0;
    int totalMove = 0;
    double total_io_time = 0; // Time I/O was actively processing requests
    IORequest* currentActiveRequest = nullptr;

    int currentHead = 0; // compare with track
    IORequest* currOperation{};


// variables for summary statistics



public:
    bool verbose = false;
    bool queueState = false;
    bool flookState = false;
    vector<IORequest*> IORequests;

    explicit Simulation(unique_ptr<Scheduler> sched) : scheduler(std::move(sched)) {}

    void run() {
        IORequest** currentOpPtr = IORequests.empty() ? nullptr : &IORequests[0];

        while (!scheduler->IOqueue.empty() || currentActiveRequest != nullptr || currentOpPtr != &IORequests[IORequests.size()]) {

            if(currentOpPtr != &IORequests[IORequests.size()] && currentTime == (*currentOpPtr)->arrival_time){
                scheduler->addRequest(*currentOpPtr);
                if (verbose) {
                    addOperation(currentTime, (*currentOpPtr)->operation_index, (*currentOpPtr)->track_number);
                }
                currentOpPtr++;
            }

            //current active
            if(currentActiveRequest != nullptr){
                if(currentHead == currOperation->track_number){
                    currOperation->end_time = currentTime;
                    if (verbose) {
                        finishOperation(currentTime, *currOperation);
                    }
                    currentActiveRequest = nullptr;
                }else {
                    int direction = currOperation->track_number > currentHead ? 1 : -1;
                    currentHead += direction;
                    totalMove++;
                }
            }
            if (currentActiveRequest != nullptr || scheduler->IOqueue.empty()){

            } else {
                currOperation = scheduler->selectRequest(currentHead);
                currOperation->start_time = currentTime;
                currentActiveRequest = currOperation;
                if (verbose) {
                    issueOperation(currentTime, *currOperation, currentHead);
                }
                continue;
            }
            currentTime++;
        }

        // Print the summary statistics

        // 计算和打印统计数据
        for (const auto& operation : IORequests) {
            printf("%5d: %5d %5d %5d\n", operation->operation_index, operation->arrival_time, operation->start_time, operation->end_time);
            int turnaround_time = operation->end_time - operation->arrival_time;
            int wait_time = operation->start_time - operation->arrival_time;
            total_io_time += operation->end_time - operation->start_time; // Assuming this represents the busy time
            avg_turnaround += turnaround_time;
            avg_waittime += wait_time;
            max_waittime = std::max(max_waittime, wait_time);
            total_time = std::max(total_time, operation->end_time); // Ensuring total_time is the max end_time observed
        }

        if (!IORequests.empty()) {
            avg_turnaround /= IORequests.size();
            avg_waittime /= IORequests.size();
        }

        double io_utilization = IORequests.empty() ? 0 : total_io_time / total_time;

        printf("SUM: %d %d %.4lf %.2lf %.2lf %d\n",
               total_time, totalMove, io_utilization,
               avg_turnaround, avg_waittime, max_waittime);
    }


};
#endif //IOSCHE_SIMULATOR_H
