#include "int.h"

object_t *int_sub(object_t **args, int count) {
    object_t *left = args[0];
    object_t *right = args[1];
    if (left->type != INT_TYPE || right->type != INT_TYPE) {
        set_exception("Both parameters should be integer\n");
        return NULL;
    }
    return new_int_internal(left->int_props->ob_ival - right->int_props->ob_ival);
}

object_t *int_add(object_t **args, int count) {
    if (args_len(args) != 2) {
        set_exception("Needs two integer arguments\n");
    }
    object_t *left = args[0];
    object_t *right = args[1];
    if (left->type != INT_TYPE || right->type != INT_TYPE) {
        set_exception("Both parameters should be integer\n");
        return NULL;
    }
    return new_int_internal(left->int_props->ob_ival + right->int_props->ob_ival);
}

object_t *int_mul(object_t **args, int count) {
    object_t *left = args[0];
    object_t *right = args[1];
    if (left->type != INT_TYPE || right->type != INT_TYPE) {
        set_exception("Both parameters should be integer\n");
        return NULL;
    }
    return new_int_internal(left->int_props->ob_ival * right->int_props->ob_ival);
}

object_t *int_div(object_t **args, int count) {
    object_t *left = args[0];
    object_t *right = args[1];
    if (left->type != INT_TYPE || right->type != INT_TYPE) {
        set_exception("Both parameters should be integer\n");
        return NULL;
    }
    return new_int_internal(left->int_props->ob_ival / right->int_props->ob_ival);
}

object_t *int_mod(object_t **args, int count) {
    object_t *left = args[0];
    object_t *right = args[1];
    if (left->type != INT_TYPE || right->type != INT_TYPE) {
        set_exception("Both parameters should be integer\n");
        return NULL;
    }
    return new_int_internal(left->int_props->ob_ival % right->int_props->ob_ival);
}


object_t *int_cmp(object_t **args, int count) {
    object_t *left = args[0];
    object_t *right = args[1];
    if (left->type != INT_TYPE || right->type != INT_TYPE) {
        set_exception("Both parameters should be integer\n");
        return NULL;
    }
    return new_int_internal(left->int_props->ob_ival > right->int_props->ob_ival? 1:left->int_props->ob_ival == right->int_props->ob_ival?0:-1);
}

object_t *int_rshift(object_t **args, int count) {
    object_t *left = args[0];
    object_t *right = args[1];
    if (left->type != INT_TYPE || right->type != INT_TYPE) {
        set_exception("Both parameters should be integer\n");
        return NULL;
    }
    return new_int_internal(left->int_props->ob_ival >> right->int_props->ob_ival);
}

object_t *int_lshift(object_t **args, int count) {
    object_t *left = args[0];
    object_t *right = args[1];
    if (left->type != INT_TYPE || right->type != INT_TYPE) {
        set_exception("Both parameters should be integer\n");
        return NULL;
    }
    return new_int_internal(left->int_props->ob_ival << right->int_props->ob_ival);
}

object_t *int_equals(object_t **args, int count) {
    object_t *left = args[0];
    object_t *right = args[1];
    if (left->type != INT_TYPE || right->type != INT_TYPE) {
        set_exception("Both parameters should be integer\n");
        return NULL;
    }
    return new_bool_from_int(left->int_props->ob_ival == right->int_props->ob_ival);
}

object_t *int_hash(object_t **args, int count) {
    object_t *self = args[0];
    return new_int_internal(g_int_hash(&self->int_props->ob_ival));
}

object_t *int_repr(object_t **args, int count) {
    object_t *self = args[0];
    char *str;
    asprintf(&str, "%d", self->int_props->ob_ival);
    object_t *repr = new_str_internal(str);
    return repr;
}

object_t *new_int(object_t **args, int count) {
    printd("NEW INT\n");
    object_t *int_obj = args[1];
    if (int_obj->type != INT_TYPE) {
        set_exception("Parameter should be integer\n");
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
    object_t *int_class = new_class(strdup("int"), NULL, new_int, 2);
    object_add_field(int_class, "__add__", new_func(int_add, strdup("__add__"), 2));
    object_add_field(int_class, "__mul__", new_func(int_mul, strdup("__mul__"), 2));
    object_add_field(int_class, "__sub__", new_func(int_sub, strdup("__sub__"), 2));
    object_add_field(int_class, "__div__", new_func(int_div, strdup("__div__"), 2));
    object_add_field(int_class, "__mod__", new_func(int_mod, strdup("__mod__"), 2));
    object_add_field(int_class, "__lshift__", new_func(int_lshift, strdup("__lshift__"), 2));
    object_add_field(int_class, "__rshift__", new_func(int_rshift, strdup("__rshift__"), 2));
    object_add_field(int_class, "__cmp__", new_func(int_cmp, strdup("__cmp__"), 2));
    object_add_field(int_class, "__eq__", new_func(int_equals, strdup("__eq__"), 2));
    object_add_field(int_class, "__hash__", new_func(int_hash, strdup("__hash__"), 1));
    object_add_field(int_class, "__repr__", new_func(int_repr, strdup("__repr__"), 1));
    register_global(strdup("int"), int_class);
}
