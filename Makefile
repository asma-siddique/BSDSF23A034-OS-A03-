kCC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude
SRC = src/main.c src/shell.c
OUT = bin/myshell

all: $(OUT)

$(OUT): $(SRC)
	mkdir -p bin
	$(CC) $(CFLAGS) -o $(OUT) $(SRC)

clean:
	rm -f bin/myshell
	rm -f *.txt

