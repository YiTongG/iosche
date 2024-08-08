
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#include <stdio.h>
#include <unistd.h>
#include <unordered_map>
#include <regex>
#include "global.h"
#include "log.h"
#include "pager.h"
using namespace std;


void read_input(const string& input_file) {
    ifstream input(input_file);

    if (!input.is_open()) {
        cerr << "Cannot open input file" << endl;
        exit(1);
    }

    string line;
    regex num_regex(R"(^\d+$)");
    regex vma_regex(R"(^\d+\s+\d+\s+\d+\s+\d+$)");
    regex instruction_regex(R"(^[crew]\s+\d+$)");

    int processNum = -1;
    int pid = 0;
    bool processNumSet = false;
    bool readingVmas = false;
    Process* currentProcess = nullptr;

    while (getline(input, line)) {
        if (line.empty() || line[0] == '#') {
            continue; // Ignore empty and comment lines
        }

        smatch match;
        if (!processNumSet && regex_match(line, match, num_regex)) {
            processNum = stoi(line);
            processNumSet = true;
            continue;
        }

        if (processNumSet && !readingVmas && regex_match(line, match, num_regex)) {
            int vmaNum = stoi(line);
            currentProcess = new Process(pid++);
            for (int i = 0; i < vmaNum; ++i) {
                getline(input, line);
                if (regex_match(line, match, vma_regex)) {
                    VMA vma;
                    stringstream vmaStream(line);
                    vmaStream >> vma.start_vpage >> vma.end_vpage >> vma.write_protected >> vma.file_mapped;
                    currentProcess->vmaList.push_back(vma);
                }
            }
            processes.push_back(*currentProcess);
            delete currentProcess;
        } else if (regex_match(line, match, instruction_regex)) {
            char instType;
            int instVal;
            stringstream linestream(line);
            linestream >> instType >> instVal;
            instructions.emplace_back(instType, instVal);
        }
    }

    input.close();
}

bool get_next_instruction (char &operation, int &vpage) {
    size_t instSize = instructions.size();
    if (countInst < int(instSize)) {
        operation = instructions[countInst].first;
        vpage = instructions[countInst].second;
        return true;
    } else {
        return false;
    };
}

unordered_map<char, bool> optionStatus;
void parseFlag(string options) {
    const string validOptions = "OPFSxyfa";  // Valid options characters
    for (char c: validOptions) {
        optionStatus[c] = options.find(c) != string::npos;
    }
}
void readRandFile(const string& randFile) {
    ifstream randIn(randFile);
    int count;
    randIn >> count;
    randvals.resize(count);

    for (int i = 0; i < count; ++i) {
        randIn >> randvals[i];
    }

    randIn.close();

}

