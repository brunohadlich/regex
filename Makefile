CC=gcc
SRC=./src
INC=./headers
OBJ=./obj
BIN=./bin
LIB=./lib
AR=ar

lib_static:
	$(CC) -I$(INC) -g -Wall -D DEBUG -c $(SRC)/*.c
	mv *.o $(OBJ)/static
	$(AR) -rcs $(LIB)/static/libregex.a $(OBJ)/static/*.o

lib_shared:
	$(CC) -I$(INC) -c -Wall -D DEBUG -fPIC $(SRC)/*.c
	mv *.o $(OBJ)/shared
	$(CC) -shared $(OBJ)/shared/automata.o $(OBJ)/shared/regex.o -Wl,-soname,libregex.so.1 -o $(LIB)/shared/libregex.so.1.0.0

tests: lib_static
	$(CC) $(OBJ)/static/tests.o -L$(LIB)/static/ -g -lregex -o $(BIN)/tests


all: lib_static lib_shared tests

clean:
	rm -f bin/* lib/static/* lib/shared/* obj/static/* obj/shared/*
