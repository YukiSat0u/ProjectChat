#include "../include/Chat.h"

int main()
{
	setlocale(LC_ALL, "");

	Chat chat;
	
	chat.bd();
	chat.tcpConnect();

	chat.startChat();

	while (chat.isChatWork())
	{
		chat.showLoginMenu();
		
		while (chat.getCurrentUser())
		{
			chat.showUserMenu();
		}
	}
#ifdef _WIN32
	closesocket(chat.clientsocket);
	WSACleanup();
#else
    close(chat.clientsocket);
#endif

	return 0;
}
