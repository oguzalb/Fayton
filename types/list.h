#ifndef FAY_LIST_H
#include "../interpret.h"

void init_list();
object_t *new_list(GArray *);
object_t *new_list_internal();
object_t *list_iter_func(GArray *);
object_t *list_append(GArray *);
object_t *list_append_internal(object_t *, object_t *);

object_t *new_listiterator(GArray *);
object_t *listiterator_next_func(GArray *);
object_t *new_listiterator_internal(object_t *);

#define FAY_LIST_H
#endif
