#include "str.h"

object_t *new_str(object_t **args) {
    printd("NEW STR\n");
    if (args_len(args) != 2) {
        set_exception("Needs two string arguments\n");
        return NULL;
    }
    object_t *str_obj = args[1];
    if (str_obj->type != STR_TYPE) {
        set_exception("Parameter should be str\n");
        return NULL;
    }
    return str_obj;
}

object_t *str_repr(object_t **args) {
    if (args_len(args) != 1) {
        set_exception("Needs string argument\n");
        return NULL;
    }
    object_t *self = args[0];
    char *str;
    asprintf(&str, "\"%s\"", self->str_props->ob_sval->str);
    return new_str_internal(str);
}

object_t *str_cmp(object_t **args) {
    if (args_len(args) != 2) {
        set_exception("Needs two string arguments\n");
        return NULL;
    }
    object_t *self = args[0];
    object_t *other = args[1];
    return new_int_internal(strcmp(self->str_props->ob_sval->str, other->str_props->ob_sval->str));
}
 
object_t *str_hash(object_t **args) {
    if (args_len(args) != 1) {
        set_exception("Needs a string argument\n");
        return NULL;
    }
    object_t *self = args[0];
    return new_int_internal(g_str_hash(self->str_props->ob_sval->str));
}

object_t *str_getitem(object_t **args) {
    // TODO
    if (args_len(args) != 2) {
        set_exception("Needs two arguments\n");
        return NULL;
    }
    object_t *str = args[0];
    object_t *slice = args[1];
    if (slice->type == INT_TYPE) {
        int i = slice->int_props->ob_ival;
        int len = str->str_props->ob_sval->len;
        if (i < 0) {
            i += len;
        }
        if (i < 0 || i >= len) {
            set_exception("index out of range\n");
            return NULL;
        }
        char buff[2];
        buff[0] = str->str_props->ob_sval->str[i];
        buff[1] = '\0';
        return new_str_internal(buff);
    }
    if (str->str_props->ob_sval->len == 0)
        return new_str_internal(NULL);
    if (slice->type != SLICE_TYPE) {
        set_exception("Type should be int or slice\n");
        return NULL;
    }
    printd("Creating slice string\n");
    char* str_begin = str->str_props->ob_sval->str;
    char* str_end = str->str_props->ob_sval->str + str->str_props->ob_sval->len - 1;
    int start, stop, step;
    int last_index = str->str_props->ob_sval->len - 1;
    set_indices(slice, last_index, &start, &stop, &step);
    if (interpreter.error == RUN_ERROR)
        return NULL;
    char* str_stop = str_begin + stop;
    char* str_start = str_begin + start;
    GString *sl_str = g_string_new("");
    int stop_cond = str_start >= str_stop;
    if (step > 0)
        for (;str_start < str_stop && str_start >= str_begin && str_start <= str_end; str_start += step) {
            g_string_append_c(sl_str, *str_start);
        }
    else
        for (;str_start > str_stop && str_start >= str_begin && str_start <= str_end; str_start += step) {
            g_string_append_c(sl_str, *str_start);
        }
    // TODO free all
    return new_str_internal(sl_str->str);
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
    object_t *str_class = new_class(strdup("str"), NULL);
    str_class->class_props->ob_func = new_str;
    object_add_field(str_class, "__repr__", new_func(str_repr, strdup("__repr__")));
    object_add_field(str_class, "__cmp__", new_func(str_cmp, strdup("__cmp__")));
    object_add_field(str_class, "__hash__", new_func(str_hash, strdup("__hash__")));
    object_add_field(str_class, "__getitem__", new_func(str_getitem, strdup("__getitem__")));
    register_global(strdup("str"), str_class);
}
