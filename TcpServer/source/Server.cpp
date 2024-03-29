#include "../include/Server.h"

int Server::init()
{
#ifdef _WIN32
	result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (result != 0) {
		std::cerr << "������ ��� ������������� Winsock" << std::endl;
		return 1;
	}
#endif

	// �������� TCP ������
	serversocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// ���������� ������ � ��������� ������ � ������
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(8000);
	result = bind(serversocket, (sockaddr*)&server_address, sizeof(server_address));

#ifdef _WIN32
	if (result != 0) {
		std::cerr << "������ ��� ���������� ������" << std::endl;
		closesocket(serversocket);
		WSACleanup();
		return 1;
	}
#else 
	if (result != 0) {
		std::cerr << "Socket binding error" << std::endl;
		close(serversocket);
		return 1;
	}
#endif

	// ������������� �������� ����������
	result = listen(serversocket, SOMAXCONN);
	if (result != 0)
	{
		std::cerr << "Error listening for connections" << std::endl;
#ifdef _WIN32
		closesocket(serversocket);
		WSACleanup();
#else
		close(serversocket);
#endif
		return 1;
	}
}

void Server::tcpConnect()
{
#ifdef _WIN32
	client_address_len = sizeof(client_address);
	clientsocket = accept(serversocket, (sockaddr*)&client_address, &client_address_len);
	inet_ntop(AF_INET, &(client_address.sin_addr), client_ip, INET_ADDRSTRLEN);
	std::cout << "Connection from " << client_ip << ":" << ntohs(client_address.sin_port) << " was successfully established" << std::endl;
#else
	client_address_len = sizeof(client_address);
	clientsocket = accept(serversocket, (sockaddr*)&client_address, &client_address_len);
	inet_ntop(AF_INET, &(client_address.sin_addr), client_ip, INET_ADDRSTRLEN);
	std::cout << "Connection from " << client_ip << ":" << ntohs(client_address.sin_port) << " was successfully established" << std::endl;
#endif
}

void Server::sentMessage()
{
	// �������� ������ �������
#ifdef _WIN32
	msg_file = std::fstream("../../messages.txt", std::ios::in | std::ios::out | std::ios::app);
#else
	msg_file = std::fstream("messages.txt", std::ios::in | std::ios::out | std::ios::app);
#endif
	std::string from, to, text;

	std::ostringstream messages;
	while (getline(msg_file, from, ' ') && getline(msg_file, to, ' ') && getline(msg_file, text))
	{
		std::string message = "Message from " + from + " to " + to + "\nText: " + text + "\n";
		messages << message;
	}
	msg_file.close();

	// �������� ��������� ��������
	std::string messages_str = messages.str();
	if (!messages_str.empty())
	{
		int bytes_sent = send(clientsocket, messages_str.c_str(), messages_str.size(), 0);
		if (bytes_sent < 0)
		{
			perror("Failed to send message");
			exit(EXIT_FAILURE);
		}

		std::cout << "Chat: \n" << messages_str << "Chat end." << std::endl;
	}
	else
		send(clientsocket, " ", 1, 0);
}

void Server::delMessage()
{
	msg_file = std::fstream("../../messages.txt", std::ios::in | std::ios::out | std::ios::app);
	temp = std::fstream("../../temp.txt", std::ios::in | std::ios::out | std::ios::app);

	char buffer[1024] = { 0 };
	recv(clientsocket, buffer, 1024, 0);

	std::string message_to_delete = buffer;

	bool message_deleted = false;

	std::string data;
	while (getline(msg_file, data))
	{
		if (data != message_to_delete)
		{
			temp << data << std::endl;
		}
		else
		{
			message_deleted = true;
		}
	}
	if (!message_deleted)
	{
		std::cout << "Message not found!" << std::endl << std::endl;
		temp.close();
		std::remove("../../temp.txt");
		send(clientsocket, "Message not found!", 1024, 0);
	}
	else
	{
		std::cout << "Message deleted!" << std::endl << std::endl;
		msg_file.close();
		temp.close();
		std::remove("../../messages.txt");
		std::rename("../../temp.txt", "../../messages.txt");
		std::remove("../../temp.txt");

		send(clientsocket, "Message deleted!", 1024, 0);
	}
}

void Server::recvMessage()
{
#ifdef _WIN32
	msg_file = std::fstream("../../messages.txt", std::ios::in | std::ios::out | std::ios::app);
#else
	msg_file = std::fstream("messages.txt", std::ios::in | std::ios::out | std::ios::app);
#endif

	// ��������� ��������� �� �������
	char buffer[1024] = { 0 };
	result = recv(clientsocket, buffer, 1024, 0);
#ifdef _WIN32
	if (result == SOCKET_ERROR) {
		int error = WSAGetLastError();
		if (error == WSAECONNRESET) {
			std::cout << "���������� ���� ������������� ���������." << std::endl;
		}
		else {
			std::cerr << "������ ��� ������ ���������: " << error << std::endl;
		}
	}
#else
	if (result == -1) {
		int error = errno;
		if (error == ECONNRESET) {
			std::cout << "The connection was terminated unexpectedly." << std::endl;
		}
		else {
			std::cerr << "Error reading message: " << error << std::endl;
	}
		break;
}
#endif

	std::string data = buffer;
	std::cout << "Message received: " << data << std::endl << std::endl;
	msg_file << data << std::endl;
	msg_file.close();
}

void Server::serverUpdate()
{
	while (true)
	{
		char buffer[1024] = { 0 };
		result = recv(clientsocket, buffer, 1024, 0);
		if (strcmp(buffer, "send") == 0)
			recvMessage();
		else if (strcmp(buffer, "recv") == 0)
			sentMessage();
		else if (strcmp(buffer, "delete") == 0)
			delMessage();
		else if (result == 0)
			break;
	}
}

void Server::serverClose()
{
	// �������� ����������
#ifdef _WIN32
	closesocket(clientsocket);
	closesocket(serversocket);
	WSACleanup();
	std::cout << "Connection closed" << std::endl;
#else
	close(clientsocket);
	close(serversocket);
	std::cout << "Connection closed" << std::endl;
#endif
}
