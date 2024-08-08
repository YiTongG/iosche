//
// Created by yitong on 2024/5/5.
//

#ifndef IOSCHE_IOREQUEST_H
#define IOSCHE_IOREQUEST_H
struct IORequest {
    int arrival_time;
    int track_number;
    int start_time;
    int end_time;
    int operation_index;

    IORequest(int arr, int track, int index)
            : arrival_time(arr), track_number(track), start_time(0), end_time(0), operation_index(index) {}
};
#endif //IOSCHE_IOREQUEST_H
