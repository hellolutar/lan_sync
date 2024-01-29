.PHONY: clean all

srcs := $(wildcard *.cpp)
targets := $(patsubst %.cpp,%,$(srcs))


CC=g++
CFLAGS=-g
CLDFLAGS:=-levent

all: ${targets}

%.out: %.c build
	${CC} ${CFLAGS} -o $@ $< 

clean:
	rm -rf $(targets)
