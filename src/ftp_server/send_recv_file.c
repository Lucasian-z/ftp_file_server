#include "../../include/ftp_server.h"

int sendFile(int socketFd, char *fileName) {
    train_t data;
    data.dataLen = strlen(fileName);
    strcpy(data.buf, fileName);
    send(socketFd, &data, 4 + data.dataLen, 0);
    char path[100] = {0};
    sprintf(path, "../../file/%s", fileName);
    struct stat buf;
    int fd = open(path, O_RDWR);
    fstat(fd, &buf);//获取该文件的信息
    data.dataLen = sizeof(buf.st_size);
    memcpy(data.buf, &buf.st_size, data.dataLen);
    send(socketFd, &data, 4 + data.dataLen, 0);

    char *pMap = (char*)mmap(NULL, buf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    ERROR_CHECK(pMap, (char*)-1, "mmap")
    int ret = send(socketFd, pMap, buf.st_size, 0);
    ERROR_CHECK(ret, -1, "send");
    // send(socketFd, &data, 4, 0);
    close(fd);
    return 0;
}

int recvFile(int sockFd, char *fileName) {
    char buf[100] = {0};
    int dataLen = 0;
    recvCycle(sockFd, &dataLen, 4);
    printf("dataLen = %d\n", dataLen);
    recvCycle(sockFd, buf, dataLen);
    bzero(buf, sizeof(buf));
    sprintf(buf, "../../file/%s", fileName);
    int fd = open(buf, O_CREAT | O_RDWR, 0666);
    ERROR_CHECK(fd, -1, "open");

    off_t fileSize = 0;
    recvCycle(sockFd, &dataLen, 4);
    recvCycle(sockFd, &fileSize, dataLen);
    printf("fileSize = %ld\n", fileSize);
    int ret = ftruncate(fd, fileSize);
    ERROR_CHECK(ret, -1, "ftruncate");

    char *pMap = (char*)mmap(NULL, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    ERROR_CHECK(pMap, (char*)-1, "mmap");
    recvCycle(sockFd, pMap, fileSize);
    munmap(pMap, fileSize);
    close(fd);
    return fileSize;    
}
