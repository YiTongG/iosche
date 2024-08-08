//
// Created by yitong on 05/03/2024.
//

#ifndef SCHEDULER_LOG_H
#define SCHEDULER_LOG_H
#include <iostream>
#include <string>

void logEvent(int timestamp, int pid, int timeInPrevState, const std::string& oldState, const std::string& newState) {
    std::cout << timestamp << " " << pid << " " << timeInPrevState << ": " << oldState << " -> " << newState << std::endl;
}

#endif //SCHEDULER_LOG_H
