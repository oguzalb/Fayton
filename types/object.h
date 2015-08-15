#ifndef FAY_OBJECT_H
#include "../interpret.h"

void object_add_field(object_t *object, char* name, object_t *field);
object_t *object_get_field(object_t *object, char* name);
object_t *object_equals(GArray *args);
object_t *new_object_instance(GArray *args);
object_t *new_object(int type);
void init_object();

#define FAY_OBJECT_H
#endif
