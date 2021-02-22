#ifndef __WORK_QUE_H__
#define __WORK_QUE_H__
#include "ftp_server.h"

// 定义单个任务
typedef struct node {
    int sockFd;
    struct node *next;
    short flag;  // 下载任务为0, 上传任务为1
    char fileName[10];  // 文件名
    int epfd;
}Node_t, *pNode_t;

// 定义任务队列
typedef struct que {
    pNode_t queHead, queTail;
    int capacity;
    int queSize;
    pthread_mutex_t mutex;
}Que_t, *pQue_t;

void queInit(pQue_t, int);
int queInsert(pQue_t, pNode_t);
int queGet(pQue_t, pNode_t*);

#endif