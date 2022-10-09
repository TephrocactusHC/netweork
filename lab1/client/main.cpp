#include <iostream>
#include <winsock2.h>
#include<time.h>
#include<windows.h>
#include<cstring>
#pragma comment(lib, "ws2_32.lib")
using namespace std;
SOCKET socket_client1;
SOCKADDR_IN addr_serve1,addr_client1;
const int buf_size=2048;//缓冲区大小常量
char send_buf1[buf_size];//发送缓冲区
char receive_buf1[buf_size];//接受缓冲区
char input_buf1[buf_size];

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
        //等于没有链接成功，失败了。
        SetColor(0,12);
        cout << "初始化socket都失败了宝贝儿，你穆哥建议还是先回家再好好学学吧！(￢_￢)" << endl;
        return 0;
    }

    //创建socket。这里我们使用流式socket。
    socket_client1 = socket(AF_INET, SOCK_STREAM, 0);

    //初始化客户端地址
    addr_client1.sin_addr.s_addr = inet_addr("127.0.0.1");//把我们本机的地址转换成网络字节二进制值序
    addr_client1.sin_family = AF_INET;//使用ipv4
    addr_client1.sin_port = htons(8000);//转换函数，也是转换成网络字节序。
    //初始化地址
    addr_serve1.sin_addr.s_addr = inet_addr("10.130.122.241");//把我们本机的地址转换成网络字节二进制值序
    addr_serve1.sin_family = AF_INET;//使用ipv4
    addr_serve1.sin_port = htons(8000);//转换函数，也是转换成网络字节序。
    if (connect(socket_client1,(SOCKADDR *) &addr_serve1,sizeof(addr_serve1) )!=SOCKET_ERROR)
    {
        strcpy(send_buf1, "你好啊！我是客户端！我已经连接到你了！");
        send(socket_client1, send_buf1, 2048, 0);
    }
    else {
        SetColor(0,12);
        cout << "连接失败了宝贝儿，快查查是不是没启动服务端呢？|_(._.)_|" << endl;
        return 0;
    }

    while (1) {
        SetColor(0,12);
        recv(socket_client1,receive_buf1,sizeof(receive_buf1),0);
        //判断是否对方要退出
        if (strlen(receive_buf1) == 0)
        {
            cout << "服务器选择结束聊天了！再见哟！（//▽//）" << endl;
            break;
        }
        else {
            time_t now_time = time(NULL);
            tm *t_tm = localtime(&now_time);
            cout << "服务端发的：" << receive_buf1 << "  " << "收到时间: " << asctime(t_tm);
        }
        cin.getline(input_buf1, 2048, '\n');
        if (!strcmp("quit", input_buf1))
        {
            send(socket_client1, {}, 2048, 0);
            cout << "您已选择结束聊天！（//▽//）" << endl;
            break;
        }
        else{
        strcpy(send_buf1, input_buf1);
        send(socket_client1, send_buf1, 2048, 0);
        }
        Sleep(30);
    }
    closesocket(socket_client1);
    WSACleanup();

}
