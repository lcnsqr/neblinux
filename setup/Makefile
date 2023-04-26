CC = gcc
FLAGS = -Wall

all : setup

setup : setup.o serial.o mat.o
	$(CC) -o $@ $^ -lreadline -pthread -lm $(FLAGS)

setup.o : setup.c
	$(CC) -c -o $@ $^ $(FLAGS)

serial.o : serial.c
	$(CC) -c -o $@ $^ $(FLAGS)

mat.o : mat.c
	$(CC) -c -o $@ $^ $(FLAGS)


clean :
	rm setup setup.o mat.o serial.o
