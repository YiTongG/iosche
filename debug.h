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


#endif //IOSCHE_DEBUG_H
