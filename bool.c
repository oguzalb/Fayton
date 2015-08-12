#include "bool.h"

object_t *new_bool_internal(object_t *object) {
    if (object->type != INT_TYPE) { 
        printd("NOT INT\n");
        interpreter.error = RUN_ERROR;
        return NULL;
    }
    object_t *bool_obj;
    bool_obj = get_global_no_check(object->int_props->ob_ival?"True":"False");
    if (bool_obj == NULL) {
        bool_obj = new_object(BOOL_TYPE);
        bool_obj->bool_props = malloc(sizeof(struct bool_type));
        bool_obj->bool_props->ob_bval = object->int_props->ob_ival? TRUE: FALSE;
        printd("\'NEW\' BOOL %s\n", bool_obj->bool_props->ob_bval?"True":"False");
    }
    printd("\'RETURNING\' BOOL %s\n", bool_obj->bool_props->ob_bval?"True":"False");
    return bool_obj;
}

object_t* new_bool(GArray *args) {
    object_t *object = g_array_index(args, object_t*, 0);
    return new_bool_internal(object);
}

void init_bool() {
    object_t *true_int = new_int_internal(TRUE);
    object_t *true_instance = new_bool_internal(true_int);
    register_global(strdup("True"), true_instance);

    object_t *false_int = new_int_internal(FALSE);
    object_t *false_instance = new_bool_internal(false_int);
    register_global(strdup("False"), false_instance);

    object_t *bool_class = new_class(strdup("bool"));
    bool_class->class_props->ob_func = new_bool;
    object_add_field(bool_class, "__eq__", new_func(object_equals));
    register_global(strdup("bool"), bool_class);
}
