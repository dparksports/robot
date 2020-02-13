#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

#include <map>
#include <vector>

#include <opencv2/core/core.hpp>

#include <thread>
#include <future>
#include <mutex>
#include <chrono>

using namespace std;
using namespace cv;

char separator = ',';

void printCWD() {
    std::filesystem::path cwd = std::filesystem::current_path() / "robot";
    std::ofstream file(cwd.string());
    file.close();
}

int StringToInt ( const std::string &Text ) {
    std::istringstream ss(Text);
    int result;
    return ss >> result ? result : 0;
}

enum NodeType {A, B};
struct NodeSpec {
    int id;
    NodeType type;
};

// map<node.id, node>
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
    _file.close();
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
            // works with any number of nodes per circuit, not just 1000.
            currentRobotId = robotIdInt;
            vector<int> newCircuit;
            _setOfPaths.push_back(newCircuit);
        }
        _setOfPaths[robotIdInt].push_back(nodeIdInt);
    }
    _file.close();
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
    _file.close();
}

map<string, int> _robotTaskTimes;
void configureTaskTimes() {
    _robotTaskTimes["mover-push"] = 20;
    _robotTaskTimes["mover-pull"] = 35;
    _robotTaskTimes["organizer-pick"] = 30;
    _robotTaskTimes["organizer-place"] = 45;
}

string taskString(const RobotSpec& robot, const NodeSpec& node) {
    int taskTime = 0;
    if (robot.type == mover) {
        if (node.type == A) {
            return "type(A): pushing (mover)";
        } else {

            // node.type == B
            return "type(B): pulling (mover)";
        }
    } else {
        // robot.type == organizer

        if (node.type == A) {
            return "type(A): picking (organizer)";
        } else {
            // node.type == B
            return "type(B): placing (organizer)";
        }
    }
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
    for (int pathIndex = 0; pathIndex < circuit.size(); ++pathIndex) {
        int nodeId = circuit.at(pathIndex);
        NodeSpec node = _mapNode[nodeId];

        // robot needs to travel to the first node.
        int travelTime = robot.speed;
        time += travelTime;

        int taskTimeInt = taskTime(robot, node);
        time += taskTimeInt;
    }

    return time;
}

string printCircuit(const int& robotId) {
    vector<int> circuit = _setOfPaths[robotId];
    RobotSpec robot = _robots[robotId];

    stringstream stream;
    for (int pathIndex = 0; pathIndex < circuit.size(); ++pathIndex) {
        int nodeId = circuit.at(pathIndex);
        NodeSpec node = _mapNode[nodeId];

        if (node.type == A) {
            stream << node.id;
        } else {
            stream << node.id;
        }

        if (pathIndex + 1 != circuit.size()) {
            stream << "->";
//            stream << "\n";
        }
    }

    return stream.str();
}


struct Billboard {
    int nodeId;
    mutex nodeMutex;
};

map<int, Billboard> _billboards;
mutex functionMutex;
int reserveBillboard(int nodeId, int taskTime, int robotId) {
    {
        stringstream stream;
        stream << nodeId << ": R" << robotId << ": arrived      <" << this_thread::get_id() << ">\n";
        string log = stream.str();
        cout << log;
    }

//    const lock_guard<std::mutex> functionGuard(functionMutex);
    Billboard &billboard = _billboards[nodeId];
    if (billboard.nodeId == 0) {
        billboard.nodeId = nodeId;
    }

    {
        stringstream stream;
        stream << nodeId << ": R" << robotId << ": working for " << taskTime << " secs." << "\n";
        string log = stream.str();
        cout << log;
    }
    const lock_guard<std::mutex> nodeLockGuard(billboard.nodeMutex);
    std::this_thread::sleep_for(std::chrono::seconds(taskTime));
    return nodeId;
}

int startRobot(int robotId) {
    vector<int> circuit = _setOfPaths[robotId];
    RobotSpec robot = _robots[robotId];

    int time = 0;
    for (int pathIndex = 0; pathIndex < circuit.size(); ++pathIndex) {
        int nodeId = circuit.at(pathIndex);
        NodeSpec node = _mapNode[nodeId];

        // robot needs to travel to the first node.
        int travelTime = robot.speed;
        time += travelTime;

        int taskTimeInt = taskTime(robot, node);
        time += taskTimeInt;

        {
            stringstream stream;
            stream << node.id << ": R" << robotId << ": traveling, assigned task:" << taskString(robot, node) << "\n";
            string log = stream.str();
            cout << log;
        }
        std::this_thread::sleep_for(std::chrono::seconds(travelTime));
        reserveBillboard(node.id,taskTimeInt,robotId);
//        cout << "R-" << robotId << ":  time:" << time << "\n";
    }

    return time;
}

int main() {
    printCWD();
    readNodes("../nodes_input.csv");
    readPaths("../paths_input.csv");
    readRobots("../robots_input.csv");

    configureTaskTimes();

    const int secondsInHour = 3600;
    const int totalRunningRobots = 2;
    int totalTime = 0;
    for (int robotId = 0; robotId < totalRunningRobots; ++robotId) {
        int measure = measureTime(robotId);
        std::cout << "R-" << robotId << ": minimum run time:" << measure / secondsInHour << " hours.\n";
        std::cout << printCircuit(robotId) << "\n";
        totalTime += measure;
    }

    std::cout << "Starting Path Simulation.\n";
    std::cout << "Estimated Total Run Time: " << totalTime / secondsInHour << " hours. \n";

    for (int robotId = 0; robotId < totalRunningRobots; ++robotId) {
        std::packaged_task<int(int)> reserveTask(startRobot);
        std::future<int> reserveFuture = reserveTask.get_future();
        std::thread reserveThread(std::move(reserveTask), robotId);
        reserveThread.detach();
    }

    std::this_thread::sleep_for(std::chrono::seconds(1200));
    std::cout << "main end " << '\n';
    return 0;
}
