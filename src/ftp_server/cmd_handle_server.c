#include "../../include/ftp_server.h"
#include "../../include/ftp_server_cmd.h"
#include "../../include/md5.h"

int logInCmd(int socketFd, char *buf, char *userName, char *passwd, char *instruction, MYSQL *mysql, MYSQL_RES *res, Client_t *clients, int c) {
    MYSQL_ROW row;
    MD5_CTX md5Info;
    unsigned char md5Res[16] = {0};
    int dataLen = 0;
    for (int i = 0; i < strlen(buf); ++i) {
        if (buf[i] == '@') {
            bzero(userName, sizeof(char) * 20);
            bzero(passwd, sizeof(char) * 30);
            strncpy(userName, buf, i);
            strcpy(passwd, buf + i + 1);
            bzeroInstr;
            sprintf(instruction,
                    "select salt from user_info where "
                    "userName = \'%s\'",
                    userName);
            mysqlCrud(mysql, instruction);
            res = mysql_store_result(mysql);
            while ((row = mysql_fetch_row(res))) {
                strcat(passwd, row[0]);
            }
            mysql_free_result(res);
            MD5Init(&md5Info);
            bzero(md5Res, sizeof(md5Res));
            MD5Update(&md5Info, (unsigned char*)passwd, strlen(passwd));
            MD5Final(&md5Info, md5Res);
            bzero(passwd, sizeof(char) * 30);
            for (int j = 0; j < 16; ++j) {
                snprintf(passwd + j * 2, 2 + 1, "%02x", md5Res[j]);
            }
            bzeroInstr;
            sprintf(instruction,
                    "select user_info.pwdHash, "
                    "files_info.no from user_info inner "
                    "join files_info on files_info.owner = "
                    "user_info.userName where "
                    "user_info.userName = \'%s\' and "
                    "files_info.fileName = \'%s\'",
                    userName, userName);
            mysqlCrud(mysql, instruction);
            res = mysql_store_result(mysql);
            if (mysql_num_rows(res) > 0) {
                while ((row = mysql_fetch_row(res))) {
                    mysql_free_result(res);
                    if (strcmp(passwd, row[0]) == 0) {
                        send(socketFd, "1", 1, 0);  // 登录成功
                        bzero(clients[c].clientName,
                              sizeof(clients[c].clientName));
                        bzero(clients[c].curDir, sizeof(clients[c].curDir));
                        strcpy(clients[c].clientName, userName);
                        strcpy(clients[c].curDir, userName);
                        clients[c].curNo = atoi(row[1]);
                        dataLen = strlen(row[1]);
                        send(socketFd, &dataLen, 4, 0);
                        send(socketFd, row[1], dataLen, 0);
                        return 0;
                    } else {
                        send(socketFd, "0", 1, 0);
                        // printf("登录失败\n");
                        break;
                    }
                }
            } else {
                mysql_free_result(res);
                send(socketFd, "2", 1, 0);
                // printf("不存在该用户\n");
            }
            break;
        }
    }
    return -1;
}


int signInCmd(int socketFd, char *buf, char *instruction, MYSQL *mysql, int c) {
    int isSignInSuccess = 0, isExist = 0;
    char userName[20] = {0}, passwd[30] = {0}, saltValue[11] = {0};
    MD5_CTX md5Info;
    unsigned char md5Res[16] = {0};
    int flag = 0, i, j;
    for (i = 1; i < strlen(buf); ++i) {
        if (buf[i] == '@' && i != strlen(buf) - 1) {
            
            for (j = i - 1; j >= 0; --j) {
                if (buf[j] == ' ') {
                    break;
                }
            }
            if (i == j + 1) break;
            strncpy(userName, buf + j + 1, i - j - 1);
            for (j = i + 1; j < strlen(buf); ++j) {
                if (buf[j] == ' ') {
                    break;
                }
            }
            if (i == j - 1) break;
            
            strncpy(passwd, buf + i + 1, j - i);
            flag = 1;
            isExist = isUserNameExist(mysql, userName);
            if (isExist == 1) {
                send(socketFd, "0", 1, 0);
                isSignInSuccess = 0;
            }else {
                getRandomStr(saltValue, 10);
                strcat(passwd, saltValue);
                MD5Init(&md5Info);
                bzero(md5Res, sizeof(md5Res));
                MD5Update(&md5Info, (unsigned char*)passwd, strlen(passwd));
                MD5Final(&md5Info, md5Res);
                bzero(passwd, sizeof(passwd));
                for (int j = 0; j < 16; ++j) {
                    snprintf(passwd + j * 2, 2 + 1, "%02x", md5Res[j]);
                }
                bzeroInstr;
                sprintf(instruction, "insert into user_info(userName, salt, pwdHash) values(\'%s\', \'%s\', \'%s\')", userName, saltValue, passwd);
                mysqlCrud(mysql, instruction);
                send(socketFd, "1", 1, 0);
                bzeroInstr;
                sprintf(instruction, "insert into files_info values(-1, %d, \'%s\', \'%s\', '0000', 4096, 0)", c, userName, userName);
                mysqlCrud(mysql, instruction);
                isSignInSuccess = 1;
            }
            break;
        }
    }
    if (!flag) {
        send(socketFd, "2", 1, 0);
    }
    return isSignInSuccess;
}

