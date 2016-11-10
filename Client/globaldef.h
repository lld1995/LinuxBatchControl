#pragma once

#include <sstream>
#include <sys/socket.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
using namespace std;
static int GetLength(int sockfd){
	char lenstr[9];
	int len = recv(sockfd, lenstr, 8, 0);
	lenstr[8] = '\0';
	if(len==0)
	{
		return -1;
	}

	//cout<<lenstr<<endl;
	return atoi(lenstr);
};
static void sendLength(int sock, string str) {
		char temp[8];
		sprintf(temp, "%d", str.length());
		int len = strlen(temp);
		int add0=8-len;
		stringstream ss;
		for(int i=0;i<add0;i++){
			ss<<'0';
		}
		ss<<temp<<'\0';
		send(sock,ss.str().c_str(),8,0);
};
static void sendLenAndData(int sock,string str){
	sendLength(sock,str);
	send(sock,str.c_str(),str.length(),0);
};
static int recvLenAndData(int sock, char * & data) {
	int len=GetLength(sock);
	
	if(len==-1||len==0)
	{
		data = NULL;
		return -1;
	}
	
	char* buf = new char[len+1];
	int len1=recv(sock,buf,len,0);
	if (len1 == -1)
	{
		data = NULL;
		delete buf;
		return 0;
	}
	buf[len] = '\0';
	data = buf;
	return 1;
};
