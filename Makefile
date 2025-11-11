CC = cc
CFLAGS = -Wall -Wextra -g -Iinclude
TARGET = bin/myshell
SOURCES = src/main.c src/shell.c src/execute.c

$(TARGET): $(SOURCES)
	mkdir -p bin
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)
	rm -f *.txt

.PHONY: clean
