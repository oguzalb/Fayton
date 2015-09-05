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
    printf("%s\n", thread->exc_msg);
}

void print_var_each(gpointer name, gpointer obj, gpointer user_data) {
    print_var((char*) name, (object_t*) obj);
}

void print_pair_each(gpointer key, gpointer value, gpointer user_data) {
    print_var("key", (object_t*) key);
    print_var("value", (object_t*) value);
}

int args_len(object_t **args) {
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

object_t *new_class(char* name) {
    object_t *cls = g_hash_table_lookup(interpreter.globals, name);
    if (cls != NULL) {
        printd("This class already exists\n");
        return NULL;
    }
    printd("Creating the class %s\n", name);
    cls = new_object(CLASS_TYPE);
    cls->class_props = malloc(sizeof(struct class_type));
    cls->class_props->inherits = object_class;
    cls->class_props->name = name;
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
    object_t *obj;
    printd("lookup var cl_index %d\n", var->cl_index);
    if (var->cl_index == -1) {
        obj = g_hash_table_lookup(interpreter.globals, var->value);
        printd("var search from globals\n");
    } else if (var->cl_index < args_len(args)){
        obj = args[var->cl_index];
        printd("var search from locals\n");
    }
    if (obj == NULL) {
        g_hash_table_foreach(interpreter.globals, print_var_each, NULL);
        set_exception(var->cl_index == -1 ?"Global not found |%s|\n":"Local not found!!!|%s|\n", var->value);
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

object_t *new_func(object_t *(*func)(object_t **), char *name) {
    object_t *func_obj = new_object(FUNC_TYPE);
    func_obj->func_props = malloc(sizeof(struct func_type));
    func_obj->func_props->ob_func = func;
    func_obj->func_props->name = name;
    func_obj->class = NULL;
    return func_obj;
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
        int_obj = object_call_func_obj(next_func, next_params);
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
    object_t *list = new_list(NULL);
    int i;
    for (i=min->int_props->ob_ival; i<max->int_props->ob_ival; i++) {
        list_append_internal(list, new_int_internal(i));
    }
    return list;
}

object_t *print_func(object_t **args) {
    object_t *obj = args[0];
    assert(args[1] == NULL);
    object_t *obj_str = object_call_str(obj);
    if (interpreter.error == RUN_ERROR)
        return NULL;
    printf("%s\n", obj_str->str_props->ob_sval->str);
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
    init_bool();
    init_none();
    init_slice();

    register_global(strdup("range"), new_func(range_func, strdup("range")));
    register_global(strdup("sum"), new_func(sum_func, strdup("sum")));

    init_list();

    init_dict();

    init_thread();
    init_generator();

    register_global(strdup("print"), new_func(print_func, strdup("print")));
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
    int inner_arg_count = 0;
    if (func->type == USERFUNC_TYPE) {
        inner_arg_count = g_hash_table_size(func->userfunc_props->ob_userfunc->context)+1;
    } else if (func->type == GENERATORFUNC_TYPE) {
        inner_arg_count = g_hash_table_size(func->generatorfunc_props->ob_generatorfunc->context)+1;
    } else {
// builtins
// TODO should be calculated once
        atom_t *param = func_call->child->next->child;
        while (param) {
            inner_arg_count++;
            param = param->next;
        }
    }
    atom_t *param = func_call->child->next->child;
    object_t *self = NULL;
    printd("deciding instance param\n");
    if (func->type == CLASS_TYPE) {
        print_var("adding param class", func);
        self = func;
    } else if (func_call->child->type == A_ACCESSOR && interpreter.last_accessed != NULL) {
        print_var("adding param", interpreter.last_accessed);
        self = interpreter.last_accessed;
    }
    if (self != NULL)
        inner_arg_count++;
    object_t *inner_args[inner_arg_count];
    inner_args[inner_arg_count] = NULL;
    int param_index = 0;
    if (self != NULL)
        inner_args[param_index++] = self;
    if (func->type == FUNC_TYPE || func->type == CLASS_TYPE) {
        while (param) {
// TODO kwargs should be supported for builtins
            object_t *value = interpret_expr(param, args, current_indent);
            if (interpreter.error == RUN_ERROR) {
                return NULL;
            }
            printd("ADDING ARGUMENT %s\n", param->value);
            print_var("", value);
            inner_args[param_index++] = value;
            param = param->next;
        }
    } else {
        atom_t *param_name = func->userfunc_props->ob_userfunc->child->child;
        if (func_call->child->type == A_ACCESSOR && interpreter.last_accessed != NULL)
            param_name = param_name->next;
        while (param && param_name && param_name->type == A_VAR) {
            object_t *value = interpret_expr(param, args, current_indent);
            if (interpreter.error == RUN_ERROR) {
                return NULL;
            }
            printd("ADDING ARGUMENT %s\n", param->value);
            print_var("", value);
            inner_args[param_index++] = value;
            param = param->next;
            param_name = param_name->next;
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
        while (kwarg_name && kwarg_name->type == A_KWARG) {
            object_t *value = g_hash_table_lookup(inner_kwargs, kwarg_name->value);
            if (value == NULL)
                value = g_hash_table_lookup(func->userfunc_props->kwargs, kwarg_name->value);
            assert(value != NULL);
            inner_args[param_index++] = value;
            kwarg_name = kwarg_name->next;
        }
        g_hash_table_destroy(inner_kwargs);
        if (param != NULL) {
            printd("param type %s\n", object_type_name(param->type));
            set_exception("Too many params to %s\n", func->userfunc_props->name);
            return NULL;
        } else if (param_name != NULL && param_name->type == A_VAR) {
            printd("param_name %s\n", param_name->value);
            set_exception("More params needed for %s\n", func->userfunc_props->name);
            return NULL;
        }
        while (param_name && param_name->type == A_KWARG) param_name = param_name->next;
        atom_t *closure_name = param_name;
        while (closure_name) {
            printd("cl_index: %d args_len: %d %s\n", closure_name->cl_index, args_len(args), atom_type_name(closure_name->type));
            assert(closure_name->cl_index < args_len(args));
            object_t *arg = args[closure_name->cl_index];
            inner_args[param_index++] = arg;
            closure_name = closure_name->next;
        }
    }
    printd("calling func type %s %s\n", object_type_name(func->type), func->func_props->name);
    printd("ADDED PARAMS\n");
    struct py_thread *thread = get_thread();
    char* func_name;
    object_t *result;
    if (func->type == USERFUNC_TYPE) {
        func_name = strdup(func->userfunc_props->name);
        g_array_append_val(thread->stack_trace, func_name);
        result = interpret_funcblock(func->userfunc_props->ob_userfunc->child->next, inner_args, current_indent);
    } else if (func->type == GENERATORFUNC_TYPE) {
        func_name = strdup(func->generatorfunc_props->name);
        g_array_append_val(thread->stack_trace, func_name);
        result = new_generator_internal(inner_args, func->generatorfunc_props->ob_generatorfunc);
    } else if (func->type == FUNC_TYPE) {
        func_name = strdup(func->func_props->name);
        g_array_append_val(thread->stack_trace, func_name);
        result = func->func_props->ob_func(inner_args);
    } else if (func->type == CLASS_TYPE) {
        func_name = strdup(func->class_props->name);
        g_array_append_val(thread->stack_trace, func_name);
        result = func->class_props->ob_func(inner_args);
    }
    if (interpreter.error != RUN_ERROR) {
        g_array_remove_index(thread->stack_trace, thread->stack_trace->len - 1);
        free(func_name);
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
    object_t *dict = new_dict(NULL);
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

        printd("ASSIGNMENT RAN\n");
        if (result) {
            print_var("result", result);
            set_var(args, left_var, result);
        }
        return NULL;
    } else if (stmt->type == A_FOR) {
        atom_t *var_name = stmt->child;
        //object_t *var = get_global(interpreter, var_name->value);
        //if (var == NULL)
        //    return;
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
        while(item = object_call_func_obj(next_func, params)) {
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
                while ((tuple_item = object_call_func_obj(next_func, next_params)) && tuple_item_name != NULL) {
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
    } else if (stmt->type == A_GENFUNCDEF) {
        object_t *generatorfunc = new_generator_func(stmt, strdup(stmt->value));
        set_var(args, stmt, generatorfunc);
    } else if (stmt->type == A_CLASS) {
        atom_t *class_name = stmt;
        object_t *class = new_class(strdup(class_name->value));
        class->class_props->ob_func = new_object_instance;
        atom_t *inherits = stmt->child->child;
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

                object_t *class_func = new_user_func(field, trace_str, kwargs);
                object_add_field(class, field->value, class_func);
                printd("added class field func %s.%s\n", class_name->value, field->value);
            } else if (field->type == A_GENFUNCDEF) {
                char *trace_str;
                asprintf(&trace_str, "%s.%s", class_name->value, field->value);
                object_t *generatorfunc = new_generator_func(field, trace_str);
                object_add_field(class, field->value, generatorfunc);
            } else if (field->type == A_VAR) {
                object_t *result = interpret_expr(field->child, args, current_indent);
                object_add_field(class, field->value, result);
                printd("added class field %s.%s\n", class_name->value, field->value);
            } else {
                assert(FALSE);
            }
            field = field->next;
        }
        set_var(args, class_name, class);
        // TODO inherits CHAIN
        if (inherits != NULL) {
            object_t *parent_class = lookup_var(args, inherits);
            if (parent_class == NULL) {
                set_exception("Class does not exist: %s\n", inherits->value);
                return NULL;
            }
            class->class_props->inherits = parent_class;
        } else
            class->class_props->inherits = NULL;
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

