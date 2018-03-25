#include <thread>
#include <chrono>
#include <vector>
#include <functional>
#include <chrono>
#include <future>
#include <iostream>
#include <mutex>
using namespace std;

//has a send function, which calls a call back function
class Helper {
public:
    int Send(function<void(void)> func) {
        return 1;
        //return func();
    }
};


class A {
public:
    std::mutex mtx, omtx;
    Helper *helper;
    A() {
        helper = new Helper();
        //helper->Send(std::bind(&A::CallBack, this));
        //helper->Send2(std::bind(&A::CallBackWithArgument, this, placeholders::_1));
    }

    int SendThread(const int &response) {
        std::this_thread::sleep_for(std::chrono::milliseconds(3 * 1000));
        return helper->Send(std::bind(&A::CallBackWithGivenArgument, this, std::ref(response)));
    }

    void BSend() {
        int response = 0;
        thread t1([=] { SendThread(std::ref(response)); });
        thread t2([=] { SendThread(std::ref(response)); });
        mtx.lock();
        cout << "before join" << endl;
        mtx.unlock();
        t1.join();
        t2.join();
        //the commands after join will wait for threads
        mtx.lock();
        cout << "after join" << endl;
        mtx.unlock();
    }

    int increase(int response) {
        return response + 1;
    }

    void ASend(int n) {
        int response = 0;
        future<int> ft = std::async(std::bind(&A::SendThread, this, std::ref(response)));
        std::cout << "checking, wait." << endl;
        chrono::milliseconds span(100);
        while (ft.wait_for(span) == std::future_status::timeout) {
            std::cout << "." << std::flush;
        }
        int now = ft.get();
        cout << now << endl;
        //vector<thread> v;
        //for (int i = 0; i < n; i++) {
        //    std::this_thread::sleep_for(std::chrono::milliseconds(3 * 1000));
        //    v.push_back(thread([=] { SendThread(std::ref(response)); }));
        //    //SendThread(std::ref(response));
        //}
        //for (int i = 0; i < v.size(); i++) {
        //    std::cout << "Sleep " << i << endl;
        //    v[i].join();
        //    mtx.lock();
        //    mtx.unlock();
        //}
        //cout <<response << endl;
    }


    int CallBackWithGivenArgument(const int &x) {
        mtx.lock();
        int &y = const_cast<int &>(x);
        cout << "call back " << ++y << endl;
        mtx.unlock();
        return 1;
    }

};


int main() {
    A a;
    a.ASend(3);
    //a.BSend();
    return 0;
}
