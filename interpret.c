#include "interpret.h"

#define g_array_set(glist, item, index) \
    {*(((object_t **)(void *) glist->data) + index) = item;}

char *object_type_name(int type) {
    switch(type) {
        case INT_TYPE: return "INT";
        case STR_TYPE: return "STR";
        case CLASS_TYPE: return "CLASS";
        case FUNC_TYPE: return "FUNC";
        case LIST_TYPE: return "LIST";
        case LISTITERATOR_TYPE: return "LISTITERATOR";
        case USERFUNC_TYPE: return "USERFUNC";
        case DICTIONARY_TYPE: return "DICTIONARY";
        case CUSTOMOBJECT_TYPE: return "CUSTOMOBJECT";
        case THREAD_TYPE: return "THREAD";
        case BOOL_TYPE: return "BOOL";
        case NONE_TYPE: return "NONE";
        case SLICE_TYPE: return "SLICE";
        case GENERATORFUNC_TYPE: return "GENERATORFUNC";
        case EXCEPTION_TYPE: return "EXCEPTION";
        default: return "UNDEFINED";
    }
    assert(FALSE);
}

#define FUNC_STRUCT_TYPE(x) object_t *(*x)(object_t **)

struct py_thread * get_thread() {
    int *index = pthread_getspecific(py_thread_key);
    printd("%d got thread key\n", *index);
    return g_array_index(interpreter.threads, struct py_thread *, *index);
}

void print_stack_trace(struct py_thread *thread) {
    GArray *stack_trace = thread->stack_trace;
    printf("stack trace:\n");
    printf("%d elements\n", stack_trace->len);
    for (int i=0; i < stack_trace->len; i++)
        printf("-> %s\n", g_array_index(stack_trace, char*, i));
    assert(thread->exc->type == EXCEPTION_TYPE);
    object_t *str = object_call_str(thread->exc);
    assert(str->type == STR_TYPE);
}

void print_var_each(gpointer name, gpointer obj, gpointer user_data) {
    print_var((char*) name, (object_t*) obj);
}

void print_pair_each(gpointer key, gpointer value, gpointer user_data) {
    print_var("key", (object_t*) key);
    print_var("value", (object_t*) value);
}

int args_len(object_t **args) {
    if (args == NULL)
        return 0;
    int count = 0;
    while (*args != NULL) {args++; count++;}
    return count;
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
        printd("userfunc %s\n", name);
    } else if (obj->type == NONE_TYPE) {
        printd("None\n");
    } else if (obj->type == DICTIONARY_TYPE) {
        g_hash_table_foreach(obj->dict_props->ob_dval, print_pair_each, NULL);
    } if (obj->type == FUNC_TYPE) {
        printd("func %s\n", name);
    } else
        printd("obj %s %s\n", name, object_type_name(obj->type));
}

object_t *new_class(char* name, object_t **inherits, object_t *(*func)(object_t **, int), int expected_args_count) {
    object_t *cls = g_hash_table_lookup(interpreter.globals, name);
    printd("Creating the class %s\n", name);
    cls = new_object(CLASS_TYPE);
    cls->class_props = malloc(sizeof(struct class_type));
    int count;
    if (inherits == NULL)
        count = 1;
    else {
        object_t **start = inherits;
        while (*start != NULL) {
            start++;
        }
        count = start - inherits;
    }
    cls->class_props->inherits = malloc((count + 1)*sizeof(object_t *));
    if (inherits == NULL) {
        cls->class_props->inherits[0] = object_class;
        cls->class_props->inherits[1] = NULL;
    } else {
       int i = 0;
        while (inherits[i] != NULL) {
            cls->class_props->inherits[i++] = inherits[i];
        }
        cls->class_props->inherits[i] = NULL;
    }
    cls->class_props->name = name;
    cls->class_props->expected_args_count = expected_args_count;
    if (func != NULL) {
        object_add_field(cls, "__new__", new_func(func, strdup("__new__"), -1));
    } else {
        // TODO prevent multiple instance layouts
        object_t *__new__ = object_get_field(cls->class_props->inherits[0], "__new__");
        object_add_field(cls, "__new__", __new__);
    }
    cls->class = NULL;
    return cls;
}

void register_global(char* name, object_t *object) {
    g_hash_table_insert(interpreter.globals, name, object);
}

void register_builtin_func(char* name, object_t *object) {
    g_hash_table_insert(interpreter.globals, name, object);
}

