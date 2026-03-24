CFLAGS=-Wall -Wextra -Werror -Wvla -std=c99 -pedantic -O1
CPPFLAGS=-Isrc
LDLIBS=-lm -lz

TARGET=bad_apple
TARGET_FUNC=tests/testsuite.sh
OBJ_MAIN=src/main.o
OBJ=

all: $(TARGET)

$(TARGET): $(OBJ_MAIN) $(OBJ)

check: all
	./$(TARGET_FUNC)

dev: CFLAGS+=-fsanitize=address -g
dev: LDLIBS+=-fsanitize=address
dev: check
