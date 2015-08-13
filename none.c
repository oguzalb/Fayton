#include "none.h"

object_t *new_none_internal() {
    object_t *none_obj = get_global_no_check("None");
    printd("\'RETURNING\' NONE\n");
    return none_obj;
}

void init_none() {
    object_t *none_instance = new_object(NONE_TYPE);
    object_t *none_class = new_class(strdup("NoneType"));
    none_instance->class = none_class;
    register_global(strdup("None"), none_instance);
}