object_t *lookup_var(object_t **args, atom_t *var) {
    object_t *obj = NULL;
    printd("lookup var cl_index %s %d arg_alloc %d\n", var->value, var->cl_index, sizeof(args) / sizeof(object_t *));
    if (var->cl_index == -1) {
        obj = g_hash_table_lookup(interpreter.globals, var->value);
        printd("var search from globals\n");
    } else if (args != NULL){
        obj = args[var->cl_index];
        printd("var search from locals\n");
    }
    if (obj == NULL) {
        g_hash_table_foreach(interpreter.globals, print_var_each, NULL);
        if (var->cl_index == -1) {
            set_exception("Global not found |%s|\n", var->value);
        } else {
            set_exception("Local not found!!!|%s|\n", var->value);
        }
        return NULL;
    }
    return obj;
}

object_t *set_var(object_t **args, atom_t *var, object_t* value) {
    object_t *obj;
    printd("set_var cl_index: %d\n", var->cl_index);
    if (var->cl_index == -1) {
        g_hash_table_insert(interpreter.globals, var->value, value);
    } else {
// maybe a length check
        args[var->cl_index] = value;
    }
}
object_t *get_global(char*name) {
    object_t *cls = g_hash_table_lookup(interpreter.globals, name);
    if (cls == NULL) {
        g_hash_table_foreach(interpreter.globals, print_var_each, NULL);
        set_exception("Global not found |%s|\n", name);
        return NULL;
    }
    return cls;
}

object_t *get_global_no_check(char*name) {
    return g_hash_table_lookup(interpreter.globals, name);
}

object_t *new_func(object_t *(*func)(object_t **, int), char *name, int expected_args_count) {
    object_t *func_obj = new_object(FUNC_TYPE);
    func_obj->func_props = malloc(sizeof(struct func_type));
    func_obj->func_props->ob_func = func;
    func_obj->func_props->name = name;
    func_obj->func_props->expected_args_count = expected_args_count;
    func_obj->class = NULL;
    return func_obj;
}

object_t *new_exception(object_t **args, int count) {
    object_t *message = args[1];
    object_t *exception_obj = new_object(EXCEPTION_TYPE);
    exception_obj->class = args[0];
    object_add_field(exception_obj, "message", message);
    return exception_obj;
}

object_t *exception_str(object_t **args, int count) {
    char *buff;
    object_t *self = args[0];
    object_t *message = object_get_field(self, "message");
    asprintf(&buff, "%s: %s", self->class->class_props->name, message->str_props->ob_sval->str);
printf("%s\n", buff);
    object_t *str = new_str_internal(buff);
    free(buff);
    return str;
}

object_t *new_user_func(atom_t *func, char* name, GHashTable *kwargs) {
    object_t *func_obj = new_object(USERFUNC_TYPE);
    func_obj->userfunc_props = malloc(sizeof(struct userfunc_type));
    func_obj->userfunc_props->ob_userfunc = func;
    func_obj->userfunc_props->name = name;
    func_obj->userfunc_props->kwargs = kwargs;
    func_obj->class = NULL;
    return func_obj;
}

object_t *new_generator_func(atom_t *func, char* name) {
    object_t *func_obj = new_object(GENERATORFUNC_TYPE);
    func_obj->generatorfunc_props = malloc(sizeof(struct generatorfunc_type));
    func_obj->generatorfunc_props->ob_generatorfunc = func;
    func_obj->generatorfunc_props->name = name;
    func_obj->class = NULL;
    return func_obj;
}

object_t *sum_func(object_t **args) {
    object_t *iterable = args[0];
    object_t *iterator = object_call_func_no_param(iterable, "__iter__");
    if (interpreter.error == RUN_ERROR)
        return NULL;
    object_t *next_func = object_get_field(iterator, "next");
    if (interpreter.error == RUN_ERROR)
        return NULL;
    int sum = 0;
    object_t *int_obj = NULL;
    object_t *next_params[2] = {iterator, NULL};
    do {
        int_obj = object_call_func_obj(next_func, next_params, 1);
        assert(int_obj != NULL?int_obj->type == INT_TYPE:TRUE);
        // TODO __add__
        if (int_obj != NULL)
            sum += int_obj->int_props->ob_ival;
    } while (int_obj != NULL);

    object_t *result = new_int_internal(sum);
    // TODO CHECKS
    return result;
}

object_t *range_func(object_t **args) {
    object_t *min = args[0];
    object_t *max = args[1];
    assert(args[2] == NULL);
    if (min->type != INT_TYPE || min->type != INT_TYPE) {
        set_exception("Range parameters should be integer\n");
        return NULL;
    }
    object_t *list = new_list(NULL, 0);
    int i;
    for (i=min->int_props->ob_ival; i<max->int_props->ob_ival; i++) {
        list_append_internal(list, new_int_internal(i));
    }
    return list;
}

