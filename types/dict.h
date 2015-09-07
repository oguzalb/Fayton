#ifndef FAY_DICT_H
#include "../interpret.h"

void init_dict();
object_t *new_dict(object_t **, int count);
object_t *dict_keys(object_t **, int count);

#define FAY_DICT_H
#endif
