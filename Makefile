LIBS=`pkg-config --cflags glib-2.0` `pkg-config --libs glib-2.0`
FLAGS=-std=gnu99 -g
DEBUG=
SOURCES=int.c bool.c list.c dict.c str.c thread.c interpret.c parse.c
OBJECTS=$(SOURCES:.c=.o)
.c.o:
	cc -c $< ${DEBUG} ${LIBS} ${FLAGS} -o $@

tests: $(OBJECTS)
	cc test_parse.c parse.o -o test_parse.out ${DEBUG} -g ${LIBS} ${FLAGS}
	cc test_interpret.c parse.o interpret.o int.o str.o bool.o list.o dict.o thread.o -o test_interpret.out ${DEBUG} -g ${LIBS} ${FLAGS}
	cc repl.c parse.o interpret.o int.o str.o bool.o list.o dict.o thread.o -o repl.out ${DEBUG} -g ${LIBS} ${FLAGS}
all: tests
clean:
	rm *.o *.out
test_run:  all
	./test_parse.out
	./test_interpret.out
