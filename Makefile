CC = gcc
CFLAGS = -Wall -Wextra -pthread
TARGETS = test test-lock-free

all: $(TARGETS)

test: test.c linked-list.c
	$(CC) $(CFLAGS) -o test test.c

test-lock-free: test-lock-free.c lock-free.c
	$(CC) $(CFLAGS) -o test-lock-free test.c

clean:
	rm -f $(TARGET)