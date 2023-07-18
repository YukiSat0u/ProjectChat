#include "../include/Server.h"

int Server::init()
{
#ifdef _WIN32
	result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (result != 0) {
		std::cerr << "Ошибка при инициализации Winsock" << std::endl;
		return 1;
	}
#endif

	// создание TCP сокета
	serversocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// связывание сокета с локальным хостом и портом
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(8000);
	result = bind(serversocket, (sockaddr*)&server_address, sizeof(server_address));

#ifdef _WIN32
	if (result != 0) {
		std::cerr << "Ошибка при связывании сокета" << std::endl;
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

	// прослушивание входящих соединений
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

	mysql_init(&mysql);
	if (&mysql == nullptr)
	{
		// Если дескриптор не получен — выводим сообщение об ошибке
		std::cout << "Error: can't create MySQL-descriptor" << std::endl;
		return 1;
	}

	// Подключаемся к серверу
	if (!mysql_real_connect(&mysql, "localhost", "root", "1237", "chatdb", NULL, NULL, 0))
	{
		// Если нет возможности установить соединение с БД выводим сообщение об ошибке
		std::cout << "Error: can't connect to database " << mysql_error(&mysql) << std::endl;
		return 1;
	}
	else
	{
		// Если соединение успешно установлено выводим фразу — "Database connected!"
		std::cout << "Database connected!" << std::endl;
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

void Server::singUp()
{
	char buffer[1024] = { 0 };
	bool result = true;
	result = recv(clientsocket, buffer, 1024, 0);
	std::string login, password, name, gender, line;
	std::vector<std::string> lines;
	std::istringstream iss = (std::istringstream)buffer;
	if (strcmp(buffer, "singup") == 0)
	{
		lines.clear();
		result = recv(clientsocket, buffer, 1024, 0);
		iss = (std::istringstream)buffer;
	}
	while (std::getline(iss, line, ' '))
		lines.push_back(line);

	login = lines[0];
	password = lines[1];
	name = lines[2];
	gender = lines[3];

	mysql_query(&mysql, "SELECT login, password, name, gender FROM users");
	res = mysql_store_result(&mysql);
	while (row = mysql_fetch_row(res))
	{
		if (row[0] == login)
		{
			send(clientsocket, "Логин занят", 1024, 0);
			result = false;
			break;
		}
	}
	if (result)
	{
		std::string query = "INSERT INTO users (login, password, name, gender) VALUES ('" + login + "', '" + password + "', '" + name + "', '" + gender + "')";

		if (mysql_query(&mysql, query.c_str()) != 0)
		{
			std::cerr << "Ошибка при выполнении запроса: " << mysql_error(&mysql) << std::endl;
			mysql_close(&mysql);
			return;
		}

		std::cout << "Пользователь успешно зарегистрирован!" << std::endl;
		send(clientsocket, "Пользователь успешно зарегистрирован!", 1024, 0);
	}
	lines.clear();
}

void Server::sentMessage()
{
	char buffer[1024] = { 0 };
	recv(clientsocket, buffer, 1024, 0);
	std::string currentUser = buffer;
	std::string messages;
	std::string query = "SELECT from_name, to_name, text FROM messages WHERE from_name = '" + currentUser + "' OR to_name = '" + currentUser + "' OR to_name = 'All'";
	mysql_query(&mysql, query.c_str());
	if (res = mysql_store_result(&mysql))
	{
		while (row = mysql_fetch_row(res))
		{
			std::string fromName = row[0];
			std::string toName = row[1];
			std::string text = row[2];
			// Формирование строки сообщения
			std::string message = "From: " + fromName + "\nTo: " + toName + "\nText: " + text + "\n\n";
			messages += message;
		}

		// Освобождение результата запроса
		mysql_free_result(res);

		// Отправка сообщений клиентам
		int bytes_sent = send(clientsocket, messages.c_str(), messages.size(), 0);
		if (bytes_sent < 0)
		{
			perror("Failed to send message");
			exit(EXIT_FAILURE);
		}

		std::cout << "Chat:\n" << messages << "Chat end." << std::endl;
	}
	else
		// Отправка пустого сообщения клиенту
		send(clientsocket, " ", 1, 0);
}

void Server::delMessage()
{
	char buffer[1024] = { 0 };
	recv(clientsocket, buffer, 1024, 0);

	std::string message_to_delete = buffer;

	std::string from, to, text;
	std::istringstream iss = (std::istringstream)buffer;

	iss >> from >> to;

	std::getline(iss, text);
	text = text.substr(1);

	std::string query = "SELECT * FROM messages WHERE from_name = '" + from + "' AND to_name = '" + to + "' AND text = '" + text + "'";
	
	mysql_query(&mysql, query.c_str());
	res = mysql_store_result(&mysql);

	if (mysql_num_rows(res) > 0)
	{
		// Удаление сообщения
		std::string deleteQuery = "DELETE FROM messages WHERE from_name = '"
			+ from + "' AND to_name = '" + to + "' AND text = '" + text + "'";
		mysql_query(&mysql, deleteQuery.c_str());

		send(clientsocket, "Message deleted!", 1024, 0);
	}
	else
		send(clientsocket, "Message not found!", 1024, 0);
}

void Server::recvMessage()
{
	// получение сообщения от клиента
	char buffer[1024] = { 0 };
	result = recv(clientsocket, buffer, 1024, 0);
#ifdef _WIN32
	if (result == SOCKET_ERROR) {
		int error = WSAGetLastError();
		if (error == WSAECONNRESET) {
			std::cout << "Соединение было непредвиденно разорвано." << std::endl;
		}
		else {
			std::cerr << "Ошибка при чтении сообщения: " << error << std::endl;
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
	std::string from, to, text;
	std::istringstream iss = (std::istringstream)buffer;

	iss >> from >> to;

	std::getline(iss, text);
	text = text.substr(1);

	std::string query = "INSERT INTO messages (from_name, to_name, text) VALUES ('" + from + "', '" + to + "', '" + text +"')";

	if (mysql_query(&mysql, query.c_str()) != 0)
	{
		std::cerr << "Ошибка при выполнении запроса: " << mysql_error(&mysql) << std::endl;
		mysql_close(&mysql);
		return;
	}
	else 
		std::cout << "Message received: " << buffer << std::endl << std::endl;
}

void Server::serverUpdate()
{
	while (true)
	{
		char buffer[1024] = { 0 };
		result = recv(clientsocket, buffer, 1024, 0);
		if (strcmp(buffer, "singup") == 0)
			singUp();
		else if (strcmp(buffer, "send") == 0)
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
	// закрытие соединений
#ifdef _WIN32
	mysql_close(&mysql);
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