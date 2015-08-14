#ifndef FAY_DICT_H
#include "../interpret.h"

void init_dict();
object_t *new_dict(GArray *);
object_t *dict_keys(GArray *);

#define FAY_DICT_H
#endif
