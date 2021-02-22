#include "../../include/ftp_client.h"
#include "../../include/ftp_server.h"

// 客户端打印提示信息
void printInfo() {
	printf("*********************欢迎使用ftp文件服务器*************************\n");
	printf("1. 登录  \tlog in\n2. 注册  \tsign in\n3. 下载文件 \tget fileName\n");
	printf("4. 上传文件  \tput fileName\n5. 查看当前路径 pwd\n6. 列出所有文件 ls\n");
	printf("7. 切换目录  \tcd dirPath\n8. 删除文件  \trm fileName\n");
}

int getConfig(char *ip, int *port) {
	FILE *fp = fopen("../../conf/ftp_server.conf", "r");
	if (fp == NULL) {
		perror("fopen");
		return -1;
	}
	fscanf(fp, "ip=%s\nport=%d\n", ip, port);
	fclose(fp);
	return 0;
}

int userLogIn(int socketFd, char *buf, char *userName, int *preDirNo) {
    int i;
    bzero(userName, sizeof(char) * 20);
    int dataLen = 0;
    dataLen = strlen(buf) - 1;
    send(socketFd, &dataLen, 4, 0);
    send(socketFd, buf, dataLen, 0);
	char logFlag[2] = {0};
    recv(socketFd, logFlag, 1, 0);
    if (strcmp(logFlag, "1") == 0) {
        printf("登录成功\n\n");
		for (i = 0; i < strlen(buf); ++i) {
			if (buf[i] == '@') {
				strncpy(userName, buf + 6, i - 6);
				break;
			}
		}
		bzeroBuf;
        // 接收目录编号
        recv(socketFd, &dataLen, 4, 0);
        recv(socketFd, buf, dataLen, 0);
        *preDirNo = atoi(buf);
        return 0;
    } else if (strcmp(logFlag, "0") == 0) {
        printf("登录失败，请再次尝试\n\n");
    } else {
        printf("用户名不存在\n\n");
    }
    return -1;
}

int userSignIn(int socketFd, char *buf) {
    int signSuccess = 0;
	int dataLen = strlen(buf) - 1;
	send(socketFd, &dataLen, 4, 0);
    send(socketFd, buf, dataLen, 0);
	bzeroBuf;
    recv(socketFd, buf, 1, 0);
    if (strcmp(buf, "1") == 0) {
        printf("注册成功\n\n");
        signSuccess = 1;
    } else if (strcmp(buf, "0") == 0) {
        printf("用户名已存在，请重新注册\n\n");
    } else {
		printf("格式错误，请检查(signin usrName@passwd)\n");
	}
    return signSuccess;
}

void pwdHandle(int socketFd, char *buf) {
	int dataLen = strlen(buf) - 1;
	send(socketFd, &dataLen, 4, 0);
	send(socketFd, buf, dataLen, 0);
	bzeroBuf;
	recv(socketFd, &dataLen, 4, 0);
	recv(socketFd, buf, dataLen, 0);
	printf("当前路径为: %s\n\n", buf);
}

void lsHandle(int socketFd, char *buf) {
	int dataLen = strlen(buf) - 1;
	send(socketFd, &dataLen, 4, 0);
	send(socketFd, buf, dataLen, 0);
	bzeroBuf;
	recv(socketFd, buf, 1, 0);
	char owner[20] = {0}, fileName[30] = {0};
	int fileSize = 0;
	if (strcmp(buf, "Y") == 0) {
		while (strcmp(buf, "1") != 0) {
			recv(socketFd, &dataLen, 4, 0);
			bzeroBuf;
			recv(socketFd, buf, dataLen, 0);
			if (strlen(buf) > 3) {
				if (buf[0] == '1') {
					printf("-  ");
				}else {
					printf("d  ");
				}
				bzero(owner, sizeof(owner));
				bzero(fileName, sizeof(fileName));
				sscanf(buf + 2, "%s %d %s ", owner, &fileSize, fileName);
				printf("%s %12d\t%s\n", owner, fileSize, fileName);
			}
		}
		printf("\n");
	}else if (strcmp(buf, "N") == 0) {
		printf("当前目录无文件\n\n");
	}
}

void mkdirHandle(int socketFd, char *buf) {
	int dataLen = strlen(buf) - 1;
	send(socketFd, &dataLen, 4, 0);
	send(socketFd, buf, dataLen, 0);
	printf("buf = %s\n", buf);
	bzeroBuf;
	recv(socketFd, buf, 1, 0);
	// printf("mkdir buf = %s0\n", buf);
	if (strcmp(buf, "1") == 0) {
		printf("创建文件夹成功\n\n");

	}else if (strcmp(buf, "0") == 0) {
		printf("创建文件夹失败\n\n");
	}
}

void cdHandle(int socketFd, char *buf, int *preDirNo) {
	int dataLen = strlen(buf) - 1;
	send(socketFd, &dataLen, 4, 0);
	send(socketFd, buf, dataLen, 0);
	bzeroBuf;
	recv(socketFd, buf, 1, 0);
	
	if (strcmp(buf, "0") == 0) {
		printf("不存在该文件夹\n\n");
	}else if (strcmp(buf, "1") == 0) {
		bzeroBuf;
		recv(socketFd, &dataLen, 4, 0);
		recv(socketFd, buf, dataLen, 0);
		printf("%s\n\n", buf);
		recv(socketFd, preDirNo, 4, 0);
	}
}

void rmHandle(int socketFd, char *buf, const int preDirNo) {
	int dataLen = strlen(buf) - 1;
    send(socketFd, &dataLen, 4, 0);
    send(socketFd, buf, dataLen, 0);
	send(socketFd, &preDirNo, 4, 0);
	char tmp[2] = {0};
	recv(socketFd, tmp, 1, 0);
	if (strcmp(tmp, "1") == 0) {
		printf("文件成功删除\n\n");
	}else {
		printf("文件不存在\n\n");
	}
}