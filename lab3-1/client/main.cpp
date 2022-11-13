
#include<iostream>
#include<WinSock2.h>
#include<string>
#include<string.h>
#include<time.h>
#include<fstream>
#include<stdio.h>
#include<vector>
#pragma comment(lib, "Ws2_32.lib")
using namespace std;
SOCKADDR_IN serverAddr, clientAddr;
const u_long clientPort=8080;
const u_long serverPort=8080;
#define TIMEOUT 200
#define SENDSUCCESS true
#define SENDFAIL false

#define WAVE1 '7'
#define ACKW1 '#'
#define WAVE2 '9'
#define ACKW2 WAVE1 + 1
#define BUFFER sizeof(message)

SOCKET client;

const int BUF_SIZE=1024;
const char *serverIP="127.0.0.1";
const char *clientIP="127.0.0.1";
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
        SendPort = clientPort;
        RecvPort = serverPort;
        SendIP = clientAddr.sin_addr.s_addr;
        RecvIP = serverAddr.sin_addr.s_addr;
    }
    void init_message(){
        memset(this, 0, sizeof(message));
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
    if (recvfrom(client,(char*)&msg, BUFFER, 0, (SOCKADDR*)&serverAddr, (int*)sizeof(SOCKADDR)) == -1 || !msg.isEXT() || msg.corrupt()) {
        return message();
    }
    return msg;
}
// ������Ϣ
void sendPacket(message msg) {
    msg.setEXT();
    msg.setchecksum();
    if (sendto(client, (char*)&msg, BUFFER, 0, (SOCKADDR*)&serverAddr, sizeof(SOCKADDR)) == (SOCKET_ERROR)) {
        cout << "socket error in sendPacket!!" << endl;
    }
    msg.output();
}
// ���ӵ�������
void ConnectToServer()
{
    cout << "��ʼ�������ף�" << endl;

    struct message recvMsg, sendMsg;
    sendMsg.setSYN();
    sendMsg.seq = 88;
    sendPacket(sendMsg);
    cout<<"�Ѿ����͵�һ��������Ӵ��(^_^)"<<endl;

    int start = clock();
    int end;
    while (1)
    {
        recvMsg = recvPacket();
        if (!recvMsg.isEXT())
        {
            end = clock();
            if (end - start > 2000) {
                cout << "���ӳ�ʱ!!!FBI WARNING!!!" << endl;
            }
            continue;
        }
        if (recvMsg.isACK() && recvMsg.isSYN()&& recvMsg.ack == sendMsg.seq + 1) {
            cout << "�յ�����˷����ĵڶ���������Ŷ��" << endl;
            break;
        }
    }
    sendMsg.init_message();
    sendMsg.setACK();
    sendMsg.seq = 89;
    sendMsg.ack = recvMsg.seq + 1;
    cout << "���͵��������ֵ����ݰ�" << endl;
    sendPacket(sendMsg);
    return ;
}
// �����ļ���
void SendName(string filename, int size)
{
    char *name = new char[size + 1];
    for (int i = 0; i < size; i++)
    {
        name[i] = filename[i];
    }
    name[size] = '$';
    sendto(client, name, size + 1, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
    delete name;
}

// ��ʼ�ͻ���
int StartClient()
{
    //�����׽��ֿ�
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cout << "WSAStartup�����˱����������Լ�����������զ���°�<(-_-;<)��" << WSAGetLastError() << endl;
        return -1;
    }
    client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (client == SOCKET_ERROR)
    {
        cout << "�׽��ִ��������������Լ�����������զ���°�<(-_-;<)��" << WSAGetLastError() << endl;
        return -1;
    }
    //�����׽���Ϊ������ģʽ
    int iMode = 1; //1����������0������
    ioctlsocket(client, FIONBIO, (u_long FAR*) & iMode);//����������


    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    clientAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(6666);
    cout << "�ɹ�����client�ˣ��۹���������(^_^)" << endl;
    cout<<"-----------------------------------------------------------"<<endl;
    return 1;
}

// �رտͻ���

//���͵�һ������ʵ��
/*int sendhead() {
    message msg;
    if (openfile() == 0) {
        return 0;
    }
    memcpy(msg.data, filepath, strlen(filepath));
    msg.len = strlen(filepath);
    msg.num = packetnum;
    msg.setFIR();
    cout << "����Ŀ���ļ�ͷ���ݰ�" << endl;
    timestart = clock();
    if (sendwait(msg, 0) == 0) {
        cout << "����Ŀ���ļ�ͷ���ݰ�ʧ��" << endl;
        return 0;
    }
    return sendpackets();
}*/
int main()
{
    cout << "����ʼ������!!! FBI WARNING!!!" << endl;
    StartClient();
    cout<<"��������ʼ������Ŷ(*`o`*)!!!"<<endl;
    ConnectToServer();
    cout << "�ɹ����ӵ����ն���Ӵ(^_^)��" << endl;

    int time_out = 1;
    setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (char*)&time_out, sizeof(time_out));
    string filename;
    cin >> filename;
    while(filename!="1.jpg"||filename!="2.jpg"||filename!="3.jpg"||filename!="helloworld.txt"){
        cout<<"û������ļ����������������"<<endl;
        cin>>filename;
    }
    SendName(filename, filename.length());

    cout << "���ͳɹ����ף�(^o^)" << endl;

    WSACleanup();
    system("pause");
    return 0;
}
