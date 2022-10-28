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
#pragma comment (lib, "ws2_32.lib")  //���� ws2_32.dll

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
                cout <<"��һλ�û�ѡ������(*_*)���������";
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

                cout << asctime(t_tm) << "�յ�����С�ɰ�"<<msg.id<<"�ŷ�������Ϣ:";
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
    cout << "��Ҫ" <<chatnumber<<"λ�û����ܿ�ʼ����Ŷ�ף�(^-^)"<< endl;
    SetColor();
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) ==
        0) //ָ���汾wVersionRequested=MAKEWORD(a,b)��MAKEWORD()��һ���������������ϳ�һ��WORD���޷��ŵĶ�����
    {
        SetColor(0, 12);
        cout << "��ʼ��socket�ɹ�����" << endl;
    } else {
        SetColor(0, 12);
        cout << "��ʼ��socket��ʧ���˱����������¸罨�黹���Ȼؼ��ٺú�ѧѧ�ɣ�(�V_�V)" << endl;
        return 0;
    }

    //�����׽���
    SOCKET sockSrv = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addrSrv;
    memset(&addrSrv, 0, sizeof(addrSrv));  //ÿ���ֽڶ���0���
    addrSrv.sin_family = AF_INET;  //ʹ��IPv4��ַ
    addrSrv.sin_addr.s_addr = inet_addr("127.0.0.1");  //�����Ǳ����ĵ�ַת���������ֽڶ�����ֵ��
    addrSrv.sin_port = htons(8000);  //�˿�,��������һ������֮�ھͿ���.//ת��������Ҳ��ת���������ֽ���
    int z = bind(sockSrv, (SOCKADDR *) &addrSrv, sizeof(addrSrv));
    if (z == -1) {
        SetColor(0, 12);
        cout << "����һ����ʧ���˱����������黹���Ȼؼ��ٺú�ѧѧ�ɣ�(�V_�V)" << endl;
        return 0;
    } else {
        SetColor(0, 12);
        cout << "�󶨳ɹ�����(^_^)" << endl;
    }
    //�������״̬
    //���տͻ�������
    SOCKADDR addrCli[2];
    int nSize = sizeof(SOCKADDR);

    if (listen(sockSrv, 5) == 0) {
        SetColor(0, 12);
        cout << "���ڼ���֮��Ŷ��(^_^)" << endl;
        for (int i = 0; i < chatnumber; i++) {
            //Ϊÿ���ͻ��˷���һ��socket���ӣ����ͻ��˵������Ϣ�洢��addrCli��
            sockConn[i] = accept(sockSrv, (SOCKADDR *) &addrCli[i], &nSize);
            if (sockConn[i] != INVALID_SOCKET)
            {
                cout << "�û�" << i << "��������" << endl;
                char buf[12] = "���id�ǣ�";
                buf[10] = 48 + i;
                buf[11] = 0;
                send(sockConn[i], buf, 50, 0);
            }
        }

    } else {
        SetColor(0, 12);
        cout << "������ʧ���˱����������¸罨�黹���Ȼؼ��ٺú�ѧѧ�ɣ�(�V_�V)" << endl;
        return 0;
    }

    //�������߳�
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