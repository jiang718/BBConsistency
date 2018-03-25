#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <cstdio>
#include <iostream>

#define MY_PORT 1234

int main() {
    int fd;
    struct sockaddr_in myaddr;
    unsigned int alen; //length of address for getsockname
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("failed to creat a socket");
        return 0;
    }
    std::cout << "create socket: descriptor = " << fd << std::endl;

    memset((void *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(0);
    if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        perror("failed to bind a socket");
        return 0;
    }

    //if (getsockname(fd, (struct sockaddr *)&myaddr, &alen) < 0) {
    //    perror("getsockname failed"); 
    //    return 0;
    //}
    
    std::cout << "Client IP Address = " << inet_ntoa(myaddr.sin_addr) << std::endl;
    std::cout << "Client Port number = " << ntohs(myaddr.sin_port) << std::endl;
}
