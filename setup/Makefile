CC = gcc
#FLAGS = -Wall
FLAGS = 

all : setup

setup : setup.o serial.o socket.o shell.o
	$(CC) -o $@ $^ -lreadline -pthread -lm $(FLAGS)

setup.o : setup.c
	$(CC) -c -o $@ $^ $(FLAGS)

shell.o : shell.c
	$(CC) -c -o $@ $^ $(FLAGS)

serial.o : serial.c
	$(CC) -c -o $@ $^ $(FLAGS)

socket.o : socket.c
	$(CC) -c -o $@ $^ $(FLAGS)


clean :
	rm setup setup.o serial.o socket.o shell.o
