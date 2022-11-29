#include <WinSock2.h>
#include<iostream>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <thread>
using namespace std;
#pragma warning(disable:4996)
#pragma comment(lib,"ws2_32.lib")
#define SERVER_IP "127.0.0.1"
#define CLIENT_IP "127.0.0.1"
#define SERVER_PORT 8888
#define CLIENT_PORT 8080
#define BUFFER sizeof(message)
#define WINDOW 10
clock_t timestart;
clock_t timeend;
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
    char data[8192]{};//数据长度
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
int recvpackets();
void sendmessage(message msg);
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
int status[20000];
bool recving = true;
bool isEnd = false;
int getFileName();
int savefile();
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
            return recvpackets();
        }
    }
}
int recvThread() {
    message recv = recvmessage();
    if (!recv.isEXT()) {
        thread recvThr(recvThread);
        if (recvThr.joinable()) {
            recvThr.detach();
        }
        return 0;
    }
    //recv.output();
    if(recv.seq==65534){
        cout<<"收到第二次挥手，文件传输结束！"<<endl;
        system("pause");
        exit(0);
        system("pause");
    }
    int ack = recv.seq;
    if (status[ack] == 1 || ack < recvbase) {
        cout << "收到无效包裹" << endl;
        message send;
        send.setACK();
        send.ack = ack;
        sendmessage(send);
    }
    if (status[ack] == 0) {
        message send;
        send.setACK();
        send.ack = ack;
        sendmessage(send);
        memcpy(buffer[ack], recv.data, recv.len);
        status[ack] = 1;
        if (ack == messagenum - 1) {
            lastlen = recv.len;
            /*return 0;*/
        }
    }
    if (isEnd) {
        return 0;
    }
    thread recvThr(recvThread);
    if (recvThr.joinable()) {
        recvThr.detach();
    }
    return 0;
}

int recvpackets() {
    recvbase = 0;
    if (messagenum <= WINDOW) {
        recvtop = messagenum - 1;
    }
    else {
        recvtop = WINDOW - 1;
    }
    cout << "开始接收文件内容！" << endl;
    message recv, send;
    int seq = 1;
    thread recvThr(recvThread);
    if (recvThr.joinable()) {
        recvThr.join();
    }
    else {
        cout << "create recvThread failed!" << endl;
    }
    while (recving) {
        while (status[recvbase] == 1) {
            cout << "滑动窗口移动" << endl;
            if (recvbase == messagenum - 1) {
                recving = false;
                break;
            }
            else {
                recvbase++;
            }
            if (recvtop < messagenum - 1) {
                recvtop++;
            }
            cout << "recv base:" << recvbase << "  recv top:" << recvtop << endl;
        }
    }
    isEnd = true;
    cout << "接收完成" << endl;
    return savefile();
}

int savefile() {
    int i;
    for (i = 0; i < messagenum - 1; i++) {
        out.write(buffer[i], 8192);
    }
    out.write(buffer[i], lastlen);
    out.close();
    out.clear();

    recvbase = 0;
    recvtop = WINDOW - 1;
    cout << "写文件完成" << endl;
    return getFileName();
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
