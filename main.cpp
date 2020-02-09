#include <fstream>
#include <iostream>
#include <filesystem>

#include <string>
#include <cstdio>

#include <map>
#include <vector>

#include <opencv2/core/core.hpp>
#include "boost/filesystem.hpp"   // includes all needed Boost.Filesystem declarations

//using boost::filesystem;          // for ease of tutorial presentation;

using namespace std;
using namespace cv;

// Converts a given string to an integer
int StringToInt ( const std::string &Text )
{
    std::istringstream ss(Text);
    int result;
    return ss >> result ? result : 0;
}

char separator = ',';
ifstream _file;
map<int, char> _mapNodeTypeA;

void readNodesInput(const cv::String& path) {
    _file.open(path.c_str(), ifstream::in);

    string index, type;
    std::string line;

    while (getline(_file, line)) {
        stringstream lineStream(line);

        getline(lineStream, index, separator);
        getline(lineStream, type);

        int indexInt = StringToInt(index);
        if (type[0] == 'A')
            _mapNodeTypeA[indexInt] = 'A';
    }
}

bool isNodeTypeA(int index) {
    if (_mapNodeTypeA[index]) {
        return true;
    } else {
        return false;
    }
}

void printCWD() {
    std::filesystem::path cwd = std::filesystem::current_path() / "robot";
    std::ofstream file(cwd.string());
    file.close();
}

int main() {
    printCWD();
    readNodesInput("../nodes_input.csv");

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
