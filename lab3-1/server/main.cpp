#include <WinSock2.h>
#include<iostream>
#include <stdlib.h>
#include <time.h>
#include <fstream>
using namespace std;
#pragma warning(disable:4996)
#pragma comment(lib,"ws2_32.lib")
#define SERVER_IP "127.0.0.1"
#define CLIENT_IP "127.0.0.1"
#define SERVER_PORT 8888
#define CLIENT_PORT 8080
#define BUFFER sizeof(message)
void SetColor(int fore = 7, int back = 0) {
    unsigned char m_color = fore;
    m_color += (back << 4);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), m_color);
    return;
}
struct message
{
#pragma pack(1)
    u_long flag{};//�ײ�
    u_short seq{};//���к�
    u_short ack{};//ȷ�Ϻ�
    u_long len{};//���ݲ��ֳ���
    u_long num{}; //���͵���Ϣ����������
    u_short checksum{};//У���
    char data[1024]{};//���ݳ���
#pragma pack()
    message(){
        memset(this, 0, sizeof(message));
    }
    bool isSYN(){
        return this->flag & 1;
    }
    bool isFIN(){
        return this->flag & 2;
    }
    bool isSTART(){
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
        for (int i = 0; i < 8; i++)
        {
            sum += (temp[i<<1] << 8) + temp[i<<1|1];
            while (sum >= 0x10000)
            {//���
                int t = sum >> 16;  // �����λ�ع���������λ
                sum += t;
            }
        }
        this->checksum = ~(u_short)sum;  // ��λȡ��������У�����
    }
    bool corrupt(){
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
    void output() {
        cout << "checksum=" << this->checksum << ", len=" << this->len << endl;
    }
};

message recvmessage();
int recvmessages();
void sendmessage(message msg);
int len =sizeof(SOCKADDR);
SOCKADDR_IN serveraddr, clientaddr;
SOCKET Server;
char filepath[20];
ofstream out;
int messagenum;
clock_t timestart;
clock_t timeend;
int getFileName();
message recvmessage()
{
    message msg;

    if (recvfrom(Server, (char*)&msg, BUFFER, 0, (SOCKADDR*)&clientaddr, &len) ==-1 || !msg.isEXT() || msg.corrupt()) {
        return message();
    }

    return msg;
}

void sendmessage(message msg) {
    msg.setEXT();
    msg.setchecksum();
    if (sendto(Server, (char*)&msg, BUFFER, 0, (SOCKADDR*)&clientaddr, sizeof(SOCKADDR)) == (SOCKET_ERROR)) {
        SetColor(14,0);
        cout << "���ʹ�����!!" << endl;
    }
}

void Start()
{
    SetColor(14,0);
    WORD version;
    WSADATA wsaData;
    //�׽��ּ���ʱ������ʾ
    int err;
    //�汾 2.2
    version = MAKEWORD(2, 2);
    //���� dll �ļ� Scoket ��
    err = WSAStartup(version, &wsaData);
    if (err != 0) {
        //�Ҳ��� winsock.dll
        SetColor(12,0);
        cout << "��ʼ���׽��ִ���: " << err << endl;
        return;
    }
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        SetColor(12,0);
        cout << "Winsock.dll�İ汾���԰�������" << endl;
        WSACleanup();
        return;
    }
    Server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    //�����׽���Ϊ������ģʽ
    int iMode = 1; //1����������0������
    ioctlsocket(Server, FIONBIO, (u_long FAR*) & iMode);//����������
    clientaddr.sin_addr.S_un.S_addr = inet_addr(CLIENT_IP);
    clientaddr.sin_family = AF_INET;
    clientaddr.sin_port = htons(CLIENT_PORT);
    serveraddr.sin_addr.S_un.S_addr = inet_addr(SERVER_IP);
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERVER_PORT);
    err = bind(Server, (SOCKADDR*)&serveraddr, sizeof(SOCKADDR));
    if (err) {
        err = GetLastError();
        SetColor(12,0);
        cout << "�󶨶˿�" << SERVER_PORT << "���ִ���" << err << endl;
        WSACleanup();
        return;
    }
    else
    {
        SetColor(12,0);
        cout << "�ɹ�������������" << endl;
    }
}

