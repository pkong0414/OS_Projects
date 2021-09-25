#!bin/bash
# this is the Makefile for Project2

CC = gcc
CFLAGS = -I. -g
TARGETS = runsim testsim
RUNSIM = runsim.c license.h detachandremove.c
TESTSIM = testsim.c
SRC = runsim.c detachandremove.c
OBJ = runsim.o detachandremove.o

all: $(TARGETS)

$(OBJ): $(SRC)
	$(CC) -c $(SRC)

runsim: $(OBJ)
	$(CC) -o $@ $(OBJ)

clean:
	/bin/rm '-f' *.o $(TARGETS) *.data