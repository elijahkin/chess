CC = g++
CFLAGS = -std=c++23 -O2 -Wall -Wextra -Wpedantic

all: chess

chess: src/chess.cpp
	$(CC) $(CFLAGS) -o bin/chess src/chess.cpp

clean:
	rm -f bin/*
