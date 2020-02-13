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

#include <ctime>

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
    vector<int> visitedRobotIds;
};

// map<node.id, node>
map<int, NodeSpec> _mapNode;
void readNodes(const cv::String& path) {
    ifstream file;
    file.open(path.c_str(), ifstream::in);

    string index, type;
    std::string line;

    while (getline(file, line)) {
        stringstream lineStream(line);

        getline(lineStream, index, separator);
        getline(lineStream, type);

        NodeSpec node;
        node.id = StringToInt(index);
        node.type = (type[0] == 'A') ? A : B;
        _mapNode[node.id] = node;
    }
    file.close();
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
    int id;
    RobotType type;
    int speed;
    int measuredTime;
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
        robot.id = StringToInt(index);
        robot.speed = StringToInt(speed);
        robot.type = (type[0] == 'm') ? mover : organizer;
        _robots[robot.id] = robot;
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

int estimateTime(const int& robotId) {
    vector<int> circuit = _setOfPaths[robotId];
    RobotSpec robot = _robots[robotId];

    int time = 0;
    for (int pathIndex = 0; pathIndex < circuit.size(); ++pathIndex) {
        int nodeId = circuit.at(pathIndex);
        NodeSpec node = _mapNode[nodeId];

        // robot starts from the first node.
        int travelTime = (pathIndex == 0) ? 0 : robot.speed;
        time += travelTime;

        int taskTimeInt = taskTime(robot, node);
        time += taskTimeInt;
    }

    return time;
}

int measuredTime(const int& robotId) {
    vector<int> circuit = _setOfPaths[robotId];
    RobotSpec robot = _robots[robotId];
    return robot.measuredTime;
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

string printTime(){
        time_t currentTime;
        struct tm *localTime;

        time( &currentTime );
        localTime = localtime( &currentTime );

        int hour   = localTime->tm_hour;
        int mins    = localTime->tm_min;
        int secs    = localTime->tm_sec;

        stringstream stream;
        stream << hour << ":" << mins << ":" << secs << "  ";
        return stream.str();
};

void logRobotWorking(const int& pathIndex, const NodeSpec &node, int taskTime, int robotId) {
    stringstream stream;
    stream << printTime() << pathIndex + 1 << "(" << node.id << "): R" << robotId << ": arrived & working for " << taskTime << " secs." << "\n";
    string log = stream.str();
    cout << log;
}

void logRobotTraveling(const int& pathIndex, const NodeSpec &node, const RobotSpec& robot, int travelTime) {
    stringstream stream;
    stream << printTime() << pathIndex + 1 << "(" << node.id << "): R" << robot.id << ": traveling(" << travelTime << "), assigned task:" << taskString(robot, node) << "\n";
    string log = stream.str();
    cout << log;
}

void logRuntime(const int& pathIndex, const NodeSpec &node, const RobotSpec& robot) {
    stringstream stream;
    stream << printTime() << pathIndex + 1 << "(" << node.id << "): R" << robot.id << ": completed run time: " << robot.measuredTime << " secs \n";
    string log = stream.str();
    cout << log;
}


struct Billboard {
    int nodeId;
    mutex nodeMutex;
};

map<int, Billboard> _billboards;

int reserveBillboard(const int& pathIndex, NodeSpec& node, int taskTime, int robotId) {
    Billboard &billboard = _billboards[node.id];
    if (billboard.nodeId == 0) {
        billboard.nodeId = node.id;
    }

    logRobotWorking(pathIndex, node, taskTime, robotId);
    const lock_guard<std::mutex> nodeLockGuard(billboard.nodeMutex);
    node.visitedRobotIds.push_back(robotId);
    std::this_thread::sleep_for(std::chrono::milliseconds(taskTime));
    return node.id;
}

const int secondsInHour = 3600;

int startRobot(int robotId) {
    vector<int> circuit = _setOfPaths[robotId];
    RobotSpec& robot = _robots[robotId];

    for (int pathIndex = 0; pathIndex < circuit.size(); ++pathIndex) {
        int nodeId = circuit.at(pathIndex);
        NodeSpec& node = _mapNode[nodeId];

        // robot starts from the first node.
        int travelTime = (pathIndex == 0) ? 0 : robot.speed;
        robot.measuredTime += travelTime;

        int taskTimeInt = taskTime(robot, node);
//        robot.measuredTime += taskTimeInt;

        logRobotTraveling(pathIndex, node, robot, travelTime);
        std::this_thread::sleep_for(std::chrono::milliseconds(travelTime));

        std::chrono::time_point<std::chrono::high_resolution_clock> start;
        start = std::chrono::high_resolution_clock::now();

        reserveBillboard(pathIndex, node,taskTimeInt,robotId);
        std::chrono::duration<double, std::milli> diff = std::chrono::high_resolution_clock::now() - start;
        robot.measuredTime += diff.count();

        logRuntime(pathIndex, node, robot);
    }

    return robot.measuredTime;
}

bool writeTimeFile(const string &fileName, const int& robotId, const int& time) {
    std::fstream file;
    file.open (fileName, std::ios::out | std::ios::app);
    if (file) {
        file << robotId << "," << time << endl;
        file.close();
        return true;
    } else {
        file.close();
        return false;
    }
}

bool writeVisitFile(const string &fileName) {
    std::fstream file;
    file.open (fileName, std::ios::out | std::ios::app);

    if (file) {
        for (int nodeId = 0; nodeId < _mapNode.size(); ++nodeId) {
            NodeSpec node = _mapNode[nodeId];
            file << node.id << ",";

            for (int i = 0; i < node.visitedRobotIds.size(); ++i) {
                file << node.visitedRobotIds[i];

                if (i + 1 != node.visitedRobotIds.size()) {
                    file << ",";
                }
            }
            file << endl;
        }

        file.close();
        return true;
    } else {
        file.close();
        return false;
    }
}

int main() {
    printCWD();
    readNodes("../nodes_input.csv");
    readPaths("../paths_input.csv");
    readRobots("../robots_input.csv");

    configureTaskTimes();

    int totalEstimatedTime = 0;
    const int totalRunningRobots = 4;
    for (int robotId = 0; robotId < totalRunningRobots; ++robotId) {
        int measure = estimateTime(robotId);
        std::cout << "R-" << robotId << ": estimated minimum run time:" << measure << " secs.\n";
        std::cout << printCircuit(robotId) << "\n";
        totalEstimatedTime += measure;
    }

    std::cout << "Starting Path Simulation.\n";
    std::cout << "Estimated Total Run Time: " << totalEstimatedTime  << " secs. \n";

    for (int robotId = 0; robotId < totalRunningRobots; ++robotId) {
        std::packaged_task<int(int)> reserveTask(startRobot);
        std::future<int> reserveFuture = reserveTask.get_future();
        std::thread reserveThread(std::move(reserveTask), robotId);
        reserveThread.detach();
    }

    std::this_thread::sleep_for(std::chrono::seconds(30 * 2));
    writeVisitFile("../visited.csv");

    string measuredtimesFile = "../measuredtimes.csv";
    for (int robotId = 0; robotId < totalRunningRobots; ++robotId) {
        int measure = measuredTime(robotId);
        if (! writeTimeFile(measuredtimesFile, robotId, measure)) {
            std::cerr << "Failed to write to file: " << measuredtimesFile << endl;
        }
        std::cout << "R-" << robotId << ": measured run time:" << measure << " secs.\n";
    }
    return 0;
}
