//
// Created by yitong on 2024/4/27.
//


#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <unordered_map>
#include <iterator>

using namespace std;




void read_input(string inputfile) {
    ifstream input;
    input.open(inputfile);

    if (input.is_open()) {
        string line;
        string inputType = "crew";
        int processNum = -1;
        int vmaNum = -1;
        int countProcess = -1;

        while (getline(input, line)) {
            if (line[0] != '#') {
                stringstream linestream(line);
                if (inputType.find(line[0]) != std::string::npos) { // eg. c 0, r 14
                    instruction inst;
                    linestream >> inst.instType >> inst.instVal;
                    instructions.push_back(inst);
                } else if (processNum == -1) { // record number of processes first
                    linestream >> processNum;
                } else if (vmaNum == -1) { // record each line of vma
                    linestream >> vmaNum;

                    Process process;
                    process.pid = ++countProcess;

                    string vmaString;
                    for (int time = 0; time < vmaNum; time++) {
                        getline(input, vmaString);
                        VMA vma;
                        stringstream vmaStream(vmaString);
                        vmaStream >> vma.start_vpage >> vma.end_vpage >> vma.write_protected >> vma.file_mapped;
                        process.vmaList.push_back(vma);
                    }

                    processes.push_back(process);
                    vmaNum = -1; // reset vmaNum
                } else {
                    cerr << "Unknown line. Exit!" << line << endl;
                    std::exit(1);
                }
            } else continue; // Start with '#'. Ignore
        }
    } else {
        cout << "Cannot open input file. Exit!" << endl;
        std::exit(1);
    }


}


void Summary () {

}



int main (int argc, char** argv) {
    int c;
    while ((c = getopt(argc, argv, "f::a::o::")) != -1) {
        switch (c) {
            case 'f':
                break;
            case 'a':
                break;
            case 'o':

                break;
            case '?':
                if (optopt == 'f' || optopt == 'a' || optopt == 'o')
                    std::cerr << "Option -" << char(optopt) << " requires an argument.\n";
                else
                    std::cerr << "Unknown option -" << char(optopt) << ".\n";
                return 1;
            default:
                std::cerr << "Unknown error.\n";
                return 1;
        }
    }
//readfile
    string input_file;
    if (optind + 1 < argc) {
        input_file = argv[optind];
    } else {
        cerr << "Error: Missing input file or random file." << endl;
        cerr << "Usage: " << argv[0] << " -f<num_frames> -a<algo> [-o<options>] inputfile randomfile" << endl;
        return 1;
    }

    read_input(input_file);

    return 0;
}