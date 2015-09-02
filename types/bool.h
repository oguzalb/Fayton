#ifndef FAY_BOOL_H
#include "../interpret.h"

void init_bool();
object_t *new_bool(object_t **);
object_t *new_bool_internal(object_t *object);
object_t *new_bool_from_int(int value);

#define FAY_BOOL_H
#endif