object_t *assert_func(object_t **args, int count) {
    if (count > 2) {
        set_exception("assert takes at most 2 arguments");
        return NULL;
    }
    if (count < 1) {
        set_exception("assert takes at least 1 arguments");
        return NULL;
    }
    object_t *expr_result = args[0];
    object_t *message;
    if (count == 2)
        message = args[1];
    else {
        message = new_str_internal("Assertion false");
    }
    object_t *bool_result = new_bool_internal(expr_result);
    if (interpreter.error == RUN_ERROR)
        return NULL;
    if (bool_result->bool_props->ob_bval == FALSE) {
        set_exception(strdup(message->str_props->ob_sval->str));
        return NULL;
    }
    return new_none_internal();
}

object_t *print_func(object_t **args, int count) {
    for (int i=0; i < count; i++) {
        object_t *obj_str = object_call_str(args[i]);
        if (interpreter.error == RUN_ERROR)
            return NULL;
        printf("%s\n", obj_str->str_props->ob_sval->str);
    }
}

struct py_thread *new_thread_struct() {
    struct py_thread *thread = malloc(sizeof(struct py_thread));
    thread->stack_trace = g_array_new(TRUE, TRUE, sizeof(char *));
    thread->generator = NULL;
    return thread;
}

void local_storage_destructor(int *value) {
}

void init_interpreter() {
    interpreter.error = 0;
    interpreter.last_accessed = NULL;

// TODO when threads get implemented
    int *p = malloc(sizeof(int));
    *p = 0;
    pthread_key_create(&py_thread_key, local_storage_destructor);
    pthread_setspecific(py_thread_key, p);
// TODO when destroy_interpreter or destroy_thread gets implemented
//   int *index = pthread_getspecific(py_thread_key);
//   free(index);
//   pthread_setspecific(py_thread_key, NULL);

    interpreter.globals = g_hash_table_new(g_str_hash, g_str_equal);
    interpreter.threads = g_array_new(TRUE, TRUE, sizeof(struct py_thread *));
    struct py_thread *main_thread = new_thread_struct();
    g_array_append_val(interpreter.threads, main_thread);

    init_object();

    init_int();
    init_str();
    
    object_t *exception_class = new_class(strdup("Exception"), NULL, new_exception, 2);
    object_add_field(exception_class, "__str__", new_func(exception_str, strdup("__str__"), 1));
    register_global(strdup("Exception"), exception_class);
    init_bool();
    init_none();
    init_slice();

    register_global(strdup("range"), new_func(range_func, strdup("range"), -1));
    register_global(strdup("sum"), new_func(sum_func, strdup("sum"), 1));

    init_list();

    init_dict();

    init_thread();
    init_generator();

    register_global(strdup("print"), new_func(print_func, strdup("print"), -1));
    register_global(strdup("assert"), new_func(assert_func, strdup("assert"), -1));
}

int count_non_global_vars(GHashTable* context) {
    GHashTableIter iter;
    gpointer _key, value;
    int count = 0;
    g_hash_table_iter_init (&iter, context);
    while (g_hash_table_iter_next (&iter, &_key, &value)) {
        char *key = _key;
        freevar_t *var = value;
        if (var->cl_index != -1) {
            printd("counting non_global var %d %d\n", var->type, var->cl_index);
            count++;
        }
    }
    return count;
}

