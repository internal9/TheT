# temp
CC = gcc
CCFLAGS = -std=c99 -O0 -pedantic -Wall -Wextra -Wfloat-equal -Wimplicit-int -Wundef -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wstrict-overflow=4 -Wwrite-strings -Waggregate-return -Wcast-qual -Wswitch-default -Wswitch-enum -Wconversion -Wunreachable-code -Wformat -Wsequence-point -fno-diagnostics-color -Wno-switch-enum -Wno-switch

b64l: main.c lex.c bytecode_gen.c hashmap.c
	$(CC) $(CCFLAGS) -o b64l main.c lex.c bytecode_gen.c hashmap.c
