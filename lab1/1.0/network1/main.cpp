#include <iostream>
#include <winsock2.h>
#include<time.h>
#include<cstring>
#include<windows.h>
#pragma comment(lib, "ws2_32.lib")
using namespace std;
SOCKET socket_serve,socket_client;
SOCKADDR_IN addr_serve,addr_client;
int size_addr=sizeof(SOCKADDR_IN);
const int buf_size=2048;
char send_buf[buf_size];
char input_buf[buf_size];



void SetColor(int fore=7,int back=0)
{
    unsigned char m_color = fore;
    m_color += (back << 4);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), m_color);
    return;
}

int main() {
    int FIRST_JUDGE;
    WSADATA wsaData;
    FIRST_JUDGE = WSAStartup(MAKEWORD(2, 2), &wsaData);
    //指定版本wVersionRequested=MAKEWORD(a,b)。MAKEWORD()是一个宏把两个数字组合成一个WORD，无符号的短整型
    if (FIRST_JUDGE != 0) {
        //如果失败了
        SetColor(0,12);
        cout << "初始化socket都失败了宝贝儿，建议还是先回家再好好学学吧！(￢_￢)" << endl;
        return 0;
    }

    //创建socket。这里我们使用流式socket。
    socket_serve = socket(AF_INET, SOCK_STREAM, 0);
    //初始化客户端地址
    addr_serve.sin_addr.s_addr = inet_addr("10.130.122.241");//把我们本机的地址转换成网络字节二进制值序
    addr_serve.sin_family = AF_INET;//使用ipv4
    addr_serve.sin_port = htons(8000);//转换函数，也是转换成网络字节序。


    if (bind(socket_serve, (SOCKADDR *) &addr_serve, sizeof(SOCKADDR)) == -1) {
        SetColor(0,12);
        cout << "绑定这一步都失败了宝贝儿，建议还是先回家再好好学学吧！(￢_￢)" << endl;
        return 0;
    }
    //绑定
    bind(socket_serve, (SOCKADDR *) &addr_serve, sizeof(SOCKADDR));

    //监听
    listen(socket_serve, 5);
    socket_client = accept(socket_serve, (SOCKADDR *) &addr_client, &size_addr);

    //接受，返回的是一个socket
    if (socket_client != INVALID_SOCKET)//判断连接成功
    {
        SetColor(0,12);
        cout << "LOGGING: 成功连接上啦！(^_^)" << endl;
//        strcpy(send_buf, "你好啊！我是客户端！你已经连接到我了！");
//        send(socket_client, send_buf, 2048, 0);
    }
    else {
        SetColor(0,12);
        cout << "连接失败了宝贝儿，快查查是不是没启动客户端呢？|_(._.)_|" << endl;
        return 0;
    }

    while (1) {
        char receive_buf_loop[buf_size]={};
//        int NetTimeout = 500; //超时时长
//        setsockopt(socket_client, SOL_SOCKET,SO_RCVTIMEO,(char *)&NetTimeout,sizeof(int));
        recv(socket_client, receive_buf_loop,2048,0);
        //判断是否对方要退出
        if (strlen(receive_buf_loop) == 0)
        {
            cout << "客户端选择结束聊天了！再见哟！（//▽//）" << endl;
            break;
        }
        else {
            time_t now_time = time(NULL);
            tm *t_tm = localtime(&now_time);
            cout << "客户端发的：" << receive_buf_loop << "  " << "自己收到时间: " << asctime(t_tm);
        }
        cin.getline(input_buf, 2048, '\n');
        if (!strcmp("quit", input_buf))
        {
//            strcpy(send_buf, '\0');
            send(socket_client, {}, 2048, 0);
            cout << "您已选择结束聊天！（//▽//）" << endl;
            break;
        }
        else{
        strcpy(send_buf, input_buf);
        send(socket_client, send_buf, 2048, 0);
        }
        Sleep(30);
    }
    closesocket(socket_serve);
    WSACleanup();

}



