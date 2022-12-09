#include<WINSOCK2.H>
#include<STDIO.H>
#include<iostream>
#include<cstring>
#include<fstream>
#include<ctime>
using namespace std;

#pragma comment(lib, "ws2_32.lib")

string getTime()
{
	time_t now = time(0);
	tm* ltm = localtime(&now);

	return to_string(1900+ltm->tm_year)+to_string(1+ltm->tm_mon)+to_string(ltm->tm_mday)
	+to_string(ltm->tm_hour)+to_string(ltm->tm_hour)+to_string(ltm->tm_min)
	+to_string(ltm->tm_sec);
}
void upload(string filename,const SOCKET& client)
{
	cout<<"Staring uploading"<<endl;
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

void download(string filename,const SOCKET& client)
{
    cout<<"Staring downloading "<<endl;
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
int main()
{

	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA data;
	if(WSAStartup(sockVersion, &data)!=0)
	{
		return 0;
	}

	while(true)
	{

		SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if(sclient == INVALID_SOCKET)
		{
			printf("invalid socket!");
			return 0;
		}

		sockaddr_in serAddr;
		serAddr.sin_family = AF_INET;
		serAddr.sin_port = htons(8080);
		serAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
		if(connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
		{  
			printf("connect error !");
			closesocket(sclient);
			return 0;
		}

		char recData[255];
		int ret = recv(sclient, recData, 255, 0);
		if(ret>0)
		{
			recData[ret] = 0x00;
			printf(recData);
		} 

		string data;
		getline(cin,data);

		char sendData[255];
		int len=data.copy(sendData,255);
		sendData[len]=0x00;
		send(sclient, sendData,len+1, 0);
		//send(sclient,"hello",255,0);
		//cout<<"hello world"<<endl;
		char buf[255];
		for(int i=0;i<data.length();i++)
		{
			buf[i]=data[i];
		}
		buf[data.length()]=0x00;
		
		char *argv[32];
		argv[0]=strtok(buf," ");//字符串分割 获取命令与参数
		int i=0;
		while(argv[i]!=NULL)
		{
			i++;
			argv[i]= strtok(NULL," ");
		}
		

		string cmd = argv[0];
		if(cmd=="dload")
		{
			string filename=getTime();
			download(filename,sclient);
		}
		else if(cmd=="uload")
		{
			cout<<"hello uload"<<endl;
			string filename = argv[1];
			//cout<<filename<<endl;
			upload(filename,sclient);
		}
		else
		{
			char recData[255];
			int ret = recv(sclient, recData, 255, 0);
			if(ret>0)
			{
				recData[ret] = 0x00;
				printf(recData);
			} 
		}
		closesocket(sclient);

	}

		WSACleanup();

		return 0;

}