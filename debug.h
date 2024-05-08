//
// Created by yitong on 2024/5/5.
//

#ifndef IOSCHE_DEBUG_H
#define IOSCHE_DEBUG_H
#include <iostream>
#include <vector>
#include <deque>
#include "IORequest.h"
using namespace std;
void logEvent(const string& eventDescription) {
    cout << eventDescription << endl;
}

vector<IORequest> operationsLog;

void addOperation(int time, int opNumber, int trackRequested) {
    operationsLog.emplace_back(opNumber, trackRequested, -1);
    logEvent(to_string(time) + ": " + to_string(opNumber) + " add " + to_string(trackRequested));
}

void issueOperation(int time, IORequest& op, int currentTrack) {
    op.track_number = currentTrack;  // Corrected to use track_number
    op.start_time = time;
    logEvent(to_string(time) + ": " + to_string(op.operation_index) + " issue " + to_string(op.track_number) + " " + to_string(currentTrack));
}

void finishOperation(int time, IORequest& op) {
    op.end_time = time;  // Corrected to use end_time
    int duration = op.end_time - op.start_time;  // Corrected to use end_time and start_time
    logEvent(to_string(time) + ": " + to_string(op.operation_index) + " finish " + to_string(duration));
}

void logQueueState(const vector<IORequest>& queue, int direction) {
    cout << "Queue State: ";
    for (const auto& req : queue) {
        cout << "[" << req.operation_index << " : " << req.track_number << "]";
    }
    cout << " Direction: " << (direction == 1 ? "up" : "down") << endl;
}
void logFlookState(const vector<IORequest>& activeQueue, const deque<IORequest>& waitQueue) {
    cout << "Active Queue: ";
    for (const auto& req : activeQueue) {
        cout << "[" << req.operation_index << " : " << req.track_number << "]";
    }
    cout << " | Wait Queue: ";
    for (const auto& req : waitQueue) {
        cout << "[" << req.operation_index << " : " << req.track_number << "]";
    }
    cout << endl;
}

#endif //IOSCHE_DEBUG_H
