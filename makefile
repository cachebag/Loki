CC = gcc
CFLAGS = -Wall -I/opt/homebrew/opt/libgit2/include -Iinclude
LDFLAGS = -L/opt/homebrew/opt/libgit2/lib -lgit2 -lncurses

SRC = src/main.c src/commands.c
OBJ = $(SRC:.c=.o)
TARGET = loki

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(OBJ) $(TARGET)

