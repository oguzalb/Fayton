#ifndef parse_python_h
#define parse_python_h
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <glib.h>

#ifdef DEBUG
#define printd(fmt, args...) \
            printf(fmt, ##args)
#else
#define printd(fmt, ...)
#endif

#define PARSE_ERROR 31
#define A_VAR 1
#define A_INTEGER 2
#define A_ASSIGNMENT 3
#define A_FUNC 4
#define A_ARGLIST 5
#define A_RETURN 6
#define A_STMT 7
#define A_BLOCK 8
#define A_MODULE 9
#define A_FUNCCALL 10
#define A_CALLPARAMS 11
#define A_EXPR 12
#define A_CLASS 13
#define A_ACCESSOR 14
#define A_WHILE 15
#define A_PHAR 16

#define A_PLUS 17
#define A_MINUS 18
#define A_MULTIPLY 19
#define A_DIVIDE 20
#define A_AND 21
#define A_OR 22
#define A_EQUALSEQUALS 23
#define A_FOR 24
#define A_PARAMS 25
#define A_FUNCDEF 26
#define A_LIST 27
#define A_DICTIONARY 28
#define A_SHIFTR 29
#define A_SHIFTL 30
#define A_STRING 31
#define A_IF 32
#define A_YIELD 33
#define A_GENFUNCDEF 34
#define A_SLICE 35
#define A_TUPLE 36

typedef struct _atom_t {
    char *value;
    long type;
// this will be refactored
    int cl_index;
// this will be refactored
// for functions
    GHashTable *context;
    struct _atom_t *child;
    struct _atom_t *next;
} atom_t;

typedef struct {
    atom_t *root;
} atom_tree_t;

typedef struct _freevar_t {
// this will be refactored
    int type;
    atom_t *function;
    int cl_index;
} freevar_t;

struct t_token {
    char* value;
    int type;
    int line;
};

struct t_tokenizer {
    struct t_token** iter;
    struct t_token** tokens;
    int current_line;
    int error;
    int cl_index;
    GArray *func_contexts;
};

int tokenize_stream(FILE *fp, atom_tree_t* root, struct t_tokenizer *tokenizer);
atom_t *parse_block(struct t_tokenizer *, int);
struct t_tokenizer *new_tokenizer();
void free_tokenizer(struct t_tokenizer *tokenizer);
void free_atom_tree(atom_t *atom);
char* print_atom(atom_t *atom, char* buff, int indent, int test);
char *atom_type_name(int type);

#endif
