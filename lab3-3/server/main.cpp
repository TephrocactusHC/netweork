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
void SetColor(int fore = 7, int back = 0) {
    unsigned char m_color = fore;
    m_color += (back << 4);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), m_color);
    return;
}

//此函数来自网络，目的是自动化证明我确实传了文件。
int scanFile()
{
    struct dirent *ptr;
    DIR *dir;
    string PATH = "./";
    dir=opendir(PATH.c_str());
    vector<string> files;
    cout << "文件列表: "<< endl;
    while((ptr=readdir(dir))!=NULL)
    {
        //跳过'.'和'..'两个目录
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
            {//溢出
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
    cout<<endl;
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
    //此处应该封装成一个类，但是封装之后完全无法调试
    //多线程会有各种问题，甚至程序断在类的中间，这都是无法想象的事情
    //因此行数很多
    //后期如果解决多线程的牛马问题可能会尝试做进一步修改。。。
    sendMsg.setSYN();
    sendMsg.setACK();
    sendMsg.ack = recvMsg.seq + 1;   // 将要发送确认包的ack设为收到包的seq+1
    sendMsg.setSYN();
    SetColor(12,0);
    cout << "发送第二次握手信息！" << endl;
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
    sendMsg.setEXT();
    sendMsg.setchecksum();
    sendto(Server, (char*)&sendMsg, BUFFER, 0, (SOCKADDR*)&clientaddr, sizeof(SOCKADDR));
    SetColor(12,0);
    cout<<"已经收到客户端发过来的挥手请求，并且发送了第二次挥手，服务器将结束运行！再见！"<<endl;
    return 0;
}
int getFileName() {
    //读取所有的文件
    scanFile();
    cout << "服务器正在等待中" << endl;
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
            messagenum = msg.num;
            SetColor(14,0);
            cout << "文件名为：" << msg.data << ", 共有" << msg.num <<"个数据包。"<< endl;
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

//关于ExitThread(TRUE);
//官方说是一个非常危险的强制杀死进程的命令，轻易不推荐使用
//理论上会释放所有占用资源，但实际上只是失去了句柄，不过也分系统和IDE。
//但是由于无法解决线程的并发控制问题，主要是死锁问题和线程释放的问题
//目前只能强制结束，不结束的后果是：陷入死循环，线程无法退出，然后疯狂重发
//在vscode上本程序貌似有一定概率实现连续发送，但是在clion和vs2022环境中一次都没成功过
//原因可能是使用gcc和clang编译器的区别导致的，也有可能是别的原因，没研究明白
//通过任务管理器查看的话，clion的线程在程序启动后会从7个增加到21个，然后迅速回落，但不少于12
//同时即使程序完全结束，线程也并不会立即释放
//而是一直交给我不知道的什么玩意托管一段时间后再释放
//因此本程序不支持连续发送，所有的路径也都被写死了
//想发送另一个文件，必须重新运行

int recvThread() {
    message recv = recvmessage();
    if(recv.seq==65534){
        disconnect();
        cout<<"收到第二次挥手，文件传输结束！"<<endl;
        system("pause");
        exit(0);
    }
    /*int id = GetCurrentThreadId();
    srand(id *10000);
    int ran = rand() % 100;
    if (ran < 5) {
        thread recvThr(recvThread);
        if (recvThr.joinable()) {
            recvThr.detach();
        }
        return 0;
    }*/
    int ack = recv.seq;
    if (state[ack] > 0 || ack < recvbase) {
        cout << "重发机制" << endl;
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
    cout << "正在接收文件" << endl;
    thread recvThr(recvThread);
    recvThr.detach();
    while (recving) {
        while (state[recvbase] == 1) {
            cout << "滑动窗口前移一位" << endl;
            if (recvbase == messagenum - 1) {
                recving = false;
                break;
            }
            recvbase=((recvbase==(messagenum-1))?recvbase:recvbase+1);
            cout << "现在窗口底部是:" << recvbase << ", 现在窗口顶部是:" << recvtop << endl;
        }
    }
    isFINISH = true;
    cout << "接收文件完成!" << endl;
    int i;
    for (i = 0; i < messagenum - 1; i++) {
        out.write(buffer[i], 8192);
    }
    out.write(buffer[i], lastlen);
    out.close();
    out.clear();
    recvbase = 0;
    recvtop = windowSize - 1;
    cout << "写文件完成" << endl;
    return getFileName();
}

int main()
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
    }
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        SetColor(12,0);
        cout << "Winsock.dll的版本不对啊宝贝儿" << endl;
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
        cout << "绑定端口" << SERVER_PORT << "出现错误：" << err << endl;
        WSACleanup();
    }
    else
    {
        SetColor(12,0);
        cout << "成功创建服务器！" << endl;
    }
    WaitConnect();
    getFileName();
    //关闭套接字
    closesocket(Server);
    WSACleanup();
    system("pause");
    return 0;
}
