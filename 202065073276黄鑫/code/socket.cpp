#include<stdio.h>
#include<winsock2.h>
#include<stdlib.h>
#include<direct.h>
#include<unistd.h>
#include<windows.h>
#include<string>
#include<dirent.h>
#include<iomanip>
#include<iostream>
#include<fstream>
#include<thread>
#include<ctime>
#pragma commet (lib,"ws2_32.lib")

//compile command g++ socket.cpp -o socket -lws2_32
const int BUFSIZE = 1024;
const int NUM = 32;

using namespace std;

struct File
{
    string path;//provide path for downloading
    string type;//dir or file
    string filename;
    File(string path,string type,string filename)
    {
        this->filename = filename;
        this->type = type;
        this->path = path;
    }   
};
string getTime()
{
	time_t now = time(0);
	tm* ltm = localtime(&now);

	return to_string(1900+ltm->tm_year)+to_string(1+ltm->tm_mon)+to_string(ltm->tm_mday)
	+to_string(ltm->tm_hour)+to_string(ltm->tm_hour)+to_string(ltm->tm_min)
	+to_string(ltm->tm_sec);
}
void cd(string f)
{
    chdir(f.c_str());
}

void ls(string dir, SOCKET& client)
{
    DIR *pdir;
    struct dirent* ptr;
    if(!(pdir = opendir(dir.c_str())))
    {
        printf("Can not open this dirctory\n");
        return;
    }
    //cout<<setw(5)<<"Type"<<setw(15)<<"Name"<<endl;
    string names="";
    while((ptr=readdir(pdir))!=0)
    {
        string type;
        string name;
        if(ptr->d_type==DT_DIR)
        {
            type="dir";
        }
        else if(ptr->d_type==DT_REG)
        {
            type="file";
        }
        name = ptr->d_name;
        names+=name+"\n";
        //cout<<setw(5)<<type<<setw(15)<<name<<endl;
    }
    char sendData[255];
    int len=names.copy(sendData,255);
    sendData[len]=0x00;
    send(client, sendData,len+1, 0);

}

void download(string filename,SOCKET& client)
{
    int haveSent=0;
    const int bufferSize=1024;
    char buffer[bufferSize]={0};
    int readLen=0;
    ifstream file;
    file.open(filename.c_str(),ios::binary);
	int index = filename.find_last_of(".");
	string type = filename.substr(index);
	char sendType[255];
	int len = type.copy(sendType,255);
	sendType[len]=0x00;
	//cout<<type<<endl;
	send(client,sendType,len+1,0);
    if(!file)
    {
        cout<<"Error"<<endl;
        return;
    }

    while(!file.eof())
    {
        file.read(buffer,bufferSize);
        readLen = file.gcount();
        send(client,buffer,readLen,0);
        haveSent+=readLen;
    }
    file.close();
    cout<<"Have sent: "<<haveSent<<endl;
}

void upload(string filename,const SOCKET& client)
{
    cout<<"Staring uploading "<<endl;
    const int bufferSize = 1024;
    char buffer[bufferSize] = {0};
    int readLen=0;
    ofstream file;
    
    char type[255];
    readLen = recv(client,type,255,0);
    filename+=type;
    cout<<filename<<endl;
    file.open(filename.c_str(),ios::binary);
    if(!file)
    {
        cout<<"error"<<endl;
        return;
    }
    do
    {
        readLen = recv(client,buffer,bufferSize,0);
        if(readLen==0)
        {
            break;
        }
        else
        {
            file.write(buffer,readLen);
        }


    } while (1);
    file.close();
    
}


void command(const string& order, SOCKET& client)
{
    
    char* buf;
    strcpy(buf,order.c_str());  

    char *argv[NUM];
    argv[0]=strtok(buf," ");//字符串分割 获取命令与参数
    int i=0;
    while(argv[i]!=NULL)
    {
        i++;
        argv[i]= strtok(NULL," ");
    }

    for(int i =0;i<sizeof(argv);i++)
    {
        string temp;
        if(argv[i]!=NULL) temp = argv[i];
        else break;

        if(temp=="cd")//ok
        {
            string path="";
            path+=argv[i+1];
            i++;
            while(argv[i+1]!=NULL)
            {
                path+=" ";
                path+=argv[i+1];
                i++;
            }
            //cout<<path<<endl;
            cd(path);
            break;
        }
        if(temp=="exit")//ok
        {
            return ;
        }
        if(temp=="create")//ok
        {
            int file = creat(argv[i+1],0777);
            close(file);
        }
        if(temp=="mkdir")//ok
        {
            _mkdir(argv[i+1]);
        }
        if(temp=="rm")//ok
        {
            remove(argv[i+1]);
        }
        if(temp=="rmdir")//ok
        {
            _rmdir(argv[i+1]);
        }
        if(temp=="dload")
        {
            download(argv[i+1],client);
        }
        if(temp=="uload")
        {
            upload(getTime(),client);
        }
        if(temp=="ls")
        {
            ls(getcwd(NULL,256),client);
        }
    }
               
    
}
//creat("a.txt",0777);
//remove("a.txt")
//_mkdir("a");
//_rmdir("a");

