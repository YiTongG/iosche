//
// Created by yitong on 18/03/2024.
//

#ifndef SCHEDULER_DESLAYER_H
#define SCHEDULER_DESLAYER_H


#include <memory>
#include <algorithm>
#include <deque>
#include <list>
#include "global.h"
using namespace std;
struct Event {
    int timestamp;
    Process* evtProcess; //use pointer instead of object
    Transition transition;//
    State oldState;
    State newState;

    Event(int timeStamp, Process* process,State oldState,State newState, Transition trans)
            : timestamp(timeStamp), evtProcess(process), oldState(oldState),newState(newState),transition(trans){}
};

class DESLayer {
public:
    list<Event*> events;
    void addEvent(Event* event) {
        auto it = std::upper_bound(events.begin(), events.end(), event, [](Event* a, Event* b) {
            return a->timestamp < b->timestamp;
        });
        events.insert(it, event);
    }

    Event* getNextEvent() {

        if (events.empty()) {
            return nullptr;
        }
        Event* evt = events.front();
        events.pop_front();

        return evt;
    }
    int getNextEventTime() {
        if(events.empty()) return -1;
        return (*events.begin())->timestamp;
    }
    Event* findEventByProcess(Process* process) {
        for (auto & event : events) {
            if (event->evtProcess->PID == process->PID) {
                return event;
            }
        }
        return nullptr;
    }
    Event* findEventByTime(Process* process,int Time) {
        for (auto & event : events) {
            if (event->timestamp == Time &&process == event->evtProcess) {
                return event;
            }
        }
        return nullptr;
    }


    void removeEvent(Event* event) {
        events.remove(event);
    }

};
#endif //SCHEDULER_DESLAYER_H