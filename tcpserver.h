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
	vector<char> Msg;
	vector<char> inMsg;

	enum status{STATE_OK, STATE_ERR, STATE_NOINIT}; ///< State this object

	void appendMsg(vector<char> data); ///< Add data to buffer

	status sendMsg(); ///< Send message
	status sendMsg(vector<char> data); ///< Send message
	status sendMsg(vector<unsigned char> data); ///< Send message

	vector<char> readMsg();

	void clearMsg();

	status disconnect();
	status connectSrv(const string &ipAddr, int port);
	status connectSrv();

	status createListener();
	status waitConnect();

	status getState();

    inline bool isValid() {return valid;}

    TCPServer(const string &dstAddr, const u_int16_t dstPort);
	~TCPServer();

private:
    bool valid{false};
    string destAddr;
    int destPort{-1};
    int localPort{-1};
	// socket
    int sockIn{-1};
    int sockOut{-1};
    int sockCurrent{-2};
    struct sockaddr_in addr;

    status currentState{STATE_NOINIT};
    bool isConnect{false};

	struct sockaddr_in serv_addr, peerAddr;
};




#endif //TCPCANSENDER_VSCOMSENDER_H

