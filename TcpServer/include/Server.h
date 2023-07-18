#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <winsock2.h>
#include <WS2tcpip.h>
#else
#include <cstring>
#include<unistd.h>
#include<sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define CloseSocket close
#endif

#include "mysql.h"

class Server
{
public:
	int init();
	void tcpConnect();
	void serverUpdate();
	void serverClose();
	void recvMessage();
	void sentMessage();
	void delMessage();
	void singUp();

private:
#ifdef _WIN32
	WSADATA wsa_data;
	SOCKET serversocket;
	SOCKET result;

	SOCKET clientsocket;
	sockaddr_in client_address;
	int client_address_len;
	
	MYSQL mysql;
	MYSQL_RES* res;
	MYSQL_ROW row;
#else
	int result;
	int serversocket;

	struct sockaddr_in client_address;
	socklen_t client_address_len;
	int clientsocket;
#endif

	sockaddr_in server_address;
	char client_ip[INET_ADDRSTRLEN];
};