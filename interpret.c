#include "parse.h"
#include "interpret.h"

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

char *object_type_name(int type) {
    switch(type) {
        case 1: return "INT";
        case 2: return "STR";
        case 3: return "CLASS";
        case 4: return "FUNC";
        case 5: return "LIST";
        case 6: return "LISTITERATOR";
        case 7: return "USERFUNC";
        case 8: return "DICTIONARY";
        case 9: return "CUSTOMOBJECT";
        case 10: return "THREAD";
        default: return "UNDEFINED";
    }
    assert(FALSE);
}

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

struct interpreter_t;
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


#define FUNC_STRUCT_TYPE(x) object_t *(*x)(interpreter_t *, GArray *)

void print_var(char*, object_t*);
void print_var_each(gpointer name, gpointer obj, gpointer user_data) {
    print_var((char*) name, (object_t*) obj);
}
void print_pair_each(gpointer key, gpointer value, gpointer user_data) {
    print_var("key", (object_t*) key);
    print_var("value", (object_t*) value);
}

void print_var(char* name, object_t* obj) {
    if (obj->type == LIST_TYPE) {
        printd("list %s\n", name);
        assert(obj->class);
    } else if (obj->type == INT_TYPE) {
        printd("int %s %d\n", name, obj->int_props->ob_ival);
    } else if (obj->type == STR_TYPE) {
        printd("str %s\n", obj->str_props->ob_sval->str);
    } else if (obj->type == USERFUNC_TYPE) {
        printd("user func %s\n", name);
    } else if (obj->type == DICTIONARY_TYPE) {
        g_hash_table_foreach(obj->dict_props->ob_dval, print_pair_each, NULL);
    } if (obj->type == FUNC_TYPE) {
        printd("func %s\n", name);
    } else
        printd("obj %s %s\n", name, object_type_name(obj->type));
}

object_t *new_object(int type) {
    object_t *object = (object_t *) malloc(sizeof(object_t));
    object->fields = g_hash_table_new(g_str_hash, g_str_equal);
    object->type = type;
    return object;
}

object_t *new_object_instance(interpreter_t *interpreter, GArray *args) {
    object_t *object = (object_t *) malloc(sizeof(object_t));
    object->fields = g_hash_table_new(g_str_hash, g_str_equal);
    object->class = g_array_index(args, object_t *, 0);
    object->type = CUSTOMOBJECT_TYPE;
    return object;
}

object_t *new_class(interpreter_t *interpreter, char* name) {
    object_t *cls = g_hash_table_lookup(interpreter->globals, name);
    if (cls != NULL) {
        printd("This class already exists\n");
        interpreter->error = RUN_ERROR;
        return NULL;
    }
    printd("Creating the class %s\n", name);
    cls = new_object(CLASS_TYPE);
    cls->class_props = malloc(sizeof(struct class_type));
    cls->class_props->inherits = NULL;
    return cls;
}

void object_add_field(object_t *object, char* name, object_t *field) {
    g_hash_table_insert(object->fields, name, field);
}

object_t *object_get_field(interpreter_t *interpreter, object_t *object, char* name) {
    printd("getting field |%s|\n", name);
    object_t *field = g_hash_table_lookup(object->fields, name);
    if (field != NULL) { 
        return field;
    }
    if (object->class == NULL) { 
        assert(FALSE);
    }
    printd("OBJECT FIELDS\n");
    g_hash_table_foreach(object->fields, print_var_each, NULL);
    field = g_hash_table_lookup(object->class->fields, name);
    if (field != NULL)
        return field;
    else if (object->class->class_props->inherits == NULL) {
        interpreter->error = RUN_ERROR;
        printf("Field not found %s\n", name);
        assert(FALSE);
    }
    printd("CLASS FIELDS\n");
printd("%d is NULL \n", NULL == object->class->class_props->inherits);
printd("%d is NULL \n", object->class->class_props->inherits);
printd("%d is NULL \n", NULL);
    g_hash_table_foreach(object->class->fields, print_var_each, NULL);
    field = g_hash_table_lookup(object->class->class_props->inherits->fields, name);
    if (field == NULL) {
        printd("INHERITS FIELDS\n");
        g_hash_table_foreach(object->class->class_props->inherits->fields, print_var_each, NULL);
        interpreter->error = RUN_ERROR;
        printf("Field not found %s\n", name);
        assert(FALSE);
    }
    printd("got field |%s|\n", name);
    
    return field;
}

void register_global(interpreter_t *interpreter, char* name, object_t *object) {
    g_hash_table_insert(interpreter->globals, name, object);
}

void register_builtin_func(interpreter_t *interpreter, char* name, object_t *object) {
    g_hash_table_insert(interpreter->globals, name, object);
}

// TODO after closures these will change
object_t *get_var(interpreter_t *interpreter, GHashTable *context, char *name) {
    object_t *var = g_hash_table_lookup(context, name);
    if (var != NULL)
        return var;
    var = g_hash_table_lookup(interpreter->globals, name);
    if (var == NULL) {
        interpreter->error = RUN_ERROR;
        printf("Global not found %s\n", name);
        return NULL;
    }
    return var;
}

