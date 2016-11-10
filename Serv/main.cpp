#include<iostream>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include <algorithm> 
#include <map>
#include<pthread.h>
#include<signal.h>
#include<string>
#include <limits>
#include "client.h"
#include "globaldef.h"
#define SERVPORT 24301
#define BACKLOG 10
#define MAXSIZE 40960
#define CONNMSG "connected"
#define KEEPMSG "keep"
#define NAME "name"
#define CLOSE "close"
#define CMD "cmd"
using namespace std;
void createThreads();
void* server(void*);
int sockfd;
int start;
extern map<int, Client*> clients;
map<int, Client*>::iterator i;
pthread_t servtid;

int main()
{
	start = 50000;
	createThreads();
	while (1)
	{
		string cmd;
		cin >> cmd;
		if (cmd == "ls")
		{
			for (i = clients.begin(); i != clients.end(); ++i)
			{
				cout << i->first << ':' << i->second->ToString() << endl;
			}
		}
		if (cmd == "close")
		{
			for (i = clients.begin(); i != clients.end(); ++i)
			{
				i->second->sendQueue->Put("close");
			}
		}
		if (cmd == "name")
		{
			for (i = clients.begin(); i != clients.end(); ++i)
			{
				i->second->sendQueue->Put("name");
			}
		}
		if (cmd == "cmd")
		{
			cout << "please enter cmd" << endl;
			string command;
			cin.ignore(numeric_limits<short>::max(), '\n');
			getline(cin, command);
			for (i = clients.begin(); i != clients.end(); ++i)
			{
				i->second->sendQueue->Put("cmd");
				i->second->sendQueue->Put(command);
			}
		}
		if (cmd == "cmdto")
		{
			int id;
			cout << "who do you want to send cmd?" << endl;
			cin >> id;
			cout << "please enter cmd" << endl;
			string command;
			cin.ignore(numeric_limits<short>::max(), '\n');
			getline(cin, command);

			map<int, Client*>::iterator iter = clients.find(id);

			if (iter != clients.end())
			{
				iter->second->sendQueue->Put(command);
			}
			else
			{
				cout << "cant't find client by this id";
			}
		}
	}

	return 0;
}

void createThreads()
{
	int err;

	err = pthread_create(&servtid, NULL, server, NULL);
	if (err)
	{
		cout << "create keepAlive failed";
	}
}

void* server(void* ptr)
{
	int client_fd;
	struct sockaddr_in my_addr;
	struct sockaddr_in remote_addr;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		cout << "establish failed";
		exit(1);
	}
	int on = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	my_addr.sin_family = AF_INET ;
	my_addr.sin_port = htons(SERVPORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(my_addr.sin_zero), 8);

	if (bind(sockfd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr)) == -1)
	{
		cout << "bind failed";
		exit(1);
	}
	if (listen(sockfd, BACKLOG) == -1)
	{
		cout << "listen error";
		exit(1);
	}
	cout << "server start" << endl;
	while (1)
	{
		socklen_t sin_size = sizeof(struct sockaddr_in);
		if ((client_fd = accept(sockfd, (struct sockaddr*)&remote_addr, &sin_size)) == -1)
		{
			cout << "accept error" << endl;
			continue;
		}
		cout << "receive a connection from " << (char*)inet_ntoa(remote_addr.sin_addr) << endl;
		Client* client = new Client(client_fd, (char*)inet_ntoa(remote_addr.sin_addr),start);
		clients.insert(pair<int, Client*>(start, client));
		start++;
	}
}

