#ifndef FAY_DICT_H
#include "../interpret.h"

void init_dict();
object_t *new_dict(object_t **);
object_t *dict_keys(object_t **);

#define FAY_DICT_H
#endif
