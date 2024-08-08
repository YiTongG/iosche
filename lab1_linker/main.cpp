#include "parser.h"
#include <iostream>
using namespace std;



int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <input_file>" << endl;
        return 1;
    }

        Parser parser(argv[1]);
        parser.parse(); // Tokenize and parse the input file


    return 0;
}
