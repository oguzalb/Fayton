#include "bool.h"

object_t *new_bool_from_int(int value) {
    object_t *bool_obj = get_global_no_check(value != 0 ? "True":"False");
    if (bool_obj == NULL) {
        bool_obj = new_object(BOOL_TYPE);
        bool_obj->bool_props = malloc(sizeof(struct bool_type));
        bool_obj->bool_props->ob_bval = value? TRUE: FALSE;
        printd("\'NEW\' BOOL %s\n", bool_obj->bool_props->ob_bval?"True":"False");
    }
    printd("\'RETURNING\' BOOL %s\n", bool_obj->bool_props->ob_bval?"True":"False");
    return bool_obj;
}

object_t *new_bool_internal(object_t *object) {
    if (object->type == INT_TYPE) { 
        return new_bool_from_int(object->int_props->ob_ival);
    } else if (object->type == STR_TYPE) { 
        return new_bool_from_int(object->str_props->ob_sval->len);
    } else if (object->type == BOOL_TYPE){
        printd("\'returning\' BOOL %s\n", object->bool_props->ob_bval?"True":"False");
        return object;
    } else if (object->type == LIST_TYPE) {
        return new_bool_from_int(object->list_props->ob_aval->len);
    } else if (object->type == DICTIONARY_TYPE) {
        return new_bool_from_int(g_hash_table_size(object->dict_props->ob_dval));
    } else {
// TODO implement non_zeros
        set_exception("NOT boolABLE ( : %s\n", object_type_name(object->type));
        interpreter.error = RUN_ERROR;
        return NULL;
    }
}

object_t* new_bool(GArray *args) {
    object_t *object = g_array_index(args, object_t*, 1);
printf("ASHDASD %s\n", object_type_name(object->type));
    return new_bool_internal(object);
}

object_t *bool_repr_func(GArray *args) {
    static object_t *true_repr = NULL;
    static object_t *false_repr = NULL;
    object_t *self = g_array_index(args, object_t *, 0);
    if (self->bool_props->ob_bval == TRUE) {
        if (true_repr != NULL)
            return true_repr;
        char *str = strdup("True");
        true_repr = new_str_internal(str);
        return true_repr;
    } else {
        if (false_repr != NULL)
            return false_repr;
        char *str = strdup("False");
        false_repr = new_str_internal(str);
        return false_repr;
    }
}

void init_bool() {
    object_t *bool_class = new_class(strdup("bool"));
    bool_class->class_props->ob_func = new_bool;
    object_add_field(bool_class, "__eq__", new_func(object_equals, strdup("__eq__")));
    object_add_field(bool_class, "__repr__", new_func(bool_repr_func, strdup("__repr__")));
    register_global(strdup("bool"), bool_class);

    object_t *true_int = new_int_internal(TRUE);
    object_t *true_instance = new_bool_internal(true_int);
    true_instance->class = bool_class;
    register_global(strdup("True"), true_instance);

    object_t *false_int = new_int_internal(FALSE);
    object_t *false_instance = new_bool_internal(false_int);
    false_instance->class = bool_class;
    register_global(strdup("False"), false_instance);
}
