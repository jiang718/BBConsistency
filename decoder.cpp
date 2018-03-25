#include <iostream>
#include <vector>
#include "decoder.h"
using namespace std;


void Decoder::EncodeServerSingle(ServerInfo &server, char *&message, int &offset) {
    EncodeString(message, server.ip, offset, MAX_IP); 
    EncodeInt(message, server.ports.size(), offset);
    for (int i = 0 ; i < server.ports.size(); i++) {
        EncodeInt(message, server.ports[i], offset);
    }
}

ResponseStruct Decoder::EncodeServerList(vector<ServerInfo> servers) {
    ResponseStruct res;
    res.message_len = 4;
    for (int i = 0; i < servers.size(); i++) {
        res.message_len += MAX_IP + 4 + servers[i].ports.size() * 4;
    }
    res.message = new char[res.message_len];
    //cout << "size: " << servers.size() << endl;
    int offset = 0;
    EncodeInt(res.message, servers.size(), offset);
    //cout << "curent offset: " << offset << endl;
    //offset -= 4;
    //cout << "sub: " << offset << endl;
    //cout << "at " << offset;
    //cout << ": " <<DecodeInt(res.message, offset) << endl;
    for (int i = 0; i < servers.size(); i++) {
        EncodeServerSingle(servers[i], res.message, offset);
    }
    return res;
}

vector<ServerInfo> Decoder::DecodeServerList(char *message, int len) {
    vector<ServerInfo> res;
    int offset = 0;
    int size = DecodeInt(message, offset);
    //cout << "decoder size: " << size << endl;
    for (int i = 0; i < size; i++) {
        ServerInfo info = DecodeServerSingle(message, offset);
        res.push_back(info);
    }
    return res;
}

ResponseStruct Decoder::EncodeServerInfo(ServerInfo server) {
    ResponseStruct res;
    res.message_len = MAX_IP + 4 + server.ports.size() * 4;
    res.message = new char[res.message_len];
    int offset = 0;
    EncodeServerSingle(server, res.message, offset);
    return res;
}

ServerInfo Decoder::DecodeServerSingle(char *message, int &offset) {
    ServerInfo info;
    //cout << "offset in decode server single: " << offset << endl;
    info.ip = DecodeString(message, offset, MAX_IP); 
    //cout << info.ip << endl;
    int size = DecodeInt(message, offset);
    for (int i = 0; i < size; i++) {
        info.ports.push_back(DecodeInt(message, offset));
    }
    return info;
}

ServerInfo Decoder::DecodeServerInfo(char *message, int len) {
    int offset = 0;
    return DecodeServerSingle(message, offset);
}

//return space for this string (including empty slots) 
int Decoder::EncodeString(char *&message, string source, int &offset, int len) {
    for (int i = 0; i < source.length(); i++) {
        message[offset + i] = source[i];
    }
    message[offset + source.length()] = '\0';
    offset += len;
    return len; 
}

string Decoder::DecodeString(char *message, int &offset, int len) {
    string res(message + offset);
    offset += len;
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
            return DecodeInt(s, offset);
        default:
            perror("Wrong string format passed to Decoder."); 
            return -1;
    }
}


//return offset
int Decoder::EncodeInt(char *&message, int x, int &offset) {
    for (int i = 0; i < 4; i++) {
        message[offset + i] = (x >> (8 * i)) & 0xff;    
    }
    offset += 4;
    return 4; 
}

int Decoder::DecodeInt(char *message, int &offset) {
    int res = 0;
    for (int i = 0; i < 4; i++) {
        res = res | ((message[offset + i] & 0xff)<< (8*i));  
    }
    offset += 4;
    return res;
}

int Decoder::EncodeIntVector(char *&message, vector<int> &v, int &offset) {
    EncodeInt(message, v.size(), offset);
    cout << "size bofe: " << v.size() << endl;
    cout << "offset now : " << offset << endl;
    for (int i = 0; i < v.size(); i++) {
        EncodeInt(message, v[i], offset);
    }
    return 4 * v.size() + 4;
}

vector<int> Decoder::DecodeIntVector(char *message, int &offset) {
    vector<int> res;
    int size = DecodeInt(message, offset);
    cout << "size: " << size << endl;
    for (int i = 0; i < size; i++) {
        res.push_back(DecodeInt(message, offset));
    }
    return res;
}


int Decoder::TestInt(int x) {
    char *message = new char[4];
    int encode_off = 0, decode_off = 0; 
    EncodeInt(message, x, encode_off);
    return DecodeInt(message, decode_off);
}

vector<int> Decoder::TestVectorInt(vector<int> &v) {
    char *message = new char[4  + 4 * v.size()];
    int encode_off = 0, decode_off = 0; 
    EncodeIntVector(message, v, encode_off);
    vector<int> res = DecodeIntVector(message, decode_off);
    return res;
}

void Decoder::PrintVectorInt(vector<int> &a) {
    for (int i = 0; i < a.size(); i++) {
        cout << a[i] << " ";
    }
    cout << endl;
}



//int main() {
//    ServerInfo server;
//    Decoder decoder;
//    server.ip = "186.123.231.110";
//    server.ports.push_back(1);
//    server.ports.push_back(2);
//    ResponseStruct res = decoder.EncodeServerInfo(server);
//    ServerInfo info = decoder.DecodeServerInfo(res.message, res.message_len);
//    cout << info.ip << endl;
//    for (int i = 0; i < info.ports.size(); i++) {
//        cout << info.ports[i] << " ";
//    }
//    cout << endl;
//    vector<ServerInfo> l;
//    l.push_back(server);
//    ServerInfo server2;
//    server2.ip = "106.123.231.110";
//    server2.ports.push_back(3);
//    server2.ports.push_back(4);
//    l.push_back(server2);
//    ResponseStruct res2 = decoder.EncodeServerList(l);
//    vector<ServerInfo> l2 = decoder.DecodeServerList(res2.message, res2.message_len);
//    for (int i = 0; i < l2.size(); i++) {
//        cout << l2[i].ip << endl;
//        for (int j = 0; j < l2[i].ports.size(); j++) {
//            cout << l2[j].ports[i] << " ";
//        }
//        cout << endl;
//    }
//    vector<ServerInfo> l3;
//    ServerInfo i;
//    i.ip = "123.234";
//    l3.push_back(i);
//    ResponseStruct res3 = decoder.EncodeServerList(l3);
//    vector<ServerInfo> l4 = decoder.DecodeServerList(res3.message, res3.message_len);
//    for (int i = 0; i < l4.size(); i++) {
//        cout << l4[i].ip << endl;
//        for (int j = 0; j < l4[i].ports.size(); j++) { 
//            cout << l4[i].ports[j] << " ";
//        }
//        cout << endl;
//    }
//
//
//    //for (int i = -1000; i <= 1000; i++) {
//    //    int k = testInt(i);
//    //    if (i != k) {
//    //        cout << i <<" " << k << endl;
//    //    }
//    //}
//    //vector<int> a {3, 2, 6};
//    //vector<int> b {};
//    //vector<int> c {-1, 3};
//    //vector<int> a1 = testVectorInt(a);
//    //vector<int> b1 = testVectorInt(b);
//    //vector<int> c1 = testVectorInt(c);
//    //printVectorInt(a1);
//    //printVectorInt(b1);
//    //printVectorInt(c1);
//    return 0;
//}
