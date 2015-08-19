#include "thread.h"
// VERY EXPERIMENTAL, nothing to see here (:
// just starts a thread and ends, doesn't check anything which is extremely dangerous

struct thread_data_t {
    GArray *args;
    object_t *func_obj;
};

object_t *thread_runner(struct thread_data_t *thread_data) {
    printd("Creating the thread\n");
    object_t *run_func = thread_data->func_obj;
    if (run_func->type == USERFUNC_TYPE) {
        GHashTable *sub_context = g_hash_table_new(object_hash, object_equal);
        interpret_funcblock(thread_data->func_obj->userfunc_props->ob_userfunc->child->next, sub_context, /* TODO */ 0);
        //g_hash_table_free(sub_context);
    } else
        run_func->func_props->ob_func(thread_data->args);
   // GArray *args = thread_data->args;
   // object_t *self = g_array_index(args, object_t*, 0);
   // these are for join, will be in the Thread.join
}

object_t *thread_start(GArray *args) {
    object_t *self = g_array_index(args, object_t*, 0);
    object_t *run_func = object_get_field(self, "run");
    struct thread_data_t thread_data;
    thread_data.args = args;
    thread_data.func_obj = run_func;
    self->thread_props->ob_thread = g_thread_new("Python thread", thread_runner, &thread_data);
}

object_t *new_thread(GArray *args) {
    object_t *thread = new_object(THREAD_TYPE);
    thread->thread_props = malloc(sizeof(struct thread_type));
    thread->thread_props->ob_thread = NULL;
    return thread;
}

void init_thread () {
    object_t *thread_class = new_class(strdup("Thread"));
    thread_class->class_props->ob_func = new_thread;
    //object_add_field(dict_class, "__iter__", new_func(iter_dict_func));
    register_global(strdup("Thread"), thread_class);
}
