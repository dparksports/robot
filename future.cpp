#include <iostream>
#include <future>
#include <thread>

using namespace std;

void future()
{
    // future from a packaged_task
    std::packaged_task<int()> task([]{ return 7; }); // wrap the function
    std::future<int> f1 = task.get_future();  // get a future
    std::thread t(std::move(task)); // launch on a thread

    // future from an async()
    std::future<int> f2 = std::async(std::launch::async, []{ return 8; });

    // future from a promise
    std::promise<int> p;
    std::future<int> f3 = p.get_future();
    std::thread( [&p]{ p.set_value_at_thread_exit(9); }).detach();

    std::cout << "Waiting..." << std::flush;
    f1.wait();
    f2.wait();
    f3.wait();
    std::cout << "Done!\nResults are: "
              << f1.get() << ' ' << f2.get() << ' ' << f3.get() << '\n';
    t.join();
}

int detach() {
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

void foo()
{
    // simulate expensive operation
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void bar()
{
    // simulate expensive operation
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

int main()
{
    std::cout << "starting first helper...\n";
    std::thread helper1(foo);

    std::cout << "starting second helper...\n";
    std::thread helper2(bar);

    std::cout << "waiting for helpers to finish..." << std::endl;
    helper1.join();
    helper2.join();

    std::cout << "done!\n";
}
