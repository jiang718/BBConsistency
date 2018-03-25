#include <iostream>
#include <mutex>
#include <functional>
#include <thread>
using namespace std;

std::mutex mtx;

class A {
public:
    void Change(const int &a) {
        int &b = const_cast<int &>(a);
        mtx.lock();
        ++b;
        cout << b << endl;
        mtx.unlock();
    }

    void TryThreads() {
        int a = 0;
        //because the object is copied twice
        std::thread t1( [=] { Change(std::ref(a)); });
        std::thread t2( [=] { Change(std::ref(a)); });
        t1.join(); 
        t2.join(); 
    }
};

int main() {
    A a;
    a.TryThreads();
    return 0;
}
