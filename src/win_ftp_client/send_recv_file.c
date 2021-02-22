#include "win_ftp_client.h"
#include "md5.h"

int sendFile(int socketFd, char* fileName) {
    FILE * fp = fopen(fileName, "ab+");
    if (fp == NULL) {
        perror("fopen");
        return -1;
    }
    struct stat buf;
    stat(fileName, &buf);  //获取该文件的信息
    train_t data;
    memset(&data, 0, sizeof(data));
    data.dataLen = strlen(fileName);
    strcpy(data.buf, fileName);
    send(socketFd, (char*)&data, 4 + data.dataLen, 0);
    
    // printf("fileSize = %ld\n", buf.st_size);
    data.dataLen = sizeof(buf.st_size);
    memcpy(data.buf, &buf.st_size, data.dataLen);
    send(socketFd, (char*)&data, 4 + data.dataLen, 0);
    off_t uploadSize = 0, fileSize = buf.st_size, lastUploadSize = 0, slice = fileSize / 1000;
    char data1[1000] = {0};
    int ret = 0, dataLen = 0;
    while (dataLen = fread(data1, sizeof(char), 1000, fp)) {
        ret = send(socketFd, data1, dataLen, 0);
        uploadSize += ret;
        if (uploadSize >= fileSize) {
            printf("100.00%%\n");
            break;
        }
        if (uploadSize - lastUploadSize >= slice) {
            printf("%5.2f%%\r", (float)uploadSize / fileSize * 100);
            fflush(stdout);
            lastUploadSize = uploadSize;
        }
    }
    fclose(fp);
    printf("上传完成\n");
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
    FILE *fp = fopen(buf, "ab+");
    if (fp == NULL) {
        perror("fopen");
        return -1;
    }

    off_t fileSize = 0;
    recvCycle(sockFd, &dataLen, 4);
    recvCycle(sockFd, &fileSize, dataLen);
    off_t downloadSize = 0, lastDownloadSize = 0, slice = fileSize / 1000;
    
    while (downloadSize < fileSize) {
        memset(buf, 0, sizeof(buf));
        dataLen = recv(sockFd, buf, sizeof(buf), 0);
        downloadSize += dataLen;
        // printf("dataLen = %d, downloadSize = %d\n", dataLen, downloadSize);
        if (downloadSize > fileSize) {
            printf("100.00%%\n");
            break;
        }
        if (downloadSize - lastDownloadSize > slice) {
            printf("%5.2f%%\r", (float)downloadSize / fileSize * 100);
            fflush(stdout);
            lastDownloadSize = downloadSize;
        }
        fwrite(buf, sizeof(char), dataLen, fp);
    }
    fclose(fp);
    printf("下载完成\n");
    return 0;
} 