#include "../../include/ftp_server.h"
#include "../../include/work_que.h"
#include "../../include/work_factory.h"
#include "../../include/md5.h"
#include "../../include/ftp_server_cmd.h"

int main(int argc, char *argv[])
{
    char ip[20] = {0};
    char port[10] = {0};
    int threadNum, capacity;
    // 读取配置文件
    FILE* fp = fopen("../../conf/ftp_server.conf", "r");
    if (fp == NULL) {
        perror("fopen");
        return -1;
    }
    fscanf(fp, "ip=%s\nport=%s\nthreadNum=%d\ncapacity=%d", ip, port, &threadNum, &capacity);
    fclose(fp);

    // 初始化socketFd
    int socketFd;
    tcpInit(&socketFd, ip, port);

    // epoll模型
    int epfd = epoll_create(1);
    ERROR_CHECK(epfd, -1, "epoll_create");
    epollAdd(epfd, socketFd);

    // 初始化线程池并启动
    Factory_t threadPoolFactory;
    factoryInit(&threadPoolFactory, capacity, threadNum);
    factoryStart(&threadPoolFactory);
    pQue_t pQue = &threadPoolFactory.que;

    struct epoll_event evs[10];
    int readyCnt, ret, newFd;
    char buf[100] = {0};
    pNode_t pNewTask;
    struct epoll_event event;
    bzero(&event, sizeof(event));
    event.events = EPOLLIN;

    // 连接数据库
    MYSQL mysql;
    MYSQL_RES *res = NULL;
    // MYSQL_ROW row;
    mysql_init(&mysql);
    mysqlConnect(&mysql, "ftp_server");
    mysql_set_character_set(&mysql, "utf8");

    char userName[20] = {0}, passwd[30] = {0};
    char instruction[1000] = {0}; // mysql语句
    MD5_CTX md5Info;
    MD5Init(&md5Info);
    int i, c;
    // 维护当前所处目录
    Client_t clients[100];
    int clientIdx = 0;
    int dataLen = 0;
    while (1) {
        readyCnt = epoll_wait(epfd, evs, 10, -1);
        for (i = 0; i < readyCnt; ++i) {
            if (evs[i].events == EPOLLIN && evs[i].data.fd == socketFd) {
                // 新连接
                newFd = accept(socketFd, NULL, NULL);
                ERROR_CHECK(newFd, -1, "accept");
                recv(newFd, &dataLen, 4, 0);
                bzero(buf, sizeof(buf));
                ret = recv(newFd, buf, dataLen, 0);
                ERROR_CHECK(ret, -1, "recv");
                if (strcmp(buf, "n") == 0) {
                    epollAdd(epfd, newFd);
                    clients[clientIdx++].clientFd = newFd;
                }
                
                // 上传、下载文件
                if (strncmp(buf, "put", 3) == 0 || strncmp(buf, "get", 3) == 0) {
                    // 将任务放入任务队列并通知子线程
                    pNewTask = (pNode_t)calloc(1, sizeof(Node_t));
                    pNewTask->sockFd = newFd;
                    pNewTask->flag = 1;
                    if (strncmp(buf, "get", 3) == 0) {
                        pNewTask->flag = 0;
                    }
                    strcpy(pNewTask->fileName, buf + 4);
                    pthread_mutex_lock(&pQue->mutex);
                    ret = queInsert(pQue, pNewTask);
                    pthread_mutex_unlock(&pQue->mutex);
                    if (ret != -1) {
                        pthread_cond_signal(&threadPoolFactory.cond);
                    }
                }
            }
            for (c = 0; c < clientIdx; ++c) {
                if (evs[i].data.fd == clients[c].clientFd) {
                    recv(evs[i].data.fd, &dataLen, 4, 0);
                    bzero(buf, sizeof(buf));
                    ret = recv(evs[i].data.fd, buf, dataLen, 0);
                    if (!ret) {
                        close(clients[c].clientFd);
                        clients[c].clientFd = -1;
                        clients[c].curNo = -1;
                        bzero(clients[c].curDir, sizeof(clients[c].curDir));
                    }
                        
                    if (strncmp(buf, "signin", 6) == 0) {
                        signInCmd(evs[i].data.fd, buf + 7, instruction, &mysql, c);
                    }else if (strncmp(buf, "login", 5) == 0) {
                        logInCmd(evs[i].data.fd, buf + 6, userName, passwd, instruction, &mysql, res, (Client_t*)&clients, c);
                    }else if (strcmp(buf, "pwd") == 0) {
                        pwdCmd(evs[i].data.fd, clients[c].curDir);
                    }else if (strncmp(buf, "mkdir", 5) == 0) {
                        mkdirCmd(evs[i].data.fd, buf + 6, instruction, clients[c].curNo, clients[c].clientName, &mysql, res);
                    }else if (strcmp(buf, "ls") == 0) {
                        lsCmd(evs[i].data.fd, buf, instruction, &mysql, res, clients[c].curNo, clients[c].clientName);
                    }else if (strncmp(buf, "cd", 2) == 0) { // 进入新的文件夹
                        cdCmd(evs[i].data.fd, buf, instruction, &mysql, res, clients, c);
                    }else if (strncmp(buf, "rm", 2) == 0) {
                        rmCmd(evs[i].data.fd, buf, instruction, &mysql, res, clients[c].clientName);
                    }
                }
            }
        }
    } 
    return 0;
}
