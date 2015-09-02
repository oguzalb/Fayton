#include "object.h"

object_t *object_class = NULL;

gboolean object_equal(gconstpointer a, gconstpointer b) {
    // TODO should call equals function for different types
    object_t *aobj = (object_t *)a;
    object_t *bobj = (object_t *)b;
    object_t *eq_func = object_get_field_no_check(aobj, "__eq__");
    if (eq_func == NULL)
        return g_direct_equal(a, b);
    object_t *params[3] = {aobj, bobj, NULL};
    object_t *res = object_call_func_obj(eq_func, params);
    if (interpreter.error == RUN_ERROR)
        return FALSE;
    if (res->type != BOOL_TYPE) {
        set_exception("__eq__ should return bool type");
        return FALSE;
    }
    return res->bool_props->ob_bval;
}

guint object_hash(gconstpointer key) {
    // TODO should call hash function for different types
    object_t *object = (object_t *)key;
    object_t *hash_func = object_get_field_no_check(object, "__hash__");
    if (hash_func == NULL)
        return g_direct_hash(object);
    object_t *hash_obj = object_call_func_no_param(object, "__hash__");
    return hash_obj->int_props->ob_ival;
}

object_t *new_object(int type) {
    object_t *object = (object_t *) malloc(sizeof(object_t));
    object->fields = g_hash_table_new(g_str_hash, g_str_equal);
    object->type = type;
    return object;
}

object_t *new_object_instance(object_t **args) {
    if (args_len(args) != 1) {
        set_exception("An argument expected\n");
        return NULL;
    }
    object_t *object = (object_t *) malloc(sizeof(object_t));
    object->fields = g_hash_table_new(g_str_hash, g_str_equal);
    object->class = args[0];
    object->type = CUSTOMOBJECT_TYPE;
    return object;
}

object_t *object_call_func_obj(object_t *func, object_t **param_objs) {
    object_t *result = NULL;
    if (func->type == USERFUNC_TYPE || func->type == GENERATORFUNC_TYPE) {
        result = interpret_funcblock(func->userfunc_props->ob_userfunc->child->next, param_objs, /* TODO */0);
    } else if (func->type == FUNC_TYPE) {
        result = func->func_props->ob_func(param_objs);
    } else {
        set_exception("field should be a function.");
        return NULL;
    }
    return result;
}

object_t *object_call_func_no_param(object_t *object, char *func_name) {
   object_t *func = object_get_field(object, func_name);
   if (interpreter.error == RUN_ERROR)
       return NULL;
   object_t *params[2] = {object, NULL};
   return object_call_func_obj(func, params);
}

object_t *object_call_repr(object_t *object) {
    object_t *item_str = object_call_func_no_param(object, "__repr__");
    if (interpreter.error == RUN_ERROR)
        return NULL;
    if (item_str->type != STR_TYPE) {
        set_exception("__repr__ should be a function returning string");
        return NULL;
    }
    return item_str;
}

object_t *object_repr(object_t **args) {
    if (args_len(args) != 1) {
        set_exception("An argument expected\n");
        return NULL;
    }
    char* str;
    object_t *self = args[0];
    asprintf(&str, "< %s instance %p >", self->class_props->name, self);
    return new_str_internal(str);
}

object_t *object_str(object_t **args) {
    if (args_len(args) != 1) {
        set_exception("An argument expected\n");
        return NULL;
    }
    object_t *self = args[0];
    return object_call_repr(self);
}

object_t *object_and(object_t **args) {
    if (args_len(args) != 2) {
        set_exception("Two arguments expected\n");
        return NULL;
    }
    object_t *self = args[0];
    object_t *other = args[1];
    object_t *self_bool = new_bool_internal(self);
    if (interpreter.error == RUN_ERROR)
        return NULL;
    object_t *other_bool = new_bool_internal(other);
    if (interpreter.error == RUN_ERROR)
        return NULL;
    return new_bool_from_int(self_bool->bool_props->ob_bval & other_bool->bool_props->ob_bval);
}

object_t *object_or(object_t **args) {
    if (args_len(args) != 2) {
        set_exception("Two arguments expected\n");
        return NULL;
    }
    object_t *self = args[0];
    object_t *other = args[1];
    object_t *self_bool = new_bool_internal(self);
    if (interpreter.error == RUN_ERROR)
        return NULL;
    object_t *other_bool = new_bool_internal(other);
    if (interpreter.error == RUN_ERROR)
        return NULL;
    return new_bool_internal(self_bool->bool_props->ob_bval | other_bool->bool_props->ob_bval);
}

object_t *object_call_str(object_t *object) {
    object_t *item_str = object_call_func_no_param(object, "__str__");
    if (interpreter.error == RUN_ERROR)
        return NULL;
    if (item_str->type != STR_TYPE) {
        set_exception("__str__ should be a function returning string");
        return NULL;
    }
    return item_str;
}

object_t *object_equals(object_t **args) {
    if (args_len(args) != 2) {
        set_exception("Two arguments expected\n");
        return NULL;
    }
    object_t *left = args[0];
    object_t *right = args[1];
    object_t *cmp_func = object_get_field_no_check(left, "__cmp__");
    if (cmp_func == NULL) {
        return new_bool_from_int(left == right);
    }
    object_t *params[3] = {left, right, NULL};
    object_t *int_result = object_call_func_obj(cmp_func, params);
    if (interpreter.error == RUN_ERROR)
        return NULL;
    return new_bool_from_int(int_result->int_props->ob_ival == 0);
}

object_t *object_get_field_no_check(object_t *object, char* name) {
    printd("getting field |%s|\n", name);
    object_t *field = g_hash_table_lookup(object->fields, name);
    if (field != NULL) { 
        return field;
    }
    printd("OBJECT FIELDS\n");
    g_hash_table_foreach(object->fields, print_var_each, NULL);
    if (object->class == NULL) {
        return NULL;
    }
    field = g_hash_table_lookup(object->class->fields, name);
    if (field != NULL)
        return field;
    printd("CLASS FIELDS: %s\n", object->class->class_props->name);
    g_hash_table_foreach(object->class->fields, print_var_each, NULL);
    if (object->class->class_props->inherits == NULL) {
        return NULL;
    }
    field = g_hash_table_lookup(object->class->class_props->inherits->fields, name);
    if (field == NULL) {
        printd("INHERITS FIELDS\n");
        g_hash_table_foreach(object->class->class_props->inherits->fields, print_var_each, NULL);
        return NULL;
    }
    printd("got field |%s|\n", name);
    return field;
}

object_t *object_get_field(object_t *object, char *name) {
    object_t *field = object_get_field_no_check(object, name);
    if (field == NULL) {
        set_exception("Field not found %s\n", name);
        return NULL;
    }
    return field;
}

void object_add_field(object_t *object, char* name, object_t *field) {
    g_hash_table_insert(object->fields, name, field);
}

void init_object() {
    object_class = new_class(strdup("object"));
    object_class->class_props->ob_func = new_object_instance;
    object_add_field(object_class, "__str__", new_func(object_str, strdup("__str__")));
    object_add_field(object_class, "__eq__", new_func(object_equals, strdup("__eq__")));
    object_add_field(object_class, "__and__", new_func(object_and, strdup("__and__")));
    object_add_field(object_class, "__or__", new_func(object_or, strdup("__or__")));
    object_add_field(object_class, "__repr__", new_func(object_repr, strdup("__repr__")));
    register_global(strdup("object"), object_class);
}
