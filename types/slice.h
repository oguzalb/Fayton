#ifndef FAY_SLICE_H
#include "../interpret.h"


void init_slice();
object_t *new_slice(GArray *);
object_t *new_slice_internal(object_t *, object_t *, object_t *);
#endif