object_t *get_global(interpreter_t *interpreter, char*name) {
    object_t *cls = g_hash_table_lookup(interpreter->globals, name);
    if (cls == NULL) {
        interpreter->error = RUN_ERROR;
        printf("Global not found %s\n", name);
        return NULL;
    }
    return cls;
}
object_t *get_global_no_check(interpreter_t *interpreter, char*name) {
    return g_hash_table_lookup(interpreter->globals, name);
}

object_t *new_func(object_t *(*func)(interpreter_t *, GArray *)) {
    object_t *func_obj = new_object(FUNC_TYPE);
    func_obj->func_props = malloc(sizeof(struct func_type));
    func_obj->func_props->ob_func = func;
    func_obj->class = NULL;
    return func_obj;
}

object_t *new_user_func(atom_t *func) {
    object_t *func_obj = new_object(USERFUNC_TYPE);
    func_obj->userfunc_props = malloc(sizeof(struct userfunc_type));
    func_obj->userfunc_props->ob_userfunc = func;
    func_obj->class = NULL;
    return func_obj;
}

object_t *new_str(interpreter_t *interpreter, GArray *args) {
    printd("NEW STR\n");
    object_t *str_obj = g_array_index(args, object_t *, 1);
    if (str_obj->type != STR_TYPE) {
        interpreter->error = RUN_ERROR;
        return NULL;
    }
    return str_obj;
}

object_t *new_int(interpreter_t *interpreter, GArray *args) {
    printd("NEW INT\n");
    object_t *int_obj = g_array_index(args, object_t *, 1);
    if (int_obj->type != INT_TYPE) {
        printd("NOT INT\n");
        interpreter->error = RUN_ERROR;
        return NULL;
    }
    return int_obj;
}

object_t *new_int_internal(interpreter_t *interpreter, int value) {
    object_t *int_obj = new_object(INT_TYPE);
    int_obj->int_props = malloc(sizeof(struct int_type));
    int_obj->class = get_global(interpreter, "int");
    int_obj->int_props->ob_ival = value;
    return int_obj;
}

object_t *new_str_internal(interpreter_t *interpreter, char* value) {
    object_t *str_obj = new_object(STR_TYPE);
    str_obj->class = get_global(interpreter, "str");
    str_obj->str_props = malloc(sizeof(struct str_type));
    str_obj->str_props->ob_sval = g_string_new(value);
    printd("NEW STR %s\n", str_obj->str_props->ob_sval->str);
    return str_obj;
}

object_t *new_bool_internal(interpreter_t *interpreter, object_t *object) {
    if (object->type != INT_TYPE) { 
        printd("NOT INT\n");
        interpreter->error = RUN_ERROR;
        return NULL;
    }
    object_t *bool_obj;
    bool_obj = get_global_no_check(interpreter, object->int_props->ob_ival?"True":"False");
    if (bool_obj == NULL) {
        bool_obj = new_object(BOOL_TYPE);
        bool_obj->bool_props = malloc(sizeof(struct bool_type));
        bool_obj->bool_props->ob_bval = object->int_props->ob_ival? TRUE: FALSE;
        printd("\'NEW\' BOOL %s\n", bool_obj->bool_props->ob_bval?"True":"False");
    }
    printd("\'RETURNING\' BOOL %s\n", bool_obj->bool_props->ob_bval?"True":"False");
    return bool_obj;
}

object_t* new_bool(interpreter_t *interpreter, GArray *args) {
    object_t *object = g_array_index(args, object_t*, 0);
    return new_bool_internal(interpreter, object);
}

object_t *add_func(interpreter_t *interpreter, GArray *args) {
    object_t *left = g_array_index(args, object_t*, 0);
    object_t *right = g_array_index(args, object_t*, 1);
    if (left->type != INT_TYPE || right->type != INT_TYPE) {
        interpreter->error = RUN_ERROR;
        return NULL;
    }
    return new_int_internal(interpreter, left->int_props->ob_ival + right->int_props->ob_ival);
}

object_t *sub_func(interpreter_t *interpreter, GArray *args) {
    object_t *left = g_array_index(args, object_t*, 0);
    object_t *right = g_array_index(args, object_t*, 1);
    if (left->type != INT_TYPE || right->type != INT_TYPE) {
        interpreter->error = RUN_ERROR;
        return NULL;
    }
    return new_int_internal(interpreter, left->int_props->ob_ival - right->int_props->ob_ival);
}

object_t *object_equals(interpreter_t *interpreter, GArray *args) {
    object_t *left = g_array_index(args, object_t*, 0);
    object_t *right = g_array_index(args, object_t*, 1);
    object_t *cmp_func = object_get_field(interpreter, left, "__cmp__");
    if (interpreter->error == RUN_ERROR)
        return NULL;
    GArray *sub_args = g_array_new(TRUE, TRUE, sizeof(object_t *));
// TODO CUSTOM?
    g_array_append_val(sub_args, left);
    g_array_append_val(sub_args, right);
    printd("calling __cmp__\n");
    object_t *int_result = cmp_func->func_props->ob_func(interpreter, sub_args);
    if (int_result == NULL) {
        interpreter->error = RUN_ERROR;
        return NULL;
    }
    object_t *cmp_result_equals_zero = new_int_internal(interpreter, int_result->int_props->ob_ival == 0);
    return new_bool_internal(interpreter, cmp_result_equals_zero);
}

