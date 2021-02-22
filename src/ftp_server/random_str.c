#include "../../include/ftp_server.h"

int getRandomStr(char *randomStr, const int randomLen) {
    char *seedStr = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz,./;\"<>?";
    int seedLen = strlen(seedStr);

    struct timeval tv;
    gettimeofday(&tv, NULL);
    unsigned int seedNum = (unsigned int)(tv.tv_sec + tv.tv_usec);
    srand(seedNum);

    int randIdx = 0;
    for (int i = 0; i < randomLen; ++i) {
        randIdx = rand() % seedLen;
        randomStr[i] = seedStr[randIdx];
    }
    return 0;
}