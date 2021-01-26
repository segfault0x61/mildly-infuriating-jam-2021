###############################################################################
# Variables
###############################################################################
CC = gcc
CC_FLAGS = -std=c99 -Wall
SRC = ./src/*.c

ifeq ($(OS),Windows_NT) 
	LINKER_FLAGS = -lmingw32 
else
	LINKER_FLAGS = -lm 
endif

LINKER_FLAGS += -lSDL2main -lSDL2 -lSDL2_image

BIN_NAME = game

###############################################################################
# Makefile rules
###############################################################################
build:
	${CC} ${CC_FLAGS} ${SRC} ${LINKER_FLAGS} -o ${BIN_NAME}

run:
	./${BIN_NAME}

clean:
	rm ${BIN_NAME}