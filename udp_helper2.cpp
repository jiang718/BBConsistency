#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <thread>
#include <mutex>
#include <iostream> 
#include <set>
#include <vector>
//random number
#include <ctime>
#include <cstdlib>
#include <chrono>
#include "udp_helper.h"


//offset: passing the previous offset, the offset will change to a newone
int Decoder::decodeInt(char *s, int &offset) {
    //s[offset] : lowest 8 digits
    int res = 0;
    for (int i = 0; i < 4; i++) {
        res = res | ((s[i + offset] & 0xff)<< (8 * i));
    }
    return res;
}

//mode = 0, normal string
//mode = 1, encoded string
int Decoder::GetMessageLength(char *s, int mode) { 
    //decode int at offset 0
    int offset = 0;
    switch (mode) {
        case NORMAL_STRING:
            return strlen(s);
        case ENCODED_STRING:
            return decodeInt(s, offset);
        default:
            //perror("Wrong string format passed to Decoder.");
            return -1;
    }
}

struct timeval UdpHelper::BuildTime(double secs, std::mutex & mtx) {
    struct timeval tv;
    std::lock_guard<std::mutex> lock(mtx);
    int usec_total = 1000000;
    int random_usec = 0;
    std::cout <<"Choose fixed" << std::endl; 
    tv.tv_sec = (int)secs;
    tv.tv_usec = (int)(secs * usec_total) % usec_total;
    std::cout << "Time: " << tv.tv_sec << ":" << tv.tv_usec << std::endl;
    return tv;
}


void UdpHelper::StartFunc(MessageHandler message_handler, char * buf, int len, int answer_mode, struct sockaddr_in remote_addr, socklen_t remote_len, int fd, std::mutex &mtx_for_fd, int delay_mode, double max_delay_secs, std::mutex &mtx_for_general, struct timeval oldtv, double send_time_out) {
    (*message_handler)(buf, len); 
}

int UdpHelper::ReceiveThread(int receive_window, const char *service_name, MessageHandler message_handler, int block_mode, int answer_mode, double max_delay_secs, int delay_mode, double send_time_out) {
    int fd;
    struct sockaddr_in myaddr, remaddr;
    socklen_t remote_len = sizeof(remaddr);
    char *buf = new char[BUF_SIZE];
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("failed to create socket in receiver.");
        return -1;
    }

    
    memset((void *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(receive_ports[receive_window]);
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        perror("failed to bind a socket in receiver.");
        return -1;
    }
    receive_sockets[receive_window] = fd;

    
    //fd_change = true;
    struct timeval oldtv;
    socklen_t time_len = sizeof(oldtv);
    if (getsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &oldtv, &time_len) < 0) {
        perror("failed to preserve time struct for receive.");
        return -1;
    }

    while (1) {
        output_mutex.lock();
        std::cout << service_name << " is listening on " << ntohs(myaddr.sin_port) << "..." << std::endl;
        output_mutex.unlock();
        int recvlen = recvfrom(fd, buf, BUF_SIZE, 0, (struct sockaddr *)&remaddr, &remote_len);
        if (recvlen > 0) {
            output_mutex.lock();
            std::cout << "Received a message from (" << inet_ntoa(remaddr.sin_addr) << ":" << ntohs(remaddr.sin_port) << ")" << std::endl;
            output_mutex.unlock();
            if (block_mode == BLOCK) {
                StartFunc(message_handler, buf, recvlen, answer_mode, remaddr, remote_len, fd, std::ref(receive_mutexes[receive_window]), delay_mode, max_delay_secs, std::ref(output_mutex), oldtv, send_time_out);
            } else {
                std::thread t(StartFunc, message_handler, buf, recvlen, answer_mode, remaddr, remote_len, fd, std::ref(receive_mutexes[receive_window]), delay_mode, max_delay_secs, std::ref(output_mutex), oldtv, send_time_out);
                t.join();
            }
        }
    }
    close(fd);
}
int UdpHelper::Sleep(int delay_mode, double max_delay_secs, std::mutex &mtx) {
    std::lock_guard<std::mutex> lock(mtx);
    int random_delay = 0;
    if (max_delay_secs <= 0) return 0;
    switch (delay_mode) {
        case DELAY_FIXED:
            std::this_thread::sleep_for(std::chrono::milliseconds((int)(max_delay_secs * 1000))); 
            std::cout << "Delayed for " << max_delay_secs << std::endl;
            return 0;
        case DELAY_RANDOM:
            srand(time(NULL));
            random_delay = rand() % (int)(max_delay_secs * 1000);
            std::this_thread::sleep_for(std::chrono::milliseconds(random_delay)); 
            std::cout << "Delayed for " << (double)random_delay / 1000.0 << std::endl;
            return 0;
        default:
            perror("wront delay mode");
            return -1;
    }
    return 0;
}

