#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <thread>
#include <mutex>
#include <iostream>
using namespace std;

#include "A.h"

static void B(A* a) {
    cout << a->x << endl;
    cout << a->GetX() << endl;
    a->C();
}
static void B(A a) {
    cout << a.x << endl;
    cout << a.m << endl;
}


class D {
private:
    A *a;
public:
    D() {
        a = new A();
    }
};

int main() {
    int N = 10;
    char **bufs;
    bufs = new char *[2*N];
    for (int i = 0 ; i < N; i++) {
        string s = std::to_string(2*i);
        string s2 = std::to_string(2*i+1);
        bufs[2*i] = new char[s.length() + 1];
        bufs[2*i+1] = new char[s2.length() + 1];
        bufs[s.length()] = '\0';
        bufs[s.length()] = '\0';
        s.copy(bufs[2*i], s.length());
        cout << bufs[2*i] << endl;
    }
    //A a;
    //B(&a);
    //B(a);
    return 0;
}
