#include "interpret.h"

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
        case 11: return "BOOL";
        case 12: return "NONE";
        case 13: return "SLICE";
        default: return "UNDEFINED";
    }
    assert(FALSE);
}

#define FUNC_STRUCT_TYPE(x) object_t *(*x)(GArray *)

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
        interpreter.error = RUN_ERROR;
        return NULL;
    }
    printd("Creating the class %s\n", name);
    cls = new_object(CLASS_TYPE);
    cls->class_props = malloc(sizeof(struct class_type));
    cls->class_props->inherits = NULL;
    cls->class_props->name = name;
    return cls;
}

void register_global(char* name, object_t *object) {
    g_hash_table_insert(interpreter.globals, name, object);
}

void register_builtin_func(char* name, object_t *object) {
    g_hash_table_insert(interpreter.globals, name, object);
}

// TODO after closures these will change
object_t *get_var(GHashTable *context, char *name) {
    object_t *var = g_hash_table_lookup(context, name);
    if (var != NULL)
        return var;
    var = g_hash_table_lookup(interpreter.globals, (void *)name);
    if (var == NULL) {
        interpreter.error = RUN_ERROR;
        g_hash_table_foreach(interpreter.globals, print_var_each, NULL);
        set_exception("Global not found |%s|\n", name);
        return NULL;
    }
    return var;
}

object_t *get_global(char*name) {
    object_t *cls = g_hash_table_lookup(interpreter.globals, name);
    if (cls == NULL) {
        interpreter.error = RUN_ERROR;
        g_hash_table_foreach(interpreter.globals, print_var_each, NULL);
        set_exception("Global not found |%s|\n", name);
        return NULL;
    }
    return cls;
}
object_t *get_global_no_check(char*name) {
    return g_hash_table_lookup(interpreter.globals, name);
}

object_t *new_func(object_t *(*func)(GArray *), char *name) {
    object_t *func_obj = new_object(FUNC_TYPE);
    func_obj->func_props = malloc(sizeof(struct func_type));
    func_obj->func_props->ob_func = func;
    func_obj->func_props->name = name;
    func_obj->class = NULL;
    return func_obj;
}

object_t *new_user_func(atom_t *func, char* name) {
    object_t *func_obj = new_object(USERFUNC_TYPE);
    func_obj->userfunc_props = malloc(sizeof(struct userfunc_type));
    func_obj->userfunc_props->ob_userfunc = func;
    func_obj->userfunc_props->name = name;
    func_obj->class = NULL;
    return func_obj;
}

gboolean object_equal(gconstpointer a, gconstpointer b) {
    // TODO should call equals function for different types
    object_t *aobj = (object_t *)a;
    object_t *bobj = (object_t *)b;
    object_t *eq_func = object_get_field(aobj, "__eq__");
    if (eq_func == NULL)
        return g_direct_equal(a, b);
    GArray *sub_args = g_array_new(TRUE, TRUE, sizeof(object_t *));
    g_array_append_val(sub_args, aobj);
    g_array_append_val(sub_args, bobj);
    object_t *equals = object_equals(sub_args);
    return TRUE;
}

guint object_hash(gconstpointer key) {
    // TODO should call hash function for different types
    object_t *object = (object_t *)key;
    return g_int_hash(&object->int_props->ob_ival);
}

