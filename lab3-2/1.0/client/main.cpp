#include <WinSock2.h>
#include<iostream>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include<thread>
using namespace std;
#pragma comment(lib,"ws2_32.lib")
#define SERVER_IP "127.0.0.1"
#define CLIENT_IP "127.0.0.1"
#define SERVER_PORT 8888
#define CLIENT_PORT 8080
#define BUFFER sizeof(message)
#define WINDOW 10
#define TIMEOUT 50
#pragma warning(disable:4996)

void SetColor(int fore = 7, int back = 0) {
    unsigned char m_color = fore;
    m_color += (back << 4);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), m_color);
    return;
}
struct message
{
#pragma pack(1)
    u_long flag{};
    u_short seq{};//���к�
    u_short ack{};//ȷ�Ϻ�
    u_long len{};//���ݲ��ֳ���
    u_long num{}; //���͵���Ϣ����������
    u_short checksum{};//У���
    char data[8192]{};//���ݳ���
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

SOCKADDR_IN serveraddr,clientaddr;
SOCKET Client;
int messagenum;
int lastlen;
int len = sizeof(SOCKADDR);
char filepath[20];
clock_t timestart;
clock_t timeend;
int filelen;
ifstream in;
int recvSize;
char buffer[20000][8192];
int status[20000];
int sendbase = 0;
int sendtop = WINDOW - 1;
bool isEnd = false;

message recvmessage();
void sendmessage(message msg);
int openFile();
int closeconnect();
int waitSend(message send, int seq);
int beginsendpackets();
message recvmessage()
{
    message msg;

    if (recvfrom(Client, (char*)&msg, BUFFER, 0, (SOCKADDR*)&serveraddr, &len) == -1 || !msg.isEXT() || msg.corrupt()) {
        return message();
    }
    /*
    if (recvfrom(Client, (char*)&msg, BUFFER, 0, (SOCKADDR*)&routeraddr, &len) == -1 || !msg.isEXT() || msg.corrupt()) {
        return message();
    }*/
    return msg;
}

void sendmessage(message msg) {
    msg.setEXT();
    msg.setchecksum();
    if (sendto(Client, (char*)&msg, BUFFER, 0, (SOCKADDR*)&serveraddr, sizeof(SOCKADDR)) == (SOCKET_ERROR)) {
        SetColor(12,0);
        cout << "���ʹ�����!!" << endl;
    }
}

void Start()
{
    //�����׽��ֿ⣨���룩
    WORD version;
    WSADATA wsaData;
    //�׽��ּ���ʱ������ʾ
    int err;
    //�汾 2.2
    version = MAKEWORD(2, 2);
    //���� dll �ļ� Scoket ��
    err = WSAStartup(version, &wsaData);
    if (err != 0)
    {
        //�Ҳ��� winsock.dll
        SetColor(12,0);
        cout << "��ʼ���׽��ִ�����: " << err << endl;
        return;
    }
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        SetColor(12,0);
        cout << "Winsock.dll�汾����" << endl;
        WSACleanup();
    }
    else
    {
        SetColor(12,0);
        cout << "�׽��ִ����ɹ�" << endl;
    }
    Client = socket(AF_INET, SOCK_DGRAM, 0);
    clientaddr.sin_addr.S_un.S_addr = inet_addr(CLIENT_IP);
    clientaddr.sin_family = AF_INET;
    clientaddr.sin_port = htons(CLIENT_PORT);
    serveraddr.sin_addr.S_un.S_addr = inet_addr(SERVER_IP);
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERVER_PORT);
    err = bind(Client, (SOCKADDR*)&clientaddr, sizeof(SOCKADDR));
    if (err) {
        err = GetLastError();
        SetColor(12,0);
        cout << "�󶨶˿�" << CLIENT_PORT << "���ִ���" << err << endl;
        WSACleanup();
        return;
    }
    else
    {
        SetColor(12,0);
        cout << "�ɹ������ͻ��ˣ�" << endl;
    }
}