object_t *cmp_func(interpreter_t *interpreter, GArray *args) {
    object_t *left = g_array_index(args, object_t*, 0);
    object_t *right = g_array_index(args, object_t*, 1);
    if (left->type != INT_TYPE || right->type != INT_TYPE) {
        interpreter->error = RUN_ERROR;
        printd("NOT INT\n");
        return NULL;
    }
    return new_int_internal(interpreter, left->int_props->ob_ival > right->int_props->ob_ival? 1:left->int_props->ob_ival == right->int_props->ob_ival?0:-1);
}

object_t *new_list(interpreter_t *interpreter, GArray *args) {
    // TODO args check
    object_t *list;
    if (g_array_get_element_size(args) == 1)
        list = g_array_index(args, object_t*, 0);
    else
        list = new_object(LIST_TYPE);
    list->list_props = malloc(sizeof(struct list_type));
    list->class = get_global(interpreter, "list");
    list->list_props->ob_aval = g_array_new(TRUE, TRUE, sizeof(object_t *));
    return list;
}

object_t *thread_new(interpreter_t *interpreter, GArray *args) {
    object_t *thread = new_object(THREAD_TYPE);
    thread->thread_props = malloc(sizeof(struct thread_type));
    thread->thread_props->ob_thread = NULL;
    return thread;
}

struct thread_data_t {
    interpreter_t *interpreter;
    GArray *args;
    object_t *func_obj;
};

guint object_hash(gconstpointer key) {
    object_t *object = (object_t *)key;
    return g_int_hash(&object->int_props->ob_ival);
}

gboolean object_equal(gconstpointer a, gconstpointer b) {
    object_t *aobj = (object_t *)a;
    object_t *bobj = (object_t *)b;
    return g_int_equal(&aobj->int_props->ob_ival, &bobj->int_props->ob_ival);
}

object_t *interpret_block(interpreter_t *, atom_t *, GHashTable *, int);
object_t *thread_runner(struct thread_data_t *thread_data) {
    printd("Creating the thread\n");
    object_t *run_func = thread_data->func_obj;
    interpreter_t *interpreter = thread_data->interpreter;
    if (run_func->type == USERFUNC_TYPE) {
        GHashTable *sub_context = g_hash_table_new(object_hash, object_equal);
        interpret_block(interpreter, thread_data->func_obj->userfunc_props->ob_userfunc->child->next, sub_context, /* TODO */ 0);
        //g_hash_table_free(sub_context);
    } else
        run_func->func_props->ob_func(interpreter, thread_data->args);
   // GArray *args = thread_data->args;
   // object_t *self = g_array_index(args, object_t*, 0);
   // these are for join, will be in the Thread.join
}

object_t *thread_start(interpreter_t *interpreter, GArray *args) {
    object_t *self = g_array_index(args, object_t*, 0);
    object_t *run_func = object_get_field(interpreter, self, "run");
    struct thread_data_t thread_data;
    thread_data.interpreter = interpreter;
    thread_data.args = args;
    thread_data.func_obj = run_func;
    self->thread_props->ob_thread = g_thread_new("Python thread", thread_runner, &thread_data);
}

object_t *dict_keys(interpreter_t *interpreter, GArray *args) {
    object_t *self = g_array_index(args, object_t*, 0);
    GList *keys = g_hash_table_get_keys(self->dict_props->ob_dval);
    object_t *keysobj = new_list(interpreter, NULL);
    GList *iter = keys;
    while (iter != NULL) {
        g_array_append_val(keysobj->list_props->ob_aval, iter->data);
        print_var("appending to keys list", iter->data);
        iter = iter->next;
    }
    g_list_free(keys);
    return keysobj;
}

object_t *new_dict(interpreter_t *interpreter, GArray *args) {
    // TODO args check
    object_t * dict = new_object(DICTIONARY_TYPE);
    dict->class = get_global(interpreter, "dict");
    dict->dict_props = malloc(sizeof(struct list_type));
    dict->dict_props->ob_dval = g_hash_table_new(object_hash, object_equal);
    return dict;
}

object_t *new_listiterator_internal(interpreter_t *interpreter, object_t *list) {
    // TODO args check
    printd("creating list iterator\n");
    object_t *listiterator = new_object(LISTITERATOR_TYPE);
    listiterator->listiterator_props = malloc(sizeof(struct listiterator_type));
    listiterator->listiterator_props->ob_ob = list;
    listiterator->class = get_global(interpreter, "listiterator");
    assert(listiterator->class != NULL);
    listiterator->listiterator_props->ob_ival = 0;
    printd("created list iterator\n");
    return listiterator;
}

object_t *new_listiterator(interpreter_t *interpreter, GArray *args) {
    // TODO args check
    object_t *list = g_array_index(args, object_t*, 0);
    return new_listiterator_internal(interpreter, list);
}

