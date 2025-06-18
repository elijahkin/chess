CC = g++
CFLAGS = -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror

all: chess

chess: chess.cpp
	$(CC) $(CFLAGS) -o chess chess.cpp