object_t *sum_func(GArray *args) {
    object_t *iterable = g_array_index(args, object_t*, 0);
    object_t *iter_func = object_get_field(iterable, "__iter__");
    if (interpreter.error == RUN_ERROR)
        return NULL;
    object_t *iterator;
    if (iter_func->type == USERFUNC_TYPE) {
        GHashTable *sub_context = g_hash_table_new(g_str_hash, g_str_equal);
        atom_t *param_name = iter_func->userfunc_props->ob_userfunc->child->child;
        g_hash_table_insert(sub_context, param_name->value, iterable);
        iterator = interpret_block(iter_func->userfunc_props->ob_userfunc->child->next, sub_context, /* TODO */0);
    } else {
        GArray *args = g_array_new(TRUE, TRUE, sizeof(object_t *));
        g_array_append_val(args, iterable);
        iterator = iter_func->func_props->ob_func(args);
    }
    object_t *next_func = object_get_field(iterator, "next");
    if (interpreter.error == RUN_ERROR)
        return NULL;
    object_t *int_obj = NULL;
    int sum = 0;
    do {
        if (next_func->type == USERFUNC_TYPE) {
            GHashTable *sub_context = g_hash_table_new(g_str_hash, g_str_equal);
            atom_t *param_name = next_func->userfunc_props->ob_userfunc->child->child;
            g_hash_table_insert(sub_context, param_name->value, iterator);
            int_obj = interpret_block(next_func->userfunc_props->ob_userfunc->child->next, sub_context, /* TODO */ 0);
        } else {
            GArray *args = g_array_new(TRUE, TRUE, sizeof(object_t *));
            g_array_append_val(args, iterator);
            int_obj = next_func->func_props->ob_func(args);
        }
        assert(int_obj != NULL?int_obj->type == INT_TYPE:TRUE);
        // TODO __add__
        if (int_obj != NULL)
            sum += int_obj->int_props->ob_ival;
    } while (int_obj != NULL);

    object_t *result = new_int_internal(sum);
    // TODO CHECKS
    return result;
}

object_t *range_func(GArray *args) {
    object_t *min = g_array_index(args, object_t*, 0);
    object_t *max = g_array_index(args, object_t*, 1);
    if (min->type != INT_TYPE || min->type != INT_TYPE) {
        interpreter.error = RUN_ERROR;
        return NULL;
    }
    object_t *list = new_list(NULL);
    int i;
    for (i=min->int_props->ob_ival; i<max->int_props->ob_ival; i++) {
        list_append_internal(list, new_int_internal(i));
    }
    return list;
}

object_t *print_func(GArray *args) {
    // TODO CHECKS AND FULL PRINT
    object_t *var = g_array_index(args, object_t*, 0);
    if (var->type == INT_TYPE) {
        printd("PRINT INT\n");
        printf("%d\n", var->int_props->ob_ival);
    } else if (var->type == STR_TYPE) {
        printf("%s\n", var->str_props->ob_sval->str);
    } else if (var->type == BOOL_TYPE) {
        printf("%s\n", var->bool_props->ob_bval ? "True":"False");
    } else if (var->type == NONE_TYPE) {
        printf("None\n");
    } else {
        printf("Print got %d!?!", var->type);
        assert(FALSE);
    }
}

struct py_thread *new_thread_struct() {
    struct py_thread *thread = malloc(sizeof(struct py_thread));
    thread->stack_trace = g_array_new(TRUE, TRUE, sizeof(char *));
    return thread;
}

void init_interpreter() {
    interpreter.error = 0;
    interpreter.last_accessed = NULL;
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

    register_global(strdup("print"), new_func(print_func, strdup("print")));
}

object_t *lookup_var(GHashTable *context, char* name) {
    printd("looking up for var %s\n", name);
    object_t *object = g_hash_table_lookup(context, name);
    if (object == NULL)
        object = g_hash_table_lookup(interpreter.globals, name);
    if (object == NULL) {
        printd("globals\n");
        g_hash_table_foreach(interpreter.globals, print_var_each, NULL);
        printd("context\n");
        g_hash_table_foreach(interpreter.globals, print_var_each, NULL);
    }
    if (object == NULL) {
        interpreter.error = RUN_ERROR;
        set_exception("NameError %s not found\n", name);
        return NULL;
    }
    return object;
}

