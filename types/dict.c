#include "dict.h"

object_t *dict_keys(object_t **args, int count) {
    object_t *self = args[0];
    GList *keys = g_hash_table_get_keys(self->dict_props->ob_dval);
    object_t *keysobj = new_list(NULL, 0);
    GList *iter = keys;
    while (iter != NULL) {
        g_array_append_val(keysobj->list_props->ob_aval, iter->data);
        print_var("appending to keys list", iter->data);
        iter = iter->next;
    }
    g_list_free(keys);
    return keysobj;
}

object_t *dict_values(object_t **args, int count) {
    object_t *self = args[0];
    GList *values = g_hash_table_get_values(self->dict_props->ob_dval);
    object_t *valuesobj = new_list(NULL, 0);
    GList *iter = values;
    while (iter != NULL) {
        g_array_append_val(valuesobj->list_props->ob_aval, iter->data);
        print_var("appending to values list", iter->data);
        iter = iter->next;
    }
    g_list_free(values);
    return valuesobj;
}

object_t *dict_repr(object_t **args, int count) {
    GHashTableIter iter;
    gpointer key, value;

    object_t *self = args[0];
    char *str = NULL;
    char *cursor = NULL;
    cursor = fay_strcat(&str, "{", cursor);
    g_hash_table_iter_init (&iter, self->dict_props->ob_dval);
    while (g_hash_table_iter_next (&iter, &key, &value)) {
        if (cursor != str+1)
            cursor = fay_strcat(&str, ", ", cursor);
	object_t *key_str = object_call_repr((object_t *)(void*)key);
        if (interpreter.error == RUN_ERROR)
            return NULL;
        cursor = fay_strcat(&str, key_str->str_props->ob_sval->str, cursor);
        cursor = fay_strcat(&str, ":", cursor);
        object_t *value_str = object_call_repr((object_t*)(void*)value);
        if (interpreter.error == RUN_ERROR)
            return NULL;
        cursor = fay_strcat(&str, value_str->str_props->ob_sval->str, cursor);
    }
    cursor = fay_strcat(&str, "}", cursor);
    return  new_str_internal(str);
}

object_t *dict_get(object_t **args, int count) {
    if (count != 2 && count != 3) {
        set_exception("Two/Three arguments expected\n");
        return NULL;
    }
    object_t *self = args[0];
    object_t *key = args[1];
    object_t *obj = g_hash_table_lookup(self->dict_props->ob_dval, key);
    return obj != NULL ? obj : (args[2]?args[2]:new_none_internal());
}

object_t *dict_getitem(object_t **args, int count) {
    object_t *value = dict_get(args, count);
    if (interpreter.error == RUN_ERROR)
        return NULL;
    if (value == NULL || value->type == NONE_TYPE) {
        set_exception("KeyError");
        return NULL;
    }
    return value;
}

object_t *dict_setitem(object_t **args, int count) {
    object_t *self = args[0];
    object_t *key = args[1];
    object_t *val = args[2];
    g_hash_table_insert(self->dict_props->ob_dval, key, val);
    return new_none_internal();
}

object_t *new_dict(object_t **args, int count) {
    if (count > 1) {
        set_exception("dict from iterable not implemented yet\n");
        return NULL;
    }
    // TODO args check
    object_t * dict = new_object(DICTIONARY_TYPE);
    dict->class = get_global("dict");
    dict->dict_props = malloc(sizeof(struct list_type));
    dict->dict_props->ob_dval = g_hash_table_new(object_hash, object_equal);
    return dict;
}

void init_dict() {
    object_t *dict_class = new_class(strdup("dict"), NULL, new_dict, 1);
    //object_add_field(dict_class, "__iter__", new_func(iter_dict_func));
    object_add_field(dict_class, "keys", new_func(dict_keys, strdup("keys"), 1));
    object_add_field(dict_class, "values", new_func(dict_keys, strdup("values"), 1));
    object_add_field(dict_class, "get", new_func(dict_get, strdup("keys"), 1));
    object_add_field(dict_class, "__getitem__", new_func(dict_getitem, strdup("__getitem__"), 2));
    object_add_field(dict_class, "__setitem__", new_func(dict_setitem, strdup("__setitem__"), 3));
    object_add_field(dict_class, "__repr__", new_func(dict_repr, strdup("__repr__"), 1));
    register_global(strdup("dict"), dict_class);
}
