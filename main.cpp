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
    int id;
    NodeType type;
};

map<int, NodeSpec> _mapNode;
void readNodes(const cv::String& path) {
    ifstream _file;
    _file.open(path.c_str(), ifstream::in);

    string index, type;
    std::string line;

    while (getline(_file, line)) {
        stringstream lineStream(line);

        getline(lineStream, index, separator);
        getline(lineStream, type);

        NodeSpec node;
        node.id = StringToInt(index);
        node.type = (type[0] == 'A') ? A : B;
        _mapNode[node.id] = node;
    }
}

vector<vector<int>> _setOfPaths;
void readPaths(const cv::String& path) {
    ifstream _file;
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
    ifstream _file;
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

map<string, int> _robotTaskTimes;
void configureTaskTimes() {
    _robotTaskTimes["mover-push"] = 20;
    _robotTaskTimes["mover-pull"] = 35;
    _robotTaskTimes["organizer-pick"] = 30;
    _robotTaskTimes["organizer-place"] = 45;
//    int time = _robotTaskTimes["pull"];
//    std::cout << "time:" << time << std::endl;
}

int taskTime(const RobotSpec& robot, const NodeSpec& node) {
    int taskTime = 0;
    if (robot.type == mover) {
        if (node.type == A) {
            taskTime = _robotTaskTimes["mover-push"];
        } else {

            // node.type == B
            taskTime = _robotTaskTimes["mover-pull"];
        }
    } else {
        // robot.type == organizer

        if (node.type == A) {
            taskTime =  _robotTaskTimes["organizer-pick"];
        } else {
            // node.type == B
            taskTime =  _robotTaskTimes["organizer-place"];
        }
    }
    return taskTime;
}

int measureTime(const int& robotId) {
    vector<int> circuit = _setOfPaths[robotId];
    RobotSpec robot = _robots[robotId];

    int time = 0;
    for (int nodeId = 0; nodeId < circuit.size(); ++nodeId) {
        NodeSpec node = _mapNode[nodeId];

        int travelTime = (nodeId == 0) ? 0 : robot.speed;
        time += travelTime;

        int taskTimeInt = taskTime(robot, node);
        time += taskTimeInt;
    }

    return time;
}

int main() {
    printCWD();
    readNodes("../nodes_input.csv");
    readPaths("../paths_input.csv");
    readRobots("../robots_input.csv");

    configureTaskTimes();

    int robotId = 0;
    int timeSigma = measureTime(robotId);

    robotId = 1;
    timeSigma += measureTime(robotId);

    robotId = 2;
    timeSigma += measureTime(robotId);

    robotId = 3;
    timeSigma += measureTime(robotId);

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
