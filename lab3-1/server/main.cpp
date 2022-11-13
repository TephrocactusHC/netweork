//server�˽����ļ�

#include<iostream>
#include<WinSock2.h>
#include<string>
#include<string.h>
#include<time.h>
#include<fstream>
#include<stdio.h>
#include<vector>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")
SOCKADDR_IN serverAddr, clientAddr;
SOCKET server;
const u_long clientPort=8080;
const u_long serverPort=8080;
const int BUF_SIZE=1024;
const char *serverIP="127.0.0.1";
const char *clientIP="127.0.0.1";
#define BUFFER sizeof(message)
struct message//���ĸ�ʽ
{
#pragma pack(1)//�������ݰ�1Byte���룬���û������ָ�����4Byte���룬����u_short���ͻ���2B����Ϣ��2B���㣬�������ת����char*��ʽ
    //ACK=0x01, SYN=0x02, FIN=0x04,EXIST 0x10,startfile 0x20,endfile 0x40
    u_long flag;//��־λ
    u_long SendIP;
    u_long RecvIP;//���Ͷ�IP�ͽ��ն�IP
    u_long SendPort, RecvPort;//���Ͷ˶˿ںͽ��ն˶˿�
    u_short seq;//��Ϣ���
    u_short ack;//�ָ�ackʱȷ�϶Է�����Ϣ�����
    u_long len;//���ݲ��ֳ���
    u_long index;//���������ļ���С����Ҫ��������Ϣ���ܴ�����ɣ�1��index=0,2��index=1���Դ����ƣ�
    u_short checksum;//У���
    char msg[BUF_SIZE];//���ĵľ������ݣ�����ʵ���Ϊ�̶�����
#pragma pack()//�ָ�4Byte��ַ��ʽ
    message(){
        memset(this, 0, sizeof(message));
        SendPort = serverPort;
        RecvPort = clientPort;
        SendIP = serverAddr.sin_addr.s_addr;
        RecvIP = clientAddr.sin_addr.s_addr;
    }
    void init_message(){
        memset(this->msg, 0, BUF_SIZE);
    }
    bool isSYN(){
        return this->flag & 1;
    }
    bool isFIN(){
        return this->flag & 2;
    }
    bool isFIR(){
        return this->flag & 4;
    }
    bool isEND(){
        return this->flag & 8;
    }
    bool isACK(){
        return this->flag & 16;
    }
    bool isEXT(){
        return this->flag & 32;
    }
    void setSYN(){
        this->flag |= 1;
    }
    void setFIN(){
        this->flag |= 2;
    }
    void setFIR(){
        this->flag |= 4;
    }
    void setEND(){
        this->flag |= 8;
    }
    void setACK(){
        this->flag |= 16;
    }
    void setEXT(){
        this->flag |= 32;
    }
    void setchecksum(){
        int sum = 0;
        u_char* temp = (u_char*)this;
        for (int i = 0; i < 12; i++)
        {
            sum += (temp[i<<1] << 8) + temp[2<<1|1];
            while (sum >= 0x10000)
            {//���
                int t = sum >> 16;  // �����λ�ع���������λ
                sum += t;
            }
        }
        this->checksum = ~(u_short)sum;  // ��λȡ��������У�����
    }
    bool corrupt() {
        // ���Ƿ���
        int sum = 0;
        u_char* temp = (u_char*)this;
        for (int i = 0; i < 8; i++)
        {
            sum += (temp[i<<1] << 8) + temp[i<<1|1];
            while (sum >= 0x10000)
            {//���
                int t = sum >> 16;//���㷽��������У�����ͬ
                sum += t;
            }
        }
        //�Ѽ��������У��ͺͱ����и��ֶε�ֵ��ӣ��������0xffff����У��ɹ�
        if (checksum + (u_short)sum == 65535)
            return false;
        return true;
    }
    void output(){
        cout << "-------------- seq=" << this->seq << ", ack=" << this->ack << " -----------" << endl;
        cout << "len:" << this->len << endl;
    }
};
struct message recvPacket()
{
    message msg;
    if (recvfrom(server,(char*)&msg, BUFFER, 0, (SOCKADDR*)&clientAddr, (int*)sizeof(SOCKADDR)) == -1 || !msg.isEXT() || msg.corrupt()) {
        msg.init_message();
    }
    return msg;


}
// ������Ϣ
void sendPacket(message msg) {
    msg.setEXT();
    msg.setchecksum();
    if (sendto(server, (char*)&msg, BUFFER, 0, (SOCKADDR*)&clientAddr, sizeof(SOCKADDR)) == (SOCKET_ERROR)) {
        cout << "socket error in sendPacket!!" << endl;
    }
    msg.output();
}


