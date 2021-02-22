#include "../../include/work_factory.h"
#include "../../include/md5.h"
void *threadFunc(void *p) {
    pFactory_t factory = (pFactory_t)p;
    pQue_t pQue = &factory->que;
    pNode_t pNew;
    int dataLen = 0, preDirNo = 0;
    char buf[1000] = {0};
    char usrName[20] = {0};
    char fileMd5Value[33] = {0};
    int ret, flag = 0, fileSize = 0;
    MYSQL_RES *res;
    MYSQL_ROW row;
    while (1) {
        pthread_mutex_lock(&pQue->mutex);
        if (!pQue->queSize) {
            pthread_cond_wait(&factory->cond, &pQue->mutex);
        }
        ret = queGet(pQue, &pNew);
        pthread_mutex_unlock(&pQue->mutex);
        if (ret == -1) {
            printf("queGet error\n");
        }
        bzero(usrName, sizeof(usrName));
        recvCycle(pNew->sockFd, &dataLen, 4); // 先接收用户名
        recvCycle(pNew->sockFd, usrName, dataLen);
        // 再接收preDirNo
        recvCycle(pNew->sockFd, &preDirNo, 4);
        // 子线程处理任务
        if (pNew->flag == 0) {
            bzero(buf, sizeof(buf));
            sprintf(buf, "select * from files_info where fileName = \'%s\' and preNo = %d and owner = \'%s\'", pNew->fileName, preDirNo, usrName);
            mysqlCrud(&factory->mysql, buf);
            res = mysql_store_result(&factory->mysql);
            if (mysql_num_rows(res) > 0) {
                send(pNew->sockFd, "Y", 1, 0); // 该用户存在此文件
                printf("start download\n");
                sendFile(pNew->sockFd, pNew->fileName);
                printf("send finish\n");
            }else {
                send(pNew->sockFd, "N", 1, 0); // 该用户不存在此文件
            }
            
            close(pNew->sockFd);
        }else if (pNew->flag == 1) {
            recvCycle(pNew->sockFd, fileMd5Value, 33);
            bzero(buf, sizeof(buf));
            sprintf(buf, "select * from files_info where MD5 = \'%s\'", fileMd5Value);
            flag = 0;
            mysqlCrud(&factory->mysql, buf);
            res = mysql_store_result(&factory->mysql);
            while ((row = mysql_fetch_row(res))) {
                fileSize = atoi(row[5]);
                if (strcmp(row[3], usrName) == 0) {
                    send(pNew->sockFd, "1", 1, 0);
                    mysql_free_result(res);
                    if (atoi(row[0]) != preDirNo) {
                        goto ins;
                    }
                    goto end;
                }
                flag = 1;
            }
            
            if (flag) {
            ins:
                bzero(buf, sizeof(buf));
                sprintf(buf, "insert into files_info(preNo, fileName, owner, MD5, fileSize, fileType) values(%d, \'%s\', \'%s\', \'%s\', %d, 1)", preDirNo, pNew->fileName, usrName, fileMd5Value, fileSize);
                mysqlCrud(&factory->mysql, buf);printf("flag = %d\n", flag);
                send(pNew->sockFd, "1", 1, 0);
                goto end;
            }
            
            
            send(pNew->sockFd, "0", 1, 0);
            printf("start upload\n");
            fileSize = recvFile(pNew->sockFd, pNew->fileName);
            bzero(buf, sizeof(buf));
            sprintf(buf, "insert into files_info(preNo, fileName, owner, MD5, fileSize, fileType) values(%d, \'%s\', \'%s\', \'%s\', %d, 1)", preDirNo, pNew->fileName, usrName, fileMd5Value, fileSize);
            mysqlCrud(&factory->mysql, buf);
        end:
            printf("recv finish\n");
            close(pNew->sockFd);
        }
        free(pNew);
        pNew = NULL;
    }
}

void factoryInit(pFactory_t p, int capacity, int threadNum) {
    pQue_t pQue = &p->que;
    queInit(pQue, capacity);
    p->pthid = (pthread_t*)calloc(threadNum, sizeof(pthread_t));
    p->threadNum = threadNum;
    p->startFlag = 0;
    pthread_cond_init(&p->cond, NULL);
    mysql_init(&p->mysql);
    mysqlConnect(&p->mysql, "ftp_server");
    mysql_set_character_set(&p->mysql, "utf8");
}

void factoryStart(pFactory_t p) {
    if (p->startFlag == 0) {
        for (int i = 0; i < p->threadNum; ++i) {
            pthread_create(p->pthid + i, NULL, threadFunc, p);
        }
        p->startFlag = 1;
    }
}
