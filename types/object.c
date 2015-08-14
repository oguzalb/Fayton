#include "object.h"

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
    if (object->class == NULL) { 
        assert(FALSE);
    }
    printd("OBJECT FIELDS\n");
    g_hash_table_foreach(object->fields, print_var_each, NULL);
    field = g_hash_table_lookup(object->class->fields, name);
    if (field != NULL)
        return field;
    else if (object->class->class_props->inherits == NULL) {
        interpreter.error = RUN_ERROR;
        printf("Field not found %s\n", name);
        assert(FALSE);
    }
    printd("CLASS FIELDS\n");
    g_hash_table_foreach(object->class->fields, print_var_each, NULL);
    field = g_hash_table_lookup(object->class->class_props->inherits->fields, name);
    if (field == NULL) {
        printd("INHERITS FIELDS\n");
        g_hash_table_foreach(object->class->class_props->inherits->fields, print_var_each, NULL);
        interpreter.error = RUN_ERROR;
        printf("Field not found %s\n", name);
        assert(FALSE);
    }
    printd("got field |%s|\n", name);
    
    return field;
}

void object_add_field(object_t *object, char* name, object_t *field) {
    g_hash_table_insert(object->fields, name, field);
}

void init_object() {
    object_t *object_class = new_class(strdup("object"));
    object_class->class_props->ob_func = new_object_instance;
    register_global(strdup("object"), object_class);
}
