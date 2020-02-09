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

char separator = ',';
ifstream _file;

void printCWD() {
    std::filesystem::path cwd = std::filesystem::current_path() / "robot";
    std::ofstream file(cwd.string());
    file.close();
}

int StringToInt ( const std::string &Text )
{
    std::istringstream ss(Text);
    int result;
    return ss >> result ? result : 0;
}

enum NodeType { A, B};
struct NodeSpec {
    int number;
    NodeType type;
};

map<int, NodeSpec> _mapNode;
void readNodes(const cv::String& path) {
    _file.open(path.c_str(), ifstream::in);

    string index, type;
    std::string line;

    while (getline(_file, line)) {
        stringstream lineStream(line);

        getline(lineStream, index, separator);
        getline(lineStream, type);

        NodeSpec node;
        node.number = StringToInt(index);
        node.type = (type[0] == 'A') ? A : B;
        _mapNode[node.number] = node;
    }
}

vector<vector<int>> _setOfPaths;
void readPaths(const cv::String& path) {
    _file.open(path.c_str(), ifstream::in);

    string robotId, nodeId;
    std::string line;

    int currentRobotId = -1;

    while (getline(_file, line)) {
        stringstream lineStream(line);

        getline(lineStream, robotId, separator);
        getline(lineStream, nodeId);

        int nodeIdInt = StringToInt(nodeId);
        int robotIdInt = StringToInt(robotId);

        if (currentRobotId != robotIdInt) {
            // supports a non predefined number of nodes per circuit.
            currentRobotId = robotIdInt;
            vector<int> newCircuit;
            _setOfPaths.push_back(newCircuit);
        }
        _setOfPaths[robotIdInt].push_back(nodeIdInt);
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

int measureTime(const int& robotId) {
    vector<int> circuit = _setOfPaths[robotId];
    RobotSpec robot = _robots[robotId];

    int time = 0;
    for (int i = 0; i < circuit.size(); ++i) {
        time += robot.speed;
    }

    return time;
}

int main() {
    printCWD();
    readPaths("../paths_input.csv");
    readRobots("../robots_input.csv");
    readNodes("../nodes_input.csv");

    int robotId = 0;
    int timeSigma = measureTime(robotId);

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