object_t *interpret_expr(atom_t *,object_t **, int);
object_t *interpret_funccall(atom_t *func_call, object_t **args, int current_indent) {
    object_t *func = interpret_expr(func_call->child, args, current_indent);
    if (interpreter.error == RUN_ERROR)
        return NULL;
    if (interpreter.last_accessed)
        print_var("last", interpreter.last_accessed);
    if (func == NULL) {
        set_exception("function not found |%s|\n", func_call->value);
        return NULL;
    }
    if (func->type != FUNC_TYPE && func->type != USERFUNC_TYPE && func->type != CLASS_TYPE && func->type != GENERATORFUNC_TYPE) {
        set_exception("OBJ IS NOT CALLABLE %s\n", object_type_name(func->type));
        return NULL;
    }
    printd("FUNC FOUND |%s|\n", func_call->value);

    atom_t *param = func_call->child->next->child;
    object_t *self = NULL;
    printd("deciding instance param\n");
    if (func->type == CLASS_TYPE) {
        print_var("adding instance param class", func);
        self = func;
    } else if (func_call->child->type == A_ACCESSOR && interpreter.last_accessed != NULL) {
        print_var("adding instance param", interpreter.last_accessed);
        self = interpreter.last_accessed;
    }
 
    object_t **inner_args = NULL;
    int inner_args_count = 0;
    if (func->type == USERFUNC_TYPE || func->type == GENERATORFUNC_TYPE) {
        atom_t *funcdef_atom;
        if (func->type == USERFUNC_TYPE)
            funcdef_atom = func->userfunc_props->ob_userfunc;
        else
            funcdef_atom = func->generatorfunc_props->ob_generatorfunc;
        inner_args = malloc(sizeof(object_t *) * (funcdef_atom->args_count + 1));
        memcpy(inner_args, funcdef_atom->args, sizeof(object_t *) * (funcdef_atom->args_count + 1));
    } else {
        if (self != NULL && (func->type == FUNC_TYPE || func->type == CLASS_TYPE))
            inner_args_count++;
// builtins
// TODO should be calculated once while parsing the call
        atom_t *param = func_call->child->next->child;
        while (param) {
            inner_args_count++;
            param = param->next;
        }
        inner_args = malloc((inner_args_count + 1) * sizeof(object_t *));
        inner_args[inner_args_count] = NULL;
        int expected_args_count;
        if (func->type == FUNC_TYPE)
            expected_args_count = func->func_props->expected_args_count;
        else if (func->type == CLASS_TYPE)
            expected_args_count = func->class_props->expected_args_count;
        else
            assert(FALSE);
        if (expected_args_count != -1 && inner_args_count != expected_args_count) {
            if (func->type == FUNC_TYPE) {
                set_exception("Argument count should be %d for %s, was %d\n", expected_args_count, func->func_props->name, inner_args_count);
            } else if (func->type == CLASS_TYPE) {
                set_exception("Argument count should be %d for %s, was %d\n", expected_args_count, func->class_props->name, inner_args_count);
            }
            return NULL;
        }
    }
    if (func->type == FUNC_TYPE || func->type == CLASS_TYPE) {
        int param_count = 0;
        if (self != NULL)
            inner_args[param_count++] = self;
        while (param) {
// TODO kwargs should be supported for builtins
            object_t *value = interpret_expr(param, args, current_indent);
            if (interpreter.error == RUN_ERROR) {
                return NULL;
            }
            printd("ADDING ARGUMENT %s\n", param->value);
            print_var("", value);
            inner_args[param_count++] = value;
            param = param->next;
        }
    } else {
        int param_count = 0;
        atom_t *param_name = func->userfunc_props->ob_userfunc->child->child;
        if (self != NULL) {
            param_name = param_name->next;
            inner_args[param_count++] = self;
        }
        while (param && param_name && param_name->type == A_VAR) {
            object_t *value = interpret_expr(param, args, current_indent);
            if (interpreter.error == RUN_ERROR) {
                return NULL;
            }
            printd("ADDING ARGUMENT %s\n", param->value);
            print_var("", value);
            inner_args[param_count++] = value;
            param = param->next;
            param_name = param_name->next;
        }
        if (param != NULL && param_name == NULL) {
            printd("param type %s\n", object_type_name(param->type));
            set_exception("Too many params to %s\n", func->userfunc_props->name);
            return NULL;
        } else if (param == NULL && param_name != NULL && param_name->type == A_VAR) {
            printd("param_name %s\n", param_name->value);
            set_exception("More params needed for %s\n", func->userfunc_props->name);
            return NULL;
        }
 
        GHashTable *inner_kwargs = g_hash_table_new(g_str_hash, g_str_equal);
        atom_t *kwarg_name = param_name;
        // kwargs without names
        while (param && (param->type != A_VAR || param->child == NULL) && param_name && param_name->type == A_KWARG) {
            printd("adding kwarg %s\n", param_name->value);
            object_t *value = interpret_expr(param, args, current_indent);
            if (interpreter.error == RUN_ERROR) {
                return NULL;
            }
            g_hash_table_insert(inner_kwargs, param_name->value, value);
            param_name = param_name->next;
            param = param->next;
        }
        // kwargs with names
        while (param) {
            assert(param->child != NULL);
            printd("adding named kwarg %s\n", param->value);
            object_t *value = interpret_expr(param->child, args, current_indent);
            if (interpreter.error == RUN_ERROR) {
                return NULL;
            }
            g_hash_table_insert(inner_kwargs, param->value, value);
            param = param->next;
        }
        inner_args_count = param_count;
        while (kwarg_name && kwarg_name->type == A_KWARG) {
            object_t *value = g_hash_table_lookup(inner_kwargs, kwarg_name->value);
            if (value == NULL)
                value = g_hash_table_lookup(func->userfunc_props->kwargs, kwarg_name->value);
            inner_args[param_count++] = value;
            kwarg_name = kwarg_name->next;
        }
        g_hash_table_destroy(inner_kwargs);
        while (param_name && param_name->type == A_KWARG) param_name = param_name->next;
        if (param_name != NULL && param_name->type != A_CLOSURE) {
            assert(FALSE);
        }
    }
    printd("calling func type %s\n", object_type_name(func->type));
    printd("ADDED PARAMS\n");
    struct py_thread *thread = get_thread();
    char *func_name;
    object_t *result;
    if (func->type == USERFUNC_TYPE) {
        func_name = strdup(func->userfunc_props->name);
        g_array_append_val(thread->stack_trace, func_name);
        result = interpret_funcblock(func->userfunc_props->ob_userfunc->child->next, inner_args, current_indent);
    } else if (func->type == GENERATORFUNC_TYPE) {
        func_name = strdup(func->generatorfunc_props->name);
        g_array_append_val(thread->stack_trace, func_name);
        result = new_generator_internal(inner_args, inner_args_count, func->generatorfunc_props->ob_generatorfunc);
    } else if (func->type == FUNC_TYPE) {
        func_name = strdup(func->func_props->name);
        g_array_append_val(thread->stack_trace, func_name);
        result = func->func_props->ob_func(inner_args, inner_args_count);
    } else if (func->type == CLASS_TYPE) {
        func_name = strdup(func->class_props->name);
        g_array_append_val(thread->stack_trace, func_name);
        result = object_call_func(func, inner_args, inner_args_count, "__new__");
        // TODO this is a workaround, new should get class
        result->class = func;
        if (interpreter.error == RUN_ERROR)
            return NULL;
    } else
        assert(FALSE);
    if (interpreter.error != RUN_ERROR) {
        g_array_remove_index(thread->stack_trace, thread->stack_trace->len - 1);
        free(func_name);
    }
    if (func->type != GENERATORFUNC_TYPE) {
// TODO generator func should free its args
        free(inner_args);
    }
    return result;
}

object_t *interpret_list(atom_t *expr, object_t **args, int current_indent) {
    object_t *list = new_list_internal();
    atom_t *elem = expr->child;
    while (elem != NULL) {
        object_t *result = interpret_expr(elem, args, current_indent);
        if (interpreter.error == RUN_ERROR) {
            return NULL;
        }
        list_append_internal(list, result);
        elem = elem->next;
    }
    return list;
}

object_t *interpret_dict(atom_t *expr, object_t **args, int current_indent) {
    object_t *dict = new_dict(NULL, 0);
    atom_t *elem = expr->child;
    while (elem != NULL) {
        object_t *key = interpret_expr(elem, args, current_indent);
        if (interpreter.error == RUN_ERROR) {
            return dict;
        }
        object_t *value = interpret_expr(elem->next, args, current_indent);
        if (interpreter.error == RUN_ERROR) {
            return dict;
        }
        g_hash_table_insert(dict->dict_props->ob_dval, key, value);
        if (interpreter.error == RUN_ERROR)
            return NULL;
        elem = elem->next->next;
    }
    return dict;
}

object_t *interpret_while(atom_t *expr, object_t **args, int current_indent) {
    atom_t *condition = expr->child;
    atom_t *block = expr->child->next;
// TODO give it to bool
    object_t *bool_obj;
    object_t *ret = NULL;
    while (bool_obj = interpret_expr(condition, args, current_indent)) {
        if (bool_obj->type != BOOL_TYPE) {
            set_exception("NOT A BOOL TYPE\n");
            return NULL;
        }
        if (bool_obj->bool_props->ob_bval == FALSE)
            return ret;
        ret = interpret_block(block, args, current_indent);
        if (ret != NULL)
            return ret;
    }
    return NULL;
}

object_t *interpret_if(atom_t *expr, object_t **args, int current_indent) {
    atom_t *if_block = expr->child;
    while (if_block) {
        if (if_block->type == A_BLOCK) {
            printd("IF CLAUSE FALSE!!!\n");
            return interpret_block(if_block, args, current_indent);
        } else {
// TODO give it to bool
            object_t *expr = interpret_expr(if_block, args, current_indent);
            if (interpreter.error == RUN_ERROR)
                return NULL;
            object_t *bool_obj = new_bool_internal(expr);
            if (interpreter.error == RUN_ERROR)
                return NULL;
            if (bool_obj->type != BOOL_TYPE) {
                set_exception("NOT A BOOL TYPE\n");
                return NULL;
            }
            if_block = if_block->next;
            assert(if_block != NULL);
            if (bool_obj->bool_props->ob_bval == TRUE) {
                printd("IF CLAUSE TRUE!!!\n");
                return interpret_block(if_block, args, current_indent);
            }
        }        if_block = if_block->next;
    }
    return NULL;
}

