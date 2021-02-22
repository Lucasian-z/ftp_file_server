#include "../../include/ftp_server.h"
void mysqlConnect(MYSQL *mysql, char *databaseName) {
    if (mysql_real_connect(mysql, "localhost", "zh_test", "zhtest", databaseName, 0, NULL, 0) == NULL) {
        printf("mysql connect error, %s\n", mysql_error(mysql));
    }
}

void mysqlCrud(MYSQL *mysql, char *instruction) {
    if (mysql_real_query(mysql, instruction, (unsigned int)strlen(instruction)) != 0) {
        printf("mysql crud error, %s\n", mysql_error(mysql));
    }
}

int isUserNameExist(MYSQL *mysql, char *userName) {
    char instruction[100] = {0};
    sprintf(instruction, "select * from user_info where userName = \'%s\'", userName);
    // printf("instruction = %s\n", instruction);
    mysqlCrud(mysql, instruction);
    MYSQL_RES *res = mysql_store_result(mysql);
    if (res == NULL) {
        printf("mysql_store_result error\n");
    }
    // printf("%lld\n", mysql_num_rows(res));
    int cnt = mysql_num_rows(res);
    mysql_free_result(res);
    return cnt > 0;
}