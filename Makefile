# Makefile for compiling main.c into hello

all: hello

hello: main.c
	gcc -g main.c -o hello

clean:
	rm -f hello
