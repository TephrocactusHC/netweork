#include <WinSock2.h>
#include<iostream>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <thread>
#include<io.h>
#include <dirent.h>
#include <vector>
using namespace std;
#pragma warning(disable:4996)
#pragma comment(lib,"ws2_32.lib")
#define SERVER_IP "127.0.0.1"
#define CLIENT_IP "127.0.0.1"
#define SERVER_PORT 8888
#define CLIENT_PORT 8080
#define BUFFER sizeof(message)
const int windowSize=10;
clock_t timestart;
clock_t timeend;
//�ڼ����֣����������API�ĵ��ã���������־�������������������
//���ǣ��ⶼ��һЩ����̨�����⣬��ʵ����Ҫ��ĳ���ĺ������޹�
void SetColor(int fore = 7, int back = 0) {
    unsigned char m_color = fore;
    m_color += (back << 4);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), m_color);
    return;
}

//�˺����޸������磬Ŀ�����Զ���֤����ȷʵ�����ļ���
//��ʵ����Ҫ�����ݹ�ϵ���󣬿���Ϊ��һЩС�ļӷ���
//����ɨ���ļ����µ��ļ�����������ǵ��ļ���
//������ɨ�����ļ���
//���ͨ��ǰ��Աȣ����Ե�֪ȷʵ����ɹ��ļ���
//��Ȼ����Ҫע����ǣ��������ļ�������˵�����⣬��Ȼ��Ҫ�鿴�ļ���������֤������ɹ�
int scanFile()
{
    struct dirent *ptr;
    DIR *dir;
    string PATH = "./";
    dir=opendir(PATH.c_str());
    vector<string> files;
    cout << "�ļ��б�: "<< endl;
    while((ptr=readdir(dir))!=NULL)
    {
        //����'.'��'..'����Ŀ¼
        if(ptr->d_name[0] == '.')
            continue;
        files.push_back(ptr->d_name);
    }

    for (int i = 0; i < files.size(); ++i)
    {
        cout << files[i] << endl;
    }
    closedir(dir);
    return 0;
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
    bool isFINISH(){
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
                int t = sum >> 16;
                sum += t;
            }
        }
        this->checksum = ~(u_short)sum;
    }
    bool corrupt(){
        int sum = 0;
        u_char* temp = (u_char*)this;
        for (int i = 0; i < 8; i++)
        {
            sum += (temp[i<<1] << 8) + temp[i<<1|1];
            while (sum >= 0x10000)
            {
                int t = sum >> 16;
                sum += t;
            }
        }
        if (checksum + (u_short)sum == 65535)
            return false;
        return true;
    }
    void output() {
        cout << "checksum=" << this->corrupt() << ", len=" << this->len << ", flag="<<this->flag<<", seq="<<this->ack<<endl;
    }
};

message recvmessage();
int recvMsg();
int len =sizeof(SOCKADDR);
int lastlen;
SOCKADDR_IN serveraddr, clientaddr;
SOCKET Server;
char filepath[20];
ofstream out;
int messagenum;
int recvbase;
int recvtop;
char buffer[20000][8192];
int state[20000];
bool recving = true;
bool isFINISH = false;
int getFileName();
message recvmessage()
{
    message msg;
    if (recvfrom(Server, (char*)&msg, BUFFER, 0, (SOCKADDR*)&clientaddr, &len) ==-1 || !msg.isEXT() || msg.corrupt()) {
        return message();
    }
    return msg;
}
int disconnect(){
    message sendMsg;
    sendMsg.setACK();
    sendMsg.ack = msg.seq + 1;
    sendmessage(sendMsg);
    SetColor(12,0);
    cout<<"�Ѿ��յ��ͻ��˷������Ļ������󣬲��ҷ����˵ڶ��λ��֣����������������У��ټ���"<<endl;
    return 0;
}
//�ȴ�����������3-1��ͬ����Ȼ��ͣ�Ȼ��ơ�
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
    //�˴�Ӧ�÷�װ��һ���࣬���Ƿ�װ֮����ȫ�޷�����
    //���̻߳��и������⣬���������������м䣬�ⶼ���޷����������
    //��������ܶ�
    //�������������̵߳�ţ��������᳢ܻ������һ���޸ġ�����
    sendMsg.setSYN();
    sendMsg.setACK();
    sendMsg.ack = recvMsg.seq + 1;   // ��Ҫ����ȷ�ϰ���ack��Ϊ�յ�����seq+1
    sendMsg.setSYN();
    SetColor(12,0);
    cout << "���͵ڶ���������Ϣ��" << endl;
    sendMsg.setEXT();
    sendMsg.setchecksum();
    sendto(Server, (char*)&sendMsg, BUFFER, 0, (SOCKADDR*)&clientaddr, sizeof(SOCKADDR));
    time_t now_time = time(NULL);
    tm *t_tm = localtime(&now_time);
    sendMsg.output();
    cout << asctime(t_tm) << endl;
    int count = 0;
    while (1) {
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
    return 0;
}
int closeconnect(message msg){
    message sendMsg;
    sendMsg.setACK();
    sendMsg.ack = msg.seq + 1;
    sendMsg.setEXT();
    sendMsg.setchecksum();
    sendto(Server, (char*)&sendMsg, BUFFER, 0, (SOCKADDR*)&clientaddr, sizeof(SOCKADDR));
    SetColor(12,0);
    cout<<"�Ѿ��յ��ͻ��˷������Ļ������󣬲��ҷ����˵ڶ��λ��֣����������������У��ټ���"<<endl;
    return 0;
}
int getFileName() {
    //��ȡ���е��ļ�
    scanFile();
    cout << "���������ڵȴ���" << endl;
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
            messagenum = msg.num;
            SetColor(14,0);
            cout << "�ļ���Ϊ��" << msg.data << ", ����" << msg.num <<"�����ݰ���"<< endl;
            sendMsg.setACK();
            sendMsg.ack = msg.seq;
            sendMsg.setEXT();
            sendMsg.setchecksum();
            sendto(Server, (char*)&sendMsg, BUFFER, 0, (SOCKADDR*)&clientaddr, sizeof(SOCKADDR));
            timestart = clock();
            return recvMsg();
        }
    }
}

