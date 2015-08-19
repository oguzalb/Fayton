#include "list.h"

#define get_garray_begin(glist) \
    ((object_t **) (void *) glist->data)
#define get_garray_end(glist) \
    (((object_t **) (void *) glist->data) + (glist->len-1))

object_t *new_listiterator(GArray *args) {
    object_t *list = g_array_index(args, object_t*, 0);
    return new_listiterator_internal(list);
}

object_t *listiterator_next_func(GArray *args) {
    // TODO args check
    object_t *iterator = g_array_index(args, object_t*, 0);
    object_t **objectp = iterator->listiterator_props->objectp;
    if (*objectp != NULL)
        iterator->listiterator_props->objectp++;;
    return *objectp;
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
    g_array_append_val(list->list_props->ob_aval, item);
    return new_none_internal();
}

object_t *list_insert(GArray *args) {
    object_t *list = g_array_index(args, object_t*, 0);
    object_t *index = g_array_index(args, object_t*, 1);
    object_t *item = g_array_index(args, object_t*, 2);
    if (index->type != INT_TYPE) {
        set_exception("Second type is not int");
        interpreter.error = RUN_ERROR;
        return NULL;
    }
    int i = index->int_props->ob_ival;
    int len = list->list_props->ob_aval->len;
    if (i > len)
        i = len;
    g_array_insert_val(list->list_props->ob_aval, i, item);
    return new_none_internal();
}

object_t *list_extend(GArray *args) {
    object_t *list = g_array_index(args, object_t*, 0);
    object_t *list2 = g_array_index(args, object_t*, 1);
    GArray *list2_ob_aval = list2->list_props->ob_aval;
    GArray *list_ob_aval = list->list_props->ob_aval;
    if (list2->type != LIST_TYPE) {
        set_exception("Second type is not list");
        interpreter.error = RUN_ERROR;
        return NULL;
    }
    for (int i=0; g_array_index(list2_ob_aval, object_t*, i) != NULL; i++)
        g_array_append_val(list_ob_aval, g_array_index(list2_ob_aval, object_t*, i));
    return new_none_internal();
}

object_t *list_reverse(GArray *args) {
    object_t *list = g_array_index(args, object_t*, 0);
    GArray *list_ob_aval = list->list_props->ob_aval;
    object_t **plast; object_t **p;
    for (p=get_garray_begin(list_ob_aval), plast=get_garray_end(list_ob_aval); *p != *plast; p++, plast--) {
        object_t *temp = *p;
        *p = *plast;
        *plast = temp;
    }
    return new_none_internal();
}

object_t *list_add(GArray *args) {
    object_t *list = g_array_index(args, object_t*, 0);
    object_t *list2 = g_array_index(args, object_t*, 1);
    object_t *list3 = new_list_internal();
    assert(list->type == LIST_TYPE);
    if (list2->type != LIST_TYPE) {
        set_exception("Second type is not list");
        interpreter.error = RUN_ERROR;
        return NULL;
    }
    object_t **list_iter = get_garray_begin(list->list_props->ob_aval);
    object_t **list2_iter = get_garray_begin(list2->list_props->ob_aval);
    GArray *list3_ob_aval = list3->list_props->ob_aval;
    for (; *list_iter != NULL; list_iter++)
        g_array_append_val(list3_ob_aval, *list_iter);
    for (; *list2_iter != NULL; list2_iter++)
        g_array_append_val(list3_ob_aval, *list2_iter);
    return list3;
}

object_t *list_pop(GArray *args) {
    object_t *self = g_array_index(args, object_t*, 0);
    GArray *self_ob_aval = self->list_props->ob_aval;
// Throw IndexError
    if (self_ob_aval->len == 0) {
        set_exception("IndexError");
        interpreter.error = RUN_ERROR;
        return NULL;
    }
    object_t *last = g_array_index(self_ob_aval, object_t*, self_ob_aval->len-1);
    g_array_remove_index(self_ob_aval, self_ob_aval->len-1);
    return last;
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
    listiterator->listiterator_props->objectp = get_garray_begin(list->list_props->ob_aval);
    listiterator->class = get_global("listiterator");
    assert(listiterator->class != NULL);
    printd("created list iterator\n");
    return listiterator;
}

