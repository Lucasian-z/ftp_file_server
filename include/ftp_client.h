#ifndef __FTP_CLIENT_H__
#define __FTP_CLIENT_H__

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