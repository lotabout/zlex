CC = gcc
CFLAGS = -Wall

COMPONENTS = escape nfa
LIBS = ${patsubst %,%.o,${COMPONENTS}}
TESTS = ${patsubst %.c,%,$(wildcard test*.c)}

all: test

%.o: %.c
	${CC} ${CFLAGS} -c $<
	
${TESTS}: ${LIBS}
${TESTS}: %: %.o
	${CC} ${CFLAGS} -o $@ $^

.PHONY: test
test: ${TESTS}
	@for exe in ${TESTS}; do \
	    echo "======== testing $$exe =======";\
	    ./$$exe ;\
	done \

.PHONY: clean
clean:
	rm *.o ${TESTS}

