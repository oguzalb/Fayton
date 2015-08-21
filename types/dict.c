#include "dict.h"

object_t *dict_keys(GArray *args) {
    object_t *self = g_array_index(args, object_t*, 0);
    GList *keys = g_hash_table_get_keys(self->dict_props->ob_dval);
    object_t *keysobj = new_list(NULL);
    GList *iter = keys;
    while (iter != NULL) {
        g_array_append_val(keysobj->list_props->ob_aval, iter->data);
        print_var("appending to keys list", iter->data);
        iter = iter->next;
    }
    g_list_free(keys);
    return keysobj;
}

object_t *dict_values(GArray *args) {
    object_t *self = g_array_index(args, object_t*, 0);
    GList *values = g_hash_table_get_values(self->dict_props->ob_dval);
    object_t *valuesobj = new_list(NULL);
    GList *iter = values;
    while (iter != NULL) {
        g_array_append_val(valuesobj->list_props->ob_aval, iter->data);
        print_var("appending to values list", iter->data);
        iter = iter->next;
    }
    g_list_free(values);
    return valuesobj;
}

object_t *dict_repr(GArray *args) {
    GHashTableIter iter;
    gpointer key, value;

    object_t *self = g_array_index(args, object_t *, 0);
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

object_t *dict_get(GArray *args) {
    if (args->len != 2) {
        interpreter.error = RUN_ERROR;
        set_exception("expected 2 arguments\n");
        return NULL;
    }
    object_t *self = g_array_index(args, object_t *, 0);
    object_t *key = g_array_index(args, object_t *, 1);
    object_t *obj = g_hash_table_lookup(self->dict_props->ob_dval, key);
    return obj != NULL ? obj : new_none_internal();
}

object_t *new_dict(GArray *args) {
    // TODO args check
    object_t * dict = new_object(DICTIONARY_TYPE);
    dict->class = get_global("dict");
    dict->dict_props = malloc(sizeof(struct list_type));
    dict->dict_props->ob_dval = g_hash_table_new(object_hash, object_equal);
    return dict;
}

void init_dict() {
    object_t *dict_class = new_class(strdup("dict"));
    dict_class->class_props->ob_func = new_dict;
    //object_add_field(dict_class, "__iter__", new_func(iter_dict_func));
    object_add_field(dict_class, "keys", new_func(dict_keys, strdup("keys")));
    object_add_field(dict_class, "values", new_func(dict_keys, strdup("values")));
    object_add_field(dict_class, "get", new_func(dict_get, strdup("keys")));
    object_add_field(dict_class, "__repr__", new_func(dict_repr, strdup("__repr__")));
    register_global(strdup("dict"), dict_class);
}
