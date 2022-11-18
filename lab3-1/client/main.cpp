#include <WinSock2.h>
#include<iostream>
#include <stdlib.h>
#include <time.h>
#include <fstream>
using namespace std;
#pragma comment(lib,"ws2_32.lib")
#define SERVER_IP "127.0.0.1"
#define CLIENT_IP "127.0.0.1"
#define SERVER_PORT 8888
#define CLIENT_PORT 8080
#define BUFFER sizeof(message)
#pragma warning(disable:4996)
int judgeRand(){
    int s=rand()%100;
    if(s<-1){return 0;}
    else{return 1;}
}
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
};

SOCKADDR_IN serveraddr,clientaddr;
SOCKET Client;
int messagenum;
int len = sizeof(SOCKADDR);
char filepath[20];
clock_t timestart;
clock_t timeend;
int filelen;
ifstream in;

message recvmessage();
void sendmessage(message msg);
int openFile();
int closeconnect();
int waitSend(message send, int seq);
int sendmessages();
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
    if(judgeRand()==1){
    if (sendto(Client, (char*)&msg, BUFFER, 0, (SOCKADDR*)&serveraddr, sizeof(SOCKADDR)) == (SOCKET_ERROR)) {
        SetColor(0,12);
        cout << "���ʹ�����!!" << endl;
    }}
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
        SetColor(0,12);
        cout << "��ʼ���׽��ִ�����: " << err << endl;
        return;
    }
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        SetColor(0,12);
        cout << "Winsock.dll�汾����" << endl;
        WSACleanup();
    }
    else
    {
        SetColor(0,12);
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
        SetColor(0,12);
        cout << "�󶨶˿�" << CLIENT_PORT << "���ִ���" << err << endl;
        WSACleanup();
        return;
    }
    else
    {
        SetColor(0,12);
        cout << "�ɹ������ͻ��ˣ�" << endl;
    }
}

int beginconnect()
{
    int iMode = 1; //1����������0������
    ioctlsocket(Client, FIONBIO, (u_long FAR*) & iMode);//����������
    SetColor(0,12);
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
                SetColor(0,12);
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
    SetColor(0,10);
    cout << "������Ҫ���͵��ļ�����";
    memset(filepath, 0, 20);
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
        messagenum = filelen / 1024 + 1;
        SetColor(0,6);
        cout << "�ļ���СΪ" << filelen << "Bytes,�ܹ���" << messagenum << "�����ݰ�" << endl;
        in.seekg(0, std::ios_base::beg);  // ���ļ���ָ�붨λ�����Ŀ�ʼ
        return 1;
    }
        else{
            SetColor(0,12);
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
        SetColor(0,12);
        cout << "�����ļ���ʧ��" << endl;
        return 0;
    }
    timestart = clock();
    return sendmessages();
}
int sendmessages() {
    SetColor(0,6);
    cout << "��ʼ�����ļ����ݣ�" << endl;
    message msg;
    int seq = 1;

    while (filelen) {
        if (filelen > 1024)
        {
            in.read(msg.data, 1024);
            msg.len = 1024;
            filelen -= 1024;
        }
        else
        {
            in.read(msg.data, filelen);
            msg.len = filelen;
            msg.setEND();
            filelen = 0;
        }
        SetColor(14,0);
        cout << "����seqΪ" << seq << "�����ݰ�" << endl;
        if (waitSend(msg, seq) == 0) {
            SetColor(0,12);
            cout << "����seqΪ" << seq << "�����ݰ�ʧ�ܣ�����" << endl << endl;
            cout << "�ط�ʧ�ܣ���ȷ������ͨ���Լ���������������������ͻ��˲����·����ļ����ټ���" << endl;
            return 0;
        }
        seq++;
    }
    SetColor(0,6);
    cout << "�ɹ������ļ���" << endl;
    timeend = clock();
    double endtime = (double)(timeend - timestart) / CLOCKS_PER_SEC;
    SetColor(0,6);
    cout << "������ʱ��" << endtime << "s" << endl;
    cout << "������" << (double)(messagenum) * sizeof(message) * 8 / endtime / 1024  << "kbps" << endl;
    in.close();
    in.clear();
    return sendFileName();  // ׼��������һ���ļ�
}
int closeconnect() {  // �Ͽ�����
    message recvMsg, sendMsg;
    sendMsg.setFIN();
    sendMsg.seq = 65534;//�˴���u_short�ı�ʾ��Χ�����ֵ-1���������յ��Ľ����ټ�һ����ô�Ѿ�����u_short�����ֵ�ˣ�����Ȼ�����ˡ�
    sendmessage(sendMsg);
    cout << "���ͳ�ȥ��һ�λ��֣�" << endl;
    int count = 0;
    while (true) {
        Sleep(100);
        if (count >= 50) {
            SetColor(0,12);
            cout << "�ȴ�ʱ��̫�����˳�����" << endl;
            return closeconnect();
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
    SetColor(0,12);
    cout << "���յ�ȷ�����ӣ��Ͽ����ӳɹ�" << endl << endl;
    return 0;
}
int waitSend(message sendMsg, int seq)
{
    message recvMsg;
    sendMsg.seq = seq;
    sendmessage(sendMsg);
    int iMode = 1; //1����������0������
    ioctlsocket(Client, FIONBIO, (u_long FAR*) & iMode);//����������
    int count = 0;
    clock_t start = clock();
    clock_t end;
    while (1) {
        end = clock();
        if (end - start > 50) {
            SetColor(0,12);
            cout << "Ӧ��ʱ�����·������ݰ�" << endl;
            sendmessage(sendMsg);
            count++;
            SetColor(0,12);
            cout<<"�������·��͵�"<<count<<"�Σ����10��"<<endl;
            if(count>=10){
                SetColor(0,12);
                break;
            }
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
    return 0;
}

int main()
{
    SetColor();
    // ��ʼ���׽���
    Start();
    SetColor(0,12);
    beginconnect();
    sendFileName();
    closesocket(Client);
    WSACleanup();
    system("pause");
    return 0;

}
