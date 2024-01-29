.PHONY: clean all

srcs := $(wildcard *.c)
targets := $(patsubst %.c,%,$(srcs))


CC=gcc
CFLAGS=-g

all: ${targets}

%.out: %.c build
	${CC} ${CFLAGS} -o $@ $< 

clean:
	rm -rf $(targets)
