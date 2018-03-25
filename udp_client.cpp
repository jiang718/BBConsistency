
#include <thread>
#include <cstdio>
#include <cstring>
#include <string>
#include <iostream>
using namespace std;

#include "udp_helper.h"

UdpHelper *udp_helper;

ResponseStruct DoSomething(char *message, int message_len) {
    std::cout << "Receive: " << message << std::endl;
    //return empty struct 
    ResponseStruct res;
    return res;
}
void SendThread(MessageHandler message_handler, char *message, char *addr, int port) { 
    for (int i = 0; i < 10; i++);
    //BOLCK, ANSWER, send_time_out, max_delay_secs, delay_mode, recv_time_out
    udp_helper->Send(std::bind(DoSomething, placeholders::_1, placeholders::_2), addr, port, message, Decoder::GetMessageLength(message, NORMAL_STRING), BLOCK, ANSWER, 3.0, 0.5, DELAY_RANDOM, 3.0);
}


int main() {
    std::vector<int> a;
    udp_helper = new UdpHelper(a);
    char *addr = (char *)"127.0.0.1"; char *msg1 = (char *)"msg for service 1"; char *msg2 = (char *)"msg for service 2";
    //multithreaded send
    //std::thread tsend1(SendThread, msg1, addr, START_PORT);
    //std::thread tsend2(SendThread, msg2, addr, START_PORT + 1);
    //tsend1.join();
    //tsend2.join();
    int N = 10;
    int service;
    char message[100];
    std::vector<std::string> sends;
    std::vector<std::thread> threads;
    char **bufs;
    bufs = new char*[2*N];
    for (int i = 0; i < 2*N; i++) {
        std::string s = std::to_string(i);
        bufs[i] = new char[s.length() + 1];
        bufs[i][s.length()] = '\0';
        s.copy(bufs[i], s.length());
        std::cout << "bufs[" << i << "]:" <<bufs[i] << std::endl;
    }
    for (int i = 0; i < N; i++) {
        //SendThread(std::bind(DoSomething, placeholders::_1, placeholders::_2), (char *)s.c_str(), addr, START_PORT);
        //SendThread(std::bind(DoSomething, placeholders::_1, placeholders::_2), (char *)s2.c_str(), addr, START_PORT + 1);
        threads.push_back(std::thread(SendThread, std::bind(DoSomething, placeholders::_1, placeholders::_2), bufs[2*i], addr, START_PORT));
        threads[2*i].join();
        threads.push_back(std::thread(SendThread, std::bind(DoSomething, placeholders::_1, placeholders::_2), bufs[2*i+1], addr, START_PORT + 1));
        threads[2*i + 1].join();
    }
    //for (int i = 0; i < 2*N; i++) {
    //    threads[i].join();
    //}
    std::thread t(SendThread, std::bind(DoSomething, placeholders::_1, placeholders::_2), (char *)"temp", addr, START_PORT + 2);
    t.join();
    SendThread(std::bind(DoSomething, placeholders::_1, placeholders::_2), (char *)"temp", addr, START_PORT + 3);
    //while (1) {
    //    std::cout << "Please choose service:" << std::endl;
    //    std::cin >> service;
    //    std::cout << "Please enter the message:" << std::endl;
    //    std::cin >> message;
    //    std::thread tsend(SendThread, std::bind(DoSomething, placeholders::_1, placeholders::_2), (char *)message, addr, START_PORT + (service - 1));
    //    tsend.join();
    //}
    return 0;
}
