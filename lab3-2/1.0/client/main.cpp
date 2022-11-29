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
        cout << "发送错误了!!" << endl;
    }
}

void Start()
{
    //加载套接字库（必须）
    WORD version;
    WSADATA wsaData;
    //套接字加载时错误提示
    int err;
    //版本 2.2
    version = MAKEWORD(2, 2);
    //加载 dll 文件 Scoket 库
    err = WSAStartup(version, &wsaData);
    if (err != 0)
    {
        //找不到 winsock.dll
        SetColor(12,0);
        cout << "初始化套接字错误了: " << err << endl;
        return;
    }
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        SetColor(12,0);
        cout << "Winsock.dll版本错误" << endl;
        WSACleanup();
    }
    else
    {
        SetColor(12,0);
        cout << "套接字创建成功" << endl;
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
        cout << "绑定端口" << CLIENT_PORT << "出现错误：" << err << endl;
        WSACleanup();
        return;
    }
    else
    {
        SetColor(12,0);
        cout << "成功创建客户端！" << endl;
    }
}

int beginconnect()
{
    //int iMode = 1; //1：非阻塞，0：阻塞
    //ioctlsocket(Client, FIONBIO, (u_long FAR*) & iMode);//非阻塞设置
    SetColor(12,0);
    cout << "开始连接！发送第一次握手！" << endl;
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
                cout << "连接超时,请确认网络通畅和服务端启动无误后再运行本程序！" << endl;
                sendmessage(sendMsg);
                break;
            }
            continue;
        }
        if (recvMsg.isACK() && recvMsg.isSYN()&& recvMsg.ack == sendMsg.seq + 1) {
            SetColor(14,0);
            cout << "收到第二次握手!" << endl;
            break;
        }
    }
    sendMsg.setACK();
    sendMsg.seq = 89;
    sendMsg.ack = recvMsg.seq + 1;
    SetColor(14,0);
    cout << "发送第三次握手的数据包" << endl;
    sendmessage(sendMsg);
    return 0;
}

int openFile() {
    SetColor(10,0);
    cout << "请输入要发送的文件名：";
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
            in.open(filepath, ifstream::in | ios::binary);// 以二进制方式打开文件
            in.seekg(0, std::ios_base::end);  // 将文件流指针定位到流的末尾
            filelen = in.tellg();
            messagenum = filelen / 8192 + 1;
            SetColor(0,6);
            lastlen = filelen - (messagenum - 1) * 8192;
            cout << "文件大小为" << filelen << "Bytes,总共有" << messagenum << "个数据包" << endl;
            in.seekg(0, std::ios_base::beg);  // 将文件流指针定位到流的开始
            int index = 0, len = 0;
            // 将文件读入缓冲区
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
            cout<<"文件不存在，请重新输入您要传输的文件名！"<<endl;
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
    cout << "发送所要传输的文件名" << endl;
    if (waitSend(msg, 0) == 0) {
        SetColor(12,0);
        cout << "发出文件名失败" << endl;
        return 0;
    }
    timestart = clock();
    return beginsendpackets();
}

int closeconnect() {  // 断开连接
    message recvMsg, sendMsg;
    sendMsg.setFIN();
    sendMsg.seq = 65534;//此处是u_short的表示范围的最大值-1，而我们收到的将会再加一，那么已经到了u_short的最大值了，就自然结束了。
    sendmessage(sendMsg);
    cout << "发送出去第一次挥手！" << endl;
    int count = 0;
    SetColor(12,0);
    cout << "接收到确认连接，断开连接成功" << endl << endl;
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
            cout << "无应答，退出" << endl;
            return sendFileName();
        }
        if (end - start > 50) {
            SetColor(12,0);
            cout << "应答超时，重新发送数据包" << endl;
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
            cout << "收到服务器发来的ack正确的确认数据包！" << endl;
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
            cout << "传输超时" << endl;
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
                status[i] = -1;  // 已发送
                msg.seq = i;
                thread sendPkt(sendPacketThread, msg, i);  // 创建线程发包
                if (sendPkt.joinable()) {
                    sendPkt.detach();
                }
                else {
                    cout << "创建线程失败" << endl;
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
        // 没收到期待的包
        thread recvThr(recvthread);
        if (recvThr.joinable()) {
            recvThr.detach();
        }
        return 0;
    }
    if (msg.isACK()) {
        cout << "收到ack为" << msg.ack << "的数据包" << endl;
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
    cout << "开始发送文件内容！" << endl;
    if (messagenum < WINDOW) {
        // 数据包总数小于窗口大小
        sendtop = messagenum;
    }
    bool sending = true;
    timestart = clock();
    // 创建一个接收线程
    thread recvThr(recvthread);
    if (recvThr.joinable()) {
        recvThr.detach();
    }
    else {
        cout << "create recvthread failed!" << endl;
        return -1;
    }
    // 创建一个发送线程
    cout << "开始发送文件" << endl;
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
            // 收到当前滑动窗口底的数据包ack
            cout << "滑动窗口前移" << endl;
            if (sendbase == messagenum - 1) { // 除最后一个包都接收到ack
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
    cout << "成功发送文件！" << endl;
    timeend = clock();
    double endtime = (double)(timeend - timestart) / CLOCKS_PER_SEC;
    cout << "传输总时间" << endtime << "s" << endl;
    cout << "吞吐率" << (double)(messagenum) * sizeof(message) * 8 / endtime / 8192  << "kbps" << endl;
    sendbase = 0;
    sendtop = WINDOW - 1;
    return sendFileName();  // 准备发送下一个文件
}
int main()
{
    SetColor();
    // 初始化套接字
    Start();
    SetColor(12,0);
    beginconnect();
    sendFileName();
    closesocket(Client);
    WSACleanup();
    system("pause");
    return 0;

}
