#ifndef interpret_python_h
#define interpret_python_h
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <glib.h>
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
    int ob_bval;
};

struct thread_type {
    GThread *ob_thread;
};

struct func_type {
    struct _object *(*ob_func)(GArray *);
};

struct userfunc_type {
    atom_t *ob_userfunc;
};

struct class_type {
// TODO CHAIN
    struct _object *inherits;
    struct _object *(*ob_func)(GArray *);
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

struct _interpreter {
    int error;
    object_t *last_accessed;
    GHashTable *globals;
    
} interpreter;

void print_var_each(gpointer, gpointer, gpointer);
object_t *interpret_block(atom_t *, GHashTable *, int);
void init_interpreter();
gboolean object_equal(gconstpointer, gconstpointer);
guint object_hash(gconstpointer);
object_t *object_equals(GArray *);
object_t *new_func(object_t *(*)(GArray *));
void object_add_field(object_t *, char*, object_t *);
object_t *object_get_field(object_t *, char*);
void register_global(char*, object_t *);
object_t *get_global(char*);
object_t *get_global_no_check(char*);
object_t *new_class(char*);
object_t *new_object(int);
void print_var(char*, object_t*);

#include "int.h"
#include "bool.h"
#include "str.h"
#include "list.h"
#include "dict.h"
#include "thread.h"
#endif
