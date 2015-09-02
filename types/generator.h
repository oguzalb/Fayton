#ifndef FAY_GENERATOR_H
#include "../interpret.h"

void init_generator();
object_t *new_generator_internal(object_t **args, object_t* run_func);


#define FAY_GENERATOR_H
#endif
