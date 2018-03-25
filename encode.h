#ifndef DECODER_H
#define DECODER_H
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
#include <iostream>
#include <vector>
using namespace std;


#define MAX_IP 16

//return changed bytes 
class ServerInfo {
public:
    std::string ip;
    std::vector<int> ports;
};

class ResponseStruct {
public:
    char * message;
    int message_len;
};

class Decoder {
public:
    //offset: passin the previous offset, the offset will change to a newone
    //static int decodeInt(char *s, int &offset);

    //mode = 0, normal string
    //mode = 1, encoded string
    static int GetMessageLength(char *s, int mode);

    static int EncodeInt(char *&message, int x, int &offset);
    static int DecodeInt(char *&message, int &offset);
    static int EncodeIntVector(char *&message, vector<int> &v, int &offset);
    static vector<int> DecodeIntVector(char *&message, int &offset);
    static int TestInt(int x);
    static vector<int> TestVectorInt(vector<int> &v);
    static void PrintVectorInt(vector<int> &a);
};
#endif
