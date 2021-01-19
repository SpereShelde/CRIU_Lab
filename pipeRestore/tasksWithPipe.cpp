#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <armadillo>
using namespace std;

int checkpointOverhead = 10; //milliseconds
int checkpointRealOverhead = 4; //milliseconds
int restoreOverhead = 10; //milliseconds
int restoreRealOverhead = 4; //milliseconds
double averageArrival = 5000; // of fault
int restoreFromStart = 1;

chrono::steady_clock::time_point ts;
ofstream thePipe;
mutex msg1, msg2, msg3, msg4, msg5;
int meetDeadline = 0;
int currentCP = 0;
int completedThreads = 0;
int threads = 4;

void recordTask(chrono::steady_clock::time_point endTime, int deadlineMilliseconds) {
    auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - ts).count();
    if (duration < deadlineMilliseconds) {
        meetDeadline += 1;
    }
}

chrono::steady_clock::time_point basicTask(mutex* inputMsg, mutex* outputMsg, int taskRealMilliseconds, bool checkpoint = false){
    if (checkpoint) {
        this_thread::sleep_for(chrono::milliseconds(checkpointOverhead - checkpointRealOverhead));
//        cout << "save cp" << currentCP << endl;
        thePipe << "save cp" << currentCP++ << endl;
    }

    if (inputMsg) {
        inputMsg->lock();
        inputMsg->unlock();
    }
    if (outputMsg) {
        outputMsg->lock();
    }
    this_thread::sleep_for(chrono::milliseconds(taskRealMilliseconds));
    if (outputMsg) {
        outputMsg->unlock();
    }
    return chrono::steady_clock::now();
}

void thread1() {
    recordTask(basicTask(nullptr, &msg1, 8000), 8100);
    cout << "task 1-1" << endl;
    recordTask(basicTask(nullptr, nullptr, 2000), 10200);
    cout << "task 1-2" << endl;
    recordTask(basicTask(&msg4, nullptr, 4000), 14300);
    cout << "task 1-3" << endl;
    completedThreads += 1;
}

void thread2() {
    recordTask(basicTask(nullptr, &msg2, 2000), 2100);
    cout << "task 2-1" << endl;
    recordTask(basicTask(nullptr, nullptr, 2000), 4200);
    cout << "task 2-2" << endl;
    recordTask(basicTask(&msg1, nullptr, 2000), 10200);
    cout << "task 2-3" << endl;
    recordTask(basicTask(nullptr, &msg4, 2000, true), 12300);
    cout << "task 2-4" << endl;
    recordTask(basicTask(nullptr, nullptr, 6000, true), 18400);
    cout << "task 2-5" << endl;
    completedThreads += 1;
}

void thread3() {
    recordTask(basicTask(nullptr, nullptr, 2000), 2100);
    cout << "task 3-1" << endl;
    recordTask(basicTask(&msg2, nullptr, 4000), 6200);
    cout << "task 3-2" << endl;
    recordTask(basicTask(nullptr, &msg5, 4000), 10300);
    cout << "task 3-3" << endl;
    recordTask(basicTask(nullptr, nullptr, 2000), 12400);
    cout << "task 3-4" << endl;
    completedThreads += 1;
}

void thread4() {
    recordTask(basicTask(nullptr, &msg3, 16000, true), 16100);
    cout << "task 4-1" << endl;
    recordTask(basicTask(nullptr, nullptr, 2000), 18200);
    cout << "task 4-2" << endl;
    recordTask(basicTask(&msg5, nullptr, 2000), 20300);
    cout << "task 4-3" << endl;
    completedThreads += 1;
}

//void thread5(int sock_fd, struct sockaddr_in addr_serv, int len) {
void thread5() {
    random_device rd; // uniformly-distributed integer random number generator
    mt19937 rng (rd ()); // mt19937: Pseudo-random number generation

    double lamda = 1 / averageArrival;
    exponential_distribution<double> exp (lamda);

    double newArrivalTime;

    while (completedThreads < threads) {

        newArrivalTime = exp.operator() (rng);
//        cout << (int)newArrivalTime << endl;
        int sleepTime = (int)newArrivalTime - (restoreOverhead - restoreRealOverhead);
        this_thread::sleep_for(chrono::milliseconds(sleepTime));

        srand((unsigned)time(NULL));
        int random = rand() % 10;
        if (random <= restoreFromStart) {
//            cout << "rest cp0" << endl;
            thePipe << "rest cp0" << endl;

        } else {
//            cout << "rest cp" << max(0, currentCP-1) << endl;
            thePipe << "rest cp" << max(0, currentCP-1) << endl;

        }
    }
}

int main(int argc, char **argv){
    thePipe.open("thePipe");
    ts = chrono::steady_clock::now();

    thread first(thread1);
    this_thread::sleep_for(1ms);
    thread second(thread2);
    this_thread::sleep_for(1ms);
    thread third(thread3);
    this_thread::sleep_for(1ms);
    thread fourth(thread4);
    this_thread::sleep_for(1ms);
    thread fifth(thread5);

    first.join();
    second.join();
    third.join();
    fourth.join();
    fifth.join();

    chrono::steady_clock::time_point te = chrono::steady_clock::now();
    chrono::duration<double> timeUsed = chrono::duration_cast<chrono::duration<double>>(te - ts);
    cout << "The used time is : " << timeUsed.count() << " s." << endl;
    cout << meetDeadline << " tasks meet deadline" << endl;

    thePipe << "exit" << endl;

    system("touch asd.asd");
    return 0;
}