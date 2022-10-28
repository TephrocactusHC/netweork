#include <stdio.h>
#include <winsock2.h>
#include <iostream>
#include<time.h>
#include<string.h>
#include<string>
#include <windows.h>
#include <stdlib.h>

using namespace std;
const int chatnumber = 3;
SOCKET sockConn[chatnumber];
#pragma comment (lib, "ws2_32.lib")  //加载 ws2_32.dll

const int BUF_SIZE = 2048;
struct message {
    string id;
    string chat;
};

struct message parseMessage(char *s) {
    message msg;
    string str(s);
    int pos = str.find("mycNB!", 0, 6);
    msg.id = str.substr(pos + 6, 1);
    msg.chat = str.substr(pos + 7, str.length());
    return msg;
}

void SetColor(int fore = 7, int back = 0) {
    unsigned char m_color = fore;
    m_color += (back << 4);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), m_color);
    return;
}


DWORD WINAPI Recv(LPVOID thesocket) {
    char recvBuf[BUF_SIZE] = {};
    SOCKET sock = sockConn[(long long) thesocket];

    if (sock != INVALID_SOCKET) {
        while (1) {
        recv(sock, recvBuf, BUF_SIZE, 0);
        if (recvBuf[0]) {
            message msg = parseMessage(recvBuf);
            SetColor(14, 0);
            if (strcmp(msg.chat.data(), "imquit") == 0) {
                cout <<"有一位用户选择下线(*_*)，会议结束";
                for (int i = 0; i < chatnumber; i++) {
                    if (sock != sockConn[i]) {
                        send(sockConn[i], recvBuf, 2048, 0);}
                }
                closesocket(sockConn[(long long) thesocket]);
                closesocket(sock);
                sockConn[(long long) thesocket]=INVALID_SOCKET;
                system("pause");
                exit(0);
            } else {
                time_t now_time = time(NULL);
                tm *t_tm = localtime(&now_time);

                cout << asctime(t_tm) << "收到您的小可爱"<<msg.id<<"号发来的消息:";
                cout<<msg.chat<<endl;
                cout<<"-----------------------------------------------"<<endl;
                for (int i = 0; i < chatnumber; i++) {
                    if (sock != sockConn[i]) {
                        send(sockConn[i], recvBuf, 2048, 0);
                    }
                }
            }
            memset(recvBuf,0,BUF_SIZE);
        }
        else{return 0;}


    }
}
    return 0;
}


int main() {
    SetColor(0, 12);
    cout << "需要" <<chatnumber<<"位用户才能开始聊天哦亲！(^-^)"<< endl;
    SetColor();
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) ==
        0) //指定版本wVersionRequested=MAKEWORD(a,b)。MAKEWORD()是一个宏把两个数字组合成一个WORD，无符号的短整型
    {
        SetColor(0, 12);
        cout << "初始化socket成功了亲" << endl;
    } else {
        SetColor(0, 12);
        cout << "初始化socket都失败了宝贝儿，你穆哥建议还是先回家再好好学学吧！(￢_￢)" << endl;
        return 0;
    }

    //创建套接字
    SOCKET sockSrv = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addrSrv;
    memset(&addrSrv, 0, sizeof(addrSrv));  //每个字节都用0填充
    addrSrv.sin_family = AF_INET;  //使用IPv4地址
    addrSrv.sin_addr.s_addr = inet_addr("127.0.0.1");  //把我们本机的地址转换成网络字节二进制值序
    addrSrv.sin_port = htons(8000);  //端口,好像是在一个区间之内就可以.//转换函数，也是转换成网络字节序
    int z = bind(sockSrv, (SOCKADDR *) &addrSrv, sizeof(addrSrv));
    if (z == -1) {
        SetColor(0, 12);
        cout << "绑定这一步都失败了宝贝儿，建议还是先回家再好好学学吧！(￢_￢)" << endl;
        return 0;
    } else {
        SetColor(0, 12);
        cout << "绑定成功了亲(^_^)" << endl;
    }
    //进入监听状态
    //接收客户端请求
    SOCKADDR addrCli[2];
    int nSize = sizeof(SOCKADDR);

    if (listen(sockSrv, 5) == 0) {
        SetColor(0, 12);
        cout << "正在监听之中哦亲(^_^)" << endl;
        for (int i = 0; i < chatnumber; i++) {
            //为每个客户端分配一个socket连接，将客户端的相关信息存储在addrCli中
            sockConn[i] = accept(sockSrv, (SOCKADDR *) &addrCli[i], &nSize);
            if (sockConn[i] != INVALID_SOCKET)
            {
                cout << "用户" << i << "进入聊天" << endl;
                char buf[12] = "你的id是：";
                buf[10] = 48 + i;
                buf[11] = 0;
                send(sockConn[i], buf, 50, 0);
            }
        }

    } else {
        SetColor(0, 12);
        cout << "监听都失败了宝贝儿，你穆哥建议还是先回家再好好学学吧！(￢_￢)" << endl;
        return 0;
    }

    //开启多线程
    HANDLE hThread;
    while (1) {
        for (int i = 0; i < chatnumber; i++) {
                hThread = CreateThread(NULL, 0, Recv, LPVOID(i), 0, NULL);
                WaitForSingleObject(hThread, 2000);
                CloseHandle(hThread);
        }

    }
    closesocket(sockSrv);
    WSACleanup();
    return 0;
}