#include <iostream>
#include <fstream>
#include <sstream>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <unordered_map>
#include <iterator>
#include <vector>
#include "IORequest.h"
#include "debug.h"

using namespace std;



class Scheduler {
public:
    virtual IORequest selectRequest(vector<IORequest>& queue, int head_position) = 0;
};

class FIFO : public Scheduler {
public:
    IORequest selectRequest(vector<IORequest>& queue, int head_position) override {
        // Implement FIFO logic
        if (queue.empty()) {
            throw std::out_of_range("Queue is empty");
        }
        // The request at the front of the queue is selected
        IORequest request = queue.front();
        queue.erase(queue.begin()); // Remove the request from the queue after it's selected
        return request;
    }
};

// Additional scheduler classes (SSTF, LOOK, CLOOK, FLOOK) would be defined here

void read_input(const string& filename, vector<IORequest>& requests) {
    ifstream file(filename);
    string line;
    while (getline(file, line)) {
        if (line[0] == '#' || line.empty()) continue;
        istringstream iss(line);
        int time, track;
        iss >> time >> track;
        IORequest ioRequest(time, track, requests.size());
        requests.push_back(ioRequest);
    }
}
void run_simulation(Scheduler* scheduler, vector<IORequest>& io_requests, bool verbose, bool queueState, bool flookState) {
    vector<IORequest> queue;
    int current_time = 0;
    int head_position = 0;
    int total_movement = 0;
    int current_request_index = 0;
    double io_busy_time = 0;
    vector<IORequest*> active_requests; // To store active requests //why pointer here?

    while (true) {
        // Handle new arrivals
        if (current_request_index < io_requests.size() && io_requests[current_request_index].arrival_time <= current_time) {
            queue.push_back(io_requests[current_request_index]);
            if (verbose) addOperation(current_time, current_request_index, io_requests[current_request_index].track_number);
            current_request_index++;
        }

        // Debug: Log the state of the queue
        if (queueState) logQueueState(queue, 1); // Assuming direction is 1 for simplicity

        // Processing active IO
        if (!active_requests.empty() && active_requests.front()->end_time == current_time) {
            IORequest* req = active_requests.front();
            if (verbose) finishOperation(current_time, *req);
            active_requests.erase(active_requests.begin());
        }

        // Fetching the next IO request
        if (active_requests.empty() && !queue.empty()) {
            IORequest next_req = scheduler->selectRequest(queue, head_position);
            if (verbose) issueOperation(current_time, next_req, head_position);
            int seek_time = abs(next_req.track_number - head_position);
            total_movement += seek_time;
            next_req.start_time = current_time;
            next_req.end_time = current_time + seek_time;
            active_requests.push_back(&next_req);
        }

        // Check for simulation end condition
        if (current_request_index >= io_requests.size() && active_requests.empty()) {
            break;
        }

        current_time++;
    }


    // Calculate summary statistics
    double total_time = current_time;
    double io_utilization = io_busy_time / total_time;
    double avg_turnaround = 0; // Compute based on actual processing times
    double avg_waittime = 0; // Compute based on wait times from arrival to start
    int max_waittime = 0; // Compute the maximum wait time
    for (const auto& req : io_requests) {
        avg_turnaround += (req.end_time - req.arrival_time);
        avg_waittime += (req.start_time - req.arrival_time);
        max_waittime = max(max_waittime, req.start_time - req.arrival_time);
    }
    avg_turnaround /= io_requests.size();
    avg_waittime /= io_requests.size();

    printf("SUM: %d %d %.4lf %.2lf %.2lf %d\n",
           (int)total_time, total_movement, io_utilization,
           avg_turnaround, avg_waittime, max_waittime);
}
int main(int argc, char** argv) {
    vector<IORequest> io_requests;
    bool verbose = false, queueState = false, flookState = false;
    string algo = "FIFO";  // Default scheduling algorithm
    int current_head_position = 0;

    int c;
    while ((c = getopt(argc, argv, "s::vqf")) != -1) {
        switch (c) {
            case 's':
                if (optarg) algo = optarg;
                break;
            case 'v':
                verbose = true;
                break;
            case 'q':
                queueState = true;
                break;
            case 'f':
                flookState = true;
                break;
            default:
                cerr << "Unknown option or missing argument." << endl;
                return 1;
        }
    }

    if (optind < argc) {
        read_input(argv[optind], io_requests);
    } else {
        cerr << "Missing input file. Usage: ./iosched -s<schedalgo> <inputfile>" << endl;
        return 1;
    }

    Scheduler* scheduler = new FIFO(); // Adjust as necessary to use other schedulers based on 'algo'
    run_simulation(scheduler, io_requests, verbose, queueState, flookState);

    //delete scheduler; // Don't forget to clean up allocated memory
    return 0;
}
