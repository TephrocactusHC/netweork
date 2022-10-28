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
    //ָ���汾wVersionRequested=MAKEWORD(a,b)��MAKEWORD()��һ���������������ϳ�һ��WORD���޷��ŵĶ�����
    if (FIRST_JUDGE != 0) {
        //���ʧ����
        SetColor(0,12);
        cout << "��ʼ��socket��ʧ���˱����������黹���Ȼؼ��ٺú�ѧѧ�ɣ�(�V_�V)" << endl;
        return 0;
    }

    //����socket����������ʹ����ʽsocket��
    socket_serve = socket(AF_INET, SOCK_STREAM, 0);
    //��ʼ���ͻ��˵�ַ
    addr_serve.sin_addr.s_addr = inet_addr("10.130.122.241");//�����Ǳ����ĵ�ַת���������ֽڶ�����ֵ��
    addr_serve.sin_family = AF_INET;//ʹ��ipv4
    addr_serve.sin_port = htons(8000);//ת��������Ҳ��ת���������ֽ���


    if (bind(socket_serve, (SOCKADDR *) &addr_serve, sizeof(SOCKADDR)) == -1) {
        SetColor(0,12);
        cout << "����һ����ʧ���˱����������黹���Ȼؼ��ٺú�ѧѧ�ɣ�(�V_�V)" << endl;
        return 0;
    }
    //��
    bind(socket_serve, (SOCKADDR *) &addr_serve, sizeof(SOCKADDR));

    //����
    listen(socket_serve, 5);
    socket_client = accept(socket_serve, (SOCKADDR *) &addr_client, &size_addr);

    //���ܣ����ص���һ��socket
    if (socket_client != INVALID_SOCKET)//�ж����ӳɹ�
    {
        SetColor(0,12);
        cout << "LOGGING: �ɹ�����������(^_^)" << endl;
//        strcpy(send_buf, "��ð������ǿͻ��ˣ����Ѿ����ӵ����ˣ�");
//        send(socket_client, send_buf, 2048, 0);
    }
    else {
        SetColor(0,12);
        cout << "����ʧ���˱������������ǲ���û�����ͻ����أ�|_(._.)_|" << endl;
        return 0;
    }

    while (1) {
        char receive_buf_loop[buf_size]={};
//        int NetTimeout = 500; //��ʱʱ��
//        setsockopt(socket_client, SOL_SOCKET,SO_RCVTIMEO,(char *)&NetTimeout,sizeof(int));
        recv(socket_client, receive_buf_loop,2048,0);
        //�ж��Ƿ�Է�Ҫ�˳�
        if (strlen(receive_buf_loop) == 0)
        {
            cout << "�ͻ���ѡ����������ˣ��ټ�Ӵ����//��//��" << endl;
            break;
        }
        else {
            time_t now_time = time(NULL);
            tm *t_tm = localtime(&now_time);
            cout << "�ͻ��˷��ģ�" << receive_buf_loop << "  " << "�Լ��յ�ʱ��: " << asctime(t_tm);
        }
        cin.getline(input_buf, 2048, '\n');
        if (!strcmp("quit", input_buf))
        {
//            strcpy(send_buf, '\0');
            send(socket_client, {}, 2048, 0);
            cout << "����ѡ��������죡��//��//��" << endl;
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



