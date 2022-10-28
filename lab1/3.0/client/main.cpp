#include <WinSock2.h>
#include <iostream>
#include <thread>
#include<string>
using namespace std;
#pragma comment(lib, "ws2_32.lib")  //���� ws2_32.dll
int ID;
const int BUF_SIZE=2048;
char send_buf1[BUF_SIZE];//���ͻ�����

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
            cout << "������" << st.wHour << "ʱ" << st.wMinute << "��" << st.wSecond << "��ѡ����������ˣ��ټ�Ӵ����//��//��" << endl;

            Sleep(1000);
            exit(0);
//printf("������%sʱ%s��%s��ѡ����������ˣ��ټ�Ӵ����//��//��",st.wHour,st.wMinute,st.wSecond);
        }
        if (sendBuf[0]) {
            SetColor(1,0);
            SYSTEMTIME st = { 0 };
            GetLocalTime(&st);
            cout << "��Ϣ����" << st.wHour << "ʱ" << st.wMinute << "��" << st.wSecond << "��ɹ�����\n" ;
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
                cout << "����С����"<<msg.id<<"������" << st.wMonth <<"��"<< st.wDay << "��" << st.wHour << "ʱ" << st.wMinute << "��" << st.wSecond << "˵�ټ���Ӵ����//��//��" << endl;
                Sleep(1000);
                exit(0);}

            else{
            cout << asctime(t_tm) << "�յ�����С�ɰ�"<<msg.id<<"�ŷ�������Ϣ:";
            cout<<msg.chat<<endl;
            cout << "-------------------------------------------------------------" << endl;
        }
        memset(recvBuf, 0, BUF_SIZE);
    }}
}


int main() {
    SetColor();
    //��ʼ��DLL
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0)
    {
        SetColor(0,12);
        cout << "��ʼ��socket�ɹ�����(^_^)" << endl;
    }
    else {
        SetColor(0,12);
        cout << "��ʼ��socket��ʧ���˱����������¸罨�黹���Ȼؼ��ٺú�ѧѧ�ɣ�(�V_�V)" << endl;
        return 0;
    }
    sockaddr_in sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));  //ÿ���ֽڶ���0���
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    sockAddr.sin_port = htons(8000);//��������һ������֮�ھͿ���
    SOCKET sock=  socket(AF_INET, SOCK_STREAM, 0);
    if (connect(sock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR)) == 0)
    {
        SetColor(0,12);
        cout << "�����ѳɹ�������Ŷ(^_^) �쿪ʼ�Ϳɰ�������������ɣ�#(^o^)#" << endl;;
        while (1)
        {
            char recvBuf[BUF_SIZE] = {};
            //���ܷ������˷���ı��
            recv(sock, recvBuf, 50, 0);
            if (recvBuf[0])
            {
                cout << recvBuf << endl;
                //��char�ͱ��תΪint��ʽ�洢
                ID = recvBuf[10] - 48;
                break;
                Sleep(30);//��ֹѭ������ִ�У�ռ�ô�����Դ
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
        cout << "����ʧ���˱������������ǲ���û����������أ�|_(._.)_|" << endl;
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