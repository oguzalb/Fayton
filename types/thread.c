#include "thread.h"
// VERY EXPERIMENTAL, nothing to see here (:
// just starts a thread and ends, doesn't check anything which is extremely dangerous

struct thread_data_t {
    object_t **args;
    object_t *thread_obj;
};

object_t *thread_runner(struct thread_data_t *thread_data) {
    printd("Creating the thread\n");
    object_call_func_no_param(thread_data->thread_obj, "run");
   // Thread.join
}

object_t *thread_start(object_t **args) {
    object_t *self = args[0];
    struct thread_data_t thread_data;
    thread_data.args = args;
    thread_data.thread_obj = self;
    self->thread_props->ob_thread = g_thread_new("Python thread", thread_runner, &thread_data);
}

object_t *new_thread(object_t **args) {
    object_t *thread = new_object(THREAD_TYPE);
    thread->thread_props = malloc(sizeof(struct thread_type));
    thread->thread_props->ob_thread = NULL;
    return thread;
}

void init_thread () {
    object_t *thread_class = new_class(strdup("Thread"), NULL, new_thread, 1);
    register_global(strdup("Thread"), thread_class);
}
