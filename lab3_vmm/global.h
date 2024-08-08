//
// Created by yitong on 19/07/2024.
//

#ifndef VMM_GLOBAL_H
#define VMM_GLOBAL_H
#include<vector>
struct pte_t {
    unsigned int PRESENT       : 1;
    unsigned int REFERENCED    : 1;
    unsigned int MODIFIED      : 1;
    unsigned int WRITE_PROTECT : 1;
    unsigned int PAGEDOUT      : 1;
    unsigned int FRAME_NUMBER  : 7; // assuming max 128 frames (7 bits)
    unsigned int VALID         : 1;
    unsigned int FILE_MAPPED   : 1;
    unsigned int UNUSED        : 18; // remaining bits for future use

    // Constructor
    pte_t() : PRESENT(0), REFERENCED(0), MODIFIED(0), WRITE_PROTECT(0), PAGEDOUT(0), FRAME_NUMBER(0), VALID(0), FILE_MAPPED(0), UNUSED(0) {}
};

struct frame_t {
    int pid;
    int vpage;
    int index;
    int mapped;
    unsigned int age;
    unsigned int lastAccess;

    // Constructor
    explicit frame_t(int p = 0, int vp = 0, int idx = 0, int m = 0, unsigned int a = 0, unsigned int la = 0)
            : pid(p), vpage(vp), index(idx), mapped(m), age(a), lastAccess(la) {}
};

struct proc_stats {
    // The same order with print
    unsigned long unmaps;
    unsigned long maps;
    unsigned long ins;
    unsigned long outs;
    unsigned long fins;
    unsigned long fouts;
    unsigned long zeros;
    unsigned long segv;
    unsigned long segprot;
} ;

struct VMA {
    unsigned int start_vpage;
    unsigned int end_vpage;
    unsigned int write_protected;
    unsigned int file_mapped;
};

const int MAX_VPAGES = 64;
std::vector<frame_t> frame_table; //gloBAL frameTABLE
std::deque<int> freeFrames;

struct Process {
public:
    Process(int pid);
    int pid;
    proc_stats pstats;
    std::vector<VMA> vmaList;
    std::vector<pte_t> page_table; // represents the translations from virtual pages to physical frames for that process
};
std::vector<Process> processes;

Process::Process(int pid) : pid(pid), pstats{0, 0, 0, 0, 0, 0, 0, 0, 0}, page_table(MAX_VPAGES) {}

unsigned int countInst = 0;
const int TAU = 49;
int MAX_FRAMES = 128;
std::vector<int> randvals;

using instruction = std::pair<char, int>;
std::vector<instruction> instructions;
char operation;
int vpage;

#endif //VMM_GLOBAL_H
