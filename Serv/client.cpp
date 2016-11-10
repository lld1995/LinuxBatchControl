#include "client.h"
#include <signal.h>
#include <unistd.h>

#include <bits/sigthread.h>
#include <map>

extern map<int, Client*> clients;
void* Send(void*);
void* Recv(void*);

Client::Client(int fd, string ipaddr, int mapid)
{
	sendQueue = new BlockingQueue<string>();
	sockfd = fd;
	id = mapid;
	ip = ipaddr;
	array[0] = this;
	pthread_create(&sendTid, NULL, Send, array);
	pthread_create(&recvTid, NULL, Recv, array);
}

void* Send(void* ptr)
{
	void** arr = (void**)ptr;
	Client* client = (Client*)arr[0];
	signal(SIGCHLD,
		[](int i)
		{
			cout << "send thread exit" << endl;
			pthread_exit(PTHREAD_CANCELED);
		});
	while (true)
	{
		string msg = client->sendQueue->Take();
		sendLenAndData(client->GetSockFd(), msg);
	}
}

void* Recv(void* ptr)
{
	void** arr = (void**)ptr;
	Client* client = (Client*)arr[0];
	while (true)
	{
		char* buf=NULL;
		if (recvLenAndData(client->GetSockFd(), buf) == -1)
		{
			delete client;
			break;
		}
		cout << buf << endl;
		if(buf!=NULL)
		delete buf;
	}
}

Client::~Client()
{
	
	pthread_kill(sendTid, SIGCHLD);
	map<int, Client*>::iterator iter = clients.find(id);
	if (iter != clients.end())
	{
		clients.erase(iter);
	}
	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);
	delete sendQueue;
}

int Client::GetSockFd()
{
	return sockfd;
}

string Client::ToString()
{
	return ip;
}

