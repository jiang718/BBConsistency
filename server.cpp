#include <thread>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <cstring>
#include <mutex>
#include <functional>
#include "udp_helper.h"
#include "decoder.h"
using namespace std;

using get_time = std::chrono::steady_clock;
using ns = std::chrono::nanoseconds;
auto start = get_time::now();

char * IntToCharStar(int x) {
    std::stringstream ss; std::string s; ss << x; ss >> s;
    return (char *)s.c_str();
}

int CharStarToInt(char * c) { std::stringstream ss(c);
    int x;
    ss >> x;
    return x;
}

string CharStarToString(char *c) {
    std::stringstream ss;
    std::string s;
    ss << c;
    ss >> s;
    return s;
}

//ResponseStruct GetElaspedTime(char *message, int message_len) {
//    std::lock_guard<std::mutex> lock(udp_helper->GetOutputMutex());
//    auto diff = get_time::now() - start;
//    std::cout << "Elapsed time is " << std::chrono::duration_cast<ns>(diff).count() << " ns " << std::endl;
//    ResponseStruct response;
//    response.message = (char *)"Print AK";
//    response.message_len = strlen("Print AK");
//    return response;
//}
//class UpdHelper;

class Server {
private:
    UdpHelper *udp_helper;
    Decoder decoder;
    MessageHandler message_handler;
    std::vector<int> service_ports;
    std::vector<string> service_names;
    std::vector<thread> service_threads;
    std::vector<ServerInfo> server_list; 
    string coordinator_ip, self_ip;
    //the first port in service_port

    //the first port is a mark port because no server on the same machine could 
    // run services in the same port
    int coordinator_markport, self_markport; 
    std::mutex server_list_ak_mutex;
public:
    
    //--------------for info of one server------//
    ServerInfo GenerateServerInfo() {
        ServerInfo info;
        //info.ip = strdup(self_ip.c_str());
        //info.ports = new int*[service_ports.size()];
        //for (int i = 0; i < service_ports.size(); i++) {
        //    info.ports[i] = new int(service_ports[i]);
        //}
        info.ip = self_ip;
        for (int i = 0; i < service_ports.size(); i++) {
            info.ports.push_back(service_ports[i]);
        }
        return info;
    }


    bool IsCoordinator(string ip, int markport) {
        return coordinator_ip == ip && coordinator_markport == markport;
    }
    bool IsCoordinator() {
        return coordinator_ip == self_ip && coordinator_markport == self_markport;
    }

    //initialize a server with receive ports list/services_names
    Server(std::vector<int> receive_ports, std::vector<string> services_names, string self_ip, int self_markport, string coordinator_ip = "", int coordinator_markport = 0) { 
        //coordinator
        if (coordinator_ip == "") {
            this->coordinator_ip = self_ip;
            this->coordinator_markport = self_markport;
        } else {
        //non coordinator
            this->coordinator_ip = coordinator_ip;
            this->coordinator_markport = coordinator_markport;
        }
        this->self_ip = self_ip;
        this->self_markport = self_markport;

        udp_helper = new UdpHelper(receive_ports); 

        for (int i = 0 ; i < services_names.size(); i++) {
            this->service_names.push_back(services_names[i]);
            this->service_ports.push_back(receive_ports[i]);
        }

        //open a thread for receiving server connection
        OpenAllThreads();
    }

    void FirstJoinRequest() {
        if (!IsCoordinator()) {
            ServerInfo self_info = GenerateServerInfo();
            ResponseStruct response = decoder.EncodeServerInfo(self_info);
            udp_helper->Send(std::bind(&Server::SendServerConnectionHandler, this, placeholders::_1, placeholders::_2), (char *)coordinator_ip.c_str(), coordinator_markport, response.message, response.message_len, BLOCK, ANSWER, 3.0, -1, DELAY_RANDOM, 3.0);
        } else {
            JoinNewServer(GenerateServerInfo());
        }
    }

    void StartThead() {
        udp_helper->Receive(0, service_names[0].c_str(), std::bind(&Server::ReceiveServerConnectionHandler, this, placeholders::_1, placeholders::_2), NON_BLOCK, ANSWER);
    }

    void SetCoordinator(ServerInfo coordinator) {
        coordinator_ip = coordinator.ip;
        coordinator_markport = coordinator.ports[0];
        cout << "Coordinator ip: " << coordinator_ip << endl;
        cout << "Coordinator markport: " << coordinator_markport << endl;
    }

    void JoinNewServer(ServerInfo new_server) {
        server_list.push_back(new_server);
        PrintServerList();
    }

