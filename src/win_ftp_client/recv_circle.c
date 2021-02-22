#include "win_ftp_client.h"

int recvCycle(int socketFd, void *buf, int dataLen) {
    char *p = (char*)buf;
    int total = 0, ret = 0;
    while (total < dataLen) {
        ret = recv(socketFd, p + total, dataLen - total, 0);
        if (ret == 0) {
            return -1;
        }
        total += ret;
    }
    return 0;
}