CC = gcc
CFLAGS = -Wall -Wextra -pthread
TARGET = test

all: $(TARGET)

$(TARGET): test.c linked-list.c
	$(CC) $(CFLAGS) -o $(TARGET) test.c

clean:
	rm -f $(TARGET)