LIBS=`pkg-config --cflags glib-2.0` `pkg-config --libs glib-2.0`
FLAGS=-std=gnu99 -g
DEBUG=
TYPES=int.c bool.c list.c dict.c str.c thread.c none.c
SOURCES=interpret.c parse.c $(TYPES:%.c=types/%.c)
OBJECTS=$(SOURCES:.c=.o)
.c.o:
	cc -c $< ${DEBUG} ${LIBS} ${FLAGS} -o $@

tests: $(OBJECTS)
	cc test_parse.c parse.o -o test_parse.out ${DEBUG} -g ${LIBS} ${FLAGS}
	cc test_interpret.c ${OBJECTS} -o test_interpret.out ${DEBUG} -g ${LIBS} ${FLAGS}
repl: $(OBJECTS)
	cc repl.c ${OBJECTS} -o repl.out ${DEBUG} -g ${LIBS} ${FLAGS}
all: repl tests
clean:
	find . -name "*.o" -delete -o -name "*.out" -delete
test_run:  all
	./test_parse.out
	./test_interpret.out
