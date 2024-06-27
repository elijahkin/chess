CC = g++
CFLAGS = -std=c++11

all: chess

chess: src/chess.cpp
	$(CC) $(CFLAGS) -o bin/chess src/chess.cpp
