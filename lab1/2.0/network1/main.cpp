#include <stdio.h>
#include <winsock2.h>
#include <iostream>
#include<time.h>
#include<string.h>
#include<string>
#include <windows.h>
#include <stdlib.h>
using namespace std;
#pragma comment (lib, "ws2_32.lib")  //加载 ws2_32.dll

const int BUF_SIZE=2048;

//string GbkToUtf8(const char *src_str)
//{
//    int len = MultiByteToWideChar(CP_ACP, 0, src_str, -1, NULL, 0);
//    wchar_t* wstr = new wchar_t[len + 1];
//    memset(wstr, 0, len + 1);
//    MultiByteToWideChar(CP_ACP, 0, src_str, -1, wstr, len);
//    len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
//    char* str = new char[len + 1];
//    memset(str, 0, len + 1);
//    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
//    string strTemp = str;
//    if (wstr) delete[] wstr;
//    if (str) delete[] str;
//    return strTemp;
//}
//
//string Utf8ToGbk(const char *src_str)
//{
//    int len = MultiByteToWideChar(CP_UTF8, 0, src_str, -1, NULL, 0);
//    wchar_t* wszGBK = new wchar_t[len + 1];
//    memset(wszGBK, 0, len * 2 + 2);
//    MultiByteToWideChar(CP_UTF8, 0, src_str, -1, wszGBK, len);
//    len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
//    char* szGBK = new char[len + 1];
//    memset(szGBK, 0, len + 1);
//    WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
//    string strTemp(szGBK);
//    if (wszGBK) delete[] wszGBK;
//    if (szGBK) delete[] szGBK;
//    return strTemp;
//}
void SetColor(int fore=7,int back=0)
{
    unsigned char m_color = fore;
    m_color += (back << 4);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), m_color);
    return;
}
string parseMessage (char *s) {
string str(s);
int pos = str.find ( "mycNB!" , 0 , 6) ;
if ( pos == -1) {
exit(0);
}

string chat = str.substr (pos + 6,str.length ()) ;
return chat ;
}




DWORD WINAPI Send(LPVOID thesocket) {
    SOCKET * sock = (SOCKET*)thesocket;
    char sendBuf[BUF_SIZE] = {};
    char inputBuf[BUF_SIZE] = {};
    while (1) {
        //printf("Input a string: ");
        cin.getline(inputBuf, 2048, '\n');
        strcpy(sendBuf, "mycNB!");
        strcat(sendBuf, inputBuf);
        int t = send(*sock, sendBuf, strlen(sendBuf), 0);
        if (strcmp(inputBuf, "imquit") == 0)
        {
            SetColor(0,12);
            SYSTEMTIME st = { 0 };
            GetLocalTime(&st);
            closesocket(*sock);
            cout << "您已于" << st.wMonth << "月"<< st.wDay << "日" << st.wHour << "时" << st.wMinute << "分" << st.wSecond << "秒选择结束聊天了！再见哟！（//▽//）" << endl;
            Sleep(10000000);
            exit (0);
            return 0;
        }
        if (t > 0) {
            SetColor(1,0);
            SYSTEMTIME st = { 0 };
            GetLocalTime(&st);
            cout << "消息已于" << st.wHour << "时" << st.wMinute << "分" << st.wSecond << "秒成功发送\n";
            cout << "-------------------------------------------------------------" << endl;
        }
        memset(sendBuf, 0, BUF_SIZE);
    }
}


DWORD WINAPI Recv(LPVOID thesocket) {
    char recvBuf[BUF_SIZE] = {};
    SOCKET *sock = (SOCKET*)thesocket;
    while (1) {
        int t = recv(*sock, recvBuf, BUF_SIZE, 0);
        if (strcmp(parseMessage(recvBuf).data(), "imquit") == 0)
        {
            SetColor(0,12);
            SYSTEMTIME st = { 0 };
            GetLocalTime(&st);
            closesocket(*sock);
            cout << "客户端已于" << st.wMonth << "月"<< st.wDay << "日" << st.wHour << "时" << st.wMinute << "分" << st.wSecond << "秒选择结束聊天了！再见哟！（//▽//）" << endl;
            exit (0);
            return 0L;
        }
        if (t > 0) {
            time_t now_time = time(NULL);
            tm *t_tm = localtime(&now_time);
            SetColor(14,0);
            cout << asctime(t_tm) << "收到您的小可爱(服务端)发来的消息:";
            cout<<parseMessage (recvBuf)<<endl;
            cout << "-------------------------------------------------------------" << endl;
        }
        memset(recvBuf, 0, BUF_SIZE);
    }
}




int main() {
    SetColor();
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0) //指定版本wVersionRequested=MAKEWORD(a,b)。MAKEWORD()是一个宏把两个数字组合成一个WORD，无符号的短整型
    {
        SetColor(0,12);
        cout << "初始化socket成功了亲" << endl;
    }
    else {
        SetColor(0,12);
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
    int z=bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(addrSrv));
    if (z == -1) {
        SetColor(0,12);
        cout << "绑定这一步都失败了宝贝儿，建议还是先回家再好好学学吧！(￢_￢)" << endl;
        return 0;
    }
    else{
        SetColor(0,12);
        cout<<"绑定成功了亲(^_^)" << endl;
    }
    //进入监听状态
    if (listen(sockSrv, 5) == 0) {
        SetColor(0,12);
        cout << "正在监听之中哦亲(^_^)" << endl;
    }
    else {
        SetColor(0,12);
        cout << "监听都失败了宝贝儿，你穆哥建议还是先回家再好好学学吧！(￢_￢)" << endl;
        return 0;
    }

    //接收客户端请求
    SOCKADDR addrClient;
    int nSize = sizeof(SOCKADDR);
    SOCKET ClientSocket = accept(sockSrv, (SOCKADDR*)&addrClient, &nSize);
    if (ClientSocket > 0) {
        SetColor(0,12);
        cout << "您的小宝贝(客户端)已上线(^_^)" << endl;
    }
    else {
        SetColor(0,12);
        cout << "连接失败了宝贝儿，快查查是不是没启动客户端呢？|_(._.)_|" << endl;
        return 0;
    }
    //开启多线程
    HANDLE hThread[2];
//    while(1) {
        hThread[0] = CreateThread(NULL, 0, Recv, (LPVOID) &ClientSocket, 0, NULL);
        hThread[1] = CreateThread(NULL, 0, Send, (LPVOID) &ClientSocket, 0, NULL);
        WaitForMultipleObjects(2, hThread, TRUE, INFINITE);
        CloseHandle(hThread[0]);
        CloseHandle(hThread[1]);

//    }
    closesocket(ClientSocket);  //关闭套接字

    //关闭套接字
    closesocket(sockSrv);

    //终止 DLL 的使用
    WSACleanup();
    return 0;
}