int beginconnect()
{
    //int iMode = 1; //1����������0������
    //ioctlsocket(Client, FIONBIO, (u_long FAR*) & iMode);//����������
    SetColor(12,0);
    cout << "��ʼ���ӣ����͵�һ�����֣�" << endl;
    message recvMsg, sendMsg;
    sendMsg.setSYN();
    sendMsg.seq = 88;

    sendmessage(sendMsg);

    int start = clock();
    int end;
    while (true)
    {
        recvMsg = recvmessage();
        if (!recvMsg.isEXT())
        {
            end = clock();
            if (end - start > 50) {
                SetColor(12,0);
                cout << "���ӳ�ʱ,��ȷ������ͨ���ͷ������������������б�����" << endl;
                sendmessage(sendMsg);
                break;
            }
            continue;
        }
        if (recvMsg.isACK() && recvMsg.isSYN()&& recvMsg.ack == sendMsg.seq + 1) {
            SetColor(14,0);
            cout << "�յ��ڶ�������!" << endl;
            break;
        }
    }
    sendMsg.setACK();
    sendMsg.seq = 89;
    sendMsg.ack = recvMsg.seq + 1;
    SetColor(14,0);
    cout << "���͵��������ֵ����ݰ�" << endl;
    sendmessage(sendMsg);
    return 0;
}

int openFile() {
    SetColor(10,0);
    cout << "������Ҫ���͵��ļ�����";
    memset(filepath, 0, 20);
    ZeroMemory(buffer, sizeof(buffer));
    memset(status, 0, sizeof(status));
    string temp;
    cin >> temp;
    if (temp == "FINISH") {
        return closeconnect();
    } else {
        if(temp=="1.jpg"||temp=="2.jpg"||temp=="3.jpg"||temp=="helloworld.txt"){
            strcpy(filepath, temp.c_str());
            in.open(filepath, ifstream::in | ios::binary);// �Զ����Ʒ�ʽ���ļ�
            in.seekg(0, std::ios_base::end);  // ���ļ���ָ�붨λ������ĩβ
            filelen = in.tellg();
            messagenum = filelen / 8192 + 1;
            SetColor(0,6);
            lastlen = filelen - (messagenum - 1) * 8192;
            cout << "�ļ���СΪ" << filelen << "Bytes,�ܹ���" << messagenum << "�����ݰ�" << endl;
            in.seekg(0, std::ios_base::beg);  // ���ļ���ָ�붨λ�����Ŀ�ʼ
            int index = 0, len = 0;
            // ���ļ����뻺����
            char t = in.get();
            while (in) {
                buffer[index][len] = t;
                len++;
                if (len == 8192) {
                    len = 0;
                    index++;
                }
                t = in.get();
            }
            in.close();
            return 1;
        }
        else{
            SetColor(12,0);
            cout<<"�ļ������ڣ�������������Ҫ������ļ�����"<<endl;
            return openFile();
        }
    }
}

int sendFileName() {
    message msg;
    if (openFile() == 0) {
        return 0;
    }
    memcpy(msg.data, filepath, strlen(filepath));
    msg.len = strlen(filepath);
    msg.num = messagenum;
    msg.setFIR();
    SetColor(0,6);
    cout << "������Ҫ������ļ���" << endl;
    if (waitSend(msg, 0) == 0) {
        SetColor(12,0);
        cout << "�����ļ���ʧ��" << endl;
        return 0;
    }
    timestart = clock();
    return beginsendpackets();
}

int closeconnect() {  // �Ͽ�����
    message recvMsg, sendMsg;
    sendMsg.setFIN();
    sendMsg.seq = 65534;//�˴���u_short�ı�ʾ��Χ�����ֵ-1���������յ��Ľ����ټ�һ����ô�Ѿ�����u_short�����ֵ�ˣ�����Ȼ�����ˡ�
    sendmessage(sendMsg);
    cout << "���ͳ�ȥ��һ�λ��֣�" << endl;
    int count = 0;
    SetColor(12,0);
    cout << "���յ�ȷ�����ӣ��Ͽ����ӳɹ�" << endl << endl;
    return 0;
}

