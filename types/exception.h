#ifndef FAY_EXCEPTION_H
#include "../interpret.h"

void init_exception();
object_t *new_exception(object_t **, int);

#define FAY_EXCEPTION_H
#endif
