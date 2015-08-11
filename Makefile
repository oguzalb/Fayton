LIBS=`pkg-config --cflags glib-2.0` `pkg-config --libs glib-2.0`
FLAGS=-std=c99
DEBUG=
parse:
	cc -c -o parse.o parse.c -g ${DEBUG} ${LIBS} ${FLAGS}
tests: parse
	cc test_parse.c parse.o -o test_parse.out ${DEBUG} -g ${LIBS} ${FLAGS}
all: parse tests
	cc interpret.c parse.o -o interpret.out ${DEBUG} -g ${LIBS} ${FLAGS}
clean:
	rm *.o *.out
test_run:  all
	./test_parse.out
	./interpret.out
