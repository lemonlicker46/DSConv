CC = gcc
CFLAGS = -Iinclude -std=gnu11 -Wall -Wextra

all: dsconv

dsconv: src/DSConv.c src/lexer.c src/parser.c src/generator.c
	$(CC) $(CFLAGS) src/DSConv.c src/lexer.c src/parser.c src/generator.c -o dsconv

clean:
	rm -f dsconv
