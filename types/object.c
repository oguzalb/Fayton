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
    object_t *res = object_call_func_obj(eq_func, params, 2);
    if (get_exception())
        return FALSE;
    if (res->type != BOOL_TYPE) {
        set_exception("Exception", "__eq__ should return bool type");
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

object_t *new_object_instance(object_t **args, int count) {
    object_t *object = (object_t *) malloc(sizeof(object_t));
    object->fields = g_hash_table_new(g_str_hash, g_str_equal);
    object->class = args[0];
    object->type = CUSTOMOBJECT_TYPE;
    object_t *init_func = object_get_field_no_check(object, "__init__");
    if (init_func != NULL) {
        args[0] = object;
        object_t *result = object_call_func_obj(init_func, args, count);
        if (result != new_none_internal()) {
            set_exception("Exception", "__init__() should return None");
            return NULL;
        }
    }
    return object;
}

object_t *object_call_func_obj(object_t *func, object_t **param_objs, int count) {
    object_t *result = NULL;
    if (func->type == USERFUNC_TYPE || func->type == GENERATORFUNC_TYPE) {
        result = interpret_funcblock(func->userfunc_props->ob_userfunc->child->next, param_objs, /* TODO */0);
    } else if (func->type == FUNC_TYPE) {
        result = func->func_props->ob_func(param_objs, count);
    } else {
        set_exception("Exception", "field should be a function.");
        return NULL;
    }
    return result;
}

object_t *object_call_func(object_t *object, object_t **args, int count, char *func_name) {
   object_t *func = object_get_field(object, func_name);
   if (get_exception())
       return NULL;
   return object_call_func_obj(func, args, count);
}

object_t *object_call_func_no_param(object_t *object, char *func_name) {
   object_t *func = object_get_field(object, func_name);
   if (get_exception())
       return NULL;
   object_t *params[2] = {object, NULL};
   return object_call_func_obj(func, params, 1);
}

object_t *object_call_repr(object_t *object) {
    object_t *item_str = object_call_func_no_param(object, "__repr__");
    if (get_exception())
        return NULL;
    if (item_str->type != STR_TYPE) {
        set_exception("Exception", "__repr__ should be a function returning string");
        return NULL;
    }
    return item_str;
}

object_t *object_repr(object_t **args, int count) {
    char* str;
    object_t *self = args[0];
    asprintf(&str, "<%s instance at %p>", self->class->class_props->name, self);
    return new_str_internal(str);
}

object_t *object_str(object_t **args, int count) {
    object_t *self = args[0];
    return object_call_repr(self);
}

object_t *object_and(object_t **args, int count) {
    object_t *self = args[0];
    object_t *other = args[1];
    object_t *self_bool = new_bool_internal(self);
    if (get_exception())
        return NULL;
    object_t *other_bool = new_bool_internal(other);
    if (get_exception())
        return NULL;
    return new_bool_from_int(self_bool->bool_props->ob_bval & other_bool->bool_props->ob_bval);
}

object_t *object_or(object_t **args, int count) {
    object_t *self = args[0];
    object_t *other = args[1];
    object_t *self_bool = new_bool_internal(self);
    if (get_exception())
        return NULL;
    object_t *other_bool = new_bool_internal(other);
    if (get_exception())
        return NULL;
    return new_bool_internal(self_bool->bool_props->ob_bval | other_bool->bool_props->ob_bval);
}

object_t *object_call_str(object_t *object) {
    object_t *item_str = object_call_func_no_param(object, "__str__");
    if (get_exception())
        return NULL;
    if (item_str->type != STR_TYPE) {
        set_exception("Exception", "__str__ should be a function returning string");
        return NULL;
    }
    return item_str;
}

object_t *object_equals(object_t **args, int count) {
    object_t *left = args[0];
    object_t *right = args[1];
    object_t *cmp_func = object_get_field_no_check(left, "__cmp__");
    if (cmp_func == NULL) {
        return new_bool_from_int(left == right);
    }
    object_t *params[3] = {left, right, NULL};
    object_t *int_result = object_call_func_obj(cmp_func, params, 2);
    if (get_exception())
        return NULL;
    return new_bool_from_int(int_result->int_props->ob_ival == 0);
}

object_t *object_get_field_from_class(object_t *class, char *name) {
    object_t *field = g_hash_table_lookup(class->fields, name);
    if (field != NULL)
        return field;
    printd("CLASS FIELDS: %s\n", class->class_props->name);
    g_hash_table_foreach(class->fields, print_var_each, NULL);
    if (class->class_props->inherits == NULL) {
        return NULL;
    }
    object_t **inherits = class->class_props->inherits;
    while (*inherits != NULL && field == NULL) {
        field = g_hash_table_lookup((*inherits)->fields, name);
        if (field == NULL)
            field = object_get_field_from_class(*inherits, name);
        inherits++;
    }
    return field;
}

object_t *object_set_field(object_t *object, char* field_name, object_t *value) {
    g_hash_table_insert(object->fields, field_name, value);
    return new_none_internal();
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
    field = object_get_field_from_class(object->class, name);
    if (field == NULL) {
        //printd("INHERITS FIELDS\n");
        //g_hash_table_foreach(object->class->class_props->inherits[0]->fields, print_var_each, NULL);
        return NULL;
    }
    printd("got field |%s|\n", name);
    return field;
}

object_t *object_get_field(object_t *object, char *name) {
    object_t *field = object_get_field_no_check(object, name);
    if (field == NULL) {
        set_exception("Exception", "Field not found %s\n", name);
        return NULL;
    }
    return field;
}

void object_add_field(object_t *object, char* name, object_t *field) {
    g_hash_table_insert(object->fields, name, field);
}

void init_object() {
    object_class = new_class(strdup("object"), NULL, new_object_instance, -1);
    object_add_field(object_class, "__str__", new_func(object_str, strdup("__str__"), 1));
    object_add_field(object_class, "__eq__", new_func(object_equals, strdup("__eq__"), 2));
    object_add_field(object_class, "__and__", new_func(object_and, strdup("__and__"), 2));
    object_add_field(object_class, "__or__", new_func(object_or, strdup("__or__"), 2));
    object_add_field(object_class, "__repr__", new_func(object_repr, strdup("__repr__"), 1));
    register_builtin(strdup("object"), object_class);
}
