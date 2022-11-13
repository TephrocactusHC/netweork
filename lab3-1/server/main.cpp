//server端接收文件

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
struct message//报文格式
{
#pragma pack(1)//以下内容按1Byte对齐，如果没有这条指令会以4Byte对齐，例如u_short类型会用2B存信息，2B补零，方便后续转换成char*格式
    //ACK=0x01, SYN=0x02, FIN=0x04,EXIST 0x10,startfile 0x20,endfile 0x40
    u_long flag;//标志位
    u_long SendIP;
    u_long RecvIP;//发送端IP和接收端IP
    u_long SendPort, RecvPort;//发送端端口和接收端端口
    u_short seq;//消息序号
    u_short ack;//恢复ack时确认对方的消息的序号
    u_long len;//数据部分长度
    u_long index;//用于描述文件大小，需要多少条消息才能传输完成（1条index=0,2条index=1，以此类推）
    u_short checksum;//校验和
    char msg[BUF_SIZE];//报文的具体内容，本次实验简化为固定长度
#pragma pack()//恢复4Byte编址格式
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
            {//溢出
                int t = sum >> 16;  // 将最高位回滚添加至最低位
                sum += t;
            }
        }
        this->checksum = ~(u_short)sum;  // 按位取反，方便校验计算
    }
    bool corrupt() {
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
// 发送消息
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
// 开启接收端
int StartServer()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cout << "WSAStartup错误：" << WSAGetLastError() << endl;
        return -1;
    }
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        cout << "版本不对啊宝贝！" << endl;
        WSACleanup();
        return -1;
    }
    server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (server == SOCKET_ERROR)
    {
        cout << "套接字错误：" << WSAGetLastError() << endl;
        return -1;
    }
    //设置套接字为非阻塞模式
    //int iMode = 1; //1：非阻塞，0：阻塞
    //ioctlsocket(server, FIONBIO, (u_long FAR*) & iMode);//非阻塞设置
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    //clientAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    //clientAddr.sin_family = AF_INET;
    //clientAddr.sin_port = htons(6666);//6666
    if (bind(server, (SOCKADDR*)&serverAddr, sizeof(serverAddr)))
    {
        cout << "绑定端口错误：" << WSAGetLastError() << endl;
        return -1;
    }
    cout << "成功启动接收端！" << endl;
    return 1;
}
// 等待连接
void WaitConnection()
{
    cout << "等待连接中哦亲！" << endl;
    struct message recvMsg, sendMsg;
    int count = 0;
    while (1)
    {
        recvMsg = recvPacket();
        if (recvMsg.isSYN())
        {
            cout << "收到连接请求第一次握手" << endl;
            break;
        }
    }
    sendMsg.setSYN();
    sendMsg.setACK();
    sendMsg.ack = recvMsg.seq + 1;   // 将要发送确认包的ack设为收到包的seq+1
    sendMsg.setSYN();
    cout << "发送收到连接请求的数据包" << endl;
    sendPacket(sendMsg);
    count = 0;
    while (true) {
        Sleep(100);
        if (count >= 50) {
            cout << "等待时间太长，退出连接" << endl;
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
    cout << "接收到确认连接，连接成功" << endl << endl;

}

// 等待断开连接

// 接收

// 接收文件名
void RecvName()
{
    char name[100];
    int clientlen = sizeof(clientAddr);
    while (recvfrom(server, name, 100, 0, (sockaddr*)&clientAddr, &clientlen) == SOCKET_ERROR);
    cout << "文件名为: ";
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
    cout << "成功连接到发送端！" << endl;
    RecvName();
    /*
    cout<<'1';
    Recvmessage();
    cout << "接收文件结束！" << endl;
    WaitDisconnection();
    closesocket(server);*/
    WSACleanup();
    system("pause");
    return 0;
}
