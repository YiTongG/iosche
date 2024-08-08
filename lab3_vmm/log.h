//
// Created by yitong on 19/07/2024.
//

#ifndef VMM_LOG_H
#define VMM_LOG_H
#include <iostream>
#include <string>
#include "global.h"
void logContextSwitch(int countInst, char operation, int vpage) {
    std::cout << countInst << ": ==> " << operation << " " << vpage << std::endl;
}

void logUnmap(int pid, int vpage) {
    std::cout << " UNMAP " << pid << ":" << vpage << std::endl;
}
void logMap(int vpage) {
    std::cout << " MAP " << vpage << std::endl;
}

void logFout() {
    std::cout << " FOUT" << std::endl;
}

void logOut() {
    std::cout << " OUT" << std::endl;
}

void logFin() {
    std::cout << " FIN" << std::endl;
}

void logIn() {
    std::cout << " IN" << std::endl;
}

void logZero() {
    std::cout << " ZERO" << std::endl;
}

void logSegv() {
    std::cout << " SEGV" << std::endl;
}

void logSegprot() {
    std::cout << " SEGPROT" << std::endl;
}

void logExit(int vpage) {
    std::cout << "EXIT current process " << vpage << std::endl;
}

#endif //VMM_LOG_H
