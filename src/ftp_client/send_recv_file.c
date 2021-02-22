#include "../../include/ftp_server.h"
#include "../../include/md5.h"
int sendFile(int socketFd, char* fileName) {
    int fd = open(fileName, O_RDWR);
    ERROR_CHECK(fd, -1, "open");
    struct stat buf;
    fstat(fd, &buf);  //获取该文件的信息
    train_t data;
    bzero(&data, sizeof(data));    
    data.dataLen = strlen(fileName);
    strcpy(data.buf, fileName);
    send(socketFd, &data, 4 + data.dataLen, 0);
    
    // printf("fileSize = %ld\n", buf.st_size);
    data.dataLen = sizeof(buf.st_size);
    memcpy(data.buf, &buf.st_size, data.dataLen);
    send(socketFd, &data, 4 + data.dataLen, 0);

    char* pMap = (char*)mmap(NULL, buf.st_size, PROT_READ | PROT_WRITE,
                             MAP_SHARED, fd, 0);
    ERROR_CHECK(pMap, (char*)-1, "mmap")
    int ret = send(socketFd, pMap, buf.st_size, 0);
    ERROR_CHECK(ret, -1, "send");
    munmap(pMap, buf.st_size);
    // send(socketFd, &data, 4, 0);
    close(fd);
    return 0;
}

int recvFile(int sockFd, char* fileName) {
    int dataLen = 0;
    char buf[1000] = {0};
    recvCycle(sockFd, &dataLen, 4);
    recvCycle(sockFd, buf, dataLen);
    int i = strlen(buf) - 1;
    while (!isalnum(buf[i])) --i;
    buf[i + 1] = '\0';
    int fd = open(buf, O_CREAT | O_RDWR, 0666);
    ERROR_CHECK(fd, -1, "open");

    off_t fileSize = 0;
    recvCycle(sockFd, &dataLen, 4);
    recvCycle(sockFd, &fileSize, dataLen);
    int ret = ftruncate(fd, fileSize);
    ERROR_CHECK(ret, -1, "ftruncate");

    char* pMap =
        (char*)mmap(NULL, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    ERROR_CHECK(pMap, (char*)-1, "mmap");
    recvCycle(sockFd, pMap, fileSize);
    munmap(pMap, fileSize);
    close(fd);
    return 0;
} 