//
// Created by yitong on 2024/2/12.
//

// Include the Parser header file instead of declaring the class again
//#include "parser.h"
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include "token.h" // Ensure this includes the Tokenizer class and TokenInfo struct
#include "error.h"
#include "warning.h"

struct SymbolInfo {
    int address;
    int moduleIndex;
    bool used;

};
const int MACHINE_SIZE = 512;

class Parser {
public:
    explicit Parser(const std::string &filename) : tokenizer(filename) {}

    void parse() {


        pass1();

        pass2();
    }

private:


    Tokenizer tokenizer;
    map<string, SymbolInfo> symbolTable;
    TokenInfo token;
    vector<int> moduleBaseAddresses;
    vector<int> moduleSizes; // A new vector to store the size of each module
    map<string, int> multipleDefinitions;


    int readInt() {

        token = tokenizer.getNextToken();
        for (char c: token.token) {
            if (!isdigit(c)) {
                printerror(NUM_EXPECTED, false, "", token.line, token.offset);
            }
        }
        return stoi(token.token);
    }

    string readSymbol() {
        try {
            token = tokenizer.getNextToken();
            if (!isalpha(token.token[0]) ) {

                printerror(SYM_EXPECTED, false, "", token.line, token.offset);
            } else if (token.token.size() > 16) {
                printerror(SYM_TOO_LONG, false, "", token.line, token.offset);
            }
        } catch (const FileParseException &e) {
            printerror(SYM_EXPECTED, false, "", e.lineNum, e.offset+1);
        }
        return token.token;
    }

    char readMARIE() {
        try{
            token = tokenizer.getNextToken();

            if (token.token.size() != 1 || !isalpha(token.token[0]) || string("MAREI").find(token.token[0]) == string::npos) {
                cout<<token.offset+token.token.size()<<endl;

                printerror(MARIE_EXPECTED, false, "", token.line, token.offset+token.token.size());
                return token.token[0];

            }
        }catch (FileParseException &e){
            printerror(MARIE_EXPECTED, false, "", token.line, token.offset+token.token.size());
        }

        return token.token[0];
    }

    void createSymbol(const string &sym, int address, int moduleIndex) {
        if (symbolTable.find(sym) != symbolTable.end()) {
            // Symbol already defined, report as error but only keep the first definition
            printwarning(SYMBOL_REDEFINED, moduleIndex, sym);
            if (multipleDefinitions.find(sym) == multipleDefinitions.end()) {
                multipleDefinitions[sym] = 2; // Already defined once, now found again
            } else {
                multipleDefinitions[sym] += 1; // Increment the count
            }
        } else {
            symbolTable[sym] = SymbolInfo{address, moduleIndex, false};
        }
    }

    void markSymbolAsUsed(const string &sym) {
        if (symbolTable.find(sym) != symbolTable.end()) {
            symbolTable[sym].used = true;
        }
    }

    void pass1() {
        int moduleBaseAddress = 0;
        int moduleIndex = 0;
        int totalInstructions = 0;

        moduleBaseAddresses.push_back(moduleBaseAddress); // First module starts at 0

        try {
            while (true) {
                int defCount = readInt();
                if (defCount > 16) {
                    // TOO_MANY_DEF_IN_MODULE error
                    printerror(TOO_MANY_DEF_IN_MODULE, false, "",
                               token.line,
                               token.offset);
                }
                for (int i = 0; i < defCount; ++i) {
                    string sym = readSymbol();
                    int relativeAddr = readInt();
                    createSymbol(sym, moduleBaseAddress + relativeAddr, moduleIndex);
                }

                int useCount = readInt(); // Read useCount but not used in pass1
                if (useCount > 16) {
                    // TOO_MANY_USE_IN_MODULE error
                    printerror(TOO_MANY_USE_IN_MODULE, false, "",
                               token.line ,
                               token.offset );
                    // Handle the error as required
                }
                for (int i = 0; i < useCount; ++i) {
                    readSymbol(); // Consume symbols
                }

                int instCount = readInt();
                totalInstructions += instCount;
                if (totalInstructions > MACHINE_SIZE) {
                    // TOO_MANY_INSTR error
                    printerror(TOO_MANY_INSTR, false, "",
                               token.line
                            ,token.offset);
                }


                for (int i = 0; i < instCount; ++i) {
                    readMARIE(); // Consume address mode
                    readInt(); // Consume operand
                }
                moduleSizes.push_back(instCount); // Store the size for later use
                for (pair<const basic_string<char>, SymbolInfo> &def: symbolTable) {
                    if (def.second.moduleIndex == moduleIndex) { // Check symbols defined in this module
                        if (def.second.address >= moduleBaseAddress + instCount) { // Address exceeds module size
                            stringstream ss;
                            ss << def.first << "=" << def.second.address - moduleBaseAddress << " valid=[0.."
                               << instCount - 1 << "]";
                            string validRange = ss.str();
                            printwarning(ADERESS_EXCEEDS_MODULE_SIZE, moduleIndex, validRange);
                            def.second.address = moduleBaseAddress; // Adjust address to zero relative
                        }
                    }
                }
                moduleBaseAddress += instCount; // Prepare base address for next module
                moduleBaseAddresses.push_back(moduleBaseAddress); // Store base for next module
                moduleIndex++;
            }
        } catch (FileParseException &e) {

        }

        // Print symbol table
        cout << "Symbol Table" << endl;

        for (const auto &entry: symbolTable) {

            cout << entry.first << "=" << entry.second.address;
            //  here  could append error messages for multiple definitions
            auto multipleDefIt = multipleDefinitions.find(entry.first);
            if (multipleDefIt != multipleDefinitions.end()) {
                printerror(SYMBOL_MULTIPLE_REDEFINED, true);
            }
            cout << endl;
        }
        cout << endl;
    }