object_t *interpret_expr(atom_t *, GHashTable *, int);
object_t *interpret_funccall(atom_t *func_call, GHashTable *context, int current_indent) {
    object_t *func = interpret_expr(func_call->child, context, current_indent);
    if (interpreter.last_accessed)
        print_var("last", interpreter.last_accessed);
    if (func == NULL) {
        printd("FUNC NOT FOUND |%s|\n", func_call->value);
        interpreter.error = RUN_ERROR;
        return NULL;
    }
    if (func->type != FUNC_TYPE && func->type != USERFUNC_TYPE && func->type != CLASS_TYPE) {
        set_exception("OBJ IS NOT CALLABLE %s\n", object_type_name(func->type));
        interpreter.error = RUN_ERROR;
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
        } else if (func_call->child->type == A_ACCESSOR && interpreter.last_accessed != NULL) {
            print_var("adding param", interpreter.last_accessed);
            g_array_append_val(args, interpreter.last_accessed);
        }
        while (param) {
            object_t *value = interpret_expr(param, context, current_indent);
            if (interpreter.error == RUN_ERROR) {
                return NULL;
            }
            printd("ADDING ARGUMENT %s\n", param->value);
            print_var("", value);
            g_array_append_val(args, value);
            param = param->next;
        }
    } else if (func->type == USERFUNC_TYPE) {
        atom_t *param = func_call->child->next->child;
        atom_t *param_name = func->userfunc_props->ob_userfunc->child->child;
        if (func_call->child->type == A_ACCESSOR && interpreter.last_accessed != NULL) {
            print_var("ADDING SELF PARAM", interpreter.last_accessed);
            g_hash_table_insert(sub_context, param_name->value, interpreter.last_accessed);
            param_name = param_name->next;
        }
        while (param_name) {
            if (param == NULL) {
                interpreter.error = RUN_ERROR;
                printd("Lesser parameter passed than needed, next: %s\n", param_name->value);
                return NULL;}
            if (param_name == NULL) {
                interpreter.error = RUN_ERROR;
                printd("More parameter passed than needed next: %s\n", param->value);
                return NULL;}
            object_t *value = interpret_expr(param, context, current_indent);
            if (interpreter.error == RUN_ERROR) {
                return NULL;
            }
            g_hash_table_insert(sub_context, param_name->value, value);
            param = param->next;
            param_name = param_name->next;
            // TODO check if they match
        }
    }
    printd("ADDED PARAMS\n");
    printd("calling func type %s\n", object_type_name(func->type));
    struct py_thread *main_thread = g_array_index(interpreter.threads, struct py_thread *,0);
    char* func_name;
    object_t *result;
    if (func->type == USERFUNC_TYPE) {
        func_name = strdup(func->userfunc_props->name);
        g_array_append_val(main_thread->stack_trace, func_name);
        result = interpret_block(func->userfunc_props->ob_userfunc->child->next, sub_context, current_indent);
    } else if (func->type == FUNC_TYPE) {
        func_name = strdup(func->func_props->name);
        g_array_append_val(main_thread->stack_trace, func_name);
        result = func->func_props->ob_func(args);
    } else if (func->type == CLASS_TYPE) {
        func_name = strdup(func->class_props->name);
        g_array_append_val(main_thread->stack_trace, func_name);
        result = func->class_props->ob_func(args);
    }
    if (interpreter.error != RUN_ERROR) {
        g_array_remove_index(main_thread->stack_trace, main_thread->stack_trace->len - 1);
        free(func_name);
    }
    return result;
}

object_t *interpret_list(atom_t *expr, GHashTable *context, int current_indent) {
    object_t *list = new_list_internal();
    atom_t *elem = expr->child;
    while (elem != NULL) {
        object_t *result = interpret_expr(elem, context, current_indent);
        if (interpreter.error) {
            return NULL;
        }
        list_append_internal(list, result);
        elem = elem->next;
    }
    return list;
}

