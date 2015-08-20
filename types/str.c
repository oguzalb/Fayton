#include "str.h"

object_t *new_str(GArray *args) {
    printd("NEW STR\n");
    object_t *str_obj = g_array_index(args, object_t *, 1);
    if (str_obj->type != STR_TYPE) {
        interpreter.error = RUN_ERROR;
        return NULL;
    }
    return str_obj;
}

object_t *str_repr_func(GArray *args) {
    object_t *self = g_array_index(args, object_t *, 0);
    return self;
}

object_t *new_str_internal(char* value) {
    object_t *str_obj = new_object(STR_TYPE);
    str_obj->class = get_global("str");
    str_obj->str_props = malloc(sizeof(struct str_type));
    str_obj->str_props->ob_sval = g_string_new(value);
    printd("NEW STR %s\n", str_obj->str_props->ob_sval->str);
    return str_obj;
}

void init_str() {
    object_t *str_class = new_class(strdup("str"));
    str_class->class_props->ob_func = new_str;
    object_add_field(str_class, "__repr__", new_func(str_repr_func, strdup("__repr__")));
    register_global(strdup("str"), str_class);
}
