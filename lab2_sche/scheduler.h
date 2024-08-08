//
// Created by yitong on 18/03/2024.
//

#ifndef SCHEDULER_SCHEDULER_H
#define SCHEDULER_SCHEDULER_H

#include <iomanip>
#include "DESLayer.h"

using namespace std;
class Scheduler {
public:
    deque<Process*> runQueue;
    virtual ~Scheduler() = default;
    virtual void addProcess(Process* p) = 0;
    virtual Process* getNextProcess() = 0;
    virtual bool testPreempt(Process* p, int curtime) = 0 ;
};

class FCFS_Scheduler: public Scheduler {
public:
    void addProcess(Process* p) override {
        runQueue.push_back(p);
    }

    Process* getNextProcess() override {
        if (runQueue.empty()) return nullptr;
        Process* p = runQueue.front();
        runQueue.pop_front();
        return p;
    }
     bool testPreempt(Process *process, int quantum) override {
        if (process->cpuBurst > quantum)
        {
            return true;
        }
        return false;
    }
};

class LCFS_Scheduler: public Scheduler {
public:
    void addProcess(Process* p) override {
        runQueue.push_back(p);
    }

    Process* getNextProcess() override {
        if (runQueue.empty()) return nullptr;
        Process* p = runQueue.back();
        runQueue.pop_back();
        return p;
    }
    bool testPreempt(Process *process, int quantum) override {
        if (process->cpuBurst > quantum)
        {
            return true;
        }
        return false;
    }
};
//
class SRTF_Scheduler: public Scheduler {
public:
    void addProcess(Process* p) override {
        auto it = runQueue.begin();
        while (it != runQueue.end() && (*it)->remainingCB <= p->remainingCB) {
            ++it;
        }
        runQueue.insert(it, p);  //
    }

    Process* getNextProcess() override {
        if (runQueue.empty()) return nullptr;
        Process* p = runQueue.front();  //
        runQueue.pop_front();  //
        return p;
    }
    bool testPreempt(Process *process, int quantum) override {
        if (process->cpuBurst > quantum)
        {
            return true;
        }
        return false;
    }

};
//
// Round Robin Scheduler
class RR_Scheduler : public Scheduler {
public:
    deque<Process*> runQueue;

    void addProcess(Process* p) override {
        runQueue.push_back(p);
    }

    Process* getNextProcess() override {
        if (runQueue.empty()) return nullptr;
        Process* p = runQueue.front();
        runQueue.pop_front();
        return p;
    }
    bool testPreempt(Process *process, int quantum) override {
        if (process->cpuBurst > quantum)
        {
            return true;
        }
        return false;
    }

};

// Priority Scheduler
class PRIO_Scheduler : public Scheduler {
public:
    vector<deque<Process*>> activeQueue;
    vector<deque<Process*>> expiredQueue;
    int maxprio;

    explicit PRIO_Scheduler(int maxp) : maxprio(maxp), activeQueue(maxp), expiredQueue(maxp) {}

    void addProcess(Process* p) override {
        if (p->state == STATE_READY) {
            p->dynamic_prio -= 1;
        } else {
            p->dynamic_prio = p->PRIO - 1;
        }
        p->state = STATE_READY;
        if (p->dynamic_prio < 0) {
            p->dynamic_prio = p->PRIO - 1;
            expiredQueue[p->dynamic_prio].push_back(p);
        } else {
            activeQueue[p->dynamic_prio].push_back(p);
        }
    }

    Process* getNextProcess() override {
        for (int prio = maxprio - 1; prio >= 0; --prio) {
            if (!activeQueue[prio].empty()) {
                Process* p = activeQueue[prio].front();
                activeQueue[prio].pop_front();
                return p;
            }
        }

        swap(activeQueue, expiredQueue);

        for (int prio = maxprio - 1; prio >= 0; --prio) {
            if (!activeQueue[prio].empty()) {
                Process* p = activeQueue[prio].front();
                activeQueue[prio].pop_front();
                return p;
            }
        }

        return nullptr;
    }
    bool testPreempt(Process *process, int quantum) override {
        if (process->cpuBurst > quantum)
        {
            return true;
        }
        return false;
    }
};


// Preemptive Priority Scheduler
class PREPRIO_Scheduler : public PRIO_Scheduler {
public:
    using PRIO_Scheduler::PRIO_Scheduler;

};


#endif //SCHEDULER_SCHEDULER_H