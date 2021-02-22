### ftp_file_server
简介：这是一个简单的文件服务器项目，主要使用了多线程、数据库等技术。其中简单命令(login、signin、pwd、ls、rm)用主线程处理，复杂命令(put、get)用子线程处理
### 使用说明
#### 编译
    - 分别进入/ftp_server,/ftp_client内使用make进行编译
    - win_client, gcc .\win_ftp_client.c win_ftp_client_cmd.c md5.c send_recv_file.c recv_circle.c ico.o -lwsock32 -finput-charset=UTF-8 -fexec-charset=GBK 其中ico.o文件需自行准备
#### 使用
	conf文件夹内的配置文件中的ip地址需自行填写
    首先运行服务端(./ftp_server)，再运行(./ftp_client)，共支持8个命令，分别是login、signin、
    pwd、cd、ls、get、put、rm，用户首先必须登录或者注册，才能进行后续操作
    - login：login usrName@passwd    #登录账户
    - signin：signin usrName@passwd  #注册账户
    - pwd: 列出用户当前所处目录
    - cd: cd dirPath  #进入文件夹，支持.和.. 暂不支持多级跳跃
    - ls: 列出当前目录的所有文件信息
    - get: get fileName  #下载文件
    - put: put fileName  #上传文件
    - rm: rm fileName    #删除文件
#### 以下是项目文件树, 存在很多冗余
├── conf
│   └── ftp_server.conf
├── include
│   ├── ftp_client.h
│   ├── ftp_server_cmd.h
│   ├── ftp_server.h
│   ├── md5.h
│   ├── work_factory.h
│   └── work_que.h
├── readme.md
└── src
    ├── ftp_client
    │   ├── child_pthread.c
    │   ├── cmd_handle.c
    │   ├── ftp_client.c
    │   ├── Makefile
    │   ├── md5.c
    │   ├── recv_circle.c
    │   └── send_recv_file.c
    ├── ftp_server
    │   ├── cmd_handle_server.c
    │   ├── epoll_add.c
    │   ├── ftp_server.c
    │   ├── Makefile
    │   ├── md5.c
    │   ├── mysql_operation.c
    │   ├── random_str.c
    │   ├── recv_circle.c
    │   ├── send_recv_file.c
    │   ├── tcp_init.c
    │   ├── work_factory.c
    │   └── work_que.c
    └── win_ftp_client
        ├── md5.c
        ├── md5.h
        ├── recv_circle.c
        ├── send_recv_file.c
        ├── win_ftp_client.c
        ├── win_ftp_client_cmd.c
        ├── win_ftp_client_cmd.h
        └── win_ftp_client.h

6 directories, 35 files
