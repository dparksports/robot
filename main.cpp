#include <fstream>
#include <iostream>
#include <filesystem>

#include <string>
#include <cstdio>

#include <map>
#include <vector>

#include <opencv2/core/core.hpp>
#include <sstream>
#include "boost/filesystem.hpp"   // includes all needed Boost.Filesystem declarations

//using boost::filesystem;

using namespace std;
using namespace cv;

void printCWD() {
    std::filesystem::path cwd = std::filesystem::current_path() / "robot";
    std::ofstream file(cwd.string());
    file.close();
}

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

enum NodeType { A, B};
struct NodeSpec {
    int number;
    NodeType type;
} NodeSpec;

void readNodes(const cv::String& path) {
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


const int numbRobots = 4;
vector<vector<int>> _setOfPaths(numbRobots, vector<int>(0,0));

void readPaths(const cv::String& path) {
    _file.open(path.c_str(), ifstream::in);

    string index, node;
    std::string line;

    while (getline(_file, line)) {
        stringstream lineStream(line);

        getline(lineStream, index, separator);
        getline(lineStream, node);

        int indexInt = StringToInt(index);
        int nodeInt = StringToInt(node);
        _setOfPaths[indexInt].push_back(nodeInt);
    }
}

enum RobotType { mover, organizer};
struct RobotSpec {
    int number;
    RobotType type;
    int speed;
};

map<int, RobotSpec> _robots;
void readRobots(const cv::String& path) {
    _file.open(path.c_str(), ifstream::in);

    string index, type, speed;
    std::string line;

    while (getline(_file, line)) {
        stringstream lineStream(line);

        getline(lineStream, index, separator);
        getline(lineStream, type, separator);
        getline(lineStream, speed);

        RobotSpec robot;
        robot.number = StringToInt(index);
        robot.speed = StringToInt(speed);
        robot.type = (type[0] == 'm') ? mover : organizer;
        _robots[robot.number] = robot;
    }
}

int main() {
    printCWD();
    readRobots("../robots_input.csv");
//    readPaths("../paths_input.csv");
//    readNodes("../nodes_input.csv");

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
