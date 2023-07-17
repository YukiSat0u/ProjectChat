#pragma once

#ifdef _WIN32
#include <winsock2.h>
#include <WS2tcpip.h>
#define CloseSocket closesocket
#else
#include<unistd.h>
#include<sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define CloseSocket close
#endif

#define MESSAGE_LENGTH 1024 
#define PORT 7777 

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <memory>
#include <algorithm>

#include "User.h"
#include "Message.h"
#include "mysql.h"

struct UserLoginExp: public std::exception
{
	const char* what() const noexcept override { return "error: Login is busy"; }
};

struct UserNameExp : public std::exception
{
	const char* what() const noexcept override { return "error: Name is busy"; }
};

class Chat
{
public:
	int bd();
	void startChat();
	bool isChatWork() const { return _isChatWork; }
	std::shared_ptr<User> getCurrentUser() const { return _currentUser; }
	void showLoginMenu();
	void showUserMenu();
	void tcpConnect();
	void sendMessage(SOCKET clientSocket, const std::string& login, const std::string& to, const std::string& text);

#ifdef _WIN32
	WSADATA wsa_data;
#endif

	int result;
	SOCKET clientsocket; 
	sockaddr_in server_address;

private:
	bool _isChatWork = false;
	std::vector<User> _users;
	std::vector<Message> _messages;
	std::shared_ptr<User> _currentUser = nullptr;
	std::fstream user_file = std::fstream("users.txt", std::ios::in | std::ios::out | std::ios::app);
	std::string strToUpper(std::string str);

	MYSQL mysql;
	MYSQL_RES* res;
	MYSQL_ROW row;

	void login();
	void singUp();
	void showChat();
	void showAllUsersName();
	void addMessage();
	void deleteMessage();
	
	std::vector<User>& getAllUsers() { return _users; }
	std::vector<Message>& getAllmessages() { return _messages; }
	std::shared_ptr<User> getUserByLogin(const std::string& login); //const;
	std::shared_ptr<User> getUserByName(const std::string& name); //const;
};
