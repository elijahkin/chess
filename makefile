CC = g++
CFLAGS = -std=c++11 -O2 -Wall -Wextra

all: chess

chess: chess.cpp
	$(CC) $(CFLAGS) -o chess chess.cpp
