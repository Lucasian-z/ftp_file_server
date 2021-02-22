#ifndef __FTP_SERVER_CMD_H__
#define __FTP_SERVER_CMD_H__

int signInCmd(int, char*, char*, MYSQL*, int);

int logInCmd(int, char*, char*, char*, char*, MYSQL*, MYSQL_RES*, Client_t*, int);

void pwdCmd(int, char*);

void mkdirCmd(int, char*, char*, int, char*, MYSQL*, MYSQL_RES*);

void lsCmd(int, char*, char*, MYSQL*, MYSQL_RES*, int, char*);

void cdCmd(int, char*, char*, MYSQL*, MYSQL_RES*, Client_t*, int);

int rmCmd(int, char*, char*, MYSQL*, MYSQL_RES *res, char*);

#endif