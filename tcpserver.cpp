/**
 *  @author Derendyaev Ilya, ilyajob05@gmail.com
 *  @date    1.1.2019
 *  TCP receiver and receiver
 **/


#include "tcpserver.h"
#include <sys/socket.h>
#include <byteswap.h>


TCPServer::TCPServer(const string &dstAddr, const u_int16_t dstPort):
    destAddr{dstAddr},
    destPort{dstPort},
    localPort{dstPort}
{
//	localPort = destPort = dstPort;
//	destAddr = dstAddr;
}


TCPServer::~TCPServer()
{
    close(sockIn);
    sockIn = 0;
}


TCPServer::status TCPServer::connectSrv()
{
	return connectSrv(destAddr, destPort);
}


TCPServer::status TCPServer::createListener()
{
	isConnect = false;

	// create socket
    sockIn = socket(AF_INET, SOCK_STREAM, 0);
	if(sockIn < 0)
	{
		cout << "ERROR opening socket\n" << endl;
		return currentState = STATE_ERR;
	}

	int yes=1;
	//char yes='1'; // use this under Solaris
	if (setsockopt(sockIn, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
	{
		cout << "setsockopt\n" << endl;
		return currentState = STATE_ERR;
	}
	if (setsockopt(sockIn, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes)) == -1)
	{
		cout << "setsockopt\n" << endl;
		return currentState = STATE_ERR;
	}


//	if (setsockopt(sockCurrent, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
//	{
//		printf("setsockopt\n");
//		return currentState = STATE_ERR;
//	}
//	if (setsockopt(sockCurrent, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes)) == -1)
//	{
//		printf("setsockopt\n");
//		return currentState = STATE_ERR;
//	}


    bzero(&serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY; //any incoming messages
	serv_addr.sin_port = htons(localPort); // is old function

	//serv_addr.sin_port =  bswap_16(localPort);

    // select listen port
    int bindRes = bind(sockIn, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
	if (bindRes < 0)
	{
		cout << "ERROR on binding\n" << endl;
		return currentState = STATE_ERR;
	}

	cout << "listen input port: " << localPort << endl;

    if(listen(sockIn, 5) != 0)
	{
		cout << "ERROR on listen socket\n" << endl;
		return currentState = STATE_ERR;
	}

	return currentState = STATE_OK;
}

TCPServer::status TCPServer::connectSrv(const string &ipAddr, int port)
{
	destAddr = ipAddr;
	destPort = port;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(destPort);
	addr.sin_addr.s_addr = inet_addr(destAddr.c_str());

    sockOut = socket(AF_INET, SOCK_STREAM, 0);
	if(sockOut < 0)
	{
		perror("ERROR on socket create\n");
		isConnect = false;
		return currentState = STATE_ERR;
	}

    // connect to server
    if(connect(sockCurrent, (const struct sockaddr *)&addr, (socklen_t)sizeof(addr)) < 0)
	{
			isConnect = false;
			return currentState = STATE_ERR;
	}
	else
	{
		isConnect = true;
		return currentState = STATE_OK;
	}
	// warning remove
	isConnect = true;
	return currentState = STATE_OK;
}


TCPServer::status TCPServer::disconnect()
{
	if(isConnect)
	{
        close(sockCurrent);
		sockCurrent = -1;

		isConnect = false;
	}

	Msg.clear();
	inMsg.clear();

	return currentState = STATE_NOINIT;
}


TCPServer::status TCPServer::sendMsg(vector<char> data)
{
	if(!isConnect)
	{
		return currentState = STATE_NOINIT;
	}

	if(data.size() > 0)
	{
		ssize_t lenSend = send(sockCurrent, data.data(), data.size(), MSG_NOSIGNAL);
		if(lenSend < 1)
		{
			return currentState = STATE_ERR;
		}
		//cout << "send " + to_string(lenSend) + "bytes" << endl;
	}

	return currentState = STATE_OK;
}

TCPServer::status TCPServer::sendMsg(vector<unsigned char> data)
{
	vector<char>tmpMsg(data.begin(), data.end());
	return sendMsg(tmpMsg);
}

TCPServer::status TCPServer::sendMsg()
{
	TCPServer::status state = sendMsg(Msg);
	Msg.clear();
	return state;
}


void TCPServer::appendMsg(vector<char> data)
{
	Msg.insert(Msg.end(), data.begin(), data.end());
}


void TCPServer::clearMsg()
{
	Msg.clear();
	inMsg.clear();
}


TCPServer::status TCPServer::waitConnect()
{
	cout << "wait connect..." << endl;
	// wait connection
	// create new socket
	socklen_t peerLen = sizeof(struct sockaddr_in);
	if(sockCurrent != -1)
	{
		sockCurrent = accept(sockIn, (struct sockaddr *) &peerAddr, &peerLen);
		if (sockCurrent < 0)
		{
			cout << "ERROR on accept socket\n" << endl;
			return currentState = STATE_ERR;
		}
	}

	cout << "    complete" << endl;
	isConnect = true;
	return currentState = STATE_OK;
}


vector<char> TCPServer::readMsg()
{
	char buff[1024];
	bzero(buff, sizeof(buff));
	int len = -1;

	valid = false;

	do
	{
        len = (int)recv(sockCurrent, buff, 1024, 0);

		if(len > 0)
		{
			inMsg.insert(inMsg.end(), buff, buff + len);
		}
		else if(len == -1)
		{
			printf("ERROR recv socket\n");
			inMsg.clear();
            return inMsg;
		}
    }while(len == 1024); // while full pack

	valid = true;

    return inMsg;
}