    void PrintServerList() {
        cout << "Current ServerList: " << endl;
        for (int i = 0; i < server_list.size(); i++) {
            cout << server_list[i].ip << endl;
            for (int j = 0; j < server_list[i].ports.size(); j++) {
                cout << server_list[i].ports[j] << " ";
            }
            cout << endl;
        }
    }

    int SendServerList(string remote_ip, int remote_port, const int &response_ak) {
        cout << "Current response_ak before send" << response_ak << endl;
        //int &response_ak2 = const_cast<int &>(response_ak);
        //st::mutex &response_ak_mutex2 = const_cast<std::mutex &>(response_ak_mutex);
        //response_ak_mutex2.lock();
        //response_ak2++;
        //response_ak_mutex2.unlock();
        ResponseStruct response = decoder.EncodeServerList(server_list);
        return udp_helper->Send(std::bind(&Server::SendServerListHandler, this, placeholders::_1, placeholders::_2, std::ref(response_ak)), (char *)remote_ip.c_str(), remote_port, response.message, response.message_len, BLOCK, ANSWER, 3.0, -1, DELAY_RANDOM, 3.0);
    }

    ////gather 
    void GatherConfirmation(int total, const int &response_ak) {
        //gather confirmation till reach total
        server_list_ak_mutex.lock();
        int &response_ak2 = const_cast<int &>(response_ak);
        server_list_ak_mutex.unlock();
        cout << "Before gather response" << response_ak << endl;
        int count = 0;
        while (response_ak2 < total) {
            count++;
            cout << "gather response at count " << count << ":" << response_ak << endl;
        }
        cout << "Get Enough Response!" << endl;
    }

    void NotifyServerListToAll() {
        vector<std::thread> send_server_list_threads;
        int response_ak = 0;
        //vector<future<int>> fts;
        //send server list asynchronizly
        for (int i = 0; i < server_list.size(); i++) {
            if (IsCoordinator(server_list[i].ip, server_list[i].ports[0])) continue;
            cout << "Starting notifying " << server_list[i].ip << " " << server_list[i].ports[1] << endl;
            //send serverlist via the port[1] for the target server
            //fts.push_back(std::async(std::bind(&Server::SendServerList, this, server_list[i].ip, server_list[i].ports[1])));
            //send_server_list_threads.push_back(std::thread([=] { SendServerList(server_list[i].ip, server_list[i].ports[1]); }));
            int ak = SendServerList(server_list[i].ip, server_list[i].ports[1], std::ref(response_ak));
            if (ak > 0) response_ak++;
        }
        if (response_ak == server_list.size() - 1) {
            cout << "Get all responses from server" << endl;
        }
        //chrono::milliseconds span(100);
        //vector<future_status> sts;
        //vector<bool> marks;
        //int total = 0; //positive responses count
        //int count = 0; //responses count
        //while (count < fts.size()) { 
        //    for (int i = 0; i < fts.size(); i++) {
        //        sts[i] = fts[i].wait_for(span);
        //        if (!marks[i] && sts[i] == std::future_status::ready) {
        //            int now = fts[i].get();
        //            if (now > 0) total += now;
        //            count++;
        //            marks[i] = true;
        //        }
        //    }
        //}
        //int response_ak = 0;
        //server_list_ak_mutex.lock();
        //for (int i = 0; i < fts.size(); i++) {
        //    int now = fts[i].get();
        //    if (now > 1) response_ak++;
        //}
        //server_list_ak_mutex.unlock();
        ////std::thread t2([=] { GatherConfirmation(server_list.size() - 1, std::ref(response_ak)); });
        //for (int i = 0; i < send_server_list_threads.size(); i++) {
        //    send_server_list_threads[i].join();
        //}
        ////t2.join();
        ////check
        //cout << "current total: " << server_list.size() -1 << endl;
        ////cout << "Total response from other servers who accepts server list: " << response_ak << endl;
        //while (1) {
        //    if (response_ak >= server_list.size() - 1) {
        //        cout << "Enough Response Ak" << endl;
        //    } else {
        //        cout << "Not Enough Response Ak" << endl;
        //    }
        //    std::this_thread::sleep_for(std::chrono::milliseconds(2 * 1000));
        //}
        ////GatherConfirmation(server_list.size() - 1, std::ref(response_ak));
    }

    //service 1: Server Connection Service
    //threads for services
    void ReceiveServerConnection() {
        udp_helper->Receive(0, service_names[0].c_str(), std::bind(&Server::ReceiveServerConnectionHandler, this, placeholders::_1, placeholders::_2), NON_BLOCK, ANSWER);
    }


