#ifndef FAY_BOOL_H
#include "interpret.h"

void init_bool();
object_t *new_bool(GArray *);
object_t *new_bool_internal(object_t *object);

#define FAY_BOOL_H
#endif
