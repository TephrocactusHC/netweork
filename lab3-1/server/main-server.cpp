#include <WinSock2.h>
#include<iostream>
#include <stdlib.h>
#include <time.h>
#include <fstream>
using namespace std;
#pragma warning(disable:4996)
#pragma comment(lib,"ws2_32.lib")
#define SERVER_IP "127.0.0.1"
#define CLIENT_IP "127.0.0.1"
#define SERVER_PORT 8888
#define CLIENT_PORT 8080
#define BUFFER sizeof(message)
void SetColor(int fore = 7, int back = 0) {
    unsigned char m_color = fore;
    m_color += (back << 4);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), m_color);
    return;
}

int main()
{
    SetColor(14,0);
    Start();
    WaitConnect();
    getFileName();
    //¹Ø±ÕÌ×½Ó×Ö
    closesocket(Server);
    WSACleanup();
    system("pause");
    return 0;
}
