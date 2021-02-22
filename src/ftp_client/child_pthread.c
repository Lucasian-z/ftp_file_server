#include "../../include/ftp_server.h"
#include "../../include/ftp_client.h"
#include "../../include/md5.h"

void *threadFunc(void *p) {
    char *ip = (char*)calloc(20, sizeof(char));
	int port;
	if (getConfig(ip, &port) == -1) {
		printf("读取配置文件错误\n");
		return NULL;
	}
    Task_t *task = (Task_t*)p;
    int sockFd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serAddr;
    bzero(&serAddr, sizeof(serAddr));
    serAddr.sin_addr.s_addr = inet_addr(ip);
    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons(port);
    
    int ret = connect(sockFd, (struct sockaddr*)&serAddr, sizeof(serAddr));
    if (ret == -1) {
        perror("connect");
    }
    
    char buf[40] = {0};
    if (task->taskKind == 0) {
        sprintf(buf, "put %s", task->fileName);
    }else {
        sprintf(buf, "get %s", task->fileName);

    }
    int dataLen = strlen(buf);
    send(sockFd, &dataLen, 4, 0);
    send(sockFd, buf, strlen(buf), 0);
    dataLen = strlen(task->userName); // 先发送用户名
    send(sockFd, &dataLen, 4, 0);
    // 发送文件路径
    send(sockFd, task->userName, dataLen, 0);
    send(sockFd, &(task->preDirNo), 4, 0);
    char bitSuccess;
    int fd = 0;
    if (task->taskKind == 0) { // 上传
        fd = open(task->fileName, O_CREAT | O_RDWR, 0666);
        if (fd == -1) {
            perror("open");
            close(fd);
            return NULL;
        }
        //首先获取文件的md5值
        MD5_CTX md5;
        unsigned char md5Value[16];
        char md5Str[33];
        MD5Init(&md5);

        struct stat fileInfo;
        fstat(fd, &fileInfo);  //获取该文件的信息
        unsigned char* fileData =
            (unsigned char*)calloc(fileInfo.st_size, sizeof(unsigned char));
        ret = read(fd, fileData, fileInfo.st_size);
        close(fd);

        MD5Update(&md5, fileData, ret);
        free(fileData);
        MD5Final(&md5, md5Value);
        for (int i = 0; i < 16; ++i) {
            snprintf(md5Str + i * 2, 2 + 1, "%02x", md5Value[i]);
        }
        md5Str[32] = '\0';
        printf("md5Str = %s\n", md5Str);
        //发送md5值
        send(sockFd, md5Str, 33, 0);
        char tmpRes[2] = {0};
        recv(sockFd, tmpRes, 1, 0);
        if (strcmp(tmpRes, "1") == 0) {
            printf("文件秒传成功\n");
            close(sockFd);
            return NULL;
        }
        sendFile(sockFd, task->fileName);
        printf("子线程上传成功\n");
        close(sockFd);
    }else { // 下载
        recv(sockFd, &bitSuccess, 1, 0);
        if (bitSuccess == 'Y') {
            recvFile(sockFd, task->fileName);
            printf("子线程下载成功\n");
        }else {
            printf("当前文件夹内不存在此文件，请仔细检查\n");
        }
        
        close(sockFd);
    }
    return NULL;
}