//
// Created by yitong on 18/03/2024.
//

#ifndef SCHEDULER_GLOBAL_H
#define SCHEDULER_GLOBAL_H
enum State {

    STATE_CREATED,
    STATE_READY,
    STATE_RUN,
    STATE_BLOCK,
    STATE_DONE,

};

enum Transition {

    TRANS_TO_READY,
    TRANS_TO_RUN,
    TRANS_TO_BLOCK,
    TRANS_TO_PREEMPT,
    TRANS_TO_DONE
};

struct Process{

    int PID; // process ID

    int remainingCB; // remainingCB: remaining CPU burst
    int cpuBurst; //get a random num;
    int ready_ts;
    int run_ts;
    int AT, TC, CB, IO, PRIO, FT, TT, IT, CW;
    int prev_cb;
    int prev_remaincb;
    int dynamic_prio;
    State state;
    Process(int id, int at, int tc, int cb, int io,int PRIO)
            : PID(id), AT(at), TC(tc), CB(cb), IO(io),
              PRIO(PRIO), dynamic_prio(PRIO), state(STATE_CREATED), ready_ts(0),run_ts(0),
              FT(0),TT(0), IT(0), CW(0),cpuBurst(0),remainingCB(tc) {}
};


#endif //SCHEDULER_GLOBAL_H