//����ExitThread(TRUE);
//�ٷ�˵��һ���ǳ�Σ�յ�ǿ��ɱ�����̵�������ײ��Ƽ�ʹ��
//�����ϻ��ͷ�����ռ����Դ����ʵ����ֻ��ʧȥ�˾��������Ҳ��ϵͳ��IDE��
//���������޷�����̵߳Ĳ����������⣬��Ҫ������������߳��ͷŵ�����
//Ŀǰֻ��ǿ�ƽ������������ĺ���ǣ�������ѭ�����߳��޷��˳���Ȼ�����ط�
//��vscode�ϱ�����ò����һ������ʵ���������ͣ�������clion��vs2022������һ�ζ�û�ɹ���
//ԭ�������ʹ��gcc��clang�������������µģ�Ҳ�п����Ǳ��ԭ��û�о�����
//ͨ������������鿴�Ļ���clion���߳��ڳ�����������7�����ӵ�21����Ȼ��Ѹ�ٻ��䣬��������12
//ͬʱ��ʹ������ȫ�������߳�Ҳ�����������ͷ�
//����һֱ�����Ҳ�֪����ʲô�����й�һ��ʱ������ͷ�
//��˱�����֧���������ͣ����е�·��Ҳ����д����
//�뷢����һ���ļ���������������

int recvThread() {
    message recv = recvmessage();
    if(recv.seq==65534){
        disconnect();//��֪��Ϊʲô����closeconnect����������ʹ����������ȫһ����������˱������disconnect
        cout<<"�յ��ڶ��λ��֣��ļ����������"<<endl;
        system("pause");
        exit(0);
    }
    int ack = recv.seq;
    if (state[ack] > 0 || ack < recvbase) {
        cout << "�ط�����" << endl;
        message sendMsg;
        sendMsg.setACK();
        sendMsg.ack = ack;
        sendMsg.setEXT();
        sendMsg.setchecksum();
        sendto(Server, (char*)&sendMsg, BUFFER, 0, (SOCKADDR*)&clientaddr, sizeof(SOCKADDR));
        time_t now_time = time(NULL);
        tm *t_tm = localtime(&now_time);
        sendMsg.output();
        cout << asctime(t_tm) << endl;
    }
    if (state[ack] == 0) {
        message sendMsg;
        sendMsg.setACK();
        sendMsg.ack = ack;
        sendMsg.setEXT();
        sendMsg.setchecksum();
        sendto(Server, (char*)&sendMsg, BUFFER, 0, (SOCKADDR*)&clientaddr, sizeof(SOCKADDR));
        memcpy(buffer[ack], recv.data, recv.len);
        state[ack] = 1;
        lastlen=((ack==(messagenum-1))? recv.len: lastlen);
        time_t now_time = time(NULL);
        tm *t_tm = localtime(&now_time);
        sendMsg.output();
        cout << asctime(t_tm) << endl;
    }
    if (isFINISH) {ExitThread(TRUE);}
    thread recvThr(recvThread);
    recvThr.detach();
    ExitThread(TRUE);
}

int recvMsg() {
    recvbase = 0;
    recvtop =((messagenum <= windowSize)?(messagenum - 1):( windowSize - 1));
    cout << "���ڽ����ļ�" << endl;
    thread recvThr(recvThread);
    recvThr.detach();
    while (recving) {
        while (state[recvbase] == 1) {
            cout << "��������ǰ��һλ" << endl;
            if (recvbase == messagenum - 1) {
                recving = false;
                break;
            }
            recvbase=((recvbase==(messagenum-1))?recvbase:recvbase+1);
            recvtop=((recvtop<(messagenum-1))?recvtop+1:recvtop);
            cout << "���ڴ��ڵײ���:" << recvbase << ", ���ڴ��ڶ�����:" << recvtop << endl;
        }
    }
    isFINISH = true;
    cout << "�����ļ����!" << endl;
    int i;
    for (i = 0; i < messagenum - 1; i++) {
        out.write(buffer[i], 8192);
    }
    out.write(buffer[i], lastlen);
    out.close();
    out.clear();
    recvbase = 0;
    recvtop = windowSize - 1;
    cout << "д�ļ����" << endl;
    return getFileName();
}

//������������ʾ������߳�����sleep()��䣬���ܻ�˳�������־������⣬���������Բ�����Ч
//�²���Ȼ��clion���µ����⡣����Ϊvs��ĳ���汾���ܻ�˳�����
//��������������ע�⣬�����ڿͻ��˷���FINISH���˳�
//����ᵼ���̵߳��ͷ�����
//�����˳�֮�󣬲鿴clion���̣߳���Ȼ����ô�����û����Ϊ����ctrl+C���������ͷ�
//�����̵߳��й��漰��֪ʶ���ڵײ�
//��������������Χ������ʹ��FINISH�˳���


int main()
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
    }
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        SetColor(12,0);
        cout << "Winsock.dll�İ汾���԰�������" << endl;
        WSACleanup();
    }
    Server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
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
    }
    else
    {
        SetColor(12,0);
        cout << "�ɹ�������������" << endl;
    }
    WaitConnect();
    getFileName();
    //�ر��׽���
    closesocket(Server);
    WSACleanup();
    system("pause");
    return 0;
}