void pwdCmd(int socketFd, char *buf) {
    int dataLen = strlen(buf);
    send(socketFd, &dataLen, 4, 0);
    send(socketFd, buf, dataLen, 0);
}

void mkdirCmd(int socketFd, char *buf, char *instruction, int curNo, char *clientName, MYSQL *mysql, MYSQL_RES *res) {
    bzeroInstr;
    sprintf(instruction, "select * from files_info where preNo = %d and owner = \'%s\' and fileName = \'%s\'", curNo, clientName, buf);
    mysqlCrud(mysql, instruction);
    res = mysql_store_result(mysql);
    if (mysql_num_rows(res) > 0) {
        send(socketFd, "0", 1, 0);
        mysql_free_result(res);        
    }else {
        send(socketFd, "1", 1, 0);
        mysql_free_result(res);
        bzeroInstr;
        sprintf(instruction, "insert into files_info(preNo, fileName, owner, MD5, fileSize, fileType) values(%d, \'%s\', \'%s\', '0000', 4096, 0)", curNo, buf, clientName);
        mysqlCrud(mysql, instruction);
    }
}

void lsCmd(int socketFd, char *buf, char *instruction, MYSQL *mysql, MYSQL_RES *res, int curNo, char *clientName) {
    bzeroInstr;
    sprintf(instruction,
            "select fileType, owner, fileSize, "
            "fileName from files_info where preNo = %d "
            "and owner = \'%s\'",
            curNo, clientName);
    mysqlCrud(mysql, instruction);
    res = mysql_store_result(mysql);
    MYSQL_ROW row;
    int dataLen = 0;
    if (mysql_num_rows(res) > 0) {
        send(socketFd, "Y", 1, 0);
        while ((row = mysql_fetch_row(res))) {
            bzeroBuf;
            for (int i = 0; i < mysql_num_fields(res); ++i) {
                strcat(buf, row[i]);
                strcat(buf, " ");
            }
            // strcat(buf, "\t\t");
            dataLen = strlen(buf);
            send(socketFd, &dataLen, 4, 0);
            send(socketFd, buf, dataLen, 0);
        }
        send(socketFd, &dataLen, 4, 0);
        send(socketFd, "1", 1, 0);

    } else {
        send(socketFd, "N", 1, 0);
    }
    mysql_free_result(res);
}

void cdCmd(int socketFd, char *buf, char *instruction, MYSQL *mysql, MYSQL_RES *res, Client_t *clients, int c) {
    int i = 0, dataLen = 0;
    MYSQL_ROW row;
    if (strcmp(buf + 3, ".") == 0) {
        goto cdDot;
    }else if (strcmp(buf + 3, "..") == 0) {
        for (i = strlen(clients[c].curDir) - 1; i >= 0; --i) {
            if (clients[c].curDir[i] == '/') {
                clients[c].curDir[i] = '\0';
                break;
            }
        }
        for (i = strlen(clients[c].curDir) - 1; i >= -1; --i) {
            if (i == -1 || clients[c].curDir[i] == '/') {
                bzeroInstr;
                sprintf(instruction,
                        "select no from files_info where fileName = \'%s\' and "
                        "owner = \'%s\'",
                        clients[c].curDir + i + 1, clients[c].clientName);
                mysqlCrud(mysql, instruction);
                res = mysql_store_result(mysql);
                row = mysql_fetch_row(res);
                clients[c].curNo = atoi(row[0]);
                mysql_free_result(res);
                goto cdDot;
            }
        }
    }else {
        bzeroInstr;
        sprintf(instruction,
                "select no from files_info where owner = \'%s\' and fileName = "
                "\'%s\'",
                clients[c].clientName, buf + 3);
        mysqlCrud(mysql, instruction);
        res = mysql_store_result(mysql);
        if (mysql_num_rows(res) == 0) {
            send(socketFd, "0", 1, 0);
            mysql_free_result(res);
        } else {
            row = mysql_fetch_row(res);
            strcat(clients[c].curDir, "/");
            strcat(clients[c].curDir, buf + 3);
            clients[c].curNo = atoi(row[0]);
            mysql_free_result(res);
        }
    cdDot:
        send(socketFd, "1", 1, 0);
        dataLen = strlen(clients[c].curDir);
        send(socketFd, &dataLen, 4, 0);
        send(socketFd, clients[c].curDir, dataLen, 0);
        //发送当前目录的下标
        send(socketFd, &clients[c].curNo, 4, 0);
    }
}

int rmCmd(int socketFd, char *buf, char *instruction, MYSQL *mysql, MYSQL_RES *res, char *clientName) {
    int preDirNo = 0;
    recv(socketFd, &preDirNo, 4, 0);
    bzeroInstr;
    sprintf(instruction, "select * from files_info where owner = \'%s\' and fileName = \'%s\' and preNo = %d", clientName, buf + 3, preDirNo);
    mysqlCrud(mysql, instruction);
    res = mysql_store_result(mysql);
    if (mysql_num_rows(res) > 0) {
        bzeroInstr;
        sprintf(instruction, "delete from files_info where owner = \'%s\' and fileName = \'%s\' and preNo = %d", clientName, buf + 3, preDirNo);
        mysqlCrud(mysql, instruction);
        send(socketFd, "1", 1, 0);
    }else {
        send(socketFd, "0", 1, 0);
        return -1;
    }
    mysql_free_result(res);
    return 0;
}