object_t *interpret_dict(atom_t *expr, GHashTable *context, int current_indent) {
    object_t *dict = new_dict(NULL);
    atom_t *elem = expr->child;
    while (elem != NULL) {
        object_t *key = interpret_expr(elem, context, current_indent);
        if (interpreter.error) {
            return dict;
        }
        object_t *value = interpret_expr(elem->next, context, current_indent);
        if (interpreter.error) {
            return dict;
        }
        g_hash_table_insert(dict->dict_props->ob_dval, key, value);
        elem = elem->next->next;
    }
    return dict;
}

object_t *interpret_if(atom_t *expr, GHashTable *context, int current_indent) {
    atom_t *if_block = expr->child;
    while (if_block) {
        if (if_block->type == A_FUNCCALL) {
            object_t *bool_obj = interpret_expr(if_block, context, current_indent);
            if (interpreter.error == RUN_ERROR)
                return NULL;
            if (bool_obj->type != BOOL_TYPE) {
                set_exception("NOT A BOOL TYPE\n");
                interpreter.error = RUN_ERROR;
                return NULL;
            }
            if_block = if_block->next;
            assert(if_block != NULL);
            if (bool_obj->bool_props->ob_bval == TRUE) {
printd("HEREEEE TRUE!!!\n");
                return interpret_block(if_block, context, current_indent);
            }
        } else if (if_block->type == A_BLOCK) {
printd("HEREEEE FALSE!!!\n");
            return interpret_block(if_block, context, current_indent);
        }
        if_block = if_block->next;
    }
    return NULL;
}

object_t *interpret_expr(atom_t *expr, GHashTable *context, int current_indent) {
// TODO
    printd("FIRST_TYPE %s\n", atom_type_name(expr->type));
    if (expr->type == A_VAR) {
        printd("VAR %s\n", expr->value);
        object_t *value = get_var(context, expr->value);
        if (value)
            print_var(expr->value, value);
        return value;
    } else if (expr->type == A_FUNCCALL) {
        printd("CALL FUNC \n");
        // TODO more than one thread and stack trace
        return interpret_funccall(expr, context, current_indent);
    } else if (expr->type == A_INTEGER) {
        object_t *int_val = new_int_internal(atoi(expr->value));
        printd("NEW INT %d\n", int_val->int_props->ob_ival);
        return int_val;
    } else if (expr->type == A_STRING) {
        object_t *str_val = new_str_internal(expr->value);
        return str_val;
    } else if (expr->type == A_LIST) {
        printd("NEW LIST %s\n", expr->value);
        object_t *list = interpret_list(expr, context, current_indent);
        return list;
    } else if (expr->type == A_DICTIONARY) {
        printd("NEW DICTIONARY %s\n", expr->value);
        object_t *dict = interpret_dict(expr, context, current_indent);
        return dict;
    } else if (expr->type == A_ACCESSOR) {
        printd("ACCESSING %s\n", expr->child->value);
        object_t *object;
        g_hash_table_foreach(context, print_var_each, NULL);
        if (expr->child->type == A_VAR) {
            printd("%s\n", expr->child->value);
            object = lookup_var(context, expr->child->value);
            if (interpreter.error == RUN_ERROR) {
                return NULL;
            }
            print_var("object", object);
            printd("getting field %s of %s\n", expr->child->next->value, expr->child->value);
        } else
            object = interpret_expr(expr->child, context, current_indent);
        if (object == NULL)
            return NULL;
        interpreter.last_accessed = object;
        printd("last accessed %p\n", object);
        object_t *field = object_get_field(object, expr->child->next->value);
        if (interpreter.error == RUN_ERROR)
            return NULL;
        return field;
    } else {
        interpreter.error = RUN_ERROR;
        set_exception("TYPE INCORRECT %s\n", atom_type_name(expr->type));
        return NULL;
    }
}

