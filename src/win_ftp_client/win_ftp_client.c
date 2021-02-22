#include "win_ftp_client.h"
#include "win_ftp_client_cmd.h"
#include "md5.h"
#pragma comment(lib, "ws2_32.lib")
#define IP "121.4.108.95"
#define PORT 20000

HANDLE hSemaphore = NULL;

// UTF-8到GB2312的转换
void U2G(char utf8[]) {
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
    wchar_t *wstr = (wchar_t *)calloc(len + 1, sizeof(wchar_t));
    memset(wstr, 0, len + 1);
    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
    len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, utf8, len, NULL, NULL);
    if (wstr) free(wstr);
}

// GB2312到UTF-8的转换
void G2U(char gb2312[]) {
    int len = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
    wchar_t *wstr = (wchar_t *)calloc(len + 1, sizeof(wchar_t));
    memset(wstr, 0, len + 1);
    MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);
    len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, gb2312, len, NULL, NULL);
    if (wstr) free(wstr);
}

DWORD WINAPI threadFunc(LPVOID lpParam) {
    int *port = (int *)lpParam;
    char *buf = (char *)calloc(1024, sizeof(char));
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    SOCKET listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(*port);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    bind(listenfd, (SOCKADDR *)&servaddr, sizeof(servaddr));
    listen(listenfd, 5);

    ReleaseSemaphore(hSemaphore, 1, NULL);

    SOCKET connfd = accept(listenfd, NULL, NULL);

    while (gets(buf)) {
        send(connfd, buf, strlen(buf) + 1, 0);
    }
    return 0;
}

