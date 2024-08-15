CC = gcc
CFLAGS = -Wall $(shell pkg-config --cflags libgit2 ncurses) -Iinclude
LDFLAGS = $(shell pkg-config --libs libgit2 ncurses)

SRC = src/main.c src/commands.c
OBJ = $(SRC:.c=.o)
TARGET = loki

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(OBJ) $(TARGET)
