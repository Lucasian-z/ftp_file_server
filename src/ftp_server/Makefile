SRCS:=$(wildcard *.c)
OBJS:=$(patsubst %.c, %.o, $(SRCS))
ELF:=ftp_server
CC:=gcc
CFLAGS:=-g -Wall 
$(ELF):$(OBJS)
	gcc -o $@ $^ -pthread -lmysqlclient
clean:
	rm -rf $(OBJS) $(ELF)