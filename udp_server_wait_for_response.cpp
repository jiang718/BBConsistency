#include <thread>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <mutex>

#include "udp_helper.h"

UdpHelper *udp_helper;
std::mutex mtx;

void DoSomething(char *message, int message_len) {
    mtx.lock();
    std::cout << "Receive: " << message << std::endl;
    mtx.unlock();
}

void ListenThread(int x) { 
    //udp_helper will open a thread that receive certain messages
    //this message will be handled by DoSomething asynchronizily
    std::string service_name = "Service " + std::to_string(x + 1);
    udp_helper->Receive(x, service_name.c_str(), &DoSomething, NON_BLOCK);
}

//

int main() {
    std::vector<int> listen_port_list { START_PORT, START_PORT + 1 };
    udp_helper = new UdpHelper(listen_port_list); 

    //multithreaded receiving ports 
    std::thread tlisten1(ListenThread, 0);
    std::thread tlisten2(ListenThread, 1);
    tlisten1.join();
    tlisten2.join();
    return 0;
}
