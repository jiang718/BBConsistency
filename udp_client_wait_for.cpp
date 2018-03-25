
#include <thread>
#include <cstdio>
#include <cstring>
#include <string>
#include <iostream>

#include "udp_helper.h"

UdpHelper *udp_helper;


void SendThread(char *message, char *addr, int port) { 
    udp_helper->Send(addr, port, message, Decoder::GetMessageLength(message, NORMAL_STRING));
}


int main() {
    std::vector<int> a;
    udp_helper = new UdpHelper(a);
    char *addr = (char *)"127.0.0.1";
    char *msg1 = (char *)"msg for service 1";
    char *msg2 = (char *)"msg for service 2";
    //multithreaded send
    //std::thread tsend1(SendThread, msg1, addr, START_PORT);
    //std::thread tsend2(SendThread, msg2, addr, START_PORT + 1);
    //tsend1.join();
    //tsend2.join();
    int service;
    char message[100];
    for (int i = 0; i < 100; i++) {
        std::string s = "Msg " + std::to_string(2 * i);
        std::string s2 = "Msg " + std::to_string(2 * i + 1);
        std::thread tsend(SendThread, (char *)s.c_str(), addr, START_PORT);
        std::thread tsend2(SendThread, (char *)s2.c_str(), addr, START_PORT + 1);
        tsend.join();
        tsend2.join();
    }
    while (1) {
        std::cout << "Please choose service:" << std::endl;
        std::cin >> service;
        std::cout << "Please enter the message:" << std::endl;
        std::cin >> message;
        std::thread tsend(SendThread, (char *)message, addr, START_PORT + (service - 1));
        tsend.join();
    }
    return 0;
}
