#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <thread>
#include <mutex>
#include <iostream>
#include <functional>
using namespace std;

//#include "A.h"

typedef void (*Handler)(char *, int);

class B;

class A {
public:
    void Exec(B *b, void (B::*Handler)(char *,int)) {
    }
};

class B {
    A *a;
public:
    B() {
        a = new A();
    }

    //bind the func with deal 
    void bind(char *m, int x) {
        Exec(m, x, std::bind(&B::deal, this, placeholders::_1, placeholders::_2));
    }

    //those are handlers
    int deal(char *m, int x){ 
        cout << "into deal" << endl;
        cout << m << " " << x << endl;
        return 1;
    }

    void Exec(char *m, int x, std::function<int(char*, int)> func) {
        func(m, x);
    }

    //create threads that listens
    std::thread deal_thread(char *m, int x) {
        //pass handlers
        return std::thread([=] { bind(m, x); });
        //a->Exec(this, deal);
    }

    
};


int main() {
    B *b = new B();
    b->bind((char *)"from normal", 1);
    auto c = b->deal_thread((char *)"from thread", 1);
    c.join();

    //start threads at main
    //auto c = b->deal_thread((char *)"ok", 1);
    //c.join();
}

