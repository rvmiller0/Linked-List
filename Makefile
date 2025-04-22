CC = gcc
CFLAGS = -Wall -Wextra -pthread -O3
TARGET = test-lock-free

all: $(TARGET)

$(TARGET): test-lock-free.c lock-free.c
	$(CC) $(CFLAGS) -o $(TARGET) test-lock-free.c

clean:
	rm -f $(TARGET)