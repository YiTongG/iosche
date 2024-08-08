//
// Created by yitong on 2024/2/13.
//

#ifndef OSLAB1_WARNING_H
#define OSLAB1_WARNING_H
#ifndef WARNING_H
#define WARNING_H

#include <iostream>

enum WarningCode {
    SYMBOL_DEFINED_BUT_NOT_USED,
    SYMBOL_REDEFINED,
    UNUSED_SYMBOL_IN_USELIST,
    ADERESS_EXCEEDS_MODULE_SIZE,


};

void printwarning(int warnCode, int moduleNum, const string& additionalInfo = "") {
    const char* warnstr[] = {
            "was defined but never used",
            "redefinition ignored",
            "was not used",
            "assume zero relative",
            // Add more warning messages as needed
    };
//Warning: Module 0: X12 was defined but never used
    cout << "Warning: Module " << moduleNum << ": " <<additionalInfo<<" "<< warnstr[warnCode]<< endl;

}

#endif // WARNING_H

#endif //OSLAB1_WARNING_H
