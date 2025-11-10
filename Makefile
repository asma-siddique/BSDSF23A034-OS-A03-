CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude
SRC = src/main.c src/shell.c src/execute.c
TARGET = bin/myshell

all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p bin
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