//WSAData功能是:存放windows socket初始化信息。结构体如下：
	/*
	 struct WSAData {
		WORD wVersion;
		WORD wHighVersion;
		char szDescription[WSADESCRIPTION_LEN+1];
		char szSystemStatus[WSASYSSTATUS_LEN+1];
		unsigned short iMaxSockets;
		unsigned short iMaxUdpDg;
		char FAR * lpVendorInfo;
		wVersion为你将使用的Winsock版本号，

    wHighVersion为载入的Winsock动态库支持的最高版本，注意，它们的高字节代表次版本，低字节代表主版本。
    szDescription与szSystemStatus由特定版本的Winsock设置，实际上没有太大用处。
    iMaxSockets表示最大数量的并发Sockets，其值依赖于可使用的硬件资源。
    iMaxUdpDg表示数据报的最大长度;然而，获取数据报的最大长度，你需要使用WSAEnumProtocols对协议进行查询。
    最大数量的并发Sockets并不是什么神奇的数字,它是由可用的物理资源来决定的.
    lpVendorInfo是为Winsock实现而保留的制造商信息,这个在Windows平台上并没有什么用处.
        };
*/
/**
 * 从明天起，做一个早起做饭的人
 * 起锅，烧油，电饭煲蒸饭
 * 从明天起，关心自己会不会被自己饿死
 * 我有一个隔离房间，面对核酸检测点，三天三检
 * 从明天起，和每一个亲人通信
 * 告诉他们我的饥饿
 * 我将告诉每一个人
 * 给每一个自己做的菜每一个粒米饭取一个温暖的名字
 * 返乡人，我也为你祝福
 * 愿你家里有人给你做饭
 * 愿你不在被饥饿烦恼
 * 愿你不会被冷到
 * 我只愿早起做饭，抵抗寒冷
*/

/**
1、加载套接字库，创建套接字（WSAStartup()/socket()）;

2、绑定套接字到一个IP地址和一个端口上（bind()）;

3、将套接字设置为监听模式等待连接请求（listen()）；

4、请求到来后，接受连接请求，返回一个新的对应于此次连接的套接字（accept()）；

5、用返回的套接字和客户端进行通信（send()/recv()）；

6、返回，等待另一个连接请求；

7、关闭套接字，关闭加载的套接字库（closesocket()/WSACleanup()）；
*/
/**
（1）SOCK_STREAM表示面向连接的数据传输方式。数据可以准确无误地到达另一台计算机，如果损坏或丢失，可以重新发送，但效率相对较慢。常用的HTTP协议就使用SOCK_STREAM传输数据，因为要确保数据的正确性，否则网页不能正常解析。

（2）SOCK_DGRAM表示无连接的数据传输方式。计算机只管传输数据，不作数据校验，如果数据在传输中损坏，或者没有到达另一台计算机，是没有办法补救的。也就是说，数据错了就错了，无法重传。因为SOCK_DGRAM所做的校验工作少，所以效率比SOCK_STREAM高。

*/

/**
 * 改进的地方
 * 1. 多线程
 * 2. 在ls那里循环发送
*/
/**
 * 牛逼的代码
 * int len = recv(socket,(char*)&student,4024,0)
 * 然后cout
 * memset(&student,0,sizeof(student))
 * send(socket,(char*)&student,sizeof(student),0)
*/
int main()
{
	//初始化WSA
	WORD sockVersion=MAKEWORD(2,2);
	WSADATA wsaData;//WSADATA结构体变量的地址值
 
	//int WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData);
	//成功时会返回0，失败时返回非零的错误代码值
	if(WSAStartup(sockVersion,&wsaData)!=0)
	{
		cout<<"WSAStartup() error!"<<endl;
		return 0;
	}

    //创建套接字
	SOCKET slisten=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(slisten==INVALID_SOCKET)
	{
		cout<<"socket error !"<<endl;
		return 0;
	}
    //绑定IP和端口
	sockaddr_in sin;//ipv4的指定方法是使用struct sockaddr_in类型的变量
	sin.sin_family = AF_INET;
	sin.sin_port = htons(8080);//设置端口。htons将主机的unsigned short int转换为网络字节顺序
	sin.sin_addr.S_un.S_addr = INADDR_ANY;//IP地址设置成INADDR_ANY，让系统自动获取本机的IP地址
	//bind函数把一个地址族中的特定地址赋给scket。
	if(bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		printf("bind error !");
	}
 
	//开始监听
	if(listen(slisten,5)==SOCKET_ERROR)
	{
		cout<<"listen error !"<<endl;
		return -1;
	}
 
	//循环接收数据
	SOCKET sclient;
	sockaddr_in remoteAddr;//sockaddr_in常用于socket定义和赋值,sockaddr用于函数参数
	int nAddrlen=sizeof(remoteAddr);
	char revData[255];
    bool first=1;//记录是否为第一次连接
	while(true)
	{
		//cout<<"等待连接。。。"<<endl;
        //cout<<"Waiting connection..."<<endl;
		sclient=accept(slisten,(sockaddr *)&remoteAddr,&nAddrlen);
		if(sclient==INVALID_SOCKET)
		{
			cout<<"accept error !"<<endl;
			continue;
		}
        
		cout<<"Get a connection: "<<inet_ntoa(remoteAddr.sin_addr)<<endl;
        //cout<<"receving..."<<endl;
        //发送数据

        string tips=getcwd(NULL,256);
        tips+=" $:";
        char sendData[255];
        int len = tips.copy(sendData,255);
        send(sclient,sendData,len,0);
        //cout<<res<<endl;
		//接收数据
		int ret=recv(sclient,revData,255,0);
		if(ret>0)
		{
            cout<<revData<<endl;
            command(revData,sclient);
		}
        cout<<endl;
		closesocket(sclient);
	}
	closesocket(slisten);
	WSACleanup();
	system("pause");
	//return 0;



}