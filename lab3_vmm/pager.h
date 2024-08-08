//
// Created by yitong on 19/07/2024.
//

#ifndef VMM_PAGER_H
#define VMM_PAGER_H
#include <iostream>
#include <vector>
#include <queue>
#include <limits>
#include "global.h"


class Pager {
public:
    virtual frame_t* select_victim_frame() = 0;
    virtual ~Pager() = default;
};

class FIFOPager : public Pager {
public:
    FIFOPager() : hand(0) {}

    frame_t* select_victim_frame() override {
        int victim_index = hand;
        hand = (hand + 1) % frame_table.size();
        return &frame_table[victim_index];
    }


    int hand;
};
class ClockPager : public FIFOPager {
public:
    frame_t* select_victim_frame() override {
        int victim_index = -1;

        while (true) {
            pte_t* pte = &processes[frame_table[hand].pid].page_table[frame_table[hand].vpage];

            if (!pte->REFERENCED) {
                victim_index = hand;
                break;
            }

            pte->REFERENCED = 0;
            hand = (hand + 1) % frame_table.size();
        }

        hand = (victim_index + 1) % frame_table.size();
        return &frame_table[victim_index];
    }
};
class RandomPager : public Pager {
public:
    RandomPager() :  ofs(0) {}

    frame_t* select_victim_frame() override {
        int index = myrandom(int(frame_table.size())) - 1;  // -1 because myrandom returns 1-based index
        return &frame_table[index];
    }

private:
    int myrandom(int burst) {
        int val = 1 + (randvals[ofs] % burst);
        ofs = (ofs + 1) % randvals.size();
        return val;
    }
    size_t ofs;
};
//NRU Done
class NRUPager : public Pager {
public:
    NRUPager() : hand(0), lastResetInst(-1) {}

    frame_t* select_victim_frame() override {
        std::vector<frame_t*> pagePrio(4, nullptr);
        bool needReset = ((countInst - lastResetInst) >= 48);
        int victim_index = -1;

        for (size_t i = 0; i < frame_table.size(); ++i) {
            int current_index = (hand + i) % frame_table.size();
            frame_t* frame = &frame_table[current_index];
            pte_t* pte = &processes[frame->pid].page_table[frame->vpage];
            int prioPos = pte->REFERENCED * 2 + pte->MODIFIED; //r class-0 (R=0,M=0)..

            if (needReset) {
                pte->REFERENCED = 0;
            }

            if (prioPos == 0 && !needReset) {
                hand = (current_index + 1) % frame_table.size();
                victim_index = current_index;
                break;
            }

            if (!pagePrio[prioPos]) {
                pagePrio[prioPos] = frame;
            }
        }

        if (victim_index == -1) {
            frame_t* victimFrame = find_victim_from_priorities(pagePrio);
            victim_index = victimFrame->index;
            hand = (victim_index + 1) % frame_table.size();
        }

        if (needReset) {
            lastResetInst = countInst;
        }

        return &frame_table[victim_index];
    }

private:
    int hand;
    int lastResetInst;

    static frame_t* find_victim_from_priorities(const std::vector<frame_t*>& pagePrio) {
        for (auto frame : pagePrio) {
            if (frame) {
                return frame;
            }
        }
        return nullptr;
    }
};

class AgingPager : public Pager {
public:
    AgingPager() : hand(0) {}

    frame_t* select_victim_frame() override {
        unsigned int minAge = std::numeric_limits<unsigned int>::max();
        int victim_index = -1;
        for (int i = 0; i < frame_table.size(); ++i) {
            int index = (i + hand) % frame_table.size();
            frame_t* frame = &frame_table[index];
            pte_t* pte = &processes[frame->pid].page_table[frame->vpage];

            frame->age >>= 1;

            if (pte->REFERENCED) {
                frame->age |= 0x80000000;
                pte->REFERENCED = 0;
            }

            if (frame->age < minAge) {
                minAge = frame->age;
                victim_index = index;
            }
        }

        hand = (victim_index + 1) % frame_table.size();
        return &frame_table[victim_index];
    }



private:
    int hand;
};
//working set done;
class WorkingSetPager : public Pager {
public:
    WorkingSetPager() : hand(0) {}

    frame_t* select_victim_frame() override {
        int victim_index = hand % frame_table.size();
        int oldestUsed = countInst;

        for (int i = 0; i < frame_table.size(); ++i) {
            int current_index = (i + hand) % frame_table.size();
            pte_t* pte = &processes[frame_table[current_index].pid].page_table[frame_table[current_index].vpage];

            if (pte->REFERENCED) {
                frame_table[current_index].lastAccess = countInst;
                pte->REFERENCED = 0;
            } else {
                if ((countInst - frame_table[current_index].lastAccess) > TAU) {
                    victim_index = current_index;
                    break;
                }
                if (frame_table[current_index].lastAccess < oldestUsed) {
                    oldestUsed = frame_table[current_index].lastAccess;
                    victim_index = current_index;
                }
            }
        }

        hand = (victim_index + 1) % frame_table.size();
        return &frame_table[victim_index];
    }

private:
    int hand;
};


#endif //VMM_PAGER_H
