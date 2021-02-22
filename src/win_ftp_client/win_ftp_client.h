#ifndef __WIN_FTP_CLIENT_H__
#define __WIN_FTP_CLIENT_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#include <winsock2.h>
#include <windows.h>
#include <winsock.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define ARGS_CHECK(argc,val) {if(argc!=val)  {printf("error args\n");return -1;}}
#define ERROR_CHECK(ret,retVal,funcName) {if(ret==retVal) {perror(funcName);return -1;}}
typedef struct Task {
    short taskKind;
    char fileName[20];
    char userName[20];
    int preDirNo;
}Task_t;

typedef struct {
    int dataLen;
    char buf[1000];
}train_t;

int sendFile(int, char*);
int recvFile(int, char*);
int recvCycle(int, void*, int);

#endif