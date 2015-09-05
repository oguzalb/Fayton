#ifndef FAY_OBJECT_H
#define FAY_OBJECT_H
#include "../interpret.h"

void object_add_field(object_t *object, char* name, object_t *field);
object_t *object_call_func_obj(object_t *func, object_t **param_objs);
object_t *object_call_func_no_param(object_t *object, char *func_name);
object_t *object_get_field(object_t *object, char* name);
object_t *object_get_field_no_check(object_t *object, char* name);
object_t *object_set_field(object_t *object, char* field_name, object_t *value);
object_t *object_call_repr(object_t *object);
object_t *object_call_str(object_t *object);
object_t *object_equals(object_t **args);
object_t *new_object_instance(object_t **args);
object_t *new_object(int type);
gboolean object_equal(gconstpointer a, gconstpointer b);
guint object_hash(gconstpointer key);
void init_object();

object_t *object_class;

#endif
