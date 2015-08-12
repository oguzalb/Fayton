#include "list.h"

object_t *new_listiterator(GArray *args) {
    // TODO args check
    object_t *list = g_array_index(args, object_t*, 0);
    return new_listiterator_internal(list);
}

object_t *listiterator_next_func(GArray *args) {
    // TODO args check
    object_t *iterator = g_array_index(args, object_t*, 0);
    return g_array_index(iterator->listiterator_props->ob_ob->list_props->ob_aval, object_t*, iterator->listiterator_props->ob_ival++);
}

object_t *list_iter_func(GArray *args) {
    printd("__iter__ creating list iterator from list\n");
    object_t *list = g_array_index(args, object_t*, 0);
    return new_listiterator_internal(list);
}

object_t *list_append(GArray *args) {
    // TODO args check
    object_t *list = g_array_index(args, object_t*, 0);
    object_t *item = g_array_index(args, object_t*, 1);
    if (list->type != LIST_TYPE) {
        interpreter.error = RUN_ERROR;
        return NULL;
    }
    g_array_append_val(list->list_props->ob_aval, item);
    return item;
}

object_t *list_append_internal(object_t *list, object_t *item) {
    g_array_append_val(list->list_props->ob_aval, item);
    return item;
}

object_t *new_listiterator_internal(object_t *list) {
    // TODO args check
    printd("creating list iterator\n");
    object_t *listiterator = new_object(LISTITERATOR_TYPE);
    listiterator->listiterator_props = malloc(sizeof(struct listiterator_type));
    listiterator->listiterator_props->ob_ob = list;
    listiterator->class = get_global("listiterator");
    assert(listiterator->class != NULL);
    listiterator->listiterator_props->ob_ival = 0;
    printd("created list iterator\n");
    return listiterator;
}

object_t *new_list(GArray *args) {
    // TODO args check
    object_t *list;
    if (g_array_get_element_size(args) == 1)
        list = g_array_index(args, object_t*, 0);
    else
        list = new_object(LIST_TYPE);
    list->list_props = malloc(sizeof(struct list_type));
    list->class = get_global("list");
    list->list_props->ob_aval = g_array_new(TRUE, TRUE, sizeof(object_t *));
    return list;
}


void init_list() {
    object_t *listiterator_class = new_class(strdup("listiterator"));
    listiterator_class->class_props->ob_func = new_listiterator;
    object_add_field(listiterator_class, "next", new_func(listiterator_next_func));
    register_global(strdup("listiterator"), listiterator_class);
    
    object_t *list_class = new_class(strdup("list"));
    list_class->class_props->ob_func = new_list;
    object_add_field(list_class, "__iter__", new_func(list_iter_func));
    register_global(strdup("list"), list_class);
}
