#ifndef __WORK_FACTORY_H__
#define __WORK_FACTORY_H__
#include "ftp_server.h"
#include "work_que.h"

typedef struct factory {
    Que_t que;
    pthread_t *pthid;
    int threadNum;
    pthread_cond_t cond;
    short startFlag;
    MYSQL mysql;
}Factory_t, *pFactory_t;

void factoryInit(pFactory_t, int, int);
void factoryStart(pFactory_t);
#endif