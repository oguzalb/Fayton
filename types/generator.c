#include "generator.h"

// Working but i don't know why and how hahahahaha
// will need to think on it to have a better design, may have race conditions etc
// it just works with absolute NULL for stopping, should ne StopIterationException when
// exceptions are implemented properly

// TODO exceptions?

struct gen_thread_data_t {
    object_t **args;
    object_t *generator;
};

object_t *generator_runner(struct gen_thread_data_t *thread_data) {
    printd("Creating the generator thread\n");
// TODO free
////
struct py_thread *gen_thread = new_thread_struct();
g_array_append_val(interpreter.threads, gen_thread);
int *p = malloc(sizeof(int));
*p = interpreter.threads->len - 1;
pthread_setspecific(py_thread_key, p);
// TODO when destroy_interpreter or destroy_thread gets implemented
//   int *index = pthread_getspecific(py_thread_key);
//   free(index);
//   pthread_setspecific(py_thread_key, NULL);

////

    gen_thread->generator = thread_data->generator;
    thread_data->generator->generatorfunc_props->gen_py_thread = get_thread();
    printd("Calling the userfunc type generator\n");
    
    printd("signalling %p from thread_runner (gen thread)\n", gen_thread->generator->generatorfunc_props->cond);
    GMutex *mutex = gen_thread->generator->generatorfunc_props->mutex;
    GCond *cond = gen_thread->generator->generatorfunc_props->cond;
    g_mutex_lock(mutex);
    g_cond_signal(gen_thread->generator->generatorfunc_props->cond);
    printd("waiting %p from thread_runner (gen thread)\n", gen_thread->generator->generatorfunc_props->cond);
    g_cond_wait(cond, mutex);
    g_mutex_unlock(mutex);
    interpret_funcblock(thread_data->generator->generatorfunc_props->ob_generatorfunc->child->next, thread_data->args, /* TODO */ 0);
// StopIterationException when exceptions get fully implemented
}

void generator_start(object_t *generator, object_t **args, object_t *run_func) {
    struct gen_thread_data_t *thread_data = malloc(sizeof(struct gen_thread_data_t));
    GMutex *mutex = malloc(sizeof(GMutex));
    g_mutex_init(mutex);
// TODO clear both
    GCond *cond = malloc(sizeof(GCond));
    g_cond_init(cond);
// TODO free?
    generator->generatorfunc_props->cond = cond;
    generator->generatorfunc_props->mutex = mutex;
    thread_data->args = args;
    thread_data->generator = generator;
    generator->generatorfunc_props->ob_thread = g_thread_new("Python thread", generator_runner, thread_data);
    printd("waiting from generator creation (caller thread)\n");
    g_mutex_lock(mutex);
    g_cond_wait(cond, mutex);
    g_mutex_unlock(mutex);
}

object_t *generator_next(object_t **args) {
    printd("Started next\n");
    if (args_len(args) != 1) {
        set_exception("Expected one argument\n");
        return NULL;
    }
    object_t *generator = args[0];
    struct py_thread *thread = get_thread();
    struct GThread *gen_thread = generator->generatorfunc_props->ob_thread;
    GCond *cond = generator->generatorfunc_props->cond;
    GMutex *mutex = generator->generatorfunc_props->mutex;
    printd("signalling %p from next (caller thread)\n", cond);
    g_mutex_lock(mutex);
    g_cond_signal(cond);
    printd("waiting %p from next (caller thread)\n", generator->generatorfunc_props->cond);
    g_cond_wait(cond, mutex);
    g_mutex_unlock(mutex);
    printd("passed next wait (caller thread)\n");
    object_t *result = generator->generatorfunc_props->gen_py_thread->generator_channel;
    if (result == NULL) {
        int *p = pthread_getspecific(py_thread_key);
        thread = g_array_index(interpreter.threads, GThread *, *p);
        free(thread);
        g_array_remove_index(interpreter.threads, *p);
    }
    return result;
}

object_t *generator_iter(object_t **args) {
    object_t *generator = args[0];
    return generator;
}

static object_t *generator_class;
object_t *new_generator_internal(object_t **args, object_t* run_func) {
    object_t *generator = new_object(GENERATORFUNC_TYPE);
    generator->generatorfunc_props = malloc(sizeof(struct generatorfunc_type));
    generator->generatorfunc_props->ob_thread = NULL;
    generator->generatorfunc_props->ob_generatorfunc = run_func;
    generator->class = generator_class;
    generator_start(generator, args, run_func);
    return generator;
}

void init_generator() {
    generator_class = new_class(strdup("generator"));
    object_add_field(generator_class, "next", new_func(generator_next, strdup("next")));
    object_add_field(generator_class, "__iter__", new_func(generator_iter, strdup("__iter__")));
}
