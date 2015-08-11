#ifndef interpret_python_h
#define interpret_python_h
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <glib.h>

#define RUN_ERROR 1
struct int_type {
    int ob_ival;
};

struct list_type {
    GArray *ob_aval;
};

struct dict_type {
    GHashTable *ob_dval;
};

struct str_type {
    GString *ob_sval;
};

struct bool_type {
    GString *ob_bval;
};

struct thread_type {
    GThread *ob_thread;
};

struct func_type {
    struct _object *(*ob_func)(struct interpreter_t *, GArray *);
};

struct userfunc_type {
    atom_t *ob_userfunc;
};

struct class_type {
// TODO CHAIN
    struct _object *inherits;
    struct _object *(*ob_func)(struct interpreter_t *, GArray *);
};

struct listiterator_type {
    struct _object *ob_ob;
    int ob_ival;
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
        struct listiterator_type *listiterator_props;
    };
} object_t;

typedef struct _interpreter {
    int error;
    object_t *last_accessed;
    GHashTable *globals;
    
} interpreter_t;

void print_var_each(gpointer, gpointer, gpointer);
object_t *interpret_block(interpreter_t *, atom_t *, GHashTable *, int);
interpreter_t * new_interpreter();
#endif
