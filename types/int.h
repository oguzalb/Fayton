#ifndef FAY_INT_H
#include "../interpret.h"

void init_int();
object_t *new_int(object_t **);
object_t *new_int_internal(int);

#define FAY_INT_H
#endif
