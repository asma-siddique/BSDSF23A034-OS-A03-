# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -g -Iinclude

# Source files
SRCS = src/main.c src/shell.c src/execute.c

# Output executable
TARGET = shell

# Default target: build the shell
all: $(TARGET)

# Compile the executable
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

# Clean build files
clean:
	rm -f $(TARGET) *.o

# Run the shell
run: $(TARGET)
	./$(TARGET)

