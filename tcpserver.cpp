/**
 *  @author Derendyaev Ilya, ilyajob05@gmail.com
 *  @date    1.1.2019
 *  TCP receiver and receiver
 **/


#include <sys/socket.h>
#include <byteswap.h>
#include <memory>
#include "tcpserver.h"


TCPServer::TCPServer(const string& dstAddr, const u_int16_t dstPort):
    destAddr{dstAddr},
    destPort{dstPort},
    localPort{dstPort}
{
}


TCPServer::~TCPServer()
{
    close(sockIn);
    sockIn = 0;
}


int TCPServer::connectSrv()
{
    return connectSrv(destAddr, destPort);
}


TCPServer::State TCPServer::createListener()
{
    unique_lock sLock(sockInLock);
    isConnect = false;
    // create socket
    sockIn = socket(AF_INET, SOCK_STREAM, 0);

    if (sockIn == kError) {
        cout << "ERROR opening socket\n" << endl;
        return currentState = kError;
    }

    const int yes = 1;

    if (setsockopt(sockIn, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == kError) {
        cout << "ERROR on setsockopt\n" << endl;
        return currentState = kError;
    }

    if (setsockopt(sockIn, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes)) == kError) {
        cout << "ERROR on setsockopt\n" << endl;
        return currentState = kError;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; //any incoming messages
    serv_addr.sin_port = htons(localPort); // is old function
    //serv_addr.sin_port =  bswap_16(localPort);
    // select listen port
    int bindRes = bind(sockIn, (struct sockaddr*) &serv_addr, sizeof(struct sockaddr_in));

    if (bindRes == kError) {
        cout << "ERROR on binding\n" << endl;
        return currentState = kError;
    }

    cout << "listen input port: " << localPort << endl;

    if (listen(sockIn, 5) != 0) {
        cout << "ERROR on listen socket\n" << endl;
        return currentState = kError;
    }

    return currentState = kOk;
}


int TCPServer::connectSrv(const string& ipAddr, int port)
{
    int sockCurrent{-1};
    destAddr = ipAddr;
    destPort = port;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(destPort);
    addr.sin_addr.s_addr = inet_addr(destAddr.c_str());
    sockOut = socket(AF_INET, SOCK_STREAM, 0);

    if (sockOut < 0) {
        perror("ERROR on socket create\n");
        isConnect = false;
        return currentState = kError;
    }

    // connect to server
    connect(sockCurrent, (const struct sockaddr*)&addr, (socklen_t)sizeof(addr));
    return sockCurrent;
}


int TCPServer::disconnect()
{
    int countMsgs{0};

    for (auto& sc : sockList) {
        unique_lock sLock(sc->scOutLock);

        if ((sc->state != kError) && (sc->state != kNotInit)) {
            close(sc->socket);
            sc->state = kNotInit;
            countMsgs++;
        }
    }

    return countMsgs;
}


TCPServer::State TCPServer::sendMsg(SocketAndState& sc)
{
    if ((sc.outMsg.size() > 0)) {
        unique_lock sLock(sc.scOutLock);
        ssize_t lenSend = send(sc.socket, sc.outMsg.data(), sc.outMsg.size(), MSG_NOSIGNAL);

        if (lenSend == kError) {
            sc.state = kError;
        } else {
            sc.state = kOk;
        }
    }

    return sc.state;
}


TCPServer::State TCPServer::sendMsg(SocketAndState& sc, std::vector<char>& outMsg)
{
    if ((sc.outMsg.size() > 0) && (sc.state != kError)) {
        unique_lock sLock(sc.scOutLock);
        ssize_t lenSend = send(sc.socket, outMsg.data(), outMsg.size(), MSG_NOSIGNAL);

        if (lenSend == kError) {
            sc.state = kError;
        } else {
            sc.state = kOk;
        }
    }

    return sc.state;
}


int TCPServer::sendMsgs(std::vector<char>& outMsg)
{
    unique_lock sLock(sockListLock);

    for (auto& sc : sockList) {
        sendMsg(*sc, outMsg);
    }

    int countMsgs = count_if(sockList.begin(), sockList.end(),
    [](const unique_ptr<SocketAndState>& i) -> bool {
        return i->state != kError;
    });
    return countMsgs;
}


int TCPServer::sendMsgs()
{
    unique_lock sLock(sockListLock);

    for (auto& sc : sockList) {
        sendMsg(*sc);
    }

    int countMsgs = count_if(sockList.begin(), sockList.end(),
    [](const unique_ptr<SocketAndState>& i) -> bool {
        return i->state != kError;
    });
    return countMsgs;
}


//void TCPServer::appendMsg(vector<char> data)
//{
//    outMsg.insert(outMsg.end(), data.begin(), data.end());
//}
void TCPServer::clearMsgs()
{
    unique_lock sLock(sockListLock);

    for (auto& sc : sockList) {
        sc->outMsg.clear();
        sc->inMsg.clear();
    }
}


void TCPServer::closeSockets(void)
{
    for (auto& sc : sockList) {
        unique_lock sLock(sc->scInLock);
        close(sc->socket);
    }
}


void TCPServer::clearErrSockets(void)
{
    unique_lock sLock(sockListLock);
    sockList.erase(remove_if(sockList.begin(), sockList.end(),
    [](unique_ptr<SocketAndState>& i) -> bool {
        return i->state == kError;
    }), sockList.end());
}


int TCPServer::waitConnect(SocketAndState& ss)
{
    cout << "wait connect..." << endl;
    // wait connection
    // create new socket
    socklen_t peerLen = sizeof(struct sockaddr_in);
    int sockCurrent{0};
    {
        unique_lock sLock(sockInLock);
        sockCurrent = accept(sockIn, (struct sockaddr*) &peerAddr, &peerLen);
    }

    if (sockCurrent == kError) {
        cout << "ERROR on accept socket\n" << endl;
        return kError;
    } else {
        unique_lock sLock(ss.scInLock);
        ss.socket = sockCurrent;
        ss.state = currentState;
    }

    cout << "    complete" << endl;
    return sockCurrent;
}


void TCPServer::readMsg(SocketAndState& sc)
{
    unique_lock sLock(sc.scInLock);
    readMsg(sc.socket, sc.inMsg);
}


void TCPServer::readMsg(int sc, vector<char>& msg)
{
    char buff[1024];
    memset(buff, 0, sizeof(buff));
    ssize_t len = -1;
    vector<char> msgTmp;

    do {
        len = recv(sc, buff, 1024, 0);

        if (len > 0) {
            msgTmp.insert(msgTmp.end(), buff, buff + len);
        } else if (len == -1) {
            printf("ERROR recv socket\n");
            return;
        }
    } while (len == 1024); // while full pack
}
