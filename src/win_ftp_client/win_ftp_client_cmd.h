#ifndef __WIN_FTP_CLIENT_CMD_H__ 
#define __WIN_FTP_CLIENT_CMD_H__
// #include "win_ftp_client.h"
#pragma comment(lib, "ws2_32.lib")

void printInfo();

int getConfig(char*, int*);

int userLogIn(int, char*, char*, int*);

int userSignIn(int, char*);

void pwdHandle(int, char*);

void lsHandle(int, char*);

void mkdirHandle(int, char*);

void cdHandle(int, char*, int*);

void rmHandle(int, char*, const int);

#endif