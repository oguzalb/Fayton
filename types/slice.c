#include "slice.h"

object_t *new_slice(object_t **args, int count) {
    printd("NEW SLICE\n");
    object_t *start = args[1];
    object_t *stop = args[2];
    object_t *step = args[3];
    if ((start->type != INT_TYPE && start->type != NONE_TYPE) || (stop->type != INT_TYPE && stop->type != NONE_TYPE) || (step->type != INT_TYPE && step->type != NONE_TYPE)) {
        set_exception("NOT INT\n");
        return NULL;
    }
    return new_slice_internal(
        start,
        stop,
        step);
}

void set_indices(object_t* slice, int last_index, int* start, int* stop, int* step) {
    object_t *none = new_none_internal();
    if (slice->slice_props->step == none)
        *step = 1;
    else
        *step = slice->slice_props->step->int_props->ob_ival;
    if (slice->slice_props->start == none)
        if (*step < 0)
            *start = last_index;
        else
            *start = 0;
    else {
        if (slice->slice_props->start->int_props->ob_ival < 0)
           *start = last_index + slice->slice_props->start->int_props->ob_ival + 1;
        else
           *start = slice->slice_props->start->int_props->ob_ival;
    }
    if (slice->slice_props->stop == none)
        if (*step < 0)
            *stop = -1;
        else
            *stop = last_index + 1;
    else {
        if (slice->slice_props->stop->int_props->ob_ival < 0)
            *stop = last_index + slice->slice_props->stop->int_props->ob_ival + 1;
        else
            *stop = slice->slice_props->stop->int_props->ob_ival;
    }
    if (*step == 0) {
        set_exception("Step can not be 0\n");
        return;
    }
    if (*start < 0)
        *start = 0;
    else if (*start > last_index)
        *start = last_index;
    if (*stop < -1)
        *stop = -1;
    else if (*stop > last_index)
        *stop = last_index + 1;
}

object_t *new_slice_internal(object_t *start, object_t *stop, object_t *step) {
    object_t *slice_obj = new_object(SLICE_TYPE);
    slice_obj->slice_props = malloc(sizeof(struct slice_type));
    slice_obj->slice_props->start = start;
    slice_obj->slice_props->stop = stop;
    slice_obj->slice_props->step = step;
    slice_obj->class = get_global("slice");
    return slice_obj;
}

void init_slice() {
    object_t *slice_class = new_class(strdup("slice"), NULL, new_slice, 4);
    register_global(strdup("slice"), slice_class);
}
