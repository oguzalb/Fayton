#include "none.h"

object_t *new_none_internal() {
    object_t *none_obj = get_global_no_check("None");
    printd("\'RETURNING\' NONE\n");
    return none_obj;
}

object_t *none_repr_func(object_t **args) {
    if (args_len(args) != 1) {
        set_exception("An argument expected\n");
        return NULL;
    }
    static object_t *none_repr = NULL;
    if (none_repr == NULL) {
        object_t *self = args[0];
        char *str = strdup("None");
        none_repr = new_str_internal(str);
    }
    return none_repr;
}

void init_none() {
    object_t *none_instance = new_object(NONE_TYPE);
    object_t *none_class = new_class(strdup("NoneType"), NULL);
    none_instance->class = none_class;
    object_add_field(none_class, "__repr__", new_func(none_repr_func, strdup("__repr__")));
    register_global(strdup("None"), none_instance);
}