    void pass2() {
        int moduleIndex = 0; // Track the current module index
        int instructionCounter = 0; // For printing instruction addresses

        tokenizer.reset(); // Reset the tokenizer to the beginning of the file
        int exceedsMachineSize = 0;
        int undifinedSymbol = 0;
        int exceedsModuleSize = 0;
        int exceedUselistSize = 0;
        int illegalOpcode = 0;
        int illegalImmediateOperand = 0;
        int illegalModuleOperand = 0;
        cout << "Memory Map" << endl;

        try {
            while (true) {

                int defCount = readInt(); // Skip definition list
                for (int i = 0; i < defCount; ++i) {
                    readSymbol(); // Consume symbol definitions
                    readInt(); // Consume symbol addresses
                }

                int useCount = readInt();
                vector<string> useListSymbols; // For 'E' type resolution

                for (int i = 0; i < useCount; ++i) {
                    useListSymbols.push_back(readSymbol());
                }
                vector<bool> usedSymbols(useListSymbols.size(), false); // Track usage of useList symbols

                int instCount = readInt();
                for (int i = 0; i < instCount; ++i) {
                    char addressMode = readMARIE();
                    int instruction = readInt(); // Original instruction
                    int opcode = instruction / 1000;
                    int operand = instruction % 1000;
                    int resolvedOperand = instruction;

                    if (opcode >= 10) {
                        illegalOpcode = 1;
                        resolvedOperand = 9999; // Convert the entire instruction to 9999
                    }

                    switch (addressMode) {
                        case 'M':
                            if (operand < 0 || operand >= moduleBaseAddresses.size() - 1) {
                                //printwarning(INVALID_MODULE_OPERAND, moduleIndex);
                                illegalModuleOperand = 1;
                                resolvedOperand = opcode * 1000; // Error indication
                            } else {
                                resolvedOperand = opcode * 1000 + moduleBaseAddresses[operand];
                            }
                            break;
                        case 'R':
                            if (operand >=
                                moduleSizes[moduleIndex]) { // You need to check against the actual module size here
                                exceedsModuleSize = 1;
                                operand = 0; // Use module relative value zero
                            }
                            resolvedOperand = opcode * 1000 + (moduleBaseAddresses[moduleIndex] + operand);

                            if (opcode >= 10) {
                                illegalOpcode = 1;
                                resolvedOperand = 9999; // Convert the entire instruction to 9999
                            }
                            break;
                        case 'I':
                            if (operand >= 900) {
                                //printerror(ILLEGAL_IMMEDIATE_OPERAND, true);
                                illegalImmediateOperand = 1;
                                resolvedOperand = opcode * 1000 + 999; // Convert operand to 999
                            }
                            break;
                        case 'E':
                            if (operand >= useListSymbols.size()) {
                                exceedUselistSize = 1;
                                resolvedOperand = opcode * 1000; // Error case: treat operand as relative=0
                            } else {
                                string symbol = useListSymbols[operand];
                                markSymbolAsUsed(symbol); // Mark symbol as used (for warning message later
                                usedSymbols[operand] = true;
                                if (symbolTable.find(symbol) == symbolTable.end()) {
                                    undifinedSymbol = 1;
                                    //printwarning(EXTERNAL_ADDR_TOO_LARGE, moduleIndex, symbol);
                                    resolvedOperand = opcode * 1000; // Use zero if symbol is not defined
                                } else {
                                    resolvedOperand = opcode * 1000 + symbolTable[symbol].address;
                                }
                            }
                            break;
                            // No specific case for 'A' as it's directly handled by initialization
                        case 'A':
                            if (operand >= MACHINE_SIZE) {
                                exceedsMachineSize = 1;
                                resolvedOperand = opcode * 1000; // Error indication
                            }

                            break;
                    }
                    cout << setfill('0') << setw(3) << instructionCounter++ << ": " << setfill('0') << setw(4) <<resolvedOperand;

                    if (exceedsMachineSize) {
                        printerror(ADDRESS_EXCEEDS_MACHINE_SIZE, true);
                        exceedsMachineSize = 0;
                    } else if (undifinedSymbol) {
                        printerror(SYMBOL_NOT_DEFINED, true, useListSymbols[operand]);
                        undifinedSymbol = 0;
                    } else if (exceedsModuleSize) {
                        printerror(RELATIVE_ADDR_EXCEEDS_MODULE_SIZE, true);
                        exceedsModuleSize = 0;
                    } else if (exceedUselistSize) {
                        printerror(EXTERNAL_ADDR_TOO_LARGE, true);
                        exceedUselistSize = 0;

                    } else if (illegalOpcode) {
                        printerror(ILLEGAL_OPCODE, true);
                        illegalOpcode = 0;
                    } else if (illegalImmediateOperand) {
                        printerror(ILLEGAL_IMMEDIATE_OPERAND, true);
                        illegalImmediateOperand = 0;
                    } else if (illegalModuleOperand) {
                        printerror(ILLEGAL_MODULE_OPERAND, true);
                        illegalModuleOperand = 0;
                    } else {
                        cout << endl;
                    }

                }
                for (int i = 0; i < useListSymbols.size(); ++i) {
                    if (!usedSymbols[i]) {
                        stringstream ss;
                        ss << "uselist[" << i << "]=" << useListSymbols[i];
                        printwarning(UNUSED_SYMBOL_IN_USELIST, moduleIndex, ss.str());
                    }
                }
                moduleIndex++; // Next module
            }
        } catch (const FileParseException &e) {

        }
        cout << endl;
        for (const auto &entry: symbolTable) {
            if (!entry.second.used)
                printwarning(SYMBOL_DEFINED_BUT_NOT_USED, entry.second.moduleIndex, entry.first);
        }

    }
};
