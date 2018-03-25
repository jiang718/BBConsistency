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

#define NORMAL_STRING 0
#define ENCODED_STRING 1
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
    ResponseStruct EncodeServerList(vector<ServerInfo> servers);
    vector<ServerInfo> DecodeServerList(char *message, int len);
    void EncodeServerSingle(ServerInfo &server, char *&message, int &offset);
    ResponseStruct EncodeServerInfo(ServerInfo server);
    ServerInfo DecodeServerInfo(char *message, int len);
    ServerInfo DecodeServerSingle(char *message, int &offset);

    int GetMessageLength(char *s, int mode);

    int EncodeString(char *&message, string source, int &offset, int len);
    string DecodeString(char *message, int &offset, int len);

    int EncodeInt(char *&message, int x, int &offset);
    int DecodeInt(char *message, int &offset);

    int EncodeIntVector(char *&message, vector<int> &v, int &offset);
    vector<int> DecodeIntVector(char *message, int &offset);
    int TestInt(int x);
    vector<int> TestVectorInt(vector<int> &v);
    void PrintVectorInt(vector<int> &a);
};
#endif