    ResponseStruct ReceiveServerConnectionHandler(char *message, int len) {
        //std::lock_guard<std::mutex> lock(udp_helper->GetOutputMutex());
        ServerInfo new_server = decoder.DecodeServerInfo(message, len);
        JoinNewServer(new_server);
        cout << endl;
        ResponseStruct response = decoder.EncodeServerInfo(GenerateServerInfo());
        //response.message = (char *)"AK";
        //response.message_len = strlen("AK");

        //Notify servers in the server_list the latest server_list
        NotifyServerListToAll();
        return response;
    }

    ResponseStruct SendServerConnectionHandler(char *message, int len) {
        //std::lock_guard<std::mutex> lock(udp_helper->GetOutputMutex());
        udp_helper->GetOutputMutex().lock();
        std::cout << "Receive from coordinator: " << std::endl;
        udp_helper->GetOutputMutex().unlock();
        ServerInfo coordinator_info = decoder.DecodeServerInfo(message, len);
        SetCoordinator(coordinator_info);
        //std::cout << message << std::endl;
        ResponseStruct response;
        response.message = (char *)"AK";
        response.message_len = strlen("AK");
        return response;
    }


    //services 2: Receive Server List Service
    //threads for services
    void ReceiveServerList() {
        udp_helper->Receive(1, service_names[1].c_str(), std::bind(&Server::ReceiveServerListHandler, this, placeholders::_1, placeholders::_2), NON_BLOCK, ANSWER);
    }

    ResponseStruct SendServerListHandler(char *message, int len, const int &response_ak) {
        //one ak counter per handler
        //TODO: change to a local locking implementation
        server_list_ak_mutex.lock();
        int & response_ak2 = const_cast<int &>(response_ak);
        cout << "Call back for send server list: " << ++response_ak2 << endl;
        server_list_ak_mutex.unlock();

        message[len] = '\0';
        //udp_helper->GetOutputMutex().lock();
        //cout << "Receive: " << message << " from other servers who accepts server list" << endl;
        //udp_helper->GetOutputMutex().unlock();
        ResponseStruct res;
        res.message = (char *)"AK";
        res.message_len = strlen("AK");
        return res;
    }

    ResponseStruct ReceiveServerListHandler(char *message, int len) {
        udp_helper->GetOutputMutex().lock();
        cout << "update server list: " << endl;
        vector<ServerInfo> new_list = decoder.DecodeServerList(message, len);
        server_list.swap(new_list);
        PrintServerList();
        udp_helper->GetOutputMutex().unlock();
        ResponseStruct res;
        res.message = (char *)"AK";
        res.message_len = strlen("AK");
        return res;
    }


    //open all threads of this servers;
    void OpenAllThreads() {
        udp_helper->GetOutputMutex().lock();
        cout << "Into open" << endl;
        udp_helper->GetOutputMutex().unlock();
        service_threads.push_back(std::thread([=] { ReceiveServerConnection(); }));
        service_threads.push_back(std::thread([=] { ReceiveServerList(); }));
        std::thread t([=] { FirstJoinRequest(); });
        for (int i = 0; i < service_threads.size(); i++) { 
            udp_helper->GetOutputMutex().lock();
            cout << "Join thread " << i << endl;
            udp_helper->GetOutputMutex().unlock();
            service_threads[i].join();
        }
        t.join();
    }
};


//

int main(int argc, char *argv[]) {
    //initiliza a server object
    //open all server threads;
    std::vector<string> service_names { "Receive Server Connection", "Receive Server List" };
    int N = service_names.size();
    std::vector<int> listen_port_list;
    if (argc != 2 + N && argc != 4 + N) {
        perror("Please Enter ./server self_ip, coordinator_ip coordinator_markport [a list of ports] (for noncoordinator)\nOr Enter ./server self_ip [a list of ports]");
        return -1;
    }
    //
    int j = (argc == 2 + N ? 2 : 4);
    for (int i = j; i < argc; i++) { 
        listen_port_list.push_back(CharStarToInt(argv[i]));
    }
    cout << "Ports total: " << listen_port_list.size() << endl;

    if (argc == 4 + N) {
        Server server(listen_port_list, service_names, CharStarToString(argv[1]), listen_port_list[0], CharStarToString(argv[2]), CharStarToInt(argv[3]));
    } else {
        Server server(listen_port_list, service_names, CharStarToString(argv[1]), listen_port_list[0]);
    }


    return 0;
}
