CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -O2 -pthread
TARGET = vm

all: $(TARGET)

$(TARGET): vm.c
	$(CC) $(CFLAGS) -o $(TARGET) vm.c

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
