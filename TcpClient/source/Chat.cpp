#include "../include/Chat.h"

int Chat::bd()
{
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

void Chat::tcpConnect()
{
#ifdef _WIN32
	result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (result != 0) {
		std::cerr << "Ошибка при инициализации Winsock" << std::endl;
		exit(1);
	}
#endif

	clientsocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_address.sin_port = htons(8000);
	result = connect(clientsocket, (sockaddr*)&server_address, sizeof(server_address));
#ifdef _WIN32
	if (result == SOCKET_ERROR) {
		std::cerr << "Error connecting to server" << std::endl;
		closesocket(clientsocket);
		WSACleanup();
		exit(1);
	}
#else
    if (result == -1)
    {
        std::cerr << "Error connecting to server" << std::endl;
        close(clientsocket);
        exit(1);
    }
#endif

}

void Chat::startChat()
{
	_isChatWork = true;
}

void Chat::showLoginMenu()
{
	_currentUser = nullptr;
	char operation;
	std::cout << "\t\tChat 2.0 is run.\n\a";
	do
	{
		std::cout << "\n(1)SignUp\t";
		std::cout << "(2)Login\t";
		std::cout << "(0)ShutDown\n>> ";
		std::cin >> operation;

		switch (operation)
		{
		case '1':
			try
			{
				singUp();
			}
			catch (const std::exception& e)
			{
				std::cout << e.what() << std::endl;
			}
			break;
		case '2':
			login();
			break;
		case'0':
			_isChatWork = false;
			break;
		default:
			std::cout << "1 or 2..." << std::endl;
			break;
		}
	} while (!_currentUser && _isChatWork);
}

void Chat::showUserMenu()
{
	char operation;
	std::cout << "Hi, " << _currentUser->getUserName()<<".\n";

	while (_currentUser)
	{
		std::cout << "Menu:(1)Show chat | (2)Add message | (3)Users | (4)Delete message | (0)Logout" << "\n>> ";
		std::cin >> operation;
		switch (operation)
		{
		case '1':
			showChat();
			break;
		case '2':
			addMessage();
			break;
		case '3':
			showAllUsersName();
			break;
		case '4':
			deleteMessage();
			break;
		case '0':
			_currentUser = nullptr;
			break;
		default:
			std::cout << "Unknown choice.." << std::endl;
			break;
		}
	}
}

void Chat::showAllUsersName()
{
#ifdef _WIN32
	std::locale::global(std::locale("en_US.UTF-8"));
	uint32_t Spades = 0x2642;
	uint32_t Spades1 = 0x2640;
#endif

	std::cout << "--- Users ---" << std::endl;

	std::string queryName = "SELECT name FROM users";
	mysql_query(&mysql, queryName.c_str());
	if (res = mysql_store_result(&mysql)) 
	{
		while (row = mysql_fetch_row(res))
		{
			for (int i = 0; i < mysql_num_fields(res); i++) 
			{
				std::cout << row[i];
				if (row[i] == _currentUser->getUserName())
				{
					std::cout << "(me)";
				}
				std::cout << std::endl;
			}
		}
	}

	/*std::string queryGender = "SELECT gender FROM users";
	mysql_query(&mysql, queryGender.c_str());
	if (res = mysql_store_result(&mysql))
	{
		while (row = mysql_fetch_row(res))
		{
			for (int i = 0; i < mysql_num_fields(res); i++)
			{
#ifdef _WIN32
				if (user.getUserGender() == "Male")
					std::wcout << (wchar_t)Spades << " ";
				else if (user.getUserGender() == "Female")
					std::wcout << (wchar_t)Spades1 << " ";
#else
				if (user.getUserGender() == "Male")
					std::cout << "M ";
				else if (user.getUserGender() == "Female")
					std::cout << "W ";
#endif
			}
		}
	}*/

	std::cout << "----------" << std::endl;
}

void Chat::singUp()
{
	char buffer[1024] = {};
	std::string requestSingUp = "singup";
	int requestLength = requestSingUp.length();
	send(clientsocket, requestSingUp.c_str(), requestLength, 0);

	std::string login, name, gender, password;
	std::cout << "Login: ";
	std::cin >> login;
	std::cout << "Password: ";
	std::cin >> password;
	std::cout << "Name: ";
	std::cin >> name;
	std::cout << "Gender:";

	do
	{
		std::cout << "\n(Male,Female) ";
		std::cin >> gender;
		if (!(gender == "Male" || gender == "Female"))
			std::cout << "Enter Male or Female";
	} while (!(gender == "Male" || gender == "Female"));

	if (login == "All")
	{
		_currentUser = nullptr;
		throw UserLoginExp();
	}
	if (name == "All")
	{
		_currentUser = nullptr;
		throw UserNameExp();
	}

	std::string users = login + " " + password + " " + name + " " + gender;
	int usersLength = users.length();
	send(clientsocket, users.c_str(), usersLength, 0);

	recv(clientsocket, buffer, sizeof(buffer), 0);
	std::cout << buffer << std::endl;

	if (strcmp(buffer, "Логин занят") == 0)
		_currentUser = nullptr;
	else
	{
		auto getUser = getUserByLogin(login);
		_currentUser = getUser;
	}
}

void Chat::login()
{
	std::string login;
	std::string password;
	char operation;

	do
	{
		std::cout << "Login: ";
		std::cin >> login;

		_currentUser = getUserByLogin(login);
		if (_currentUser == nullptr)
		{
			std::cout << "Login invalid." << std::endl;
			std::cout << "(0)exit or any key to return menu: ";
			std::cin >> operation;

			if (operation == '0')
				break;

			continue;
		}

		std::cout << "Password: ";
		std::cin >> password;

		if (password != _currentUser->getUserPassword())
		{
			_currentUser = nullptr;

			std::cout << "Login invalid." << std::endl;
			std::cout << "(0)exit or any key to return menu: ";
			std::cin >> operation;

			if (operation == '0')
				break;
		}

	} while (!_currentUser);
}

void Chat::showChat()
{
	std::string requestRecv = "recv";
	int requestLength = requestRecv.length();
	send(clientsocket, requestRecv.c_str(), requestLength, 0);

	std::string userName;

	userName = _currentUser->getUserName();
	int userNameLength = userName.length();
	send(clientsocket, userName.c_str(), userNameLength, 0);

	char buffer[1024] = {};

	std::cout << "____START____ "<< std::endl << std::endl;

	while (true) 
	{
		int recv_size = recv(clientsocket, buffer, sizeof(buffer), 0);
		if (recv_size < 0) {
			std::cout << "Ошибка при чтении сообщения" << std::endl;
			break;
		}
		else if (recv_size == 0) {
			std::cout << "Сервер закрыл соединение" << std::endl;
			break;
		}
		else 
		{
			std::cout << buffer;
			break;
		}
	}
	std::cout << "____END____ " << std::endl << std::endl;
}

void Chat::sendMessage(SOCKET clientSocket, const std::string& login, const std::string& to, const std::string& text) 
{
	std::string message = _currentUser->getUserName() + " " + to + " " + text;
	int messageLength = message.length();

	// Отправляем сообщение на сервер
	send(clientSocket, message.c_str(), messageLength, 0);
}

void Chat::addMessage()
{
	std::string requestSend = "send";
	int requestLength = requestSend.length();
	send(clientsocket, requestSend.c_str(), requestLength, 0);

	std::string to, text;

	std::cout << "To (name or All): ";
	std::cin >> to;
	std::cout << "Text: ";
	std::cin.ignore();
	std::getline(std::cin, text);

	if (strToUpper(to) == "ALL")
	{
		Message message(_currentUser->getUserLogin(), "All", text);
		_messages.push_back(message);
		sendMessage(clientsocket, _currentUser->getUserLogin(), "All", text); // Отправляем сообщение на сервер
	}
	else
	{
		std::string login, password, name, gender;

		std::string queryName = "SELECT name FROM users WHERE name = '" + to + "'";
		mysql_query(&mysql, queryName.c_str());
		res = mysql_store_result(&mysql);

		bool found = false;

		while (row = mysql_fetch_row(res))
		{
			if (strToUpper(row[0]) == strToUpper(to))
			{
				found = true;
				Message message(_currentUser->getUserLogin(), to, text);
				_messages.push_back(message);

				sendMessage(clientsocket, _currentUser->getUserName(), getUserByName(to)->getUserName(), text); // Отправляем сообщение на сервер
				break;
			}
		}
		if (!found)
		{
			std::cout << "Error: user not found\n";
			return;
		}
	}
}

void Chat::deleteMessage()
{
	char buffer[1024] = {};
	std::string requestDelete = "delete";
	int requestLength = requestDelete.length();
	send(clientsocket, requestDelete.c_str(), requestLength, 0);

	std::string to, text_to_remove;

	std::cout << "To (name or All): ";
	std::cin >> to;
	std::cout << "Text: ";
	std::cin.ignore();
	std::getline(std::cin, text_to_remove);

	std::ifstream users_file("users.txt");

	if (strToUpper(to) == "ALL")
	{
		sendMessage(clientsocket, _currentUser->getUserLogin(), "All", text_to_remove); // Отправляем сообщение на сервер
	}
	else
	{
		std::string login, password, name, gender;

		bool found = false;
		while (users_file >> login >> password >> name >> gender)
		{
			if (strToUpper(name) == strToUpper(to))
			{
				found = true;
				sendMessage(clientsocket, _currentUser->getUserLogin(), getUserByName(to)->getUserName(), text_to_remove); // Отправляем сообщение на сервер
				break;
			}
		}
		if (!found)
		{
			std::cout << "Error: user not found\n";
			return;
		}
	}
	send(clientsocket, text_to_remove.c_str(), 1024, 0);

	int recv_size = recv(clientsocket, buffer, sizeof(buffer), 0);
	if (recv_size < 0)
		std::cout << "Ошибка при чтении сообщения" << std::endl;
	else if (recv_size == 0)
		std::cout << "Сервер закрыл соединение" << std::endl;
	else
		std::cout << buffer << std::endl << std::endl;
}

std::shared_ptr<User> Chat::getUserByLogin(const std::string& login)
{
	std::string password, name, gender;
	std::string query = "SELECT login, password, name, gender FROM users WHERE login = '" + login + "'";
	mysql_query(&mysql, query.c_str());

	if (res = mysql_store_result(&mysql))
	{
		while (row = mysql_fetch_row(res))
		{
			if (row[0] == login)
			{
				password = row[1];
				name = row[2];
				gender = row[3];

				return std::make_shared<User>(login, password, name, gender);
			}
			else
				return nullptr;
		}
	}
	return nullptr;
}

std::shared_ptr<User> Chat::getUserByName(const std::string& name)
{
	/*for (auto& user : _users)
	{
		if (name == user.getUserName())
			return std::make_shared<User>(user);
	}
	return nullptr;*/

	std::string query = "SELECT name FROM users WHERE name = '" + name + "'";
	mysql_query(&mysql, query.c_str());

	if (res = mysql_store_result(&mysql))
	{
		while (row = mysql_fetch_row(res))
		{
			if (row[0])
				return std::make_shared<User>(name);
			else
				return nullptr;
		}
	}
	else
		return nullptr;
}

std::fstream& operator >>(std::fstream& is, User& obj)
{
	is >> obj._name;
	is >> obj._login;
	is >> obj._password;
	is >> obj._gender;
	return is;
}
std::ostream& operator <<(std::ostream& os, const User& obj)
{
	os << obj._login;
	os << ' ';
	os << obj._password;
	os << ' ';
	os << obj._name;
	os << ' ';
	os << obj._gender;
	return os;
}

std::ostream& operator <<(std::ostream& os, const Message& msg)
{
	os << msg._from;;
	os << ' ';
	os << msg._to;
	os << ' ';
	os << msg._text;
	return os;
}

std::string Chat::strToUpper(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(),
		[](unsigned char c) { return toupper(c); });
	return str;
}
