CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude
LDFLAGS = 

SRC = src/main.c src/shell.c
OUT = bin/myshell

all:
	mkdir -p bin
	$(CC) $(CFLAGS) -o $(OUT) $(SRC) $(LDFLAGS)

clean:
	rm -f $(OUT)
	rm -f *.txt
