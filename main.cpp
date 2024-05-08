#include <iostream>
#include <fstream>
#include <sstream>

#include <unistd.h>
#include <unordered_map>
#include <vector>
#include <memory>
#include <list>
#include "IORequest.h"
#include "scheduler.h"
#include "simulator.h"

void read_input(const string& filename, vector<IORequest*>& requests) {
    ifstream file(filename);
    string line;
    int index = 0;  // Initialize index to keep track of the operation index
    while (getline(file, line)) {
        if (line[0] == '#' || line.empty()) continue;
        istringstream iss(line);
        int time, track;
        if (!(iss >> time >> track)) {
            continue;  // Error handling if input is malformed
        }
        auto* ioRequest = new IORequest(time, track, index++);
        requests.push_back(ioRequest);
    }
}




int main(int argc, char** argv) {
    vector<IORequest*> IORequests;
    bool verbose = false, queueState = false, flookState = false;
    string algo = "FIFO";  // Default scheduling algorithm
    int current_head_position = 0;
    //FIFO (N), SSTF (S), LOOK (L), CLOOK (C), and FLOOK (F)

    int c;
    while ((c = getopt(argc, argv, "s::vqf")) != -1) {
        switch (c) {
            case 's':
                if (strcmp(optarg, "N") == 0) {
                    algo = "FIFO";
                } else if (strcmp(optarg, "S") == 0) {
                    algo = "SSTF";
                } else if (strcmp(optarg, "L") == 0) {
                    algo = "LOOK";
                } else if (strcmp(optarg, "C") == 0) {
                    algo = "CLOOK";
                } else if (strcmp(optarg, "F") == 0) {
                    algo = "FLOOK";
                } else {
                    return 1;
                }

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


    Scheduler* scheduler = nullptr; // Adjust as necessary to use other schedulers based on 'algo'

    if (algo == "FIFO") {
        scheduler = new FIFO();
    } else if (algo == "SSTF") {
        scheduler = new SSTF();
    } else if (algo == "LOOK") {
        scheduler = new LOOK();
    } else if (algo == "CLOOK") {
        scheduler = new CLOOK();
    } else if (algo == "FLOOK") {
        scheduler = new FLOOK();
    } else {
        cerr << "Unknown scheduling algorithm." << endl;
        return 1;
    }
    Simulation sim(static_cast<unique_ptr<Scheduler>>(scheduler));

    if (optind < argc) {
        read_input(argv[optind], sim.IORequests);
    } else {
        cerr << "Missing input file. Usage: ./iosched -s<schedalgo> <inputfile>" << endl;
        return 1;
    }
    sim.verbose = verbose;
    sim.queueState = queueState;
    sim.flookState = flookState;
    sim.run();


    //delete scheduler; // Don't forget to clean up allocated memory
    return 0;
}