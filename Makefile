LIBS=`pkg-config --cflags glib-2.0` `pkg-config --libs glib-2.0`
FLAGS=-std=c99
DEBUG=
parse:
	cc -c -o parse.o parse.c -g ${DEBUG} ${LIBS} ${FLAGS}
interpret:
	cc -c -o interpret.o interpret.c -g ${DEBUG} ${LIBS} ${FLAGS}
tests: parse interpret
	cc test_parse.c parse.o -o test_parse.out ${DEBUG} -g ${LIBS} ${FLAGS}
	cc test_interpret.c parse.o interpret.o -o test_interpret.out ${DEBUG} -g ${LIBS} ${FLAGS}
all: tests
clean:
	rm *.o *.out
test_run:  all
	./test_parse.out
	./interpret.out
