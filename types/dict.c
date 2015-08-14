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
    register_global(strdup("dict"), dict_class);
}
