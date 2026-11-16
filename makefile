CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic

banker: banker.c
	$(CC) $(CFLAGS) -o banker banker.c

.PHONY: clean
clean:
	rm -f banker