object_t *listiterator_next_func(interpreter_t *interpreter, GArray *args) {
    // TODO args check
    object_t *iterator = g_array_index(args, object_t*, 0);
    return g_array_index(iterator->listiterator_props->ob_ob->list_props->ob_aval, object_t*, iterator->listiterator_props->ob_ival++);
}

object_t *iter_list_func(interpreter_t *interpreter, GArray *args) {
    printd("__iter__ creating list iterator from list\n");
    object_t *list = g_array_index(args, object_t*, 0);
    return new_listiterator_internal(interpreter, list);
}

object_t *list_append(interpreter_t *interpreter, GArray *args) {
    // TODO args check
    object_t *list = g_array_index(args, object_t*, 0);
    object_t *item = g_array_index(args, object_t*, 1);
    if (list->type != LIST_TYPE) {
        interpreter->error = RUN_ERROR;
        return NULL;
    }
    g_array_append_val(list->list_props->ob_aval, item);
    return item;
}

object_t *list_append_internal(object_t *list, object_t *item) {
    g_array_append_val(list->list_props->ob_aval, item);
    return item;
}

object_t *sum_func(interpreter_t *interpreter, GArray *args) {
    object_t *iterable = g_array_index(args, object_t*, 0);
    object_t *iter_func = object_get_field(interpreter, iterable, "__iter__");
    object_t *iterator;
    if (iter_func->type == USERFUNC_TYPE) {
        GHashTable *sub_context = g_hash_table_new(g_str_hash, g_str_equal);
        atom_t *param_name = iter_func->userfunc_props->ob_userfunc->child->child;
        g_hash_table_insert(sub_context, param_name->value, iterable);
        iterator = interpret_block(interpreter, iter_func->userfunc_props->ob_userfunc->child->next, sub_context, /* TODO */0);
    } else {
        GArray *args = g_array_new(TRUE, TRUE, sizeof(object_t *));
        g_array_append_val(args, iterable);
        iterator = iter_func->func_props->ob_func(interpreter, args);
    }
    object_t *next_func = object_get_field(interpreter, iterator, "next");
    object_t *int_obj = NULL;
    int sum = 0;
    do {
        if (next_func->type == USERFUNC_TYPE) {
            GHashTable *sub_context = g_hash_table_new(g_str_hash, g_str_equal);
            atom_t *param_name = next_func->userfunc_props->ob_userfunc->child->child;
            g_hash_table_insert(sub_context, param_name->value, iterator);
            int_obj = interpret_block(interpreter, next_func->userfunc_props->ob_userfunc->child->next, sub_context, /* TODO */ 0);
        } else {
            GArray *args = g_array_new(TRUE, TRUE, sizeof(object_t *));
            g_array_append_val(args, iterator);
            int_obj = next_func->func_props->ob_func(interpreter, args);
        }
        assert(int_obj != NULL?int_obj->type == INT_TYPE:TRUE);
        // TODO __add__
        if (int_obj != NULL)
            sum += int_obj->int_props->ob_ival;
    } while (int_obj != NULL);

    object_t *result = new_int_internal(interpreter, sum);
    // TODO CHECKS
    return result;
}

object_t *range_func(interpreter_t *interpreter, GArray *args) {
    object_t *min = g_array_index(args, object_t*, 0);
    object_t *max = g_array_index(args, object_t*, 1);
    if (min->type != INT_TYPE || min->type != INT_TYPE) {
        interpreter->error = RUN_ERROR;
        return NULL;
    }
    object_t *list = new_list(interpreter, NULL);
    int i;
    for (i=min->int_props->ob_ival; i<max->int_props->ob_ival; i++) {
        list_append_internal(list, new_int_internal(interpreter, i));
    }
    return list;
}

object_t *print_func(interpreter_t *interpreter, GArray *args) {
    // TODO CHECKS AND FULL PRINT
    object_t *var = g_array_index(args, object_t*, 0);
    if (var->type == INT_TYPE) {
        printd("PRINT INT\n");
        printf("%d\n", var->int_props->ob_ival);
    } else if (var->type == STR_TYPE) {
        printf("%s\n", var->str_props->ob_sval->str);
    } else if (var->type == BOOL_TYPE) {
        printf("%s\n", var->bool_props->ob_bval ? "True":"False");
    } else {
        print_var("Print got %d!?!", var->type);
        assert(FALSE);
    }
}

interpreter_t * new_interpreter() {
    interpreter_t *interpreter = malloc(sizeof(interpreter_t));
    interpreter->error = 0;
    interpreter->last_accessed = NULL;
    interpreter->globals = g_hash_table_new(g_str_hash, g_str_equal);

    object_t *object_class = new_class(interpreter, strdup("object"));
    object_class->class_props->ob_func = new_object_instance;
    register_global(interpreter, strdup("object"), object_class);

    object_t *int_class = new_class(interpreter, strdup("int"));
    int_class->class_props->ob_func = new_int;
    object_add_field(int_class, "__add__", new_func(add_func));
    object_add_field(int_class, "__sub__", new_func(sub_func));
    object_add_field(int_class, "__cmp__", new_func(cmp_func));
    object_add_field(int_class, "__eq__", new_func(object_equals));
    register_global(interpreter, strdup("int"), int_class);
 
    object_t *str_class = new_class(interpreter, strdup("str"));
    str_class->class_props->ob_func = new_str;
    register_global(interpreter, strdup("str"), str_class);

    object_t *true_int = new_int_internal(interpreter, TRUE);
    object_t *true_instance = new_bool_internal(interpreter, true_int);
    true_instance->class_props->ob_func = new_bool;
    register_global(interpreter, strdup("True"), true_instance);

    object_t *false_int = new_int_internal(interpreter, FALSE);
    object_t *false_instance = new_bool_internal(interpreter, false_int);
    false_instance->class_props->ob_func = new_bool;
    register_global(interpreter, strdup("False"), false_instance);

    object_t *bool_class = new_class(interpreter, strdup("bool"));
    bool_class->class_props->ob_func = new_bool;
    object_add_field(bool_class, "__eq__", new_func(object_equals));
    register_global(interpreter, strdup("bool"), bool_class);
   
    register_global(interpreter, strdup("range"), new_func(range_func));
    register_global(interpreter, strdup("sum"), new_func(sum_func));
    
    object_t *listiterator_class = new_class(interpreter, strdup("listiterator"));
    listiterator_class->class_props->ob_func = new_listiterator;
    object_add_field(listiterator_class, "next", new_func(listiterator_next_func));
    register_global(interpreter, strdup("listiterator"), listiterator_class);
    
    object_t *list_class = new_class(interpreter, strdup("list"));
    list_class->class_props->ob_func = new_list;
    object_add_field(list_class, "__iter__", new_func(iter_list_func));
    register_global(interpreter, strdup("list"), list_class);
    
    object_t *dict_class = new_class(interpreter, strdup("dict"));
    dict_class->class_props->ob_func = new_dict;
    //object_add_field(dict_class, "__iter__", new_func(iter_dict_func));
    object_add_field(dict_class, "keys", new_func(dict_keys));
    register_global(interpreter, strdup("dict"), dict_class);

    object_t *thread_class = new_class(interpreter, strdup("Thread"));
    thread_class->class_props->ob_func = thread_new;
    //object_add_field(dict_class, "__iter__", new_func(iter_dict_func));
    register_global(interpreter, strdup("Thread"), thread_class);
    
    register_global(interpreter, strdup("print"), new_func(print_func));
    
    return interpreter;
}

object_t *lookup_var(interpreter_t *interpreter, GHashTable *context, char* name) {
    printd("looking up for var %s\n", name);
    object_t *object = g_hash_table_lookup(context, name);
    if (object == NULL)
        object = g_hash_table_lookup(interpreter->globals, name);
    if (object == NULL) {
        printd("globals\n");
        g_hash_table_foreach(interpreter->globals, print_var_each, NULL);
        printd("context\n");
        g_hash_table_foreach(interpreter->globals, print_var_each, NULL);
    }
    assert(object != NULL);
    return object;
}

object_t *interpret_expr(interpreter_t *, atom_t *, GHashTable *, int);
object_t *interpret_funccall(interpreter_t *interpreter, atom_t *func_call, GHashTable *context, int current_indent) {
    object_t *func = interpret_expr(interpreter, func_call->child, context, current_indent);
    if (interpreter->last_accessed)
        print_var("last", interpreter->last_accessed);
    if (func == NULL) {
        printf("FUNC NOT FOUND |%s|\n", func_call->value);
        interpreter->error = RUN_ERROR;
        return NULL;
    }
    if (func->type != FUNC_TYPE && func->type != USERFUNC_TYPE && func->type != CLASS_TYPE) {
        printf("OBJ IS NOT CALLABLE %s\n", object_type_name(func->type));
        interpreter->error = RUN_ERROR;
        return NULL;
    }
    printd("FUNC FOUND |%s|\n", func_call->value);
    
    GArray *args;
    GHashTable *sub_context;
    // TODO should be done in a better way, just poc
    sub_context = g_hash_table_new(g_str_hash, g_str_equal);
    if (func->type == FUNC_TYPE || func->type == CLASS_TYPE) {
        args = g_array_new(TRUE, TRUE, sizeof(object_t *));
        atom_t *param = func_call->child->next->child;
        if (func->type == CLASS_TYPE) {
            print_var("adding param class", func);
            g_array_append_val(args, func);
        } else if (func_call->child->type == A_ACCESSOR && interpreter->last_accessed != NULL) {
            print_var("adding param", interpreter->last_accessed);
            g_array_append_val(args, interpreter->last_accessed);
        }
        while (param) {
            if (param->type == A_VAR) {
                object_t *value = lookup_var(interpreter, context, param->value);
                if (value == NULL) {
                    printf("var not found %s\n", param->value);
                    interpreter->error = RUN_ERROR;
                    return NULL;
                }
                printd("ADDING ARGUMENT %s\n", param->value);
                print_var("", value);
                g_array_append_val(args, value);
            } else if (param->type == A_INTEGER) {
                object_t * int_val = new_int_internal(interpreter, atoi(param->value));
                printd("ADDING ARGUMENT %s\n", param->value);
                g_array_append_val(args, int_val);
                printd("ADDED ARGUMENT\n");
            } else if (param->type == A_FUNCCALL) {
                object_t *param_object = interpret_funccall(interpreter, param, context, current_indent);
                if (interpreter->error == RUN_ERROR)
                    return NULL;
                if (param_object == NULL) {
                    printf("Param expr returned NULL!?!\n");
                    interpreter->error = RUN_ERROR;
                    return NULL;
                }
                g_array_append_val(args, param_object);
            } else {
                interpreter->error = RUN_ERROR;
                printf("TYPE NOT PARAM %s\n", atom_type_name(param->type));
                return NULL;
            }
            param = param->next;
        }
    } else if (func->type == USERFUNC_TYPE) {
        atom_t *param = func_call->child->next->child;
        atom_t *param_name = func->userfunc_props->ob_userfunc->child->child;
        if (func_call->child->type == A_ACCESSOR && interpreter->last_accessed != NULL) {
            print_var("ADDING SELF PARAM", interpreter->last_accessed);
            g_hash_table_insert(sub_context, param_name->value, interpreter->last_accessed);
            param_name = param_name->next;
        }
        while (param_name) {
            if (param == NULL) {
                interpreter->error = RUN_ERROR;
                printd("Lesser parameter passed than needed, next: %s\n", param_name->value);
                return NULL;}
            if (param_name == NULL) {
                interpreter->error = RUN_ERROR;
                printd("More parameter passed than needed next: %s\n", param->value);
                return NULL;}
            if (param->type == A_VAR) {
                printd("ADDING ARGUMENT %s\n", param->value);
                g_hash_table_insert(sub_context, param_name->value, param->value);
            } else if (param->type == A_INTEGER) {
                object_t * int_val = new_int_internal(interpreter, atoi(param->value));
                printd("ADDING ARGUMENT %s\n", param->value);
                g_hash_table_insert(sub_context, param_name->value, int_val);
                printd("ADDED ARGUMENT\n");
            } else if (param->type == A_FUNCCALL) {
                object_t *param_object = interpret_funccall(interpreter, param, context, current_indent);
                if (interpreter->error == RUN_ERROR)
                    return NULL;
                if (param_object == NULL) {
                    printf("Param expr returned NULL!?!\n");
                    interpreter->error = RUN_ERROR;
                    return NULL;
                }
                g_hash_table_insert(sub_context, param_name->value, param_object);
            } else {
                interpreter->error = RUN_ERROR;
                printf("TYPE NOT PARAM %s\n", atom_type_name(param->type));
                return NULL;
            }
            param = param->next;
            param_name = param_name->next;
            // TODO check if they match
        }
    }
    printd("ADDED PARAMS\n");
    printd("calling func type %s\n", object_type_name(func->type));
    if (func->type == USERFUNC_TYPE)
        return interpret_block(interpreter, func->userfunc_props->ob_userfunc->child->next, sub_context, current_indent);
    else if (func->type == FUNC_TYPE)
        return func->func_props->ob_func(interpreter, args);
    else if (func->type == CLASS_TYPE)
        return func->class_props->ob_func(interpreter, args);
}

object_t *interpret_list(interpreter_t *interpreter, atom_t *expr, GHashTable *context, int current_indent) {
    object_t *list = new_list(interpreter, NULL);
    atom_t *elem = expr->child;
    while (elem != NULL) {
        object_t *result = interpret_expr(interpreter, elem, context, current_indent);
        if (interpreter->error) {
            return list;
        }
        list_append_internal(list, result);
        elem = elem->next;
    }
    return list;
}

object_t *interpret_dict(interpreter_t *interpreter, atom_t *expr, GHashTable *context, int current_indent) {
    object_t *dict = new_dict(interpreter, NULL);
    atom_t *elem = expr->child;
    while (elem != NULL) {
        object_t *key = interpret_expr(interpreter, elem, context, current_indent);
        if (interpreter->error) {
            return dict;
        }
        object_t *value = interpret_expr(interpreter, elem->next, context, current_indent);
        if (interpreter->error) {
            return dict;
        }
        g_hash_table_insert(dict->dict_props->ob_dval, key, value);
        elem = elem->next->next;
    }
    return dict;
}

object_t *interpret_if(interpreter_t *interpreter, atom_t *expr, GHashTable *context, int current_indent) {
    atom_t *if_block = expr->child;
    while (if_block) {
        if (if_block->type == A_FUNCCALL) {
            object_t *bool_obj = interpret_expr(interpreter, if_block, context, current_indent);
            if (interpreter->error == RUN_ERROR)
                return NULL;
            if (bool_obj->type != BOOL_TYPE) {
                printd("NOT A BOOL TYPE\n");
                interpreter->error = RUN_ERROR;
                return NULL;
            }
            if_block = if_block->next;
            assert(if_block != NULL);
            if (bool_obj->bool_props->ob_bval == TRUE) {
printd("HEREEEE TRUE!!!\n");
                return interpret_block(interpreter, if_block, context, current_indent);
            }
        } else if (if_block->type == A_BLOCK) {
printd("HEREEEE FALSE!!!\n");
            return interpret_block(interpreter, if_block, context, current_indent);
        }
        if_block = if_block->next;
    }
    return NULL;
}

object_t *interpret_expr(interpreter_t *interpreter, atom_t *expr, GHashTable *context, int current_indent) {
// TODO
    printd("FIRST_TYPE %s\n", atom_type_name(expr->type));
    if (expr->type == A_VAR) {
        printd("VAR %s\n", expr->value);
        object_t *value = get_var(interpreter, context, expr->value);
        if (value)
            print_var(expr->value, value);
        return value;
    } else if (expr->type == A_FUNCCALL) {
        printd("CALL FUNC \n");
        return interpret_funccall(interpreter, expr, context, current_indent);
    } else if (expr->type == A_INTEGER) {
        object_t *int_val = new_int_internal(interpreter, atoi(expr->value));
        printd("NEW INT %d\n", int_val->int_props->ob_ival);
        return int_val;
    } else if (expr->type == A_STRING) {
        object_t *str_val = new_str_internal(interpreter, expr->value);
        return str_val;
    } else if (expr->type == A_LIST) {
        printd("NEW LIST %s\n", expr->value);
        object_t *list = interpret_list(interpreter, expr, context, current_indent);
        return list;
    } else if (expr->type == A_DICTIONARY) {
        printd("NEW DICTIONARY %s\n", expr->value);
        object_t *dict = interpret_dict(interpreter, expr, context, current_indent);
        printd("%x %ddict address\n", dict, dict->type);
        return dict;
    } else if (expr->type == A_ACCESSOR) {
        printd("ACCESSING %s\n", expr->child->value);
        object_t *object;
        g_hash_table_foreach(context, print_pair_each, NULL);
        if (expr->child->type == A_VAR) {
            printd("%s\n", expr->child->value);
            object = lookup_var(interpreter, context, expr->child->value);
            if (object == NULL) {
                interpreter->error = RUN_ERROR;
                printd("object %s could not be found in context or globals\n", expr->child->value);
                return NULL;
            }
            print_var("object", object);
            printd("%x %ddict address\n", object, object->type);
            printd("getting field %s of %s\n", expr->child->next->value, expr->child->value);
        } else
            object = interpret_expr(interpreter, expr->child, context, current_indent);
        if (object == NULL)
            return NULL;
        interpreter->last_accessed = object;
        printd("last accessed %x\n", object);
        object_t *field = object_get_field(interpreter, object, expr->child->next->value);
        if (field == NULL) {
            interpreter->error = RUN_ERROR;
            printd("field not found %s\n", expr->child->next->value);
            return NULL;
        }
        return field;
    } else {
        interpreter->error = RUN_ERROR;
        printf("TYPE INCORRECT %s\n", atom_type_name(expr->type));
        return NULL;
    }
}

object_t *interpret_stmt(interpreter_t *interpreter, atom_t *stmt, GHashTable *context, int current_indent) {
    if (stmt->type == A_FUNCCALL) {
        interpret_expr(interpreter, stmt, context, current_indent);
    } else if (stmt->type == A_ASSIGNMENT) {
        atom_t *left_var = stmt->child;
        printd("ASSIGNING TO %s\n", left_var->value);
        object_t *result = interpret_expr(interpreter, left_var->next, context, current_indent);
        if (interpreter->error == RUN_ERROR)
            return NULL;
        if (result == NULL) {
            printd("GOT NOTHING, can't assign\n");
            interpreter->error = RUN_ERROR;
            return NULL;
        }

        printd("ASSIGNMENT RAN\n");
        if (result) {
            print_var("result", result);
            g_hash_table_insert(context, strdup(left_var->value), result);
        }
        return NULL;
    } else if (stmt->type == A_FOR) {
        atom_t *var_name = stmt->child;
        //object_t *var = get_global(interpreter, var_name->value);
        //if (var == NULL)
        //    return;
        atom_t *expr = var_name->next;
        object_t *iterable = interpret_expr(interpreter, expr, context, current_indent);
        if (interpreter->error == RUN_ERROR)
            return NULL;
        object_t *iter_func = object_get_field(interpreter, iterable, "__iter__");
        if (interpreter->error == RUN_ERROR)
            return NULL;
        GArray *args = g_array_new(TRUE, TRUE, sizeof(object_t *));
        g_array_append_val(args, iterable);
        printd("calling __iter__\n");
        object_t *iterator = iter_func->func_props->ob_func(interpreter, args);
        assert(iterator->class != NULL);
        printd("calling __iter__ END\n");
        atom_t *block = expr->next;
        object_t *next_func = object_get_field(interpreter, iterator, "next");
        if (interpreter->error == RUN_ERROR)
            return NULL;
        g_array_remove_index(args, 0);
        g_array_append_val(args, iterator);
        object_t *item;
        while(item = next_func->func_props->ob_func(interpreter, args)) {
            register_global(interpreter, var_name->value, item);
            interpret_block(interpreter, block, context, current_indent);
            if (interpreter->error == RUN_ERROR) {
                printf("ERROR OCCURED WHILE FOR STMTS\n");
                return NULL;
            }
        }
    } else if (stmt->type == A_IF) { 
printd("A_IF\n");
        return interpret_if(interpreter, stmt, context, current_indent);
    } else if (stmt->type == A_FUNCDEF) {
        object_t *userfunc = new_user_func(stmt);
        register_global(interpreter, stmt->value, userfunc);
    } else if (stmt->type == A_CLASS) {
        atom_t *class_name = stmt;
        object_t *class = new_class(interpreter, strdup(class_name->value));
        class->class_props->ob_func = new_object_instance;
        atom_t *inherits = stmt->child->child;
        atom_t *field = stmt->child->next;
        while (field) {
            if (field->type == A_FUNCDEF) {
                object_t *class_func = new_user_func(field);
                object_add_field(class, field->value, class_func);
                printd("added class field func %s.%s\n", class_name->value, field->value);
            } else if (field->type == A_VAR) {
                object_t *result = interpret_expr(interpreter, field->child, context, current_indent);
                object_add_field(class, field->value, result);
                printd("added class field %s.%s\n", class_name->value, field->value);
            } else {
                assert(FALSE);
            }
            field = field->next;
        }
        register_global(interpreter, strdup(class_name->value), class);
        // TODO inherits CHAIN
        if (inherits != NULL) {
            object_t *parent_class = get_global(interpreter, inherits->value);
            class->class_props->inherits = parent_class;
        } else
            class->class_props->inherits = NULL;
    } else if (stmt->type == A_RETURN) {
        printd("returning something\n");
        if (stmt->child)
            return interpret_expr(interpreter, stmt->child, context, current_indent);
        else
            // TODO will implement NoneType, this won't work for now
            return NULL;
    }
    return NULL;
}

object_t *interpret_block(interpreter_t *interpreter, atom_t *block, GHashTable *context, int current_indent) {
    interpreter->error = 0;
    printd("interpreting block\n");
    if (block->type != A_BLOCK) {
        interpreter->error = RUN_ERROR;
        printf("NOT A BLOCK\n");
        return NULL;
    }
    atom_t *stmt = block->child;
    object_t *last_result;
    do {
        object_t *ret = interpret_stmt(interpreter, stmt, context, current_indent);
        if (interpreter->error == RUN_ERROR)
            return NULL;
        if (ret != NULL)
            return ret;
    } while (stmt = stmt->next);
}

// for mac os
FILE *fmemopen (void *buf, size_t size, const char *opentype)
{
    FILE *f;

    assert(strcmp(opentype, "r") == 0);

    f = tmpfile();
    fwrite(buf, 1, size, f);
    rewind(f);

    return f;
}

void test_interpret_block(char *code, atom_tree_t *tree) {
    FILE *stream;
    stream = fmemopen(code, strlen(code), "r");
    struct t_tokenizer *tokenizer = new_tokenizer();
    int success = tokenize_stream(stream, tree, tokenizer);
    assert(tokenizer->error != PARSE_ERROR);
    tree->root = parse_block(tokenizer, -1);
    free_tokenizer(tokenizer);
    assert(tokenizer->error != PARSE_ERROR);
    char buff[2048];
    buff[0] = '\0';
    print_atom(tree->root, buff, 0, FALSE);
    printf("%s\n", buff);
    printd("creating interpreter\n", buff);
    interpreter_t * interpreter = new_interpreter();
    printd("interpreting\n", buff);
    interpret_block(interpreter, tree->root, interpreter->globals, 0);
    assert(interpreter->error != RUN_ERROR);
    g_hash_table_foreach(interpreter->globals, print_var_each, NULL);
    free_atom_tree(tree->root);
}

int main() {
    static char buffer[] = "a+b+(c*3)";
    atom_tree_t tree;
    test_interpret_block("a = int(1)\nb = a + 3", &tree);
    test_interpret_block("for i in range(1, 10):\n    print(i)", &tree);
    test_interpret_block("def add(a, b):\n    return a + b\nc = add(1,2)", &tree);
    test_interpret_block("for i in [1, 2, 3, 4]:\n    print(i)", &tree);
    test_interpret_block("a = {1:5, 2:6, 3:7, 4:8}", &tree);
    test_interpret_block("a = {1:5, 2:6, 3:7, 4:8}\nfor i in a.keys():\n    print(i)", &tree);
    test_interpret_block("class Foo(object):\n    def __add__(self, other):\n        return self.value + other\n    value = 10\nfoo = Foo()\nprint(foo+4)", &tree);
    test_interpret_block("l = [\"one\", \"two\", \"three\", 4]\nfor i in l:\n    print(i)\n", &tree);
    test_interpret_block("l = [1, 2, 3, 4, 5]\nprint(sum(l))", &tree);
    test_interpret_block("class MyThread(Thread):\n    def run(self):\n        print(255)\nthread = MyThread()\nthread.run()\n", &tree);
    test_interpret_block("print(1==2)", &tree);
    test_interpret_block(
"def recsum(n, sum):\n\
    if n < 1:\n\
        return sum\n\
    else:\n\
        return recsum(n-1, sum+n)\n\
print(recsum(10, 0))", &tree);
    return 0;
}
