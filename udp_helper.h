#ifndef UDP_HELPER_H
#define UDP_HELPER_H
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <thread>
#include <mutex>
#include <vector>
#include <iostream>
#include <functional>
#include <set>
//random number
#include <ctime>
#include <cstdlib>

#include "decoder.h"

#define START_PORT 32190 

#define DELAY_FIXED 0
#define DELAY_RANDOM 1 
#define BLOCK 0
#define NON_BLOCK 1

#define ANSWER 0
#define NO_ANSWER 1

#define BUF_SIZE 1000

typedef std::function<ResponseStruct(char *, int)> MessageHandler;
//typedef ResponseStruct (*MessageHandler)(char * message, int message_len); 

class UdpHelper {
    std::vector<int> send_ports;
    std::vector<int> receive_ports;
    std::vector<int> receive_sockets;
    std::vector<std::mutex> receive_mutexes; //protect receive_fd
    std::mutex output_mutex;
    std::mutex send_mutex;
private:
    static struct timeval BuildTime(double secs, std::mutex &mtx);
    static int Sleep(int delay_mode, double max_delay_secs, std::mutex &output_mutex);
    static void StartFunc(MessageHandler message_handler, char *buf, int len, int answer_mode, struct sockaddr_in remote_addr, socklen_t remote_len, int fd, std::mutex &mtx, int delay_mode, double max_delay_secs, std::mutex &output_mutex, struct timeval oldtv, double send_timeout);
    int ReceiveThread(int receive_window, const char * service_name, MessageHandler message_handler, int block_mode, int answer_mode, double max_delay_secs, int delay_mode, double send_time_out);
    int SendThread(MessageHandler message_handler, char *remote_addr, int remote_port, char *msg, int message_len, int block_mode , int answer_mode, double send_time_out, double max_delay_secs, int delay_mode, double recv_time_out);

public:
    //constructor and deconstructor
    UdpHelper(std::vector<int> port_list);
    ~UdpHelper();

    //send udp information via addr, port with a message
    //timeout_mode
    int Send(MessageHandler message_handler, char *remote_addr, int remote_port, char *message, int message_len, int block_mode = BLOCK, int answer_mode = ANSWER, double send_time_out = -1, double max_delay_secs = -1, int delay_mode = DELAY_FIXED, double recv_time_out = -1);
    int Receive(int receive_window, const char * service_name, MessageHandler message_handler, int block_mode = BLOCK, int answer_mode = ANSWER, double max_delay_secs = -1, int delay_mode = DELAY_FIXED, double send_time_out = -1);
    std::mutex & GetOutputMutex();
};
#endif
