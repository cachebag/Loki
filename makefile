CC = gcc
CFLAGS = -Wall -Iinclude
LDFLAGS = -lgit2

SRC = src/main.c src/commands.c
OBJ = $(SRC:.c=.o)
TARGET = loki

all: $(TARGET)

$(TARGET): $(OBJ)
    $(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(OBJ) $(TARGET)
