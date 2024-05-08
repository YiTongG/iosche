//
// Created by yitong on 2024/5/7.
//

#ifndef IOSCHE_SCHEDULER_H
#define IOSCHE_SCHEDULER_H

#include <algorithm>
#include "IORequest.h"
#include <vector>
#include <string>
#include <climits>

using namespace std;

class Scheduler {
public:
    vector<IORequest*> IOqueue;

    virtual void addRequest(IORequest* request) {
        IOqueue.push_back(request);
    }
    virtual IORequest* selectRequest(int& curr_track) = 0;
    Scheduler()= default;
    virtual ~Scheduler() = default;
    vector<IORequest*> activeQueue;
    vector<IORequest*> addOnQueue;
};

class FIFO : public Scheduler {
public:
    IORequest* selectRequest( int& curr_track) override {
        if(IOqueue.empty()) return nullptr;
        IORequest* op = IOqueue[0];
        IOqueue.erase(IOqueue.begin());
        return op;
    }

};
class SSTF : public Scheduler {
public:
    IORequest* selectRequest(int& curr_track) override {
        if (IOqueue.empty()) return nullptr;

        // min_element returns an iterator to the smallest element in the range
        auto closest = std::min_element(IOqueue.begin(), IOqueue.end(),
                                        [&curr_track](const IORequest* a, const IORequest* b) {
                                            return abs(a->track_number - curr_track) < abs(b->track_number - curr_track);
                                        });

        if (closest == IOqueue.end()) {
            return nullptr; //  if the queue is empty
        }

        IORequest* selectedRequest = *closest;
        IOqueue.erase(closest); //  remove the selected request from the queue
        return selectedRequest;
    }
};
//
class LOOK : public Scheduler {
public:
    int curr_direction = 1;
    IORequest* selectRequest(int& curr_track) override {
        if (IOqueue.empty()) return nullptr;

        int closestDistance = INT_MAX;
        int closestIndex = -1;
        bool foundRequest = false;

        //  iterate through the queue to find the closest request in the current direction
        for (int i = 0; i < IOqueue.size(); i++) {
            int distance = IOqueue[i]->track_number - curr_track;
            bool isForward = curr_direction ? (distance >= 0) : (distance <= 0);
            if (isForward && abs(distance) < closestDistance) {
                closestDistance = abs(distance);
                closestIndex = i;
                foundRequest = true;
            }
        }

        // if no request was found in the current direction, reverse the direction and search again
        if (!foundRequest) {
            curr_direction = !curr_direction; //    reverse the direction
            closestDistance = INT_MAX; //   reset the closest distance

            for (int i = 0; i < IOqueue.size(); i++) {
                int distance = IOqueue[i]->track_number - curr_track;
                bool isForward = curr_direction ? (distance >= 0) : (distance <= 0);
                if (isForward && abs(distance) < closestDistance) {
                    closestDistance = abs(distance);
                    closestIndex = i;
                }
            }
        }

        // if a request was found, remove it from the queue and return it
        if (closestIndex != -1) {
            IORequest* op = IOqueue[closestIndex];
            IOqueue.erase(IOqueue.begin() + closestIndex);
            return op;
        }else {     // if no request was found, remove the first request in the queue
            IORequest* op = IOqueue[0];
            IOqueue.erase(IOqueue.begin());
            return op;
        }
    }
};
//
class CLOOK : public Scheduler {
public:
    IORequest* selectRequest(int &curr_track) override {
        if (IOqueue.empty()) return nullptr;

        int closestDistance = INT_MAX;
        int closestIndex = 0;
        bool foundForward = false;

        //  iterate through the queue to find the closest request in the current direction
        for (int i = 0; i < IOqueue.size(); i++) {
            int trackDistance = IOqueue[i]->track_number - curr_track;

            //  only consider forward requests
            if (trackDistance >= 0 && trackDistance < closestDistance) {
                closestDistance = IOqueue[i]->track_number - curr_track;
                closestIndex = i;
                foundForward = true;
            }
        }

        //      if no request was found in the current direction, search for the closest request in the opposite direction
        if (!foundForward) {
            for (int i = 0; i < IOqueue.size(); i++) {
                int trackDistance = abs(IOqueue[i]->track_number -0);

                //  only consider backward requests
                if (IOqueue[i]->track_number - curr_track <= 0 && trackDistance < closestDistance) {
                    closestDistance = trackDistance;
                    closestIndex = i;
                }
            }
        }

        //  if a request was found, remove it from the queue and return it
        if (closestIndex != -1) {
            IORequest* op = IOqueue[closestIndex];
            IOqueue.erase(IOqueue.begin() + closestIndex);
            return op;
        }else {
            IORequest* op = IOqueue[0];
            IOqueue.erase(IOqueue.begin());
            return op;
        }




    }
};
//
class FLOOK : public Scheduler {
public:
    int curr_direction = 1;
    void addRequest(IORequest* request) override {
        IOqueue.push_back(request);
        addOnQueue.push_back(request);  // FLOOK-specific logic
    }
    IORequest* selectRequest(int &curr_track) override{
        if(IOqueue.empty() || (activeQueue.empty() && addOnQueue.empty() )) {
            return nullptr;
        }
        if (activeQueue.empty() && !addOnQueue.empty()) {
            activeQueue.swap(addOnQueue);
        }

        IORequest* op = nullptr;
        int closestDistance = INT_MAX;
        int closestIndex = -1;
        for (int attempt = 0; attempt < 2; ++attempt) {
            for (int i = 0; i < activeQueue.size(); i++) {
                int distance = abs(activeQueue[i]->track_number - curr_track);

                bool isCloserInCurrentDirection = (curr_direction && activeQueue[i]->track_number >= curr_track && distance < closestDistance) ||
                                                  (!curr_direction && activeQueue[i]->track_number <= curr_track && distance < closestDistance);

                if (isCloserInCurrentDirection) {
                    closestDistance = distance;
                    closestIndex = i;
                }
            }

            if (closestIndex != -1) break;

            curr_direction = !curr_direction;
        }

        if (closestIndex != -1) {
            op = activeQueue[closestIndex];
            activeQueue.erase(activeQueue.begin() + closestIndex);
        }

        if (!IOqueue.empty()) {
            IOqueue.erase(IOqueue.begin());  // 只有当 IOqueue 不空时才执行删除
        }

        return op;
    }
};
#endif //IOSCHE_SCHEDULER_H
