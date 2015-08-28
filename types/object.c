#include "object.h"

object_t *object_class = NULL;

object_t *object_call_func_obj(object_t *func, object_t **param_objs);

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
        interpreter.error == RUN_ERROR;
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
    object_t *hash_obj = object_call_func_obj_no_param(object, hash_func);
    return hash_obj->int_props->ob_ival;
}

object_t *new_object(int type) {
    object_t *object = (object_t *) malloc(sizeof(object_t));
    object->fields = g_hash_table_new(g_str_hash, g_str_equal);
    object->type = type;
    return object;
}

object_t *new_object_instance(GArray *args) {
    object_t *object = (object_t *) malloc(sizeof(object_t));
    object->fields = g_hash_table_new(g_str_hash, g_str_equal);
    object->class = (object_t *)g_array_index(args, object_t *, 0);
    object->type = CUSTOMOBJECT_TYPE;
    return object;
}

object_t *object_call_func_obj(object_t *func, object_t **param_objs) {
   object_t *result;
   if (func->type == USERFUNC_TYPE || func->type == GENERATORFUNC_TYPE) {
        set_exception("Not supported yet\n");
        interpreter.error = RUN_ERROR;
        return NULL;
    } else if (func->type == FUNC_TYPE) {
        GArray *params = g_array_new(TRUE, TRUE, sizeof(object_t *));
        while (*param_objs != NULL) {
            g_array_append_val(params, (*param_objs));
            param_objs++;
        }
        result = func->func_props->ob_func(params);
        g_array_free(params, FALSE);
    } else {
        set_exception("field should be a function.");
        interpreter.error = RUN_ERROR;
        return NULL;
    }
    return result;
}

object_t *object_call_func_obj_no_param(object_t *object, object_t *func) {
   object_t *result;
   if (func->type == USERFUNC_TYPE || func->type == GENERATORFUNC_TYPE) {
        GHashTable *sub_context = g_hash_table_new(g_str_hash, g_str_equal);
        g_hash_table_insert(sub_context, "self", object);
        result = interpret_funcblock(func->userfunc_props->ob_userfunc->child->next, sub_context, 0);
        g_hash_table_destroy(sub_context);
    } else if (func->type == FUNC_TYPE) {
        GArray *params = g_array_new(TRUE, TRUE, sizeof(object_t *));
        g_array_append_val(params, object);
        result = func->func_props->ob_func(params);
        g_array_free(params, FALSE);
    } else {
        set_exception("field should be a function.");
        interpreter.error = RUN_ERROR;
        return NULL;
    }
    return result;
}

object_t *object_call_func_no_param(object_t *object, char *func_name) {
   object_t *func = object_get_field(object, func_name);
   if (interpreter.error == RUN_ERROR)
       return NULL;
   return object_call_func_obj_no_param(object, func);
}

object_t *object_call_repr(object_t *object) {
    object_t *item_str = object_call_func_no_param(object, "__repr__");
    if (interpreter.error == RUN_ERROR)
        return NULL;
    if (item_str->type != STR_TYPE) {
        set_exception("__repr__ should be a function returning string");
        interpreter.error = RUN_ERROR;
        return NULL;
    }
    return item_str;
}

object_t *object_repr(GArray *args) {
    char* str;
    object_t *self = (object_t *)g_array_index(args, object_t *, 0);
    asprintf(&str, "< %s instance %p >", self->class_props->name, self);
    return new_str_internal(str);
}

object_t *object_str(GArray *args) {
    object_t *self = (object_t *)g_array_index(args, object_t *, 0);
    return object_call_repr(self);
}

object_t *object_and(GArray *args) {
    object_t *self = (object_t *)g_array_index(args, object_t *, 0);
    object_t *other = (object_t *)g_array_index(args, object_t *, 1);
    object_t *self_bool = new_bool_internal(self);
    if (interpreter.error == RUN_ERROR)
        return NULL;
    object_t *other_bool = new_bool_internal(other);
    if (interpreter.error == RUN_ERROR)
        return NULL;
    return new_bool_from_int(self_bool->bool_props->ob_bval & other_bool->bool_props->ob_bval);
}

object_t *object_or(GArray *args) {
    object_t *self = (object_t *)g_array_index(args, object_t *, 0);
    object_t *other = (object_t *)g_array_index(args, object_t *, 1);
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
        interpreter.error = RUN_ERROR;
        return NULL;
    }
    return item_str;
}

object_t *object_equals(GArray *args) {
    object_t *left = g_array_index(args, object_t*, 0);
    object_t *right = g_array_index(args, object_t*, 1);
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
        interpreter.error = RUN_ERROR;
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
