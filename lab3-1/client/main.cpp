
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
    if (recvfrom(client,(char*)&msg, BUFFER, 0, (SOCKADDR*)&serverAddr, (int*)sizeof(SOCKADDR)) == -1 || !msg.isEXT() || msg.corrupt()) {
        return message();
    }
    return msg;
}
// 发送消息
void sendPacket(message msg) {
    msg.setEXT();
    msg.setchecksum();
    if (sendto(client, (char*)&msg, BUFFER, 0, (SOCKADDR*)&serverAddr, sizeof(SOCKADDR)) == (SOCKET_ERROR)) {
        cout << "socket error in sendPacket!!" << endl;
    }
    msg.output();
}
// 连接到服务器
void ConnectToServer()
{
    cout << "开始连接了亲！" << endl;

    struct message recvMsg, sendMsg;
    sendMsg.setSYN();
    sendMsg.seq = 88;
    sendPacket(sendMsg);
    cout<<"已经发送第一次握手了哟亲(^_^)"<<endl;

    int start = clock();
    int end;
    while (1)
    {
        recvMsg = recvPacket();
        if (!recvMsg.isEXT())
        {
            end = clock();
            if (end - start > 2000) {
                cout << "连接超时!!!FBI WARNING!!!" << endl;
            }
            continue;
        }
        if (recvMsg.isACK() && recvMsg.isSYN()&& recvMsg.ack == sendMsg.seq + 1) {
            cout << "收到服务端发来的第二次握手了哦！" << endl;
            break;
        }
    }
    sendMsg.init_message();
    sendMsg.setACK();
    sendMsg.seq = 89;
    sendMsg.ack = recvMsg.seq + 1;
    cout << "发送第三次握手的数据包" << endl;
    sendPacket(sendMsg);
    return ;
}
// 发送文件名
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

// 开始客户端
int StartClient()
{
    //加载套接字库
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cout << "WSAStartup错误了宝贝儿，快自己看看报错是咋回事吧<(-_-;<)：" << WSAGetLastError() << endl;
        return -1;
    }
    client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (client == SOCKET_ERROR)
    {
        cout << "套接字错误啦宝贝，快自己看看报错是咋回事吧<(-_-;<)：" << WSAGetLastError() << endl;
        return -1;
    }
    //设置套接字为非阻塞模式
    int iMode = 1; //1：非阻塞，0：阻塞
    ioctlsocket(client, FIONBIO, (u_long FAR*) & iMode);//非阻塞设置


    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    clientAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(6666);
    cout << "成功启动client端！哇哈哈哈哈哈(^_^)" << endl;
    cout<<"-----------------------------------------------------------"<<endl;
    return 1;
}

// 关闭客户端

//发送第一个包其实是
/*int sendhead() {
    message msg;
    if (openfile() == 0) {
        return 0;
    }
    memcpy(msg.data, filepath, strlen(filepath));
    msg.len = strlen(filepath);
    msg.num = packetnum;
    msg.setFIR();
    cout << "发出目标文件头数据包" << endl;
    timestart = clock();
    if (sendwait(msg, 0) == 0) {
        cout << "发出目标文件头数据包失败" << endl;
        return 0;
    }
    return sendpackets();
}*/
int main()
{
    cout << "程序开始运行了!!! FBI WARNING!!!" << endl;
    StartClient();
    cout<<"接下来开始握手了哦(*`o`*)!!!"<<endl;
    ConnectToServer();
    cout << "成功连接到接收端了哟(^_^)！" << endl;

    int time_out = 1;
    setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (char*)&time_out, sizeof(time_out));
    string filename;
    cin >> filename;
    while(filename!="1.jpg"||filename!="2.jpg"||filename!="3.jpg"||filename!="helloworld.txt"){
        cout<<"没有这个文件，请重新输入好吗！"<<endl;
        cin>>filename;
    }
    SendName(filename, filename.length());

    cout << "发送成功了亲！(^o^)" << endl;

    WSACleanup();
    system("pause");
    return 0;
}
