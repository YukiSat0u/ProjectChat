#include "Chat.h"

void menu()
{
    int choice = 0;
    int id = -1;
    while (choice < 1 || choice > 3)
    {
        std::cout << "1 - �����������/����\n2 - ���\n3 - �����" << std::endl;
        std::cin >> choice;

        switch (choice)
        {
            //�����������/����
        case 1:
            //user;
            if (id == 0)
                return;//registration;
            else if (id > 0)
                return;//login;
            else
            {
                std::cout << "������������ id" << std::endl;
                choice = 0;
            }
            break;

            //���������
        case 2:
        {
            int choice2 = 0;
            while (choice2 < 1 || choice2 > 3)
            {
                std::cout << "1 - �������� ���������\n2 - �������� ���������\n3 - �����" << std::endl;
                std::cin >> choice2;

                switch (choice2)
                {
                case 1:
                    return; //writeMessage;
                    break;
                case 2:
                    return; //getMessage;
                    break;
                case 3:
                    choice = 0;
                    break;
                default:
                    std::cout << "�������� ��������!" << std::endl;
                    break;
                }
            }
            break;
        }
        //�����
        case 3:
            exit(0);
            break;
        default:
            std::cout << "�������� ��������!" << std::endl;
            break;
        }
    }
}