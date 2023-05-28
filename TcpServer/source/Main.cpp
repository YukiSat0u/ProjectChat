#include <iostream>
#include <string>
#include <fstream>
#include <winsock2.h>
#include <WS2tcpip.h>

int main() {
	std::setlocale(LC_ALL, "");
	// ������������� ���������� Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (result != 0) {
		std::cerr << "������ ��� ������������� Winsock" << std::endl;
		return 1;
	}

	// �������� TCP ������
	SOCKET serversocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// ���������� ������ � ��������� ������ � ������
	sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(8000);
	result = bind(serversocket, (sockaddr*)&server_address, sizeof(server_address));
	if (result != 0) {
		std::cerr << "������ ��� ���������� ������" << std::endl;
		closesocket(serversocket);
		WSACleanup();
		return 1;
	}

	// ������������� �������� ����������
	result = listen(serversocket, SOMAXCONN);
	if (result != 0) {
		std::cerr << "������ ��� ������������� ����������" << std::endl;
		closesocket(serversocket);
		WSACleanup();
		return 1;
	}

	// �������� ����������� �������
	sockaddr_in client_address;
	int client_address_len = sizeof(client_address);
	SOCKET clientsocket = accept(serversocket, (sockaddr*)&client_address, &client_address_len);
	char client_ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(client_address.sin_addr), client_ip, INET_ADDRSTRLEN);
	std::cout << "����������� �� " << client_ip << ":" << ntohs(client_address.sin_port) << " ���� ������� �����������" << std::endl;

	while (true) {
		// ��������� ��������� �� �������
		char buffer[1024] = { 0 };
		result = recv(clientsocket, buffer, 1024, 0);
		if (result == SOCKET_ERROR) {
			std::cerr << "������ ��� ������ ���������" << std::endl;
			break;
		}
		else if (result == 0) {
			std::cout << "������ ������ ����������" << std::endl;
			break;
		}
		std::string data = buffer;
		std::cout << "���������: " << data << std::endl;

		// �������� ������ �������
		std::string response = "��������� ����������";
		result = send(clientsocket, response.c_str(), response.length(), 0);
		if (result == SOCKET_ERROR) {
			std::cerr << "������ ��� �������� ���������" << std::endl;
			break;
		}
	}

	// �������� ����������
	closesocket(clientsocket);
	closesocket(serversocket);
	WSACleanup();

	return 0;
}