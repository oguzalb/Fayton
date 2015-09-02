#include "list.h"

#define get_garray_begin(glist) \
    ((object_t **) (void *) glist->data)
#define get_garray_end(glist) \
    (((object_t **) (void *) glist->data) + (glist->len-1))
#define g_array_set(glist, item, index) \
    *((((object_t **))(void *) glist->data) + index) = item


object_t *new_listiterator(object_t **args) {
    object_t *list = args[0];
    return new_listiterator_internal(list);
}

object_t *listiterator_next(object_t **args) {
    if (args_len(args) != 1) {
        set_exception("No arguments");
        return NULL;
    }
    object_t *iterator = args[0];
    object_t **objectp = iterator->listiterator_props->objectp;
    if (*objectp != NULL)
        iterator->listiterator_props->objectp++;
    return *objectp;
}

object_t *list_iter(object_t **args) {
    if (args_len(args) != 1) {
        set_exception("No arguments");
        return NULL;
    }
    printd("__iter__ creating list iterator from list\n");
    object_t *list = args[0];
    return new_listiterator_internal(list);
}

object_t *list_append(object_t **args) {
    if (args_len(args) != 2) {
        set_exception("needs two arguments");
        return NULL;
    }
    object_t *list = args[0];
    object_t *item = args[1];
    g_array_append_val(list->list_props->ob_aval, item);
    return new_none_internal();
}

object_t *list_insert(object_t **args) {
    object_t *list = args[0];
    object_t *index = args[1];
    object_t *item = args[2];
    if (args_len(args) != 3) {
        set_exception("needs three arguments");
        return NULL;
    }
    if (index->type != INT_TYPE) {
        set_exception("Second type is not int");
        return NULL;
    }
    int i = index->int_props->ob_ival;
    int len = list->list_props->ob_aval->len;
    if (i > len)
        i = len;
    g_array_insert_val(list->list_props->ob_aval, i, item);
    return new_none_internal();
}

object_t *list_extend(object_t **args) {
    if (args_len(args) != 2) {
        set_exception("needs two arguments");
        return NULL;
    }
    object_t *list = args[0];
    object_t *list2 = args[1];
    GArray *list2_ob_aval = list2->list_props->ob_aval;
    GArray *list_ob_aval = list->list_props->ob_aval;
    if (list2->type != LIST_TYPE) {
        set_exception("Second type is not list");
        return NULL;
    }
    for (int i=0; g_array_index(list2_ob_aval, object_t*, i) != NULL; i++)
        g_array_append_val(list_ob_aval, g_array_index(list2_ob_aval, object_t*, i));
    return new_none_internal();
}

object_t *list_reverse(object_t **args) {
    if (args_len(args) != 1) {
        set_exception("needs one argument");
        return NULL;
    }
    object_t *list = args[0];
    GArray *list_ob_aval = list->list_props->ob_aval;
    object_t **plast; object_t **p;
    for (p=get_garray_begin(list_ob_aval), plast=get_garray_end(list_ob_aval); *p != *plast; p++, plast--) {
        object_t *temp = *p;
        *p = *plast;
        *plast = temp;
    }
    return new_none_internal();
}

object_t *list_add(object_t **args) {
    if (args_len(args) != 2) {
        set_exception("needs two arguments");
        return NULL;
    }
    object_t *list = args[0];
    object_t *list2 = args[1];
    object_t *list3 = new_list_internal();
    assert(list->type == LIST_TYPE);
    if (list2->type != LIST_TYPE) {
        set_exception("Second type is not list");
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

object_t *list_pop(object_t **args) {
    if (args_len(args) != 1) {
        set_exception("needs an argument");
        return NULL;
    }
    object_t *self = args[0];
    GArray *self_ob_aval = self->list_props->ob_aval;
// Throw IndexError
    if (self_ob_aval->len == 0) {
        set_exception("IndexError");
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
    printd("creating list iterator\n");
    object_t *listiterator = new_object(LISTITERATOR_TYPE);
    listiterator->listiterator_props = malloc(sizeof(struct listiterator_type));
    listiterator->listiterator_props->objectp = get_garray_begin(list->list_props->ob_aval);
    listiterator->class = get_global("listiterator");
    assert(listiterator->class != NULL);
    printd("created list iterator\n");
    return listiterator;
}

object_t *list_setitem(object_t **args) {
    if (args_len(args) != 3) {
        set_exception("needs three argument");
        return NULL;
    }
    object_t *self = args[0];
    object_t *index = args[1];
    object_t *item = args[2];
    if (index->type != INT_TYPE) {
        set_exception("Type should be int\n");
        return NULL;
    }
    if (index->int_props->ob_ival >= self->list_props->ob_aval->len) {
        set_exception("index bigger than list size\n");
        return NULL;
    }
    int len = self->list_props->ob_aval->len;
    int i = index->int_props->ob_ival;
    if (i < 0) {
        i = len + i;
        if (i < 0) {
            set_exception("index out of range\n");
            return NULL;
        }
    }
    object_t **place = ((object_t **) (void *)self->list_props->ob_aval->data) + i;
    *place = item;
    return new_none_internal();
}

object_t *list_getitem(object_t **args) {
    if (args_len(args) != 2) {
        set_exception("needs two arguments");
        return NULL;
    }
    object_t *list = args[0];
    object_t *slice = args[1];
    if (slice->type == INT_TYPE) {
        int i = slice->int_props->ob_ival;
        int len = list->list_props->ob_aval->len;
        if (i < 0) {
            i += len;
        }
        if (i < 0 || i >= len) {
            set_exception("index out of range\n");
            return NULL;
        }
        return g_array_index(list->list_props->ob_aval, object_t *, i);
    }
    if (slice->type != SLICE_TYPE) {
        set_exception("Type should be int or slice\n");
        return NULL;
    }
    if (list->list_props->ob_aval->len == 0)
        return new_list_internal();
    printd("Creating slice list\n");
    object_t **glist_begin = get_garray_begin(list->list_props->ob_aval);
    object_t **glist_end = get_garray_end(list->list_props->ob_aval);

    int start, stop, step;
    int last_index = list->list_props->ob_aval->len - 1;
    set_indices(slice, last_index, &start, &stop, &step);
    if (interpreter.error == RUN_ERROR)
        return NULL;
    object_t **glist_stop = glist_begin + stop;
    object_t **glist_start = glist_begin + start;
    object_t *sl_list = new_list_internal();
    int stop_cond = glist_start >= glist_stop;
    if (step > 0)
        for (;glist_start < glist_stop && glist_start >= glist_begin && glist_start <= glist_end; glist_start += step) {
            g_array_append_val(sl_list->list_props->ob_aval, *glist_start);
        }
    else
        for (;glist_start > glist_stop && glist_start >= glist_begin && glist_start <= glist_end; glist_start += step) {
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

object_t *list_repr(object_t **args) {
    if (args_len(args) != 1) {
        set_exception("needs an argument");
        return NULL;
    }
    object_t *self = args[0];
    char *str = NULL;
    char *cursor = NULL;
    cursor = fay_strcat(&str, "[", cursor);
    object_t **obp;
    for (obp=get_garray_begin(self->list_props->ob_aval); *obp != NULL; obp++) {
        object_t *item_str = object_call_repr(*obp);
        if (cursor != str+1)
            cursor = fay_strcat(&str, ", ", cursor);
        cursor = fay_strcat(&str, item_str->str_props->ob_sval->str, cursor);
    }
    cursor = fay_strcat(&str, "]", cursor);
    object_t *repr = new_str_internal(str);
    return repr;
}

object_t *new_list(object_t **args) {
    object_t *list;
    // TODO this should iterate and copy list
    if (args != NULL && args[0] != NULL)
        list = args[0];
    else
        list = new_list_internal();
    return list;
}

void init_list() {
    object_t *listiterator_class = new_class(strdup("listiterator"));
    listiterator_class->class_props->ob_func = new_listiterator;
    object_add_field(listiterator_class, "next", new_func(listiterator_next, strdup("next")));
    register_global(strdup("listiterator"), listiterator_class);
    
    object_t *list_class = new_class(strdup("list"));
    list_class->class_props->ob_func = new_list;
    object_add_field(list_class, "__iter__", new_func(list_iter, strdup("__iter__")));
    object_add_field(list_class, "__getitem__", new_func(list_getitem, strdup("__getitem__")));
    object_add_field(list_class, "__setitem__", new_func(list_setitem, strdup("__setitem__")));
    object_add_field(list_class, "append", new_func(list_append, strdup("append")));
    object_add_field(list_class, "extend", new_func(list_extend, strdup("extend")));
    object_add_field(list_class, "insert", new_func(list_insert, strdup("insert")));
    object_add_field(list_class, "__add__", new_func(list_add, strdup("__add__")));
    object_add_field(list_class, "__repr__", new_func(list_repr, strdup("__repr__")));
    object_add_field(list_class, "pop", new_func(list_pop, strdup("pop")));
    object_add_field(list_class, "reverse", new_func(list_reverse, strdup("reverse")));
    register_global(strdup("list"), list_class);
}
