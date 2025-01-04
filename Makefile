CC = gcc
CFLAGS = -lncurses

main: main.c
	$(CC) main.c -o main $(CFLAGS)

clean:
	rm -f main