#include <iostream>
#include <winsock2.h>
#include<time.h>
#include<windows.h>
#include<cstring>
#pragma comment(lib, "ws2_32.lib")
using namespace std;
SOCKET socket_client1;
SOCKADDR_IN addr_serve1,addr_client1;
const int buf_size=2048;//��������С����
char send_buf1[buf_size];//���ͻ�����
char receive_buf1[buf_size];//���ܻ�����
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
    //ָ���汾wVersionRequested=MAKEWORD(a,b)��MAKEWORD()��һ���������������ϳ�һ��WORD���޷��ŵĶ�����
    if (FIRST_JUDGE != 0) {
        //����û�����ӳɹ���ʧ���ˡ�
        SetColor(0,12);
        cout << "��ʼ��socket��ʧ���˱����������¸罨�黹���Ȼؼ��ٺú�ѧѧ�ɣ�(�V_�V)" << endl;
        return 0;
    }

    //����socket����������ʹ����ʽsocket��
    socket_client1 = socket(AF_INET, SOCK_STREAM, 0);

    //��ʼ���ͻ��˵�ַ
    addr_client1.sin_addr.s_addr = inet_addr("127.0.0.1");//�����Ǳ����ĵ�ַת���������ֽڶ�����ֵ��
    addr_client1.sin_family = AF_INET;//ʹ��ipv4
    addr_client1.sin_port = htons(8000);//ת��������Ҳ��ת���������ֽ���
    //��ʼ����ַ
    addr_serve1.sin_addr.s_addr = inet_addr("10.130.122.241");//�����Ǳ����ĵ�ַת���������ֽڶ�����ֵ��
    addr_serve1.sin_family = AF_INET;//ʹ��ipv4
    addr_serve1.sin_port = htons(8000);//ת��������Ҳ��ת���������ֽ���
    if (connect(socket_client1,(SOCKADDR *) &addr_serve1,sizeof(addr_serve1) )!=SOCKET_ERROR)
    {
        strcpy(send_buf1, "��ð������ǿͻ��ˣ����Ѿ����ӵ����ˣ�");
        send(socket_client1, send_buf1, 2048, 0);
    }
    else {
        SetColor(0,12);
        cout << "����ʧ���˱������������ǲ���û����������أ�|_(._.)_|" << endl;
        return 0;
    }

    while (1) {
        SetColor(0,12);
        recv(socket_client1,receive_buf1,sizeof(receive_buf1),0);
        //�ж��Ƿ�Է�Ҫ�˳�
        if (strlen(receive_buf1) == 0)
        {
            cout << "������ѡ����������ˣ��ټ�Ӵ����//��//��" << endl;
            break;
        }
        else {
            time_t now_time = time(NULL);
            tm *t_tm = localtime(&now_time);
            cout << "����˷��ģ�" << receive_buf1 << "  " << "�յ�ʱ��: " << asctime(t_tm);
        }
        cin.getline(input_buf1, 2048, '\n');
        if (!strcmp("quit", input_buf1))
        {
            send(socket_client1, {}, 2048, 0);
            cout << "����ѡ��������죡��//��//��" << endl;
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