object_t *interpret_stmt(atom_t *stmt, GHashTable *context, int current_indent) {
    if (stmt->type == A_FUNCCALL) {
        interpret_expr(stmt, context, current_indent);
    } else if (stmt->type == A_ASSIGNMENT) {
        atom_t *left_var = stmt->child;
        printd("ASSIGNING TO %s\n", left_var->value);
        object_t *result = interpret_expr(left_var->next, context, current_indent);
        if (interpreter.error == RUN_ERROR)
            return NULL;
        if (result == NULL) {
            printd("GOT NOTHING, can't assign\n");
            interpreter.error = RUN_ERROR;
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
        object_t *iterable = interpret_expr(expr, context, current_indent);
        if (interpreter.error == RUN_ERROR)
            return NULL;
        object_t *iter_func = object_get_field(iterable, "__iter__");
        if (interpreter.error == RUN_ERROR)
            return NULL;
        GArray *args = g_array_new(TRUE, TRUE, sizeof(object_t *));
        g_array_append_val(args, iterable);
        printd("calling __iter__\n");
        object_t *iterator = iter_func->func_props->ob_func(args);
        assert(iterator->class != NULL);
        printd("calling __iter__ END\n");
        atom_t *block = expr->next;
        object_t *next_func = object_get_field(iterator, "next");
        if (interpreter.error == RUN_ERROR)
            return NULL;
        g_array_remove_index(args, 0);
        g_array_append_val(args, iterator);
        object_t *item;
        while(item = next_func->func_props->ob_func(args)) {
            register_global(var_name->value, item);
            interpret_block(block, context, current_indent);
            if (interpreter.error == RUN_ERROR) {
                set_exception("ERROR OCCURED WHILE FOR STMTS\n");
                return NULL;
            }
        }
    } else if (stmt->type == A_IF) { 
printd("A_IF\n");
        return interpret_if(stmt, context, current_indent);
    } else if (stmt->type == A_FUNCDEF) {
        object_t *userfunc = new_user_func(stmt, stmt->value);
        register_global(stmt->value, userfunc);
    } else if (stmt->type == A_CLASS) {
        atom_t *class_name = stmt;
        object_t *class = new_class(strdup(class_name->value));
        class->class_props->ob_func = new_object_instance;
        atom_t *inherits = stmt->child->child;
        atom_t *field = stmt->child->next;
        while (field) {
            if (field->type == A_FUNCDEF) {
                object_t *class_func = new_user_func(field, field->value);
                object_add_field(class, field->value, class_func);
                printd("added class field func %s.%s\n", class_name->value, field->value);
            } else if (field->type == A_VAR) {
                object_t *result = interpret_expr(field->child, context, current_indent);
                object_add_field(class, field->value, result);
                printd("added class field %s.%s\n", class_name->value, field->value);
            } else {
                assert(FALSE);
            }
            field = field->next;
        }
        register_global(strdup(class_name->value), class);
        // TODO inherits CHAIN
        if (inherits != NULL) {
            object_t *parent_class = get_var(context, inherits->value);
            if (parent_class == NULL) {
                interpreter.error = RUN_ERROR;
                set_exception("Class does not exist: %s\n", inherits->value);
                return NULL;
            }
            class->class_props->inherits = parent_class;
        } else
            class->class_props->inherits = NULL;
    } else if (stmt->type == A_RETURN) {
        printd("returning something\n");
        if (stmt->child)
            return interpret_expr(stmt->child, context, current_indent);
        else
            return new_none_internal();
    }
    return NULL;
}

object_t *interpret_block(atom_t *block, GHashTable *context, int current_indent) {
    interpreter.error = 0;
    printd("interpreting block\n");
    if (block->type != A_BLOCK) {
        interpreter.error = RUN_ERROR;
        set_exception("NOT A BLOCK\n");
        return NULL;
    }
    atom_t *stmt = block->child;
    if (stmt == NULL)
        return new_none_internal();
    object_t *last_result;
    do {
        object_t *ret = interpret_stmt(stmt, context, current_indent);
        if (interpreter.error == RUN_ERROR)
            return NULL;
        if (ret != NULL)
            return ret;
    } while (stmt = stmt->next);
    return new_none_internal();
}