object_t *interpret_expr(atom_t *expr, object_t **args, int current_indent) {
// TODO
    printd("FIRST_TYPE %s\n", atom_type_name(expr->type));
    if (expr->type == A_VAR) {
        printd("VAR %s\n", expr->value);
        object_t *value = lookup_var(args, expr);
        if (value)
            print_var(expr->value, value);
        return value;
    } else if (expr->type == A_FUNCCALL) {
        printd("CALL FUNC \n");
        // TODO more than one thread and stack trace
        return interpret_funccall(expr, args, current_indent);
    } else if (expr->type == A_INTEGER) {
        object_t *int_val = new_int_internal(atoi(expr->value));
        printd("NEW INT %d\n", int_val->int_props->ob_ival);
        return int_val;
    } else if (expr->type == A_STRING) {
        object_t *str_val = new_str_internal(expr->value);
        return str_val;
    } else if (expr->type == A_LIST) {
        printd("NEW LIST %s\n", expr->value);
        object_t *list = interpret_list(expr, args, current_indent);
        return list;
    } else if (expr->type == A_DICTIONARY) {
        printd("NEW DICTIONARY %s\n", expr->value);
        object_t *dict = interpret_dict(expr, args, current_indent);
        return dict;
    } else if (expr->type == A_ACCESSOR) {
        printd("ACCESSING %s\n", expr->child->value);
        object_t *object;
        if (expr->child->type == A_VAR) {
            printd("%s\n", expr->child->value);
            object = lookup_var(args, expr->child);
            if (interpreter.error == RUN_ERROR) {
                return NULL;
            }
            print_var("object", object);
            printd("getting field %s of %s\n", expr->child->next->value, expr->child->value);
        } else
            object = interpret_expr(expr->child, args, current_indent);
        if (object == NULL)
            return NULL;
        interpreter.last_accessed = object;
        printd("last accessed %p\n", object);
        object_t *field = object_get_field(object, expr->child->next->value);
        if (interpreter.error == RUN_ERROR)
            return NULL;
        return field;
    } else if (expr->type == A_NOT) {
        object_t *result = interpret_expr(expr->child, args, current_indent);
        if (interpreter.error == RUN_ERROR)
            return NULL;
        object_t *bool_obj = new_bool_internal(result);
        if (interpreter.error == RUN_ERROR)
            return NULL;
        return new_bool_from_int(!bool_obj->bool_props->ob_bval);
    } else {
        set_exception("TYPE INCORRECT %s\n", atom_type_name(expr->type));
        return NULL;
    }
}

void prepare_func_args(object_t *func, object_t **args, int current_indent) {
    atom_t *funcdef;
    if (func->type == USERFUNC_TYPE)
        funcdef = func->userfunc_props->ob_userfunc;
    else if (func->type == GENERATORFUNC_TYPE)
        funcdef = func->generatorfunc_props->ob_generatorfunc;
    else
        assert(FALSE);
    int args_count = g_hash_table_size(funcdef->context);
    printd("(user/generator)func args_count %d\n", args_count);
    funcdef->args = malloc(sizeof(object_t *) * (args_count + 1));
    memset(funcdef->args, 0, sizeof(object_t *) * (args_count + 1));
    funcdef->args[args_count] = NULL;
    funcdef->args_count = args_count;
    int param_index = 0;
    atom_t *param_name = funcdef->child->child;
    while (param_name != NULL && param_name->type == A_VAR) {
        param_index++;
        param_name = param_name->next;
    }
    printd("Binding kwargs\n");
    while (param_name != NULL && param_name->type == A_KWARG) {
        printd("Binding default kwarg %s to %d\n", param_name->value, param_index);
        atom_t *value_expr = param_name->child;
        object_t *def_val = interpret_expr(value_expr, args, current_indent);
        funcdef->args[param_index++] = def_val;
        param_name = param_name->next;
    }
    while (param_name != NULL && param_name->type == A_KWARG) {
        param_index++;
        param_name = param_name->next;
    }
    printd("binding closures\n");
    while (param_name != NULL) {
        printd("Binding closure %p %s %d to %d\n", args[param_name->cl_index], param_name->value, param_name->cl_index, param_index);
        assert(param_name->cl_index < args_count);
        //printd("binding arg %d as %s %d\n", closure_name->cl_index, closure_name->value, param_index);
        object_t *arg = args[param_name->cl_index];
        funcdef->args[param_index++] = arg;
        param_name = param_name->next;
    }
}

