#include "object.h"

object_t *object_class = NULL;

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

object_t *object_call_func_no_param(object_t *object, char *func_name) {
   object_t *func = object_get_field(object, func_name);
   if (interpreter.error == RUN_ERROR)
       return NULL;
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

object_t *object_str(GArray *args) {
   object_t *self = (object_t *)g_array_index(args, object_t *, 0);
   return object_call_repr(self);
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
    object_t *cmp_func = object_get_field(left, "__cmp__");
    if (interpreter.error == RUN_ERROR)
        return NULL;
    GArray *sub_args = g_array_new(TRUE, TRUE, sizeof(object_t *));
// TODO CUSTOM?
    g_array_append_val(sub_args, left);
    g_array_append_val(sub_args, right);
    printd("calling __cmp__\n");
    object_t *int_result = cmp_func->func_props->ob_func(sub_args);
    if (int_result == NULL) {
        interpreter.error = RUN_ERROR;
        return NULL;
    }
    object_t *cmp_result_equals_zero = new_int_internal(int_result->int_props->ob_ival == 0);
    return new_bool_internal(cmp_result_equals_zero);
}

object_t *object_get_field(object_t *object, char* name) {
    printd("getting field |%s|\n", name);
    object_t *field = g_hash_table_lookup(object->fields, name);
    if (field != NULL) { 
        return field;
    }
    printd("OBJECT FIELDS\n");
    g_hash_table_foreach(object->fields, print_var_each, NULL);
    if (object->class == NULL) {
        interpreter.error = RUN_ERROR;
        set_exception("Field not found %s\n", name);
        return NULL;
    }
    field = g_hash_table_lookup(object->class->fields, name);
    if (field != NULL)
        return field;
    printd("CLASS FIELDS\n");
    g_hash_table_foreach(object->class->fields, print_var_each, NULL);
    if (object->class->class_props->inherits == NULL) {
        interpreter.error = RUN_ERROR;
        set_exception("Field not found %s\n", name);
        return NULL;
    }
    field = g_hash_table_lookup(object->class->class_props->inherits->fields, name);
    if (field == NULL) {
        printd("INHERITS FIELDS\n");
        g_hash_table_foreach(object->class->class_props->inherits->fields, print_var_each, NULL);
        interpreter.error = RUN_ERROR;
        set_exception("Field not found %s\n", name);
    }
    printd("got field |%s|\n", name);
    
    return field;
}

void object_add_field(object_t *object, char* name, object_t *field) {
    g_hash_table_insert(object->fields, name, field);
}

void init_object() {
    object_class = new_class(strdup("object"));
    object_class->class_props->ob_func = new_object_instance;
    object_add_field(object_class, "__str__", new_func(object_str, strdup("__str__")));
    register_global(strdup("object"), object_class);
}