int waitSend(message sendMsg, int seq)
{
    message recvMsg;
    sendMsg.seq = seq;
    sendmessage(sendMsg);
    int count = 0;
    clock_t start = clock();
    clock_t end;
    while (1) {
        end = clock();
        if (count == 10) {
            cout << "��Ӧ���˳�" << endl;
            return sendFileName();
        }
        if (end - start > 50) {
            SetColor(12,0);
            cout << "Ӧ��ʱ�����·������ݰ�" << endl;
            sendmessage(sendMsg);
            count++;
            SetColor(12,0);
            start = clock();
        }
        recvMsg = recvmessage();
        if (!recvMsg.isEXT()) {
            continue;
        }
        if (recvMsg.isACK() && recvMsg.ack == seq) {
            SetColor(14,0);
            cout << "�յ�������������ack��ȷ��ȷ�����ݰ���" << endl;
            cout << endl;
            return 1;
        }
    }
}

int sendPacketThread(message msg,int num) {
    if (num != messagenum - 1) {
        memcpy(msg.data, buffer[num], 8192);
        msg.len = 8192;
    }
    else {
        memcpy(msg.data, buffer[num], lastlen);
        msg.len = lastlen;
    }
    msg.seq = num;
    sendmessage(msg);
    clock_t start = clock();
    while (1) {
        clock_t end = clock();
        if (end - start > TIMEOUT) {
            cout << "���䳬ʱ" << endl;
            sendmessage(msg);
            start = clock();
        }
        if (status[num] == 1) {
            return 1;
        }
    }
}
int sendthread() {
    bool sending = true;
    message msg;
    while (sending) {
        for (int i = sendbase; i <= sendtop; i++) {
            if (status[i] == 0) {
                status[i] = -1;  // �ѷ���
                msg.seq = i;
                thread sendPkt(sendPacketThread, msg, i);  // �����̷߳���
                if (sendPkt.joinable()) {
                    sendPkt.detach();
                }
                else {
                    cout << "�����߳�ʧ��" << endl;
                }
                if (i == messagenum - 1) {
                    sending = false;
                }
            }
        }
    }
    return 1;
}

int recvthread() {
    message msg = recvmessage();
    if (!msg.isEXT()) {
        // û�յ��ڴ��İ�
        thread recvThr(recvthread);
        if (recvThr.joinable()) {
            recvThr.detach();
        }
        return 0;
    }
    if (msg.isACK()) {
        cout << "�յ�ackΪ" << msg.ack << "�����ݰ�" << endl;
        status[msg.ack] = 1;
        cout << endl;
        if (msg.ack == messagenum - 1) {
            return 0;
        }
    }
    thread recvThr(recvthread);
    if (recvThr.joinable()) {
        recvThr.detach();
    }
    return 1;
}
int beginsendpackets() {
    cout << "��ʼ�����ļ����ݣ�" << endl;
    if (messagenum < WINDOW) {
        // ���ݰ�����С�ڴ��ڴ�С
        sendtop = messagenum;
    }
    bool sending = true;
    timestart = clock();
    // ����һ�������߳�
    thread recvThr(recvthread);
    if (recvThr.joinable()) {
        recvThr.detach();
    }
    else {
        cout << "create recvthread failed!" << endl;
        return -1;
    }
    // ����һ�������߳�
    cout << "��ʼ�����ļ�" << endl;
    thread sendThr(sendthread);
    if (sendThr.joinable()) {
        sendThr.detach();
    }
    else {
        cout << "create sendthread failed!" << endl;
        return -1;
    }
    while (sending) {
        while (status[sendbase] == 1) {
            // �յ���ǰ�������ڵ׵����ݰ�ack
            cout << "��������ǰ��" << endl;
            if (sendbase == messagenum - 1) { // �����һ���������յ�ack
                sending = false;
                break;
            }
            sendbase++;
            if (sendtop < messagenum - 1) {
                sendtop++;
            }
            cout << "send base:" << sendbase << "  send top:" << sendtop << endl;
        }
    }
    isEnd = true;
    cout << "�ɹ������ļ���" << endl;
    timeend = clock();
    double endtime = (double)(timeend - timestart) / CLOCKS_PER_SEC;
    cout << "������ʱ��" << endtime << "s" << endl;
    cout << "������" << (double)(messagenum) * sizeof(message) * 8 / endtime / 8192  << "kbps" << endl;
    sendbase = 0;
    sendtop = WINDOW - 1;
    return sendFileName();  // ׼��������һ���ļ�
}
int main()
{
    SetColor();
    // ��ʼ���׽���
    Start();
    SetColor(12,0);
    beginconnect();
    sendFileName();
    closesocket(Client);
    WSACleanup();
    system("pause");
    return 0;

}
