#include <vector>
#include <map>

#include <numeric>
#include <iostream>
#include <sstream>
#include <string>

#include <thread>
#include <future>
#include <mutex>
#include <chrono>

using namespace std;

void detach() {
    promise<int> taskPromise;
    std::future<int> taskFuture = taskPromise.get_future();
    std::thread robotThread = std::thread([&taskPromise]{
        cout << "Thread closure:" << '\n';
        taskPromise.set_value_at_thread_exit(8);
    });
    robotThread.detach();

    cout << "Wait For Task Completion" << '\n';
    taskFuture.wait();
    cout << taskFuture.get() << '\n';

//    robotThread.join();
    cout << "ending" << '\n';
}

void workTask1() {
    std::cout << "started a robot task." << std::endl;
    this_thread::sleep_for(chrono::seconds(3));
    std::cout << "ended a robot task." << std::endl;
}

void detachJoin() {
    thread threadRobot(workTask1);
    threadRobot.detach();
//    threadRobot.join();

    this_thread::sleep_for(chrono::seconds(2));
    std::cout << "exiting main.." << std::endl;
}


void accumulate(std::vector<int>::iterator first,
                std::vector<int>::iterator last,
                std::promise<int> accumulate_promise)
{
    int sum = std::accumulate(first, last, 0);
    accumulate_promise.set_value(sum);  // Notify future
}

void do_work(std::promise<void> barrier)
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    barrier.set_value();
}

void signalThreadsUsingPromise() {
//    // Demonstrate using promise<int> to transmit a result between threads.
//    std::vector<int> numbers = { 1, 2, 3, 4, 5, 6 };
//    std::promise<int> accumulate_promise;
//    std::future<int> accumulate_future = accumulate_promise.get_future();
//    std::thread work_thread(accumulate, numbers.begin(), numbers.end(),
//                            std::move(accumulate_promise));
//
//    // future::get() will wait until the future has a valid result and retrieves it.
//    // Calling wait() before get() is not needed
//    //accumulate_future.wait();  // wait for result
//    std::cout << "result=" << accumulate_future.get() << '\n';
//    work_thread.join();  // wait for thread completion

    // Demonstrate using promise<void> to signal state between threads.
    std::promise<void> barrier;
    std::future<void> barrier_future = barrier.get_future();
    std::thread new_work_thread(do_work, std::move(barrier));
    barrier_future.wait();
    new_work_thread.join();
}

void workTask2(std::promise<void> taskPromise) {
    std::cout << "started a robot task." << std::endl;
    this_thread::sleep_for(chrono::seconds(3));
    taskPromise.set_value();
    std::cout << "ended a robot task." << std::endl;
}

void setValueByPromise() {
    promise<void> taskPromise;
    future<void> taskFuture = taskPromise.get_future();
    thread robotThread(workTask2, move(taskPromise));
    taskFuture.wait();

    cout << "starting join" << endl;
    robotThread.join();
    cout << "ending join" << endl;

}

int g_i = 0;
std::mutex g_i_mutex;  // protects g_i

void safe_increment()
{
    const std::lock_guard<std::mutex> lock(g_i_mutex);
    ++g_i;

    std::cout << std::this_thread::get_id() << ": " << g_i << '\n';

    // g_i_mutex is automatically released when lock
    // goes out of scope
}

void lockGuardMutex()
{
    std::cout << "main: " << g_i << '\n';

    std::thread t1(safe_increment);
    std::thread t2(safe_increment);

    t1.join();
    t2.join();

    std::cout << "main: " << g_i << '\n';
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
        stream << this_thread::get_id() << " nodeId:" << nodeId << " robotId:" << robotId << '\n';
        string log = stream.str();
        cout << log;
    }

    const lock_guard<std::mutex> functionGuard(functionMutex);
    Billboard& billboard = _billboards[nodeId];
    if (billboard.nodeId == 0) {
        billboard.nodeId = nodeId;
    }

    const lock_guard<std::mutex> nodeLockGuard(billboard.nodeMutex);
    std::this_thread::sleep_for(std::chrono::seconds(taskTime));
//    cout  << "End:" << this_thread::get_id() << '\n';
    cout  << this_thread::get_id() << " billboard.nodeId:" << billboard.nodeId << " nodeId:" << nodeId << " robotId:" << robotId << " Succeded" << '\n';

    return nodeId;
}

int main() {
    int taskTime = 3;
    int nodeId = 123;
    int robotId = 0;

    for (int i = 0; i < 5; ++i) {
        std::packaged_task<int(int, int, int)> reserveTask(reserveBillboard);
        std::future<int> reserveFuture = reserveTask.get_future();
        std::thread reserveThread(std::move(reserveTask), nodeId, taskTime, robotId);
        reserveThread.detach();
        robotId++;
    }

//    taskTime = 20;
//    nodeId = 888;
//    robotId = 777;
//    std::packaged_task<int(int, int, int)> reserveTask(reserveBillboard);
//    std::future<int> reserveFuture = reserveTask.get_future();
//    std::thread reserveThread(std::move(reserveTask), nodeId, taskTime, robotId);
//    reserveThread.join();

    std::this_thread::sleep_for(std::chrono::seconds(30));
    std::cout << "main end " << '\n';
}