object_t *interpret_stmt(atom_t *stmt, object_t **args, int current_indent) {
    if (stmt->type == A_FUNCCALL) {
        interpret_expr(stmt, args, current_indent);
    } else if (stmt->type == A_ASSIGNMENT) {
        atom_t *left_var = stmt->child;
        printd("ASSIGNING TO %s\n", left_var->value);
        object_t *result = interpret_expr(left_var->next, args, current_indent);
        if (interpreter.error == RUN_ERROR)
            return NULL;
        if (result == NULL) {
            printd("GOT NOTHING, can't assign\n");
            return NULL;
        }

        if (left_var->type == A_ACCESSOR) {
            object_t *object = interpret_expr(left_var->child, args, current_indent);
            if (interpreter.error == RUN_ERROR)
                return NULL;
            object_t *set_attr = object_get_field_no_check(object, "__setattr__");
            if (set_attr != NULL) {
                printd("%p object has __setattr__\n", object);
                object_t *field_name = new_str_internal(left_var->child->next->value);
                object_t *params[4] = {object, field_name, result, NULL};
                object_call_func_obj(set_attr, params, 3);
            } else {
                printd("%p object does not have __setattr__\n", object);
                object_set_field(object, left_var->child->next->value, result);
            }
        } else {
            print_var("result", result);
            set_var(args, left_var, result);
        }
        return NULL;
    } else if (stmt->type == A_FOR) {
        atom_t *var_name = stmt->child;
        atom_t *expr = var_name->next;
        object_t *iterable = interpret_expr(expr, args, current_indent);
        if (interpreter.error == RUN_ERROR)
            return NULL;
        printd("calling __iter__\n");
        object_t *iterator = object_call_func_no_param(iterable, "__iter__");
        if (interpreter.error == RUN_ERROR)
            return NULL;
        printd("calling __iter__ END\n");
        assert(iterator->class != NULL);
        atom_t *block = expr->next;
        object_t *next_func = object_get_field(iterator, "next");
        if (interpreter.error == RUN_ERROR)
            return NULL;
        object_t *params[2] = {iterator, NULL};
        object_t *item;
        while(item = object_call_func_obj(next_func, params, 1)) {
            if (interpreter.error == RUN_ERROR)
                return NULL;
            if (var_name->type == A_TUPLE) {
                object_t *tuple_it = object_call_func_no_param(item, "__iter__");
                if (interpreter.error == RUN_ERROR)
                    return NULL;
                object_t *next_func = object_get_field(tuple_it, "next");
                if (interpreter.error == RUN_ERROR)
                    return NULL;
                object_t *next_params[2] = {tuple_it, NULL};
                object_t *tuple_item;
                atom_t *tuple_item_name = var_name->child;
                while ((tuple_item = object_call_func_obj(next_func, next_params, 1)) && tuple_item_name != NULL) {
                    if (interpreter.error == RUN_ERROR)
                        return NULL;
                    if (tuple_item_name->type != A_VAR) {
                        set_exception("recursuve tuple unpack is not implemented yet\n");
                        return NULL;
                    }
                    set_var(args, tuple_item_name, tuple_item);
                    tuple_item_name = tuple_item_name->next;
                }
                if (tuple_item != NULL || tuple_item_name != NULL) {
                    set_exception("Error while unpacking %p %s\n", tuple_item, tuple_item_name->value);
                    return NULL;
                }
            } else {
                set_var(args, var_name, item);
            }
            interpret_block(block, args, current_indent);
            if (interpreter.error == RUN_ERROR) {
                return NULL;
            }
        }
    } else if (stmt->type == A_IF) {
printd("A_IF\n");
        return interpret_if(stmt, args, current_indent);
    } else if (stmt->type == A_WHILE) {
printd("A_WHILE\n");
        return interpret_while(stmt, args, current_indent);
    } else if (stmt->type == A_FUNCDEF) {
        atom_t *param = stmt->child->child;
        GHashTable *kwargs = NULL;
        while (param && param->child == NULL)
            param = param->next;
        if (param != NULL) {
            kwargs = g_hash_table_new(g_str_hash, g_str_equal);
            while (param != NULL) {
                object_t *value = interpret_expr(param->child, args, current_indent);
                if (interpreter.error == RUN_ERROR)
                    return NULL;
                g_hash_table_insert(kwargs, strdup(param->value), value);
                param = param->next;
            }
        }
        object_t *userfunc = new_user_func(stmt, strdup(stmt->value), kwargs);
        set_var(args, stmt, userfunc);
        prepare_func_args(userfunc, args, current_indent);
    } else if (stmt->type == A_GENFUNCDEF) {
        object_t *generatorfunc = new_generator_func(stmt, strdup(stmt->value));
        set_var(args, stmt, generatorfunc);
        prepare_func_args(generatorfunc, args, current_indent);
    } else if (stmt->type == A_CLASS) {
        atom_t *class_name = stmt;
        atom_t *inherits = stmt->child->child;
        int count = 0;
        while (inherits != NULL) {count++; inherits = inherits->next;}
        object_t *class;
        if (count == 0) {
            class = new_class(strdup(class_name->value), NULL, NULL, -1);
        } else {
            object_t *param_inherits[count+1];
            inherits = stmt->child->child;
            int i = 0;
	    while (inherits != NULL) {
                object_t *parent_class = lookup_var(args, inherits);
                if (parent_class == NULL) {
                    set_exception("Class does not exist: %s\n", inherits->value);
                    return NULL;
                }
                param_inherits[i++] = parent_class;
                inherits = inherits->next;
            }
            param_inherits[i] = NULL;
            class = new_class(strdup(class_name->value), param_inherits, NULL, -1);
        }
        atom_t *field = stmt->child->next;
        while (field) {
            if (field->type == A_FUNCDEF) {
                char *trace_str;
                asprintf(&trace_str, "%s.%s", class_name->value, field->value);

                atom_t *param = stmt->child->child;
                GHashTable *kwargs = NULL;
                while (param && param->child == NULL)
                    param = param->next;
                if (param != NULL) {
                    kwargs = g_hash_table_new(g_str_hash, g_str_equal);
                    while (param != NULL && param->child != NULL) {
                        object_t *value = interpret_expr(param->child, args, current_indent);
                       if (interpreter.error == RUN_ERROR)
                            return NULL;
                        g_hash_table_insert(kwargs, param->value, value);
                    }
                }
                object_t *userfunc = new_user_func(field, trace_str, kwargs);
                object_add_field(class, field->value, userfunc);
                printd("added class field func %s.%s\n", class_name->value, field->value);
                prepare_func_args(userfunc, args, current_indent);
            } else if (field->type == A_GENFUNCDEF) {
                char *trace_str;
                asprintf(&trace_str, "%s.%s", class_name->value, field->value);
                object_t *generatorfunc = new_generator_func(field, trace_str);
                object_add_field(class, field->value, generatorfunc);
                prepare_func_args(generatorfunc, args, current_indent);
            } else if (field->type == A_VAR) {
                object_t *result = interpret_expr(field->child, args, current_indent);
                if (interpreter.error == RUN_ERROR)
                    return NULL;
                object_add_field(class, field->value, result);
                printd("added class field %s.%s\n", class_name->value, field->value);
            } else {
                assert(FALSE);
            }
            field = field->next;
        }
        set_var(args, class_name, class);
   } else if (stmt->type == A_RETURN) {
        printd("returning something\n");
        if (stmt->child)
            return interpret_expr(stmt->child, args, current_indent);
        else
            return new_none_internal();
    } else if (stmt->type == A_YIELD) {
        printd("yielding something\n");
        struct py_thread *thread = get_thread();
        if (stmt->child)
            thread->generator_channel = interpret_expr(stmt->child, args, current_indent);
        else
            thread->generator_channel = new_none_internal();
        GCond *cond = thread->generator->generatorfunc_props->cond;
        GMutex *mutex = thread->generator->generatorfunc_props->mutex;
        g_mutex_lock(mutex);
        printd("signalling %p from yield (gen thread)\n", cond);
        g_cond_signal(cond);
        printd("waiting %p from yield (gen thread)\n", cond);
        g_cond_wait(cond, mutex);
        g_mutex_unlock(mutex);
        printd("passed yield wait (gen thread)\n");
    } else {
        set_exception("UNKNOWN ATOM %s\n", atom_type_name(stmt->type));
        return NULL;
    }
    return NULL;
}

