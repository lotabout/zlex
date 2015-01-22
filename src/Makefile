CC = gcc
CFLAGS = -Wall


COMPONENTS = escape nfa set printnfa hash terp dfa
LIBS = ${patsubst %,%.o,${COMPONENTS}}
TESTS = ${patsubst %.c,%,$(wildcard test*.c)}

all: test

debug: CFLAGS += -DDEBUG -g
debug: test

%.o: %.c
	${CC} ${CFLAGS} -c $<

${TESTS}: ${LIBS}
${TESTS}: %: %.o
	${CC} ${CFLAGS} -o $@ $^

.PHONY: test
test: ${TESTS}

.PHONY: run_test
run_test: ${TESTS}
	@for exe in ${TESTS}; do \
	    echo "======== testing $$exe =======";\
	    ./$$exe ;\
	done \

.PHONY: clean
clean:
	rm *.o ${TESTS}