object_t *list_getitem_func(GArray *args) {
    assert(args->len == 2);
    object_t *list = g_array_index(args, object_t *, 0);
    object_t *slice = g_array_index(args, object_t *, 1);
    if (slice->type == INT_TYPE)
        return g_array_index(list->list_props->ob_aval, object_t *, slice->int_props->ob_ival);
    if (slice->type != SLICE_TYPE) {
        set_exception("Type should be int or slice\n");
        interpreter.error = RUN_ERROR;
        return NULL;
    }
    printd("Creating slice list\n");
    object_t *none = new_none_internal();
    object_t **glist_begin = get_garray_begin(list->list_props->ob_aval);
    object_t **glist_end = get_garray_end(list->list_props->ob_aval);
    object_t **glist_stop;
    object_t **glist_start;
    int step;
    int stop;
    if (slice->slice_props->step == none)
        step = 1;
    else
        step = slice->slice_props->step->int_props->ob_ival;
    if (slice->slice_props->start == none)
        if (step < 0)
            glist_start = get_garray_end(list->list_props->ob_aval);
        else
            glist_start = glist_begin;
    else {
        if (slice->slice_props->start->int_props->ob_ival < 0)
           glist_start = glist_end - slice->slice_props->start->int_props->ob_ival + 1;
        else
           glist_start = glist_begin + slice->slice_props->start->int_props->ob_ival;
    }
    if (slice->slice_props->stop == none)
        if (step < 0)
            glist_stop = glist_begin - 1;
        else
            glist_stop = glist_end + 1;
    else {
        if (slice->slice_props->stop->int_props->ob_ival < 0)
            glist_stop = glist_end - slice->slice_props->stop->int_props->ob_ival + 1;
        else
            glist_stop = glist_begin + slice->slice_props->stop->int_props->ob_ival;
    }
    if (step == 0) {
        set_exception("Step can not be 0\n");
        interpreter.error = RUN_ERROR;
        return NULL;
    }
    object_t *sl_list = new_list_internal();
    for (;glist_start < glist_stop && glist_start > glist_begin; glist_start += step) {
        g_array_append_val(sl_list->list_props->ob_aval, *glist_start);
    }
    return sl_list;
}

object_t *new_list_internal() {
    object_t* list = new_object(LIST_TYPE);
    list->list_props = malloc(sizeof(struct list_type));
    list->class = get_global("list");
    list->list_props->ob_aval = g_array_new(TRUE, TRUE, sizeof(object_t *));
    return list;
}

object_t *new_list(GArray *args) {
    // TODO args check
    object_t *list;
    // TODO this should iterate and copy list
    if (args != NULL && args->len == 1)
        list = g_array_index(args, object_t*, 0);
    else
        list = new_list_internal();
    return list;
}

void init_list() {
    object_t *listiterator_class = new_class(strdup("listiterator"));
    listiterator_class->class_props->ob_func = new_listiterator;
    object_add_field(listiterator_class, "next", new_func(listiterator_next_func, strdup("next")));
    register_global(strdup("listiterator"), listiterator_class);
    
    object_t *list_class = new_class(strdup("list"));
    list_class->class_props->ob_func = new_list;
    object_add_field(list_class, "__iter__", new_func(list_iter_func, strdup("__iter__")));
    object_add_field(list_class, "__getitem__", new_func(list_getitem_func, strdup("__getitem__")));
    object_add_field(list_class, "append", new_func(list_append, strdup("append")));
    object_add_field(list_class, "extend", new_func(list_extend, strdup("extend")));
    object_add_field(list_class, "insert", new_func(list_insert, strdup("insert")));
    object_add_field(list_class, "__add__", new_func(list_add, strdup("__add__")));
    object_add_field(list_class, "pop", new_func(list_pop, strdup("pop")));
    object_add_field(list_class, "reverse", new_func(list_reverse, strdup("reverse")));
    register_global(strdup("list"), list_class);
}
