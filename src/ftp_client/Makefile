SRCS:=$(wildcard *.c)
OBJS:=$(patsubst %.c, %.o, $(SRCS))
ELF:=ftp_client
CC:=gcc
CFLAGS:=-g -Wall
$(ELF):$(OBJS)
	gcc -pthread -o $@ $^
clean:
	rm -rf $(OBJS) $(ELF)