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
    u_long flag{};//首部
    u_short seq{};//序列号
    u_short ack{};//确认号
    u_long len{};//数据部分长度
    u_long num{}; //发送的消息包含几个包
    u_short checksum{};//校验和
    char data[1024]{};//数据长度
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
            {//溢出
                int t = sum >> 16;  // 将最高位回滚添加至最低位
                sum += t;
            }
        }
        this->checksum = ~(u_short)sum;  // 按位取反，方便校验计算
    }
    bool corrupt(){
        // 包是否损坏
        int sum = 0;
        u_char* temp = (u_char*)this;
        for (int i = 0; i < 8; i++)
        {
            sum += (temp[i<<1] << 8) + temp[i<<1|1];
            while (sum >= 0x10000)
            {//溢出
                int t = sum >> 16;//计算方法与设置校验和相同
                sum += t;
            }
        }
        //把计算出来的校验和和报文中该字段的值相加，如果等于0xffff，则校验成功
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
        cout << "发送错误了!!" << endl;
    }
}

void Start()
{
    SetColor(14,0);
    WORD version;
    WSADATA wsaData;
    //套接字加载时错误提示
    int err;
    //版本 2.2
    version = MAKEWORD(2, 2);
    //加载 dll 文件 Scoket 库
    err = WSAStartup(version, &wsaData);
    if (err != 0) {
        //找不到 winsock.dll
        SetColor(12,0);
        cout << "初始化套接字错误: " << err << endl;
        return;
    }
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        SetColor(12,0);
        cout << "Winsock.dll的版本不对啊宝贝儿" << endl;
        WSACleanup();
        return;
    }
    Server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    //设置套接字为非阻塞模式
    int iMode = 1; //1：非阻塞，0：阻塞
    ioctlsocket(Server, FIONBIO, (u_long FAR*) & iMode);//非阻塞设置
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
        cout << "绑定端口" << SERVER_PORT << "出现错误：" << err << endl;
        WSACleanup();
        return;
    }
    else
    {
        SetColor(12,0);
        cout << "成功创建服务器！" << endl;
    }
}

int WaitConnect()
{
    SetColor(12,0);
    cout << "服务器等待连接" << endl;
    message recvMsg, sendMsg;
    while (true)
    {
        recvMsg = recvmessage();
        if (recvMsg.isSYN())
        {
            SetColor(12,0);
            cout << "收到第一次握手成功！" << endl;
            break;
        }
    }
    sendMsg.setSYN();
    sendMsg.setACK();
    sendMsg.ack = recvMsg.seq + 1;   // 将要发送确认包的ack设为收到包的seq+1
    sendMsg.setSYN();
    SetColor(12,0);
    cout << "发送第二次握手信息！" << endl;
    sendmessage(sendMsg);
    sendMsg.output();
    int count = 0;
    while (true) {
        Sleep(100);
        if (count >= 50) {
            SetColor(12,0);
            cout << "等待时间太长，退出连接" << endl;
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
    cout << "接收到确认连接，连接成功" << endl;
    int iMode = 0; //1：非阻塞，0：阻塞
    ioctlsocket(Server, FIONBIO, (u_long FAR*) & iMode);//非阻塞设置
    return 0;
}
int closeconnect(message msg){
    message sendMsg;
    sendMsg.setACK();
    sendMsg.ack = msg.seq + 1;
    sendmessage(sendMsg);
    SetColor(12,0);
    cout<<"已经收到客户端发过来的挥手请求，并且发送了第二次挥手，服务器将结束运行！再见！"<<endl;
    return 0;
}
int getFileName() {
    cout << "服务器正在等待中......" << endl;
    message msg, sendMsg;
    int start = clock();
    int end;
    while (true) {
        msg = recvmessage();
        if (msg.isFIN()) {
            SetColor(12,0);
            cout << "客户端准备断开连接！进入挥手模式！" << endl;
            closeconnect(msg);
            break;
        }
        if (!msg.isEXT())
        {
            end = clock();
            if (end - start > 2000) {
                SetColor(14,0);
                cout << "连接超时" << endl;
                cout << "重新进入等待接收文件的模式" << endl;
            }
            continue;
        }
        if (msg.isSTART()) {
            ZeroMemory(filepath, 20);
            memcpy(filepath, msg.data, msg.len);
            out.open(filepath, std::ios::out | std::ios::binary);
            SetColor(14,0);
            cout << "文件名为：" << filepath << endl;
            if (!out.is_open())
            {
                SetColor(14,0);
                cout << "文件打开失败！！！" << endl;
                exit(1);
            }
            messagenum = msg.num;
            SetColor(14,0);
            cout << "文件名为：" << msg.data << "共有" << msg.num <<"个数据包。"<< endl;
            sendMsg.setACK();
            sendMsg.ack = msg.seq;
            sendmessage(sendMsg);
            timestart = clock();
            return recvmessages();
        }
    }
}
int recvmessages() {
    cout << "开始接收文件内容！" << endl;
    message recvMsg, sendMsg;
    int seq = 1;
    int iMode = 1; //1：非阻塞，0：阻塞
    ioctlsocket(Server, FIONBIO, (u_long FAR*) & iMode);//非阻塞设置
    for (int i = 0; i < messagenum; i++) {
        int start = clock();
        int end;
        while (1) {
            end = clock();
            if (end - start > 2000) {
                SetColor(14,0);
                cout << "传输超时失败" << endl;
                cout << "请确认网络通畅后重新进行文件接收工作！谢谢！" << endl;
                int iMode = 0; //1：非阻塞，0：阻塞
                ioctlsocket(Server, FIONBIO, (u_long FAR*) & iMode);//非阻塞设置
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
                cout << "收到seq为" << recvMsg.seq << "的数据包" << endl;
                cout<<"checksum="<<recvMsg.corrupt()<<", len="<<recvMsg.len<<endl;
                SetColor(14,0);
                cout << "发送确认收到的数据包(对应的ack)" << endl;
                sendmessage(sendMsg);
                cout << endl;
                out.write(recvMsg.data, recvMsg.len);
                break;
            }
        }
        if (recvMsg.isEND()) {
            SetColor(14,0);
            cout << "接收文件成功！！" << endl << endl;
            out.close();
            out.clear();
            timeend = clock();
            double endtime = (double)(timeend - timestart) / CLOCKS_PER_SEC;
            cout << "传输总时间" << endtime << "s" << endl;
            cout << "吞吐率" << (double)(messagenum) * sizeof(message) * 8 / endtime / 1024  << "kbps" << endl;
            int iMode = 0; //1：非阻塞，0：阻塞
            ioctlsocket(Server, FIONBIO, (u_long FAR*) & iMode);//非阻塞设置
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
    //关闭套接字
    closesocket(Server);
    WSACleanup();
    system("pause");
    return 0;
}