frame_t* allocate_frame_from_free_list() {
    if (frame_table.size() >= MAX_FRAMES && freeFrames.empty()) { // Illegal state
        return nullptr;
    } else if (frame_table.size() < MAX_FRAMES) { // Create new frame item in traditional frame table
        frame_t insertFrame((int)frame_table.size(), 0, (int)frame_table.size(), false, 0, 0); // Initialize using constructor
        frame_table.push_back(insertFrame);
        return &frame_table.back();
    } else if (!freeFrames.empty()) { // Get from free frame deque
        int frameNo = freeFrames.front();
        freeFrames.pop_front(); // Remove front element
        return &frame_table[frameNo];
    } else {
        return nullptr;
    }
}
int main (int argc, char* argv[]) {

    bool ohOption = false;
    bool pagetableOption = false;
    bool frameTableOption = false;
    bool processStatistics = false;

    string method;
    string options;

    int c;
    while ((c = getopt(argc, argv, "f::a::o::")) != -1) {
        switch (c) {
            case 'f':
                MAX_FRAMES = stoi(optarg);
                break;
            case 'a':
                method = optarg;
                break;
            case 'o':
                options = optarg;
                parseFlag(options);
                if (optionStatus['O']) ohOption = true;
                if (optionStatus['P']) pagetableOption = true;
                if (optionStatus['F']) frameTableOption = true;
                if (optionStatus['S']) processStatistics = true;
                break;
            default:
                cerr << "Unknown error.\n";
                return 1;
        }
    }
    string inputfile;
    string randFile;
    if (optind < argc) {
        inputfile = argv[optind++];
    }
    if (optind < argc) {
        randFile = argv[optind++];
    }

    readRandFile(randFile);

    read_input(inputfile);
    Pager *THE_PAGER;
    switch (method[0]) {  // Assuming method is a string
        case 'a':
            THE_PAGER = new AgingPager();
            break;
        case 'c':
            THE_PAGER = new ClockPager();
            break;
        case 'e':
            THE_PAGER = new NRUPager();
            break;
        case 'f':
            THE_PAGER = new FIFOPager();
            break;
        case 'r':
            THE_PAGER = new RandomPager();
            break;
        case 'w':
            THE_PAGER = new WorkingSetPager();
            break;
        default:
            cerr << "Pager Undefined" << endl;
            exit(1);
    }


    int current_process_id = -1;


    unsigned long long cost = 0;
    unsigned long ctx_switches = 0;
    unsigned long process_exits = 0;

    while (get_next_instruction(operation, vpage)) {
        if (operation == 'r' || operation == 'w') {
            if (current_process_id == -1) {
                cerr << "Error: No process is currently active." << endl;
                exit(1);
            }
            if(ohOption) logContextSwitch(countInst, operation, vpage);

            Process& currentProcess = processes[current_process_id]; // reference for currProc
            pte_t *currentPte = &currentProcess.page_table[vpage];
            ++cost;
            if(currentPte->PRESENT){
                currentPte->REFERENCED = 1;
            }else{
                if(currentPte->VALID){
                    frame_t *victimFrame = allocate_frame_from_free_list();
                    if (victimFrame == nullptr) {
                        victimFrame = THE_PAGER->select_victim_frame();
                    }
                    // Determine if victimFrame was mapped
                    if (victimFrame->mapped) {
                        Process& victimProcess = processes[victimFrame->pid];
                        pte_t* replacedPte = &(victimProcess.page_table[victimFrame->vpage]);

                        // Reset frame_num and unmap
                        frame_table[replacedPte->FRAME_NUMBER].mapped = 0;

                        replacedPte->PRESENT = 0;
                        replacedPte->FRAME_NUMBER = 0;

                        if (ohOption) logUnmap(victimFrame->pid, victimFrame->vpage);
                        victimProcess.pstats.unmaps++;

                        if (replacedPte->MODIFIED) {
                            if (replacedPte->FILE_MAPPED) {
                                victimProcess.pstats.fouts++;
                                if (ohOption) logFout();
                            } else {
                                replacedPte->PAGEDOUT = 1;
                                victimProcess.pstats.outs++;
                                if (ohOption) logOut();
                            }
                        }
                    }

                    if (!currentPte->FILE_MAPPED) {
                        if (currentPte->PAGEDOUT) {
                            if (ohOption) logIn();
                            currentProcess.pstats.ins++;
                        } else {
                            if (ohOption) logZero();
                            currentProcess.pstats.zeros++;
                        }
                    } else {
                        currentProcess.pstats.fins++;
                        if (ohOption) logFin();
                    }
                    if (ohOption) logMap(victimFrame->index);

                    // Update currentPte

                    currentPte->PRESENT = 1;
                    currentPte->FRAME_NUMBER = victimFrame->index;
                    currentPte->MODIFIED = 0;
                    currentPte->REFERENCED = 1;
                    currentProcess.pstats.maps++;

                    //Update victimFrame
                    *victimFrame = frame_t(currentProcess.pid, vpage, victimFrame->index, 1, 0, countInst);
                } else { // Page fault handling
                    if(ohOption) logSegv();
                    currentProcess.pstats.segv++;
                    ++countInst;
                    continue; // move on to the next instruction
                }


            }

            if (operation == 'w') {
                if (currentPte->WRITE_PROTECT) {
                    if(ohOption) logSegprot();
                    currentProcess.pstats.segprot++;

                } else {
                    currentPte->MODIFIED = 1;
                }
            }
        } else if (operation == 'c') {
            current_process_id = vpage; // Update current process ID
            Process& currentProcess = processes[current_process_id]; // Get current process

            if(ohOption) logContextSwitch(countInst, operation, vpage);

            if (currentProcess.page_table.empty()) {
                currentProcess.page_table.resize(MAX_VPAGES, pte_t());
            }

            for (const VMA& vma : currentProcess.vmaList) {
                for (int j = vma.start_vpage; j <= vma.end_vpage; ++j) {
                    currentProcess.page_table[j].VALID = 1;
                    currentProcess.page_table[j].WRITE_PROTECT = vma.write_protected;
                    currentProcess.page_table[j].FILE_MAPPED = vma.file_mapped;
                }
            }
            cost += 130;
            ++ctx_switches;
        } else if (operation == 'e') {
            if(ohOption) logContextSwitch(countInst, operation, vpage);
            if(ohOption) logExit(vpage);

            Process& exitingProcess = processes[vpage];
            int pteNum = 0;
            for (auto& pageTableEntry : exitingProcess.page_table) {
                pageTableEntry.PAGEDOUT = 0;
                if (pageTableEntry.PRESENT) {
                    freeFrames.push_back(pageTableEntry.FRAME_NUMBER);
                    frame_table[pageTableEntry.FRAME_NUMBER].mapped = 0;
                    exitingProcess.pstats.unmaps++;
                        if(ohOption) logUnmap(vpage, pteNum);


                    if (pageTableEntry.FILE_MAPPED && pageTableEntry.MODIFIED) {

                            if(ohOption) logFout();
                        exitingProcess.pstats.fouts++;
                    }

                    pageTableEntry.PRESENT = 0;
                    pageTableEntry.FRAME_NUMBER = 0;
                    pageTableEntry.MODIFIED = 0;
                }
                pteNum++;
            }
            ++process_exits;
            cost += 1230;
        } else {
            cerr << "instruction undefined" << endl;
            exit(1);
        }
        ++countInst;
    }


    if (pagetableOption) {
        for (const auto& proc:processes) {
            cout<<"PT["<<proc.pid<<"]:";
            for (int ptNum = 0; ptNum < proc.page_table.size(); ++ptNum) {
                const auto& pageTable = proc.page_table[ptNum];
                if (pageTable.PRESENT) {
                    cout << " " << ptNum << ":";
                    cout << (pageTable.REFERENCED ? "R" : "-");
                    cout << (pageTable.MODIFIED ? "M" : "-");
                    cout << (pageTable.PAGEDOUT ? "S" : "-");
                } else {
                    cout << (pageTable.PAGEDOUT ? " #" : " *");
                }
            }
            cout << endl;
        }
    }
    if (frameTableOption) {
        cout<<"FT:";
        for (auto & frame : frame_table) { //
            if(frame.mapped){
                cout<<" "<<frame.pid<<":"<<frame.vpage;
            } else cout<<" *";
        }
        int remainingFrames = MAX_FRAMES - static_cast<int>(frame_table.size());
        for (int i = 0; i < remainingFrames; ++i) {
            std::cout << " *";
        }

        cout<<endl;
    }

    if (processStatistics) {
        for(auto proc:processes) {
            proc_stats* pstats = &proc.pstats;
            printf("PROC[%d]: U=%lu M=%lu I=%lu O=%lu FI=%lu FO=%lu Z=%lu SV=%lu SP=%lu\n",
                   proc.pid,
                   pstats->unmaps, pstats->maps, pstats->ins, pstats->outs,
                   pstats->fins, pstats->fouts, pstats->zeros,
                   pstats->segv, pstats->segprot);
            cost += pstats->maps*350 + pstats->unmaps*410 + pstats->ins*3200 + pstats->outs*2750 +
                    pstats->fins*2350 + pstats->fouts*2800 + pstats->zeros*150 + pstats->segv*440 + pstats->segprot*410;
        }
        printf("TOTALCOST %lu %lu %lu %llu %lu\n",
               (unsigned long)instructions.size(), ctx_switches, process_exits, cost, (unsigned long)sizeof(pte_t));
    }

    return 0;
}