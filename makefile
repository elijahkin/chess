CC = clang++
CFLAGS = -std=c++23 -O3 -Wall -Wextra -Wpedantic -Werror -fno-exceptions -fno-rtti -flto -s

all: chess

chess: src/main.cpp
	$(CC) $(CFLAGS) -o bin/chess src/main.cpp

clean:
	rm -f bin/*