string filename;
char message[200000000];
// �������ն�
int StartServer()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cout << "WSAStartup����" << WSAGetLastError() << endl;
        return -1;
    }
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        cout << "�汾���԰�������" << endl;
        WSACleanup();
        return -1;
    }
    server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (server == SOCKET_ERROR)
    {
        cout << "�׽��ִ���" << WSAGetLastError() << endl;
        return -1;
    }
    //�����׽���Ϊ������ģʽ
    //int iMode = 1; //1����������0������
    //ioctlsocket(server, FIONBIO, (u_long FAR*) & iMode);//����������
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    //clientAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    //clientAddr.sin_family = AF_INET;
    //clientAddr.sin_port = htons(6666);//6666
    if (bind(server, (SOCKADDR*)&serverAddr, sizeof(serverAddr)))
    {
        cout << "�󶨶˿ڴ���" << WSAGetLastError() << endl;
        return -1;
    }
    cout << "�ɹ��������նˣ�" << endl;
    return 1;
}
// �ȴ�����
void WaitConnection()
{
    cout << "�ȴ�������Ŷ�ף�" << endl;
    struct message recvMsg, sendMsg;
    int count = 0;
    while (1)
    {
        recvMsg = recvPacket();
        if (recvMsg.isSYN())
        {
            cout << "�յ����������һ������" << endl;
            break;
        }
    }
    sendMsg.setSYN();
    sendMsg.setACK();
    sendMsg.ack = recvMsg.seq + 1;   // ��Ҫ����ȷ�ϰ���ack��Ϊ�յ�����seq+1
    sendMsg.setSYN();
    cout << "�����յ�������������ݰ�" << endl;
    sendPacket(sendMsg);
    count = 0;
    while (true) {
        Sleep(100);
        if (count >= 50) {
            cout << "�ȴ�ʱ��̫�����˳�����" << endl;
            return WaitConnection();
        }
        recvMsg = recvPacket();
        if (!recvMsg.isEXT()) {
            continue;
        }
        if (recvMsg.isACK() && recvMsg.ack == sendMsg.seq + 1) {
            break;
        }
        count++;
    }
    cout << "���յ�ȷ�����ӣ����ӳɹ�" << endl << endl;

}

// �ȴ��Ͽ�����

// ����

// �����ļ���
void RecvName()
{
    char name[100];
    int clientlen = sizeof(clientAddr);
    while (recvfrom(server, name, 100, 0, (sockaddr*)&clientAddr, &clientlen) == SOCKET_ERROR);
    cout << "�ļ���Ϊ: ";
    for (int i = 0; name[i] != '$'; i++)
    {
        filename += name[i];
        putchar(name[i]);
    }
    cout << endl;
}

int main()
{
    StartServer();
    WaitConnection();
    cout << "�ɹ����ӵ����Ͷˣ�" << endl;
    RecvName();
    /*
    cout<<'1';
    Recvmessage();
    cout << "�����ļ�������" << endl;
    WaitDisconnection();
    closesocket(server);*/
    WSACleanup();
    system("pause");
    return 0;
}
