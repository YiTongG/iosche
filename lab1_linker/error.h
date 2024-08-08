//
// Created by yitong on 2024/2/12.
//

#ifndef OSLAB1_ERROR_H
#define OSLAB1_ERROR_H
#include <iostream>
#include <cstdlib> // For exit()
enum errorCode {
    SYMBOL_MULTIPLE_REDEFINED,
    ADDRESS_EXCEEDS_MACHINE_SIZE,
    SYMBOL_NOT_DEFINED,
    RELATIVE_ADDR_EXCEEDS_MODULE_SIZE,
    EXTERNAL_ADDR_TOO_LARGE,
    SYM_EXPECTED,
    MARIE_EXPECTED,
    NUM_EXPECTED,
    TOO_MANY_DEF_IN_MODULE,
    TOO_MANY_USE_IN_MODULE,
    TOO_MANY_INSTR,
    ILLEGAL_MODULE_OPERAND,
    ILLEGAL_IMMEDIATE_OPERAND,
    ILLEGAL_OPCODE,
    SYM_TOO_LONG

    // Add more warning codes as needed
};
// Function to report parsing errors and exit the program
void printerror(int errcode, bool messageOnly = false, const string& additionalInfo = "", int linenum = 0, int lineoffset = 0) {
    const char* errstr[] = {
            "This variable is multiple times defined; first value used",
            "Absolute address exceeds machine size; zero used",              // Absolute address exceeds machine size; zero used
            "is not defined; zero used",             // is not defined; zero used
            "Relative address exceeds module size; relative zero used", //Relative address exceeds module size; relative zero used
            "External operand exceeds length of uselist; treated as relative=0",    // External operand exceeds length of uselist; treated as relative=0
            "SYM_EXPECTED",    // > 16
            "MARIE_EXPECTED",
            "NUM_EXPECTED",// > 16,
            "TOO_MANY_DEF_IN_MODULE",
            "TOO_MANY_USE_IN_MODULE",
            "TOO_MANY_INSTR",
            "Illegal module operand ; treated as module=0",
            "Illegal immediate operand; treated as 999",
            "Illegal opcode; treated as 9999",
            "SYM_TOO_LONG"
    };
    if (messageOnly){
        if(!additionalInfo.empty()){
            if(SYMBOL_NOT_DEFINED == errcode)
                cout << " Error: " << additionalInfo << " " << errstr[errcode] << endl;
            else
                cout << " Error: " << additionalInfo << ": " << errstr[errcode] << endl;
            //cout << "Error: Module " << moduleNum << ": " << errstr[errcode] << endl;

        } else
            cout << " Error: " << errstr[errcode] << endl;

    }
    else{
        cout << "Parse Error line " << linenum << " offset " << lineoffset << ": " << errstr[errcode] << endl;
        exit(1);
    }

}

#endif //OSLAB1_ERROR_H
