#pragma once
#include <pthread.h>
#include <iostream>
#include <string>
#include "BlockingQueue.h"
#include "globaldef.h"
using namespace std;
class Client
{
void* array[5];
int sockfd;
pthread_t sendTid;
pthread_t recvTid;
	string ip;
	int id;
public:
	Client(int sockfd,string ip,int mapid);
	~Client();
	int GetSockFd();
	string ToString();
	BlockingQueue<string>* sendQueue;
};

