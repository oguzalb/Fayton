#ifndef interpret_python_h
#define interpret_python_h
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <glib.h>
#include <pthread.h>
#include "parse.h"

#define INT_TYPE 1
#define STR_TYPE 2
#define CLASS_TYPE 3
#define FUNC_TYPE 4
#define LIST_TYPE 5
#define LISTITERATOR_TYPE 6
#define USERFUNC_TYPE 7
#define DICTIONARY_TYPE 8
#define CUSTOMOBJECT_TYPE 9
#define THREAD_TYPE 10
#define BOOL_TYPE 11
#define NONE_TYPE 12
#define SLICE_TYPE 13
#define GENERATORFUNC_TYPE 14
#define EXCEPTION_TYPE 15
#define MODULE_TYPE 16

pthread_key_t py_thread_key;

#define RUN_ERROR 1

struct int_type {
    int ob_ival;
};

struct list_type {
    GArray *ob_aval;
};

struct listiterator_type {
    struct _object **objectp;
};

struct dict_type {
    GHashTable *ob_dval;
};

struct str_type {
    GString *ob_sval;
};

struct bool_type {
    int ob_bval;
};

struct thread_type {
    GThread *ob_thread;
};

struct func_type {
    struct _object *(*ob_func)(struct _object **, int);
    int expected_args_count;
    char *name;
};

struct userfunc_type {
    atom_t *ob_userfunc;
    char *name;
    GHashTable *kwargs;
};

struct generatorfunc_type {
    atom_t *ob_generatorfunc;
    GThread *ob_thread;
    struct py_thread *gen_py_thread;
    GCond *cond;
    GMutex *mutex;
    struct _object *channel;
    char *name;
    GHashTable *kwargs;
};

struct class_type {
// TODO CHAIN
    struct _object **inherits;
    struct _object *(*ob_func)(struct _object **, int);
    char *name;
    int expected_args_count;
};

struct slice_type {
    struct _object *start;
    struct _object *stop;
    struct _object *step;
};

struct exception_type {
    struct py_thread *thread;
};

struct module_type {
    char *name;
    atom_tree_t *tree;
};

typedef struct _object {
    int type;
    struct _object *class;
    GHashTable *fields;
    union {
        struct int_type *int_props;
        struct list_type *list_props;
        struct dict_type *dict_props;
        struct str_type *str_props;
        struct bool_type *bool_props;
        struct thread_type *thread_props;
        struct func_type *func_props;
        struct class_type *class_props;
        struct userfunc_type *userfunc_props;
        struct generatorfunc_type *generatorfunc_props;
        struct listiterator_type *listiterator_props;
        struct slice_type *slice_props;
        struct exception_type *exception_props;
        struct module_type *module_props;
    };
} object_t;

typedef struct _kwarg_t {
    object_t *default_val;
} kwarg_t;

struct py_thread {
    GArray *stack_trace;
    object_t *generator_channel;
    object_t *generator;
    object_t *exc;
};

// TODO last accessed should be moved to threads sacrebleu!
struct _interpreter {
    int error;
    object_t *last_accessed;
    GHashTable *base_context;
    GHashTable *globals;
    GArray *threads;
    GHashTable *modules;
} interpreter;

void print_var_each(gpointer, gpointer, gpointer);
object_t *interpret_block(atom_t *, object_t **, int);
object_t *interpret_funcblock(atom_t *, object_t **, int);
void init_interpreter();
object_t *new_func(object_t *(*)(object_t **, int), char *, int);
void register_global(char*, object_t *);
object_t *get_builtin(char*);
object_t *new_class(char*, object_t **, object_t *(*)(object_t **, int), int);
object_t *new_exception(object_t **, int);
int args_len(object_t **args);
void print_var(char*, object_t*);
int evaluate(FILE *stream, atom_tree_t *tree, int is_repl);
int evaluate_main(FILE *stream, atom_tree_t *tree, int is_repl);
#define set_exception(type, fmt, args...) \
    {char *msg; struct py_thread *mt = get_thread(); asprintf(&msg, fmt, ##args); object_t *params[2] = {get_builtin(type), new_str_internal(msg)}; mt->exc = new_exception(params, 2);}

#define get_exception() get_thread()->exc
#define clear_exception() {get_thread()->exc = NULL;}

struct py_thread *get_thread();
struct py_thread *new_thread_struct();

#include "utils.h"
#include "types/object.h"
#include "types/int.h"
#include "types/bool.h"
#include "types/none.h"
#include "types/str.h"
#include "types/list.h"
#include "types/dict.h"
#include "types/thread.h"
#include "types/slice.h"
#include "types/generator.h"

#endif
