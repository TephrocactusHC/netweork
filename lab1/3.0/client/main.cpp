#include <WinSock2.h>
#include <iostream>
#include <thread>
#include<string>
using namespace std;
#pragma comment(lib, "ws2_32.lib")  //加载 ws2_32.dll
int ID;
const int BUF_SIZE=2048;
char send_buf1[BUF_SIZE];//发送缓冲区

struct message{
    string id;
    string chat;
};
void SetColor(int fore=7,int back=0)
{
    unsigned char m_color = fore;
    m_color += (back << 4);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), m_color);
    return;
}
struct message parseMessage(char *s) {
    message msg;
    string str(s);
    int pos = str.find ( "mycNB!" , 0 , 6) ;
    msg.id = str.substr(pos + 6,1) ;
    msg.chat = str.substr (pos + 7,str.length ()) ;
    return msg;
}


DWORD WINAPI Send(LPVOID thesocket) {
    SOCKET * sock = (SOCKET*)thesocket;
    char sendBuf[BUF_SIZE] = {};
    char inputBuf[BUF_SIZE] = {};
    while (1) {
        //printf("Input a string: ");
        cin.getline(inputBuf, 2048, '\n');
        strcpy(sendBuf, "mycNB!");
        strcat(sendBuf,to_string(ID).c_str());
        strcat(sendBuf, inputBuf);
        send(*sock, sendBuf, strlen(sendBuf), 0);
        if (strcmp(inputBuf, "imquit") == 0)
        {
            SYSTEMTIME st = { 0 };
            GetLocalTime(&st);
            closesocket(*sock);
            SetColor(0,12);
            cout << "您已于" << st.wHour << "时" << st.wMinute << "分" << st.wSecond << "秒选择结束聊天了！再见哟！（//▽//）" << endl;

            Sleep(1000);
            exit(0);
//printf("您已于%s时%s分%s秒选择结束聊天了！再见哟！（//▽//）",st.wHour,st.wMinute,st.wSecond);
        }
        if (sendBuf[0]) {
            SetColor(1,0);
            SYSTEMTIME st = { 0 };
            GetLocalTime(&st);
            cout << "消息已于" << st.wHour << "时" << st.wMinute << "分" << st.wSecond << "秒成功发送\n" ;
            cout << "-------------------------------------------------------------" << endl;
        }
        memset(sendBuf, 0, BUF_SIZE);
    }
}


DWORD WINAPI Recv(LPVOID thesocket) {
    char recvBuf[BUF_SIZE] = { 0 };
    SOCKET *sock = (SOCKET*)thesocket;
    while (1) {
        SetColor();
        int t = recv(*sock, recvBuf, BUF_SIZE, 0);
        if (t > 0) {

            time_t now_time = time(NULL);
            tm *t_tm = localtime(&now_time);
            SetColor(14,0);

            message msg=parseMessage(recvBuf);
            if (strcmp(msg.chat.data(), "imquit") == 0)
            {
                SetColor(0,12);
                SYSTEMTIME st = { 0 };
                GetLocalTime(&st);
                closesocket(*sock);
                cout << "您的小宝贝"<<msg.id<<"号已于" << st.wMonth <<"月"<< st.wDay << "日" << st.wHour << "时" << st.wMinute << "分" << st.wSecond << "说再见了哟！（//▽//）" << endl;
                Sleep(1000);
                exit(0);}

            else{
            cout << asctime(t_tm) << "收到您的小可爱"<<msg.id<<"号发来的消息:";
            cout<<msg.chat<<endl;
            cout << "-------------------------------------------------------------" << endl;
        }
        memset(recvBuf, 0, BUF_SIZE);
    }}
}


int main() {
    SetColor();
    //初始化DLL
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0)
    {
        SetColor(0,12);
        cout << "初始化socket成功了亲(^_^)" << endl;
    }
    else {
        SetColor(0,12);
        cout << "初始化socket都失败了宝贝儿，你穆哥建议还是先回家再好好学学吧！(￢_￢)" << endl;
        return 0;
    }
    sockaddr_in sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));  //每个字节都用0填充
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    sockAddr.sin_port = htons(8000);//好像是在一个区间之内就可以
    SOCKET sock=  socket(AF_INET, SOCK_STREAM, 0);
    if (connect(sock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR)) == 0)
    {
        SetColor(0,12);
        cout << "亲您已成功上线了哦(^_^) 快开始和可爱的朋友们聊天吧！#(^o^)#" << endl;;
        while (1)
        {
            char recvBuf[BUF_SIZE] = {};
            //接受服务器端分配的编号
            recv(sock, recvBuf, 50, 0);
            if (recvBuf[0])
            {
                cout << recvBuf << endl;
                //将char型编号转为int格式存储
                ID = recvBuf[10] - 48;
                break;
                Sleep(30);//防止循环快速执行，占用大量资源
            }
        }
        HANDLE hThread[2];
        while(1) {
            hThread[0] = CreateThread(NULL, 0, Send, (LPVOID) &sock, 0, NULL);
            hThread[1] = CreateThread(NULL, 0, Recv, (LPVOID) &sock, 0, NULL);
            WaitForMultipleObjects(2, hThread, TRUE, INFINITE);
            CloseHandle(hThread[0]);
            CloseHandle(hThread[1]);
        }
    }
    else {
        SetColor(0,12);
        cout << "连接失败了宝贝儿，快查查是不是没启动服务端呢？|_(._.)_|" << endl;
        return 0;
    }

//    HANDLE hThread[2];
//    while(1) {
//        hThread[0] = CreateThread(NULL, 0, Send, (LPVOID) &sock, 0, NULL);
//        hThread[1] = CreateThread(NULL, 0, Recv, (LPVOID) &sock, 0, NULL);
//        WaitForMultipleObjects(2, hThread, TRUE, INFINITE);
//        CloseHandle(hThread[0]);
//        CloseHandle(hThread[1]);
//    }
    closesocket(sock);
    WSACleanup();
    return 0;
}