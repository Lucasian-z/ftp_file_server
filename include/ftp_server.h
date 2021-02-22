#ifndef __FTP_SERVER_H__
#define __FTP_SERVER_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/mman.h>
#include <ctype.h>
#include <mysql.h>
#include <my_global.h>
#include <sys/time.h>

#define _GNU_SOURCE
#define ARGS_CHECK(argc,val) {if(argc!=val) {printf("error args\n");return -1;}}
#define ERROR_CHECK(ret,retVal,funcName) {if(ret==retVal) {perror(funcName);return -1;}}
#define bzeroBuf bzero(buf, sizeof(char) * 100)
#define bzeroInstr bzero(instruction, sizeof(char) * 1000)

typedef struct train {
    int dataLen;
    char buf[1000];
}train_t;

typedef struct Task {
    short taskKind;
    char fileName[20];
    char userName[20];
    int preDirNo;
}Task_t;

typedef struct {
    char clientName[20]; // 
    char curDir[20];
    int curNo;
    int clientFd;
}Client_t;

int tcpInit(int *, char*, char*);
int epollAdd(int, int);
int sendFile(int, char*);
int recvFile(int, char*);
int recvCycle(int, void*, int);
void *threadFunc(void*);

// mysql相关
void mysqlConnect(MYSQL*, char*);
void mysqlCrud(MYSQL*, char*);
int isUserNameExist(MYSQL*, char*);

// salt值
int getRandomStr(char *randomStr, const int randomLen);

#endif
