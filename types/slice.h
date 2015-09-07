#ifndef FAY_SLICE_H
#include "../interpret.h"


void init_slice();
object_t *new_slice(object_t **, int);
void set_indices(object_t* slice, int last_index, int* start, int* stop, int* step);
object_t *new_slice_internal(object_t *, object_t *, object_t *);
#endif
