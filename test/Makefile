CC = gcc
SRC = ../logic.c main.c
OBJ = $(SRC:.c=.o)
BIN = test
%.o: %.c
	$(CC) -c -o $@ $<
$(BIN): $(OBJ)
	$(CC) -o test $(OBJ)