object_t *interpret_block(atom_t *block, object_t **args, int current_indent) {
    // TODO WHY?
    interpreter.error = 0;
    printd("interpreting block\n");
    if (block->type != A_BLOCK) {
        set_exception("NOT A BLOCK\n");
        return NULL;
    }
    atom_t *stmt = block->child;
    if (stmt == NULL)
        return new_none_internal();
    object_t *last_result;
    do {
        object_t *ret = interpret_stmt(stmt, args, current_indent);
        if (interpreter.error == RUN_ERROR)
            return NULL;
        if (ret != NULL)
            return ret;
    } while (stmt = stmt->next);
    return NULL;
}

object_t *interpret_funcblock(atom_t *block, object_t **args, int current_indent) {
    object_t *result = interpret_block(block, args, current_indent);
    struct py_thread *thread = get_thread();
    if (thread->generator != NULL) {
        thread->generator_channel = NULL;
        GMutex *mutex = thread->generator->generatorfunc_props->mutex;
        GCond *cond = thread->generator->generatorfunc_props->cond;
        printd("signalling %p from block end (gen thread)\n", cond);
        g_mutex_lock(mutex);
        g_cond_signal(thread->generator->generatorfunc_props->cond);
        g_mutex_unlock(mutex);
    }
    return result != NULL? result : new_none_internal();
}

