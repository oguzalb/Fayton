#include "slice.h"

object_t *new_slice(GArray *args) {
    printd("NEW SLICE\n");
    object_t *start = g_array_index(args, object_t *, 1);
    object_t *stop = g_array_index(args, object_t *, 2);
    object_t *step = g_array_index(args, object_t *, 3);
    if ((start->type != INT_TYPE && start->type != NONE_TYPE) || (stop->type != INT_TYPE && stop->type != NONE_TYPE) || (step->type != INT_TYPE && step->type != NONE_TYPE)) {
        printd("NOT INT\n");
        interpreter.error = RUN_ERROR;
        return NULL;
    }
    return new_slice_internal(
        start,
        stop,
        step);
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
    object_t *slice_class = new_class(strdup("slice"));
    slice_class->class_props->ob_func = new_slice;
    register_global(strdup("slice"), slice_class);
}
