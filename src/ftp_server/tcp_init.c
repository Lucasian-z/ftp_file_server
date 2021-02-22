#include "../../include/ftp_server.h"

int tcpInit(int *sockFd, char *ip, char *port) {
    int socketFd = socket(AF_INET, SOCK_STREAM, 0);
    ERROR_CHECK(socketFd, -1, "socket");

    struct sockaddr_in serAddr;
    bzero(&serAddr, sizeof(serAddr));
    serAddr.sin_addr.s_addr = inet_addr(ip);
    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons(atoi(port));

    int reuse = 1;
    int ret = setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    ERROR_CHECK(ret, -1, "setsockopt");
    ret = bind(socketFd, (struct sockaddr*)&serAddr, sizeof(serAddr));
    ERROR_CHECK(ret, -1, "bind");

    ret = listen(socketFd, 10);
    ERROR_CHECK(ret, -1, "listen");
    *sockFd = socketFd;
    return 0;
}