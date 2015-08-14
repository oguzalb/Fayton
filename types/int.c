#include "int.h"

object_t *int_sub_func(GArray *args) {
    object_t *left = g_array_index(args, object_t*, 0);
    object_t *right = g_array_index(args, object_t*, 1);
    if (left->type != INT_TYPE || right->type != INT_TYPE) {
        interpreter.error = RUN_ERROR;
        return NULL;
    }
    return new_int_internal(left->int_props->ob_ival - right->int_props->ob_ival);
}

object_t *int_add_func(GArray *args) {
    object_t *left = g_array_index(args, object_t*, 0);
    object_t *right = g_array_index(args, object_t*, 1);
    if (left->type != INT_TYPE || right->type != INT_TYPE) {
        interpreter.error = RUN_ERROR;
        return NULL;
    }
    return new_int_internal(left->int_props->ob_ival + right->int_props->ob_ival);
}

object_t *int_cmp_func(GArray *args) {
    object_t *left = g_array_index(args, object_t*, 0);
    object_t *right = g_array_index(args, object_t*, 1);
    if (left->type != INT_TYPE || right->type != INT_TYPE) {
        interpreter.error = RUN_ERROR;
        printd("NOT INT\n");
        return NULL;
    }
    return new_int_internal(left->int_props->ob_ival > right->int_props->ob_ival? 1:left->int_props->ob_ival == right->int_props->ob_ival?0:-1);
}

object_t *new_int(GArray *args) {
    printd("NEW INT\n");
    object_t *int_obj = g_array_index(args, object_t *, 1);
    if (int_obj->type != INT_TYPE) {
        printd("NOT INT\n");
        interpreter.error = RUN_ERROR;
        return NULL;
    }
    return int_obj;
}

object_t *new_int_internal(int value) {
    object_t *int_obj = new_object(INT_TYPE);
    int_obj->int_props = malloc(sizeof(struct int_type));
    int_obj->class = get_global("int");
    int_obj->int_props->ob_ival = value;
    return int_obj;
}

void init_int() {
    object_t *int_class = new_class(strdup("int"));
    int_class->class_props->ob_func = new_int;
    object_add_field(int_class, "__add__", new_func(int_add_func, strdup("__add__")));
    object_add_field(int_class, "__sub__", new_func(int_sub_func, strdup("__sub__")));
    object_add_field(int_class, "__cmp__", new_func(int_cmp_func, strdup("__cmp__")));
    object_add_field(int_class, "__eq__", new_func(object_equals, strdup("__eq__")));
    register_global(strdup("int"), int_class);
}
