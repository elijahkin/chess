CC = g++
CFLAGS = -std=c++23 -O2 -Wall -Wextra -Wpedantic -Werror -fno-exceptions

all: chess

chess: src/main.cpp
	$(CC) $(CFLAGS) -o bin/chess src/main.cpp

clean:
	rm -f bin/*
