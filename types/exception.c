#include "exception.h"

object_t *new_exception(object_t **args, int count) {
    object_t *message = args[1];
    if (message->type != STR_TYPE) {
        set_exception("TypeError", "Exception message should be a string\n");
    }
    object_t *exception_obj = new_object(EXCEPTION_TYPE);
    exception_obj->class = args[0];
    exception_obj->exception_props = malloc(sizeof(struct exception_type));
    exception_obj->exception_props->thread = get_thread();
    object_add_field(exception_obj, "message", message);
    return exception_obj;
}

object_t *exception_str(object_t **args, int count) {
    object_t *self = args[0];
    return object_get_field(self, "message");
}

void init_exception() {
    object_t *exception_class = new_class(strdup("Exception"), NULL, new_exception, 2);
    object_add_field(exception_class, "__str__", new_func(exception_str, strdup("__str__"), 1));
    register_builtin(strdup("Exception"), exception_class);

    object_t **exc_inherits[2] = {exception_class, NULL};
    object_t *assert_error_class = new_class(strdup("AssertionError"), exc_inherits, NULL, 2);
    register_builtin(strdup("AssertionError"), assert_error_class);
    object_t *import_error_class = new_class(strdup("ImportError"), exc_inherits, NULL, 2);
    register_builtin(strdup("ImportError"), import_error_class);
    object_t *type_error_class = new_class(strdup("TypeError"), exc_inherits, NULL, 2);
    register_builtin(strdup("TypeError"), type_error_class);
    object_t *not_implemented_error_class = new_class(strdup("NotImplementedError"), exc_inherits, NULL, 2);
    register_builtin(strdup("NotImplementedError"), not_implemented_error_class);
}