int UdpHelper::SendThread(char *remote_ip, int remote_port, char *msg, int message_len, int block_mode, int answer_mode, double send_time_out, double max_delay_secs, int delay_mode, double recv_time_out) {
    //std::lock_guard<std::mutex> lock(send_mutex);
    //create socket
    int fd;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("failed create");
        return -1;
    }

    //bind the socket to (addr: port)
    struct sockaddr_in myaddr;
    struct sockaddr_in remote_addr;

    memset((void *)&myaddr, 0, sizeof(myaddr)); 
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(0);
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    memset((void *)&remote_addr, 0, sizeof(remote_addr)); 
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(remote_port);
    inet_pton(remote_addr.sin_family, remote_ip, &remote_addr.sin_addr.s_addr);
    if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        perror("failed bind");
        return -1;
    }
    socklen_t myaddr_len = sizeof(myaddr);
    if (getsockname(fd, (struct sockaddr *)&myaddr, &myaddr_len) < 0) {
        perror("get sock name and port fails");
        return -1;
    }
    output_mutex.lock();
    std::cout << "Open socket " << fd << std::endl;
    std::cout << "Connect to addr: " << inet_ntoa(myaddr.sin_addr) << std::endl;
    std::cout << "Bind to port: " << ntohs(myaddr.sin_port) << std::endl;
    send_ports.push_back(ntohs(myaddr.sin_port));
    std::cout << "Send to addr: " << inet_ntoa(remote_addr.sin_addr) << std::endl;
    std::cout << "Send to port: " << (int)ntohs(remote_addr.sin_port) << std::endl;
    output_mutex.unlock();

    //set time out
    //if max_secs == 0, skip
    if (send_time_out > 0) {
        struct timeval tv = BuildTime(send_time_out, std::ref(output_mutex)); 
        if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
            perror("set timeout failes");
        }
    }

    Sleep(delay_mode, max_delay_secs, std::ref(output_mutex));

    //send udp message 
    if (sendto(fd, msg, message_len, 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) < 0) {
        perror("send message fails");
        return -1;
    }

    output_mutex.lock(); 
    std::cout << "Successfully send the message to (" << inet_ntoa(remote_addr.sin_addr) << ":" << ntohs(remote_addr.sin_port) << ")" << std::endl; 
    output_mutex.unlock(); 

    //send Ak
    if (answer_mode == ANSWER) {
        //set time out
        //if max_secs == 0, skip
        if (recv_time_out > 0) {
            struct timeval tv = BuildTime(recv_time_out,std::ref(output_mutex)); 
            if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
                perror("set timeout failes");
                return -1;
            }
        }

        //receive message from remote server using the secret port binded to fd
        //block till receive
        char *buf = new char[BUF_SIZE];
        socklen_t remote_len = sizeof(remote_addr);
        int recvlen = recvfrom(fd, buf, BUF_SIZE, 0, (struct sockaddr *)&remote_addr, &remote_len);
        if (recvlen > 0) {
            output_mutex.lock(); 
            std::cout << "Receive AK FROM (" << inet_ntoa(remote_addr.sin_addr) << ":" << ntohs(remote_addr.sin_port) << ")" << std::endl;
            output_mutex.unlock(); 
        }
    }
    close(fd);
    return 0;
}

UdpHelper::UdpHelper(std::vector<int> &port_list) {
    for (int i = 0; i < port_list.size(); i++) {
        //initlialized public listening port list
        receive_ports.push_back(port_list[i]);
        //uninitialzied sockets list
        auto mtx = std::unique_ptr<std::mutex>(new std::mutex);
        //receive_mutexes.push_back(mtx);
        receive_sockets.push_back(0);
    }
    std::vector<std::mutex> list(port_list.size());
    receive_mutexes.swap(list);
}
//
UdpHelper::~UdpHelper() {
}

//send udp information via addr, port with a message
//timeout_mode
int UdpHelper::Send(char *remote_addr, int remote_port, char *message, int message_len, int block_mode, int answer_mode, double send_time_out, double max_delay_secs, int delay_mode, double recv_time_out) {
    SendThread(remote_addr, remote_port, message, message_len, block_mode, answer_mode, send_time_out, max_delay_secs, delay_mode, recv_time_out);
}

int UdpHelper::Receive(int receive_window, const char *service_name, MessageHandler message_handler, int block_mode, int answer_mode, double max_delay_secs, int delay_mode, double send_time_out) {
    if (receive_window >= receive_ports.size()) {
        perror("Please define more receive ports to open a new receive window");
        return -1;
    }
    ReceiveThread(receive_window, service_name, message_handler, block_mode, answer_mode, max_delay_secs, delay_mode, send_time_out);
}

std::mutex & UdpHelper::GetOutputMutex() {
    return std::ref(output_mutex);
}
