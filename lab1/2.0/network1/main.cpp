#include <stdio.h>
#include <winsock2.h>
#include <iostream>
#include<time.h>
#include<string.h>
#include<string>
#include <windows.h>
#include <stdlib.h>
using namespace std;
#pragma comment (lib, "ws2_32.lib")  //���� ws2_32.dll

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
            cout << "������" << st.wMonth << "��"<< st.wDay << "��" << st.wHour << "ʱ" << st.wMinute << "��" << st.wSecond << "��ѡ����������ˣ��ټ�Ӵ����//��//��" << endl;
            Sleep(10000000);
            exit (0);
            return 0;
        }
        if (t > 0) {
            SetColor(1,0);
            SYSTEMTIME st = { 0 };
            GetLocalTime(&st);
            cout << "��Ϣ����" << st.wHour << "ʱ" << st.wMinute << "��" << st.wSecond << "��ɹ�����\n";
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
            cout << "�ͻ�������" << st.wMonth << "��"<< st.wDay << "��" << st.wHour << "ʱ" << st.wMinute << "��" << st.wSecond << "��ѡ����������ˣ��ټ�Ӵ����//��//��" << endl;
            exit (0);
            return 0L;
        }
        if (t > 0) {
            time_t now_time = time(NULL);
            tm *t_tm = localtime(&now_time);
            SetColor(14,0);
            cout << asctime(t_tm) << "�յ�����С�ɰ�(�����)��������Ϣ:";
            cout<<parseMessage (recvBuf)<<endl;
            cout << "-------------------------------------------------------------" << endl;
        }
        memset(recvBuf, 0, BUF_SIZE);
    }
}




int main() {
    SetColor();
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0) //ָ���汾wVersionRequested=MAKEWORD(a,b)��MAKEWORD()��һ���������������ϳ�һ��WORD���޷��ŵĶ�����
    {
        SetColor(0,12);
        cout << "��ʼ��socket�ɹ�����" << endl;
    }
    else {
        SetColor(0,12);
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
    int z=bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(addrSrv));
    if (z == -1) {
        SetColor(0,12);
        cout << "����һ����ʧ���˱����������黹���Ȼؼ��ٺú�ѧѧ�ɣ�(�V_�V)" << endl;
        return 0;
    }
    else{
        SetColor(0,12);
        cout<<"�󶨳ɹ�����(^_^)" << endl;
    }
    //�������״̬
    if (listen(sockSrv, 5) == 0) {
        SetColor(0,12);
        cout << "���ڼ���֮��Ŷ��(^_^)" << endl;
    }
    else {
        SetColor(0,12);
        cout << "������ʧ���˱����������¸罨�黹���Ȼؼ��ٺú�ѧѧ�ɣ�(�V_�V)" << endl;
        return 0;
    }

    //���տͻ�������
    SOCKADDR addrClient;
    int nSize = sizeof(SOCKADDR);
    SOCKET ClientSocket = accept(sockSrv, (SOCKADDR*)&addrClient, &nSize);
    if (ClientSocket > 0) {
        SetColor(0,12);
        cout << "����С����(�ͻ���)������(^_^)" << endl;
    }
    else {
        SetColor(0,12);
        cout << "����ʧ���˱������������ǲ���û�����ͻ����أ�|_(._.)_|" << endl;
        return 0;
    }
    //�������߳�
    HANDLE hThread[2];
//    while(1) {
        hThread[0] = CreateThread(NULL, 0, Recv, (LPVOID) &ClientSocket, 0, NULL);
        hThread[1] = CreateThread(NULL, 0, Send, (LPVOID) &ClientSocket, 0, NULL);
        WaitForMultipleObjects(2, hThread, TRUE, INFINITE);
        CloseHandle(hThread[0]);
        CloseHandle(hThread[1]);

//    }
    closesocket(ClientSocket);  //�ر��׽���

    //�ر��׽���
    closesocket(sockSrv);

    //��ֹ DLL ��ʹ��
    WSACleanup();
    return 0;
}


