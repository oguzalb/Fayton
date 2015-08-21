#include "int.h"

object_t *int_sub(GArray *args) {
    assert(args->len == 2);
    object_t *left = g_array_index(args, object_t*, 0);
    object_t *right = g_array_index(args, object_t*, 1);
    if (left->type != INT_TYPE || right->type != INT_TYPE) {
        interpreter.error = RUN_ERROR;
        return NULL;
    }
    return new_int_internal(left->int_props->ob_ival - right->int_props->ob_ival);
}

object_t *int_add(GArray *args) {
    assert(args->len == 2);
    object_t *left = g_array_index(args, object_t*, 0);
    object_t *right = g_array_index(args, object_t*, 1);
    if (left->type != INT_TYPE || right->type != INT_TYPE) {
        interpreter.error = RUN_ERROR;
        return NULL;
    }
    return new_int_internal(left->int_props->ob_ival + right->int_props->ob_ival);
}

object_t *int_cmp(GArray *args) {
    assert(args->len == 2);
    object_t *left = g_array_index(args, object_t*, 0);
    object_t *right = g_array_index(args, object_t*, 1);
    if (left->type != INT_TYPE || right->type != INT_TYPE) {
        interpreter.error = RUN_ERROR;
        printd("NOT INT\n");
        return NULL;
    }
    return new_int_internal(left->int_props->ob_ival > right->int_props->ob_ival? 1:left->int_props->ob_ival == right->int_props->ob_ival?0:-1);
}

object_t *int_equals(GArray *args) {
    assert(args->len == 2);
    object_t *left = g_array_index(args, object_t*, 0);
    object_t *right = g_array_index(args, object_t*, 1);
    if (left->type != INT_TYPE || right->type != INT_TYPE) {
        interpreter.error = RUN_ERROR;
        printd("NOT INT\n");
        return NULL;
    }
    return new_bool_from_int(left->int_props->ob_ival == right->int_props->ob_ival);
}

object_t *int_hash(GArray *args) {
    object_t *self = g_array_index(args, object_t *, 0);
    return new_int_internal(g_int_hash(&self->int_props->ob_ival));
}

object_t *int_repr(GArray *args) {
    object_t *self = g_array_index(args, object_t *, 0);
    char *str;
    asprintf(&str, "%d", self->int_props->ob_ival);
    object_t *repr = new_str_internal(str);
    return repr;
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
    object_add_field(int_class, "__add__", new_func(int_add, strdup("__add__")));
    object_add_field(int_class, "__sub__", new_func(int_sub, strdup("__sub__")));
    object_add_field(int_class, "__cmp__", new_func(int_cmp, strdup("__cmp__")));
    object_add_field(int_class, "__eq__", new_func(int_equals, strdup("__eq__")));
    object_add_field(int_class, "__hash__", new_func(int_hash, strdup("__hash__")));
    object_add_field(int_class, "__repr__", new_func(int_repr, strdup("__repr__")));
    register_global(strdup("int"), int_class);
}