int WaitConnect()
{
    SetColor(12,0);
    cout << "�������ȴ�����" << endl;
    message recvMsg, sendMsg;
    while (true)
    {
        recvMsg = recvmessage();
        if (recvMsg.isSYN())
        {
            SetColor(12,0);
            cout << "�յ���һ�����ֳɹ���" << endl;
            break;
        }
    }
    sendMsg.setSYN();
    sendMsg.setACK();
    sendMsg.ack = recvMsg.seq + 1;   // ��Ҫ����ȷ�ϰ���ack��Ϊ�յ�����seq+1
    sendMsg.setSYN();
    SetColor(12,0);
    cout << "���͵ڶ���������Ϣ��" << endl;
    sendmessage(sendMsg);
    sendMsg.output();
    int count = 0;
    while (true) {
        Sleep(100);
        if (count >= 50) {
            SetColor(12,0);
            cout << "�ȴ�ʱ��̫�����˳�����" << endl;
            return WaitConnect();
        }
        recvMsg = recvmessage();
        if (!recvMsg.isEXT()) {
            continue;
        }
        if (recvMsg.isACK() && recvMsg.ack == sendMsg.seq + 1) {
            break;
        }
        count++;
    }
    SetColor(14,0);
    cout << "���յ�ȷ�����ӣ����ӳɹ�" << endl;
    int iMode = 0; //1����������0������
    ioctlsocket(Server, FIONBIO, (u_long FAR*) & iMode);//����������
    return 0;
}
int closeconnect(message msg){
    message sendMsg;
    sendMsg.setACK();
    sendMsg.ack = msg.seq + 1;
    sendmessage(sendMsg);
    SetColor(12,0);
    cout<<"�Ѿ��յ��ͻ��˷������Ļ������󣬲��ҷ����˵ڶ��λ��֣����������������У��ټ���"<<endl;
    return 0;
}
int getFileName() {
    cout << "���������ڵȴ���......" << endl;
    message msg, sendMsg;
    int start = clock();
    int end;
    while (true) {
        msg = recvmessage();
        if (msg.isFIN()) {
            SetColor(12,0);
            cout << "�ͻ���׼���Ͽ����ӣ��������ģʽ��" << endl;
            closeconnect(msg);
            break;
        }
        if (!msg.isEXT())
        {
            end = clock();
            if (end - start > 2000) {
                SetColor(14,0);
                cout << "���ӳ�ʱ" << endl;
                cout << "���½���ȴ������ļ���ģʽ" << endl;
            }
            continue;
        }
        if (msg.isSTART()) {
            ZeroMemory(filepath, 20);
            memcpy(filepath, msg.data, msg.len);
            out.open(filepath, std::ios::out | std::ios::binary);
            SetColor(14,0);
            cout << "�ļ���Ϊ��" << filepath << endl;
            if (!out.is_open())
            {
                SetColor(14,0);
                cout << "�ļ���ʧ�ܣ�����" << endl;
                exit(1);
            }
            messagenum = msg.num;
            SetColor(14,0);
            cout << "�ļ���Ϊ��" << msg.data << "����" << msg.num <<"�����ݰ���"<< endl;
            sendMsg.setACK();
            sendMsg.ack = msg.seq;
            sendmessage(sendMsg);
            timestart = clock();
            return recvmessages();
        }
    }
}
int recvmessages() {
    cout << "��ʼ�����ļ����ݣ�" << endl;
    message recvMsg, sendMsg;
    int seq = 1;
    int iMode = 1; //1����������0������
    ioctlsocket(Server, FIONBIO, (u_long FAR*) & iMode);//����������
    for (int i = 0; i < messagenum; i++) {
        int start = clock();
        int end;
        while (1) {
            end = clock();
            if (end - start > 2000) {
                SetColor(14,0);
                cout << "���䳬ʱʧ��" << endl;
                cout << "��ȷ������ͨ�������½����ļ����չ�����лл��" << endl;
                int iMode = 0; //1����������0������
                ioctlsocket(Server, FIONBIO, (u_long FAR*) & iMode);//����������
                return getFileName();
            }
            recvMsg = recvmessage();
            if (!recvMsg.isEXT()) {
                continue;
            }
            if (recvMsg.seq == seq) {
                sendMsg.setACK();
                sendMsg.ack = recvMsg.seq;
                SetColor(14,0);
                cout << "�յ�seqΪ" << recvMsg.seq << "�����ݰ�" << endl;
                cout<<"checksum="<<recvMsg.corrupt()<<", len="<<recvMsg.len<<endl;
                SetColor(14,0);
                cout << "����ȷ���յ������ݰ�(��Ӧ��ack)" << endl;
                sendmessage(sendMsg);
                cout << endl;
                out.write(recvMsg.data, recvMsg.len);
                break;
            }
        }
        if (recvMsg.isEND()) {
            SetColor(14,0);
            cout << "�����ļ��ɹ�����" << endl << endl;
            out.close();
            out.clear();
            timeend = clock();
            double endtime = (double)(timeend - timestart) / CLOCKS_PER_SEC;
            cout << "������ʱ��" << endtime << "s" << endl;
            cout << "������" << (double)(messagenum) * sizeof(message) * 8 / endtime / 1024  << "kbps" << endl;
            int iMode = 0; //1����������0������
            ioctlsocket(Server, FIONBIO, (u_long FAR*) & iMode);//����������
            return getFileName();
        }
        seq++;
    }
}

int main()
{
    SetColor(14,0);
    Start();
    WaitConnect();
    getFileName();
    //�ر��׽���
    closesocket(Server);
    WSACleanup();
    system("pause");
    return 0;
}
