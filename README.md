# Fayton
A minimalistic python interpreter written in c, just to see where i can get to :)

Currently in a very initial version with mem leaks all over the place since i don't care for now

Things that -barely- work can be seen inside test_interpret.c and under tests/ directory

Needs glib-dev and readline library installed

make all and everything is compiled

Things can be tried using repl.out

Classes and inheritance: implemented

Exceptions: implemented

Garbage collection: not implemented

list: implemented mostly

dictionary: half implemented

int: infinite numbers not implemented, rest of the functionality is mostly implemented

str: format does not accept tuple since tuples are not implemented, rest is working

magic functions: infrastructure and most important ones implemented

functions: working

closures: working

kwargs: working

generators: working but not stable, exceptions will be covered

for loop: working

while loop: working

arithmetic and tests: mostly working, 1 < a < 3 style does not work yet

list comprehensions: not implemented

metaclasses: not implemented

threads: added poc, but does not have concurrency checks, this is the last work that will be done
