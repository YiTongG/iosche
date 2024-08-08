
#ifndef TOKEN_H
#define TOKEN_H

#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include <string>
using namespace std;

class FileParseException : public std::runtime_error {
public:
    int errorCode;
    int lineNum;
    int offset;

    FileParseException(const std::string& msg, int errorCode, int lineNum, int offset)
            : std::runtime_error(msg), errorCode(errorCode), lineNum(lineNum),offset(offset) {}
};
// Assuming TokenInfo struct is here or included from another header
struct TokenInfo {
    string token;
    int line;
    int offset;
};

class Tokenizer {
public:
    explicit Tokenizer(const std::string &filename) : filename(filename), currentLineNum(0), lineOffset(0),
                                                      lineFinished(true) {
        openFile();
    }

    TokenInfo getNextToken() {
        int totalLines = countLinesInFile(filename);
        while (true) {
            if (lineFinished) {
                if (!getline(file, currentLine)) {
                    if(currentLineNum > 1){
                        currentLineNum = totalLines;
                        lineOffset = 0;
                    }
                    throw FileParseException("End of file", 1001, currentLineNum,lineOffset);// End of file
                }
                currentLineNum++;
                lineOffset = 0;
                lineFinished = false;
            }
            lineOffset = currentLine.find_first_not_of(" \t", lineOffset);
            if (lineOffset != string::npos) {
                // Find the end of the token
                size_t end = currentLine.find_first_of(" \t", lineOffset);
                size_t tokenLength = (end == string::npos) ? currentLine.length() - lineOffset : end - lineOffset;
                string token = currentLine.substr(lineOffset, tokenLength);

                TokenInfo tokenInfo{token, currentLineNum, static_cast<int>(lineOffset + 1)};

                // Prepare for the next token
                lineOffset = (end == string::npos) ? currentLine.length() : end;
                if (lineOffset >= currentLine.length()|| currentLine[lineOffset] == '\n') {
                    lineFinished = true; // Mark end of line
                }

                return tokenInfo;
            } else {
                lineFinished = true; // Prepare for potentially reading the next line.

            }


        }
    }


    // Method to open the file, used in constructor and reset
    void openFile() {
        file.open(filename);
        if (!file.is_open()) {
            std::cerr << "Could not open file: " << filename << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    void reset() {
        if (file.is_open()) {
            file.close(); // Close the current file stream
        }
        currentLineNum = 0;
        lineOffset = 0;
        lineFinished = true;
        openFile(); // Re-open the file to start from the beginning
    };
    // countLinesInFile: Counts the number of lines in a file
    static int countLinesInFile(const std::string& filePath) {
        std::ifstream file(filePath);
        int lineCount = 0;
        std::string line;

        // check if the file is open
        if (!file.is_open()) {
            throw std::runtime_error("" + filePath);
        }

        // read every line of the file
        while (std::getline(file, line)) {
            lineCount++;
        }

        file.close(); //
        return lineCount;
    }

private:
    ifstream file;
    string currentLine;
    string filename; // Store filename for resetting
    int currentLineNum = 0;
    size_t lineOffset = 0;
    bool lineFinished = true; // Indicates if we're at the end of the current line

};

#endif // TOKEN_H
