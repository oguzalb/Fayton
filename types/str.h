#ifndef FAY_STR_H
#include "../interpret.h"

void init_str();
object_t *new_str(GArray *);
object_t *new_str_internal(char*);

#define FAY_STR_H
#endif
