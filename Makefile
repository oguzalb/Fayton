LIBS=`pkg-config --cflags glib-2.0` `pkg-config --libs glib-2.0` -lreadline -lpthread
FLAGS=-std=gnu99 -g
DEBUG=
TYPES=object.c int.c bool.c list.c dict.c str.c thread.c none.c slice.c generator.c
SOURCES=interpret.c parse.c $(TYPES:%.c=types/%.c) utils.c
OBJECTS=$(SOURCES:.c=.o)
PY_TESTS=test_list.py test_int.py test_for.py test_bool.py test_class.py test_dict.py test_try.py
PY_TEST_PATHS=$(PY_TESTS:%.py=tests/%.py)
.c.o:
	cc -c $< ${DEBUG} ${LIBS} ${FLAGS} -o $@

tests: $(OBJECTS)
	cc test_parse.c parse.o -o test_parse.out ${DEBUG} -g ${LIBS} ${FLAGS} utils.o
	cc test_interpret.c ${OBJECTS} -o test_interpret.out ${DEBUG} -g ${LIBS} ${FLAGS}
repl: $(OBJECTS)
	cc repl.c ${OBJECTS} -o repl.out ${DEBUG} -g ${LIBS} ${FLAGS}
all: repl tests
clean:
	find . -name "*.o" -delete -o -name "*.out" -delete
test_run:  all
	./test_parse.out
	./test_interpret.out
	for py_test in $(PY_TEST_PATHS); do \
	    ./repl.out $$py_test || break; \
	done
test_valgrind_run:  all
	valgrind ./test_parse.out
	valgrind ./test_interpret.out
	for py_test in $(PY_TEST_PATHS); do \
	    valgrind ./repl.out $$py_test || break; \
	done
