
CC     := gcc
CFLAGS := -O3 -Wall -Werror

sources := \
	main.c

all: gbz80-none-any-objdump

clean:
	-rm -f gbz80-none-any-objdump

gbz80-none-any-objdump: $(sources)
	${CC} $(CFLAGS) $< -o $@
