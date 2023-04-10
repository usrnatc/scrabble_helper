CC=clang
CFLAGS=-std=gnu99 -Wall -lm -pedantic -Werror -g
SRC=scrabble_helper.c
OBJ=$(SRC:.c = .o)
PROG=scrabble_helper

.DEFAULT_GOAL: all
.PHONY:= clean

all: $(OBJ); $(CC) $(CFLAGS) -o $(PROG) $(OBJ);

clean: ; rm -rf $(PROG) *.o;