DWORD WINAPI threadFunc1(LPVOID lpParam) {
    Task_t *task = (Task_t*)lpParam;
    // char *buf = (char *)calloc(1024, sizeof(char));
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    SOCKET socketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr(IP);

    ReleaseSemaphore(hSemaphore, 1, NULL);

    int ret = connect(socketFd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if (ret == -1) {
        perror("connect");
    }
    char buf[40] = {0};
    if (task->taskKind == 0) {
        sprintf(buf, "put %s", task->fileName);
    } else {
        sprintf(buf, "get %s", task->fileName);
    }
    int dataLen = strlen(buf);
    send(socketFd, (char*)&dataLen, 4, 0);
    send(socketFd, buf, dataLen, 0);
    dataLen = strlen(task->userName);
    send(socketFd, (char*)&dataLen, 4, 0);
    send(socketFd, task->userName, dataLen, 0);
    send(socketFd, (char*)&(task->preDirNo), 4, 0);
    char bitSuccess;
    FILE *fp;
    if (task->taskKind == 0) {
        fp = fopen(task->fileName, "ab+");
        if (fp == NULL) {
            perror("fopen");
            fclose(fp);
            return 0;
        }
        MD5_CTX md5;
        unsigned char md5Value[16];
        char md5Str[33];
        MD5Init(&md5);

        struct stat fileInfo;
        stat(task->fileName, &fileInfo); //获取该文件的信息
        printf("size = %d\n", fileInfo.st_size);
        unsigned char *fileData =
            (unsigned char *)calloc(fileInfo.st_size, sizeof(unsigned char));
        ret = fread(fileData, sizeof(char), fileInfo.st_size, fp);
        fclose(fp);

        MD5Update(&md5, fileData, ret);
        free(fileData);
        MD5Final(&md5, md5Value);
        for (int i = 0; i < 16; ++i) {
            snprintf(md5Str + i * 2, 2 + 1, "%02x", md5Value[i]);
        }
        md5Str[32] = '\0';
        // printf("md5Str = %s\n", md5Str);
        //发送md5值
        send(socketFd, md5Str, 33, 0);
        char tmpRes[2] = {0};
        recv(socketFd, tmpRes, 1, 0);
        if (strcmp(tmpRes, "1") == 0) {
            printf("文件秒传成功\n\n");
            close(socketFd);
            return 0;
        }
        sendFile(socketFd, task->fileName);
        printf("子线程上传成功\n\n");
        close(socketFd);
    } else { // 下载
        // printf("start download\n");
        recv(socketFd, &bitSuccess, 1, 0);
        if (bitSuccess == 'Y') {
            recvFile(socketFd, task->fileName);
            printf("子线程下载成功\n\n");
        } else {
            printf("当前文件夹内不存在此文件，请再次检查\n\n");
        }
        close(socketFd);
    }
    return 0;
}

int main()
{
    char buf[1024] = {0};
    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA data;

    int port = 8000;
    hSemaphore = CreateSemaphore(NULL, 0, 1, NULL);  //创建信号量
    HANDLE stdThread = CreateThread(NULL, 0, threadFunc, &port, 0, NULL);  //创建线程
    if (stdThread == NULL) {
        printf("create thread failed\n");
        return -1;
    }
    

    DWORD waitRet = WaitForSingleObject(hSemaphore, INFINITE);
    switch (waitRet) {
        case WAIT_OBJECT_0:
            break;
        default:
            printf("wait single failed\n");
            break;
    }

    // wsastartup用于相应的socket库绑定
    if (WSAStartup(sockVersion, &data) != 0) {
        return -1;
    }
    SOCKET socketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socketFd == INVALID_SOCKET) {
        perror("socket");
        return -1;
    }
    struct sockaddr_in serAddr;
    serAddr.sin_family = AF_INET;
    serAddr.sin_addr.S_un.S_addr = inet_addr(IP);
    serAddr.sin_port = htons(PORT);

    int ret = connect(socketFd, (struct sockaddr *)&serAddr, sizeof(serAddr));
    if (ret == SOCKET_ERROR) {
        perror("connect");
        return -1;
    }
    int dataLen = 1;
	send(socketFd, (char*)&dataLen, 4, 0);
	send(socketFd, "n", 1, 0);

    SOCKET stdinFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in serStdin;
    serStdin.sin_family = AF_INET;
    serStdin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    serStdin.sin_port = htons(8000);
    ret = connect(stdinFd, (struct sockaddr *)&serStdin, sizeof(serStdin));
    if (ret == SOCKET_ERROR) {
        perror("connect");
        return -1;
    }
    printInfo();

    fd_set rdset;
    int isLogin = -1, preDirNo = -1, flag = 0;
    char userName[20] = {0};
    Task_t task;
    while (1) {
        FD_ZERO(&rdset);
        FD_SET(stdinFd, &rdset);
        FD_SET(socketFd, &rdset);
        ret = select(0, &rdset, NULL, NULL, NULL);
        if (ret == SOCKET_ERROR) {
            break;
        }
        if (FD_ISSET(socketFd, &rdset)) {
            memset(buf, 0, sizeof(buf));
            ret = recv(socketFd, buf, sizeof(buf), 0);
            U2G(buf);
            if (ret == 0) {
                printf("byebye\n");
                break;
            }
            printf("%s\n", buf);
        }
        if (FD_ISSET(stdinFd, &rdset)) {
            memset(buf, 0, sizeof(buf));
            ret = recv(stdinFd, buf, sizeof(buf), 0);
            if (ret == 0) {
                printf("byebye\n");
                break;
            }
            G2U(buf);
            // printf("buf = %s0\n", buf);
            if (strncmp(buf, "login", 5) == 0 && isLogin == -1) {
                isLogin = userLogIn(socketFd, buf, userName, &preDirNo);
            } else if (strncmp(buf, "signin", 6) == 0) {
                userSignIn(socketFd, buf);
            }
            if (isLogin == -1) {
                continue;
            }
            if (strncmp(buf, "pwd", 3) == 0) {
                // printf("buf = %s\n", buf);
                pwdHandle(socketFd, buf);
            } else if (strncmp(buf, "ls", 2) == 0) {
                lsHandle(socketFd, buf);
            } else if (strncmp(buf, "mkdir", 5) == 0) {
                mkdirHandle(socketFd, buf);
            } else if (strncmp(buf, "cd", 2) == 0) {
                cdHandle(socketFd, buf, &preDirNo);
            } else if (strncmp(buf, "rm", 2) == 0) {
                rmHandle(socketFd, buf, preDirNo);
            } else if (strncmp(buf, "get", 3) == 0 || strncmp(buf, "put", 3) == 0) {
                memset(&task, 0, sizeof(task));
                flag = 1;
                if (strncmp(buf, "get", 3) == 0) {
                    task.taskKind = 1;
                    sscanf(buf, "get %s\n", task.fileName);
                    // printf("buf = %s, len = %d\n", task.fileName, strlen(task.fileName));
                } else {
                    task.taskKind = 0;
                    sscanf(buf, "put %s\n", task.fileName);
                    if (access(task.fileName, 0) == -1) {
                        printf("当前文件夹内不存在该文件\n\n");
                        flag = 0;
                    }
                }
                if (flag) {
                    strcpy(task.userName, userName);
                    task.preDirNo = preDirNo;
                    // printf("preDirNo = %d\n", preDirNo);
                    //创建上传/下载线程
                    HANDLE thread1 = CreateThread(NULL, 0, threadFunc1, &task, 0, NULL);
                    if (thread1 == NULL) {
                        printf("create thread failed\n");
                        return -1;
                    }
                    WaitForSingleObject(thread1, INFINITE);

                }
            }
        }
    }
    closesocket(socketFd);
    CloseHandle(stdThread);
    WSACleanup();
    return 0;
}