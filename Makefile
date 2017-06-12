
CC := gcc

all: gbz80-none-any-objdump

gbz80-none-any-objdump: main.c ; ${CC} -O3 $< -o $@
