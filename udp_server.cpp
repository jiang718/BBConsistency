#include <thread>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <mutex>
using namespace std;

#include "udp_helper.h"

UdpHelper *udp_helper;
std::mutex mtx;
int count;
std::vector<int> quantity;
using get_time = std::chrono::steady_clock;
using ns = std::chrono::nanoseconds;
auto start = get_time::now();
int latestId = 0;


char *intToCharStar(int x) {
    std::stringstream ss;
    std::string s;
    ss << x;
    ss >> s;
    return (char *)s.c_str();
}

//return message and message length
ResponseStruct DoSomething(char *message, int message_len) {
    std::lock_guard<std::mutex> lock(udp_helper->GetOutputMutex());
    std::stringstream ss(message);
    int index;
    ss >> index;
    //always update together
    latestId++;
    count++;
    quantity[index]++;
    //the sum of quantity[index] = count / latestId
    std::cout << "Receive: " << message << " at count " << count << std::endl;
    ResponseStruct response;
    response.message = intToCharStar(count);
    response.message_len = strlen(response.message);
    return response;
}


//return message and message length
ResponseStruct Print(char *messsage, int message_len) {
    std::lock_guard<std::mutex> lock(udp_helper->GetOutputMutex());
    std::cout << "Print receive quantity" << std::endl;
    int sum = 0;
    for (int i = 0; i < 1000; i++) {
        sum += quantity[i];
        if (quantity[i] > 0) {
            std::cout << "receive " << i << " : " << quantity[i] << std::endl;
        }
    }
    std::cout << "Sum of quantity: " << sum << std::endl;
    ResponseStruct response;
    response.message = (char *)"Print AK";
    response.message_len = strlen("Print AK");
    return response;
}


void ListenThread(MessageHandler message_handler, int x) { 
    //udp_helper will open a thread that receive certain messages
    //this message will be handled by DoSomething asynchronizily
    std::string service_name = "Service " + std::to_string(x + 1);
    udp_helper->Receive(x, service_name.c_str(), message_handler, NON_BLOCK, ANSWER);
}

ResponseStruct GetElaspedTime(char *message, int message_len) {
    std::lock_guard<std::mutex> lock(udp_helper->GetOutputMutex());
    auto diff = get_time::now() - start;
    std::cout << "Elapsed time is " << std::chrono::duration_cast<ns>(diff).count() << " ns " << std::endl;
    ResponseStruct response;
    response.message = (char *)"Print AK";
    response.message_len = strlen("Print AK");
    return response;
}

//

int main() {
    std::vector<int> listen_port_list { START_PORT, START_PORT + 1 , START_PORT + 2, START_PORT + 3};
    udp_helper = new UdpHelper(listen_port_list); 
    count = 0;
    for (int i = 0; i < 1000; i++) {
        quantity.push_back(0);
    }

    //multithreaded receiving ports 
    std::thread tlisten1(ListenThread, std::bind(DoSomething, placeholders::_1, placeholders::_2), 0);
    std::thread tlisten2(ListenThread, std::bind(DoSomething, placeholders::_1, placeholders::_2), 1);
    std::thread tlisten3(ListenThread, std::bind(Print, placeholders::_1, placeholders::_2), 2);
    std::thread tlisten4(ListenThread, std::bind(GetElaspedTime, placeholders::_1, placeholders::_2), 3);
    tlisten1.join();
    tlisten2.join();
    tlisten3.join();
    tlisten4.join();
    return 4;
}
