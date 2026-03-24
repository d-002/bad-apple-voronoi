CFLAGS=-Wall -Wextra -Werror -Wvla -std=c99 -pedantic -O1
CPPFLAGS=-Isrc
LDLIBS=-lm -lSDL2 -lSDL2_image

TARGET=bad_apple
TARGET_FUNC=tests/testsuite.sh
OBJ_MAIN=src/main.o
OBJ=src/image/loadimage.o \
	src/logger/logger.o

.PHONY: all dev clean

all: $(TARGET)

$(TARGET): $(OBJ_MAIN) $(OBJ)
	$(CC) -o $@ $^ $(LDLIBS) $(LDFLAGS)

check: all
	./$(TARGET_FUNC)

dev: CFLAGS+=-fsanitize=address -g
dev: LDLIBS+=-fsanitize=address
dev: check

clean:
	$(RM) $(TARGET) $(OBJ_MAIN) $(OBJ)
