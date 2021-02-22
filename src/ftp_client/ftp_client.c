#define _GNU_SOURCE
#include "../../include/ftp_server.h"
#include "../../include/ftp_client.h"



int main(int argc, char *argv[])
{
	// 读取配置文件
	char *ip = (char*)calloc(20, sizeof(char));
	int port;
	if (getConfig(ip, &port) == -1) {
		printf("读取配置文件错误\n");
		return -1;
	}
	printInfo();

	// 初始化socketFd
	int socketFd = socket(AF_INET, SOCK_STREAM, 0);
	ERROR_CHECK(socketFd, -1, "socket");
	struct sockaddr_in serAddr;
	bzero(&serAddr, sizeof(serAddr));
	serAddr.sin_family = AF_INET;
	serAddr.sin_addr.s_addr = inet_addr(ip);
	serAddr.sin_port = htons(port);
	int ret = connect(socketFd, (struct sockaddr*)&serAddr, sizeof(serAddr));
	ERROR_CHECK(ret, -1, "connect");

	// 主线程发送"n"与子线程区分
	int dataLen = 1;
	send(socketFd, &dataLen, 4, 0);
	send(socketFd, "n", 1, 0);

	// epoll同时监听标准输入和sockfd接收文件
	int epfd = epoll_create(1);
	ERROR_CHECK(epfd, -1, "epoll_create");
	struct epoll_event event, evs[10];
	bzero(&event, sizeof(event));
	event.events = EPOLLIN;
	event.data.fd = STDIN_FILENO;
	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &event);
	ERROR_CHECK(ret, -1, "epoll_ctl");
	event.data.fd = socketFd;
	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, socketFd, &event);
	ERROR_CHECK(ret, -1, "epoll_ctl");
	

	int readyCnt, i;
	char buf[100] = {0};
	Task_t task;
	int isLogin = -1;
	char userName[20] = {0}; // 存储用户名，便于传输文件时进行数据库查询
	int flag = 0;
	int preDirNo = 0;
	while (1) {
		readyCnt = epoll_wait(epfd, evs, 10, -1);
		for (i = 0; i < readyCnt; ++i) {
			if (evs[i].events == EPOLLIN && evs[i].data.fd == STDIN_FILENO) {
				bzero(buf, sizeof(buf));
				ret = read(STDIN_FILENO, buf, sizeof(buf) - 1);
				// 注册or登录
				if (strncmp(buf, "login", 5) == 0 && isLogin == -1) {
					isLogin = userLogIn(socketFd, buf, userName, &preDirNo);
				}else if (strncmp(buf, "signin", 6) == 0) {
					userSignIn(socketFd, buf);
				}
				if (isLogin == -1) {
					continue;
				}
				if (strcmp(buf, "pwd\n") == 0) {
					pwdHandle(socketFd, buf);
				}else if (strcmp(buf, "ls\n") == 0) {
					lsHandle(socketFd, buf);
				}else if (strncmp(buf, "mkdir", 5) == 0) {
					mkdirHandle(socketFd, buf);
				}else if (strncmp(buf, "cd", 2) == 0) {
					cdHandle(socketFd, buf, &preDirNo);
				}else if (strncmp(buf, "rm", 2) == 0) {
					rmHandle(socketFd, buf, preDirNo);
				}
				

				// 传输文件
				if (!isLogin && (strncmp(buf, "put", 3) == 0 || strncmp(buf, "get", 3) == 0)) {
					bzero(&task, sizeof(task));
					flag = 1;
					if (strncmp(buf, "get", 3) == 0) {
						task.taskKind = 1;
						sscanf(buf, "get %s\n", task.fileName);
					}else {
						task.taskKind = 0;
						sscanf(buf, "put %s\n", task.fileName);
						if (access(task.fileName, 0) == -1) {
							printf("当前文件夹内不存在该文件\n");
							flag = 0;
						}
					}
					if (flag) {
						strcpy(task.userName, userName);
						task.preDirNo = preDirNo;
						pthread_t pthid1;
						pthread_create(&pthid1, NULL, threadFunc, &task);
						pthread_join(pthid1, NULL);
					}
				}
			}
		}
	}
	close(socketFd);
	return 0;
}