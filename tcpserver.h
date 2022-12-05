/**
 *  @author Derendyaev Ilya, ilyajob05@gmail.com
 *  @date    1.1.2019
 *  TCP receiver and receiver
 **/


#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <list>
#include <mutex>
#include <opencv2/opencv.hpp>

// net socket
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


using namespace std;

/// \brief Class using to TCP connect server and client
/// \details
///
class TCPServer
{
    public:
        enum State {kOk = 0, kError = -1, kSendHeader = -2, kNotInit = -3, kSendStream = -4}; ///< State this object

        struct SocketAndState {
            State state {kNotInit};
            int socket{kNotInit};
            mutex scOutLock;
            mutex scInLock;
            vector<char> inMsg;
            vector<char> outMsg;
        };

        cv::Mat emptyImage;

        std::list<unique_ptr<SocketAndState>> sockList;
        std::mutex sockListLock;

        void appendMsg(vector<char> data); ///< Add data to buffer

        int sendMsgs(); ///< Send message
        State sendMsg(SocketAndState& sc); ///< Send message
        State sendMsg(SocketAndState& sc, std::vector<char>& outMsg);
        int sendMsgs(std::vector<char>& outMsg);


        void readMsg(int sc, vector<char>& msg);
        void readMsg(SocketAndState& sc);

        void clearMsgs();
        void clearErrSockets(void);
        void closeSockets(void);

        int disconnect();
        int connectSrv(const string& ipAddr, int port);
        int connectSrv();

        State createListener();
        int waitConnect(SocketAndState& ss);

        State getState();

        inline bool isValid()
        {
            return valid;
        }

        TCPServer(const string& dstAddr, const u_int16_t dstPort);
        ~TCPServer();

    private:
        bool valid{false};
        string destAddr;
        int destPort{kNotInit};
        int localPort{kNotInit};
        // socket
        int sockIn{kNotInit};
        std::mutex sockInLock;

        int sockOut{kNotInit};

        struct sockaddr_in addr;

        State currentState{kNotInit};
        bool isConnect{false};

        struct sockaddr_in serv_addr, peerAddr;

};




#endif //TCPCANSENDER_VSCOMSENDER_H

