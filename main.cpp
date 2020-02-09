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


int main() {
    std::filesystem::path cwd = std::filesystem::current_path() / "robot";
    std::ofstream file(cwd.string());
    file.close();

    const cv::String& path = "../nodes_input.csv";

    ifstream _file;
    char separator = ',';

    _file.open(path.c_str(), ifstream::in);
    std::string line;

//    vector<int> list_nodeA;
    map<int, char> mapNodeA;

    while (getline(_file, line)) {
        stringstream lineStream(line);

        string index, type;
        getline(lineStream, index, separator);
        getline(lineStream, type);

        int indexInt = StringToInt(index);
        mapNodeA[indexInt] = 'A';
    }

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
