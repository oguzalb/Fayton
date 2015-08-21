#include "parse.h"

#define T_IDENTIFIER 1
#define T_MODULE 1 << 1
#define T_IMPORT 1 << 2
#define T_DOTCHAR 1 << 3
#define T_SPACE 1 << 4
#define T_NUMBER 1 << 5
#define T_INITIAL 1 << 6
#define T_COLUMN 1 << 7
#define T_LT 1 << 8
#define T_GT 1 << 9
#define T_OPHAR 1 << 10
#define T_CPHAR 1 << 11
#define T_COMMA 1 << 12
#define T_EQUALS 1 << 13
#define T_STRING 1 << 14
#define T_EOL 1 << 15
#define T_ELSE 1 << 16
#define T_OBRACKET 1 << 17
#define T_CBRACKET 1 << 18
#define T_PLUS 1 << 19
#define T_MINUS 1 << 20
#define T_MULTIPLY 1 << 21
#define T_DIVIDE 1 << 22
#define T_AND 1 << 23
#define T_OR 1 << 24
#define T_EQUALSEQUALS 1 << 25
#define T_INDENT 1 << 26
#define T_OCURLY 1 << 27
#define T_CCURLY 1 << 28
#define T_RSHIFT 1 << 29
#define T_LSHIFT 1 << 30

struct t_tokenizer *new_tokenizer(){
    struct t_tokenizer *tokenizer = malloc(sizeof(struct t_tokenizer));
    tokenizer->error = 0;
    tokenizer->tokens = NULL;
    return tokenizer;
}

char *token_type_name(int type) {
    switch(type) {
        case T_IDENTIFIER: return "IDENTIFIER";
        case T_MODULE: return "MODULE";
        case T_IMPORT: return "IMPORT";
        case T_DOTCHAR: return "DOTCHAR";
        case T_SPACE: return "SPACE";
        case T_NUMBER: return "NUMBER";
        case T_INITIAL: return "INITIAL";
        case T_COLUMN: return "COLUMN";
        case T_LT: return "LT";
        case T_GT: return "GT";
        case T_OPHAR: return "OPHAR";
        case T_CPHAR: return "CPHAR";
        case T_COMMA: return "COMMA";
        case T_EQUALS: return "EQUALS";
        case T_STRING: return "STRING";
        case T_EOL: return "EOL";
        case T_ELSE: return "ELSE";
        case T_OBRACKET: return "OBRACKET";
        case T_CBRACKET: return "CBRACKET";
        case T_PLUS: return "PLUS";
        case T_MINUS: return "MINUS";
        case T_MULTIPLY: return "MULTIPLY";
        case T_DIVIDE: return "DIVIDE";
        case T_AND: return "AND";
        case T_OR: return "OR";
        case T_EQUALSEQUALS: return "EQUALSEQUALS";
        case T_INDENT: return "INDENT";
        case T_OCURLY: return "OCURLY";
        case T_CCURLY: return "CCURLY";
        case T_RSHIFT: return "RSHIFT";
        case T_LSHIFT: return "LSHIFT";
    printf("COULDNT FIND TOKEN TYPE\n");
    assert(FALSE);
    }
    return NULL;
}

char *atom_type_name(int type) {
    switch(type) {
        case A_VAR: return "VAR";
        case A_INTEGER: return "INTEGER";
        case A_ASSIGNMENT: return "ASSIGNMENT";
        case A_FUNC: return "FUNC";
        case A_ARGLIST: return "ARGLIST";
        case A_RETURN: return "RETURN";
        case A_STMT: return "STMT";
        case A_BLOCK: return "BLOCK";
        case A_MODULE: return "MODULE";
        case A_FUNCCALL: return "FUNCCALL";
        case A_CALLPARAMS: return "CALLPARAMS";
        case A_EXPR: return "EXPR";
        case A_CLASS: return "CLASS";
        case A_ACCESSOR: return "ACCESSOR";
        case A_WHILE: return "WHILE";
        case A_PHAR: return "PHAR";

        case A_PLUS: return "PLUS";
        case A_MINUS: return "MINUS";
        case A_MULTIPLY: return "MULTIPLY";
        case A_DIVIDE: return "DIVIDE";
        case A_AND: return "AND";
        case A_OR: return "OR";
        case A_EQUALSEQUALS: return "EQUALSEQUALS";
        case A_FOR: return "FOR";
        case A_PARAMS: return "PARAMS";
        case A_FUNCDEF: return "FUNCDEF";
        case A_LIST: return "LIST";
        case A_DICTIONARY: return "DICTIONARY";
        case A_SHIFTR: return "RSHIFT";
        case A_SHIFTL: return "LSHIFT";
        case A_STRING: return "STRING";
        case A_IF: return "IF";
        case A_YIELD: return "YIELD";
        case A_GENFUNCDEF: return "GENFUNCDEF";
        default: return "UNDEFINED";
    }
    assert(FALSE);
}

atom_t *new_atom(char *value, int type) {
    atom_t *atom = (atom_t *) malloc(sizeof(atom_t));
    atom->type = type;
    atom->value = value;
    atom->next = NULL;
    atom->child = NULL;
    printd("CREATING ATOM: %s %s\n", value, atom_type_name(type));
    return atom;
}

atom_t *add_next_atom(atom_t *atom, void *value, int type) {
    atom_t *n_atom = new_atom(value, type);
    atom->next = n_atom;
    return n_atom;
}

atom_t *switch_children_atom(atom_t *atom) {
printd("switching %p %s\n", atom, atom->value);
    atom_t *left = atom->child;
    atom->child = atom->child->next;
    atom->child->next = left;
    left->next = NULL;
}

atom_t *add_child_atom(atom_t *atom, atom_t *new_atom) {
printd("adding %p %s to %p %s\n", new_atom, new_atom->value, atom, atom->value);
    if (atom->child == NULL)
        atom->child = new_atom;
    else {
        atom_t *n_atom = atom->child;
        while (n_atom->next != NULL) {n_atom = n_atom->next;}
        n_atom->next = new_atom;
    }
    return atom;
}

char* print_atom(atom_t *atom, char* buff, int indent, int test) {
    int i;
    for (i=0; i<indent; i++)
        sprintf(buff++, test ? "|" : " ");
    //TODO if (atom->type == )
    buff += sprintf(buff, "%s:%s", atom_type_name(atom->type), atom->value);
    if (test != TRUE) {
        sprintf(buff++, "\n");
    }
    if (atom->child) {
        atom_t *child = atom->child;
        buff = print_atom(child, buff, indent+2, test);
        while (child->next) {
            buff = print_atom(child->next, buff, indent+2, test);
            child = child->next;
        }
    }
    return buff;
}

#define NEXT_TOKEN() {tokenizer.iter++;tokenizer.iter}
#include <sys/queue.h>

void print_token(struct t_token *token) {
    if (token->type == T_INDENT)
        printd("TOKEN %d, %s\n", *token->value, token_type_name(token->type));
    else
        printd("TOKEN %s, %s\n", token->value, token_type_name(token->type));
}

int token(struct t_tokenizer *tokenizer, char *buffer, FILE *fp) {
    int i = 0;
    char c;
    int type = 0;
    int cur_type = T_INITIAL;
    while((c = fgetc(fp)) != EOF) {
        if (isdigit(c)){
            buffer[i] = c;
            if (cur_type == T_NUMBER || cur_type == T_INITIAL)
                cur_type = T_NUMBER;
            else if (cur_type != T_IDENTIFIER) {
                buffer[i] = '\0';
                return PARSE_ERROR;
            } else if (cur_type == T_IDENTIFIER) {
            
            }
            else break;
            i++;
        } else if (isalnum(c) || c == '_') {
            buffer[i] = c;
            if (cur_type == T_IDENTIFIER || cur_type == T_INITIAL)
                cur_type = T_IDENTIFIER;
            else break;
            i++;
        } else if (cur_type == T_INITIAL && (c == '"' || c == '\'')) {
            cur_type = T_STRING;
            char str_start = c;
            while ((c = fgetc(fp)) != EOF) {
                // TODO escape will be implemented
                if (c == str_start)
                    break;
                buffer[i++] = c;
            }
            if (c == EOF)
                return PARSE_ERROR;
            break;
        } else if (cur_type == T_INITIAL) {
            if (c == '<') {
                c = fgetc(fp);
                buffer[0] = '<';
                if (c == '<') {
                    buffer[1] = '<';
                    buffer[2] = '\0';
                    return T_LSHIFT;
                }
                buffer[1] = '\0';
                fseek(fp, -1, SEEK_CUR);
                return T_LT;
            } else if (c == '>') {
                c = fgetc(fp);
                buffer[0] = '>';
                if (c == '>') {
                    buffer[1] = '>';
                    buffer[2] = '\0';
                    return T_RSHIFT;}
                fseek(fp, -1, SEEK_CUR);
                buffer[1] = '\0';
                return T_GT;
            } else if (c == '=') {
                c = fgetc(fp);
                if (c == '=') {
                    buffer[0] = '=';
                    buffer[1] = '=';
                    buffer[2] = '\0';
                    return T_EQUALSEQUALS;
                }
                fseek(fp, -1, SEEK_CUR);
                c = '=';
            }
            int types[] = {T_SPACE, T_DOTCHAR, T_PLUS, T_MULTIPLY, T_OPHAR, T_CPHAR, T_COMMA, T_EOL, T_EQUALS, T_MINUS, T_COLUMN, T_OBRACKET, T_CBRACKET, T_OCURLY, T_CCURLY, T_DIVIDE};
            char chars[] = {' ', '.', '+', '*', '(', ')', ',', '\n', '=', '-', ':', '[', ']', '{', '}', '/'};
            int ocount = sizeof(chars);
            int j;
            for (j=0; j < ocount && cur_type == T_INITIAL; j++) {
                if (c == chars[j])
                    cur_type = types[j];
            }
            if (cur_type == T_INITIAL) {
                printd("token not identified %c\n", c);
                return PARSE_ERROR;
            }
            buffer[i++] = c;
            buffer[i] = '\0';
            if (cur_type == T_EOL) {
                tokenizer->current_line++;
            }
            break;
        } else if (cur_type != T_INITIAL) {
            fseek(fp, -1, SEEK_CUR);
            break;
        }
    }
    buffer[i] = '\0';
    return cur_type;
}

atom_t *parse_comp(struct t_tokenizer *tokenizer);

atom_t *parse_var(struct t_tokenizer *tokenizer, atom_t *prev_arg) {
    struct t_token *token = *tokenizer->iter;
    atom_t *var = NULL;
    if (prev_arg == NULL) {
        if (token == NULL || token->type != T_IDENTIFIER) {
            tokenizer->error = PARSE_ERROR;
            printf("PARSE_VAR PASS_SPACE PARSE ERROR 1\n", token_type_name(token->type));
            return NULL;
        }
        var = new_atom(strdup(token->value), A_VAR);
        tokenizer->iter++;
        token = *tokenizer->iter;
    }
    if (token == NULL || token->type != T_DOTCHAR) {
        return var;
    }
    if (tokenizer->error == PARSE_ERROR) {
        if (var != NULL)
            free_atom_tree(var);
        return NULL;
    }
    tokenizer->iter++;
    token = *tokenizer->iter;
    if (token == NULL || token->type != T_IDENTIFIER) {
        tokenizer->error = PARSE_ERROR;
        printf("PARSE_VAR PASS_SPACE PARSE ERROR 2\n");
        if (var != NULL)
            free_atom_tree(var);
        return NULL;
    }
    atom_t *sec_var = new_atom(strdup(token->value), A_VAR);
    tokenizer->iter++;
    atom_t *accessor = new_atom(strdup("."), A_ACCESSOR);
    if (prev_arg == NULL) {
        add_child_atom(accessor, var);
        add_child_atom(accessor, sec_var);
        atom_t *right = parse_var(tokenizer, accessor);
        if (tokenizer->error == PARSE_ERROR)
            return NULL;
        return right;
    } else {
        add_child_atom(accessor, prev_arg);
        add_child_atom(accessor, sec_var);
        return accessor;
    }
}

atom_t *parse_yield(struct t_tokenizer *tokenizer) {
    atom_t *yield = new_atom(strdup("yield"), A_YIELD);
    if ((*tokenizer->iter)->type == T_EOL)
        return yield;
    atom_t *val = parse_comp(tokenizer);
    if (tokenizer->error == PARSE_ERROR)
        return NULL;
    add_child_atom(yield, val);
    return yield;
}

atom_t *parse_return(struct t_tokenizer *tokenizer) {
    atom_t *ret = new_atom(strdup("return"), A_RETURN);
    if ((*tokenizer->iter)->type == T_EOL)
        return ret;
    atom_t *val = parse_comp(tokenizer);
    if (tokenizer->error == PARSE_ERROR)
        return NULL;
    add_child_atom(ret, val);
    return ret;
}

atom_t *parse_shift(struct t_tokenizer *tokenizer);
atom_t *parse_callparams(struct t_tokenizer *tokenizer) {
    printd("PARSE_CALLPARAMS START\n");
    struct t_token *token = *tokenizer->iter;
    if (token == NULL) {
        tokenizer->error = PARSE_ERROR;
        printf("PARSE_CALLPARAMS PARSE_ERROR\n");
        return NULL;
    }
    atom_t *params = new_atom(strdup("params"), A_PARAMS);
    if (token->type == T_CPHAR)
        return params;
// TODO expr
    atom_t *first_arg = parse_comp(tokenizer);
    if (first_arg == NULL)
        return NULL;
    add_child_atom(params, first_arg);
    atom_t *prev_arg = first_arg;
    while (TRUE) {
        token = *tokenizer->iter;
printd("%s++\n", token->value);
        if (token->type == T_CPHAR) {
            return params;
        }
        if (token->type != T_COMMA) {
printf("PARSE_CALLPARAMS COMMA ERR %s %s\n", token->value, token_type_name(token->type));
            tokenizer->error = PARSE_ERROR;
            free_atom_tree(params);
            return NULL;
        }
        tokenizer->iter++;
        atom_t *arg = parse_comp(tokenizer);
        if (tokenizer->error == PARSE_ERROR)
            return NULL;
        prev_arg->next = arg;
        prev_arg = arg;
    }
}

atom_t *parse_slice(struct t_tokenizer *tokenizer) {
    printd("PARSE_GETITEM START\n");
    struct t_token* token = *tokenizer->iter;
    if (token == NULL) {
        tokenizer->error = PARSE_ERROR;
        printf("PARSE_CALLPARAMS PARSE_ERROR\n");
        return NULL;
    }
// TODO expr
    atom_t *first_arg = parse_comp(tokenizer);
    if (first_arg == NULL)
        return NULL;
    atom_t *prev_arg = first_arg;
    while (TRUE) {
        token = *tokenizer->iter;
printd("%s++\n", token->value);
        if (token->type == T_CBRACKET) {
            if (first_arg->next == NULL) {
                return first_arg;
            }
            atom_t *funccall = new_atom(strdup("slicecall"), A_FUNCCALL);
            atom_t *slice = new_atom(strdup("slice"), A_VAR);
            add_child_atom(funccall, slice);
            atom_t *params = new_atom(strdup("params"), A_PARAMS);
            add_child_atom(funccall, params);
            add_child_atom(params, first_arg);
            return funccall;
        }
        if (token->type != T_COLUMN) {
printf("PARSE_GETITEM COLUMN ERR %s %s\n", token->value, token_type_name(token->type));
            tokenizer->error = PARSE_ERROR;
            free_atom_tree(first_arg);
            return NULL;
        }
        tokenizer->iter++;
        atom_t *arg = parse_comp(tokenizer);
        if (tokenizer->error == PARSE_ERROR)
            return NULL;
        prev_arg->next = arg;
        prev_arg = arg;
    }
}

atom_t *parse_list(struct t_tokenizer *tokenizer) {
printd("PARSE_LIST START\n");
    struct t_token *token = *tokenizer->iter;
    if (token == NULL || token->type != T_OBRACKET) {
        tokenizer->error = PARSE_ERROR;
        printf("PARSE_LIST PARSE_ERROR\n");
        return NULL;
    }
    atom_t *list = new_atom(strdup("list"), A_LIST);
    tokenizer->iter++;
    token = *tokenizer->iter;
    if (token->type == T_CBRACKET) {
        tokenizer->iter++;
        return list;
    }
    atom_t *first_arg = parse_comp(tokenizer);
    add_child_atom(list, first_arg);
    atom_t *prev_arg = first_arg;
    while (TRUE) {
        token = *tokenizer->iter;
        if (token->type == T_CBRACKET) {
            tokenizer->iter++;
            return list;
        }
        if (token->type != T_COMMA) {
            printf("PARSE_LIST COMMA ERROR\n");
            tokenizer->error = PARSE_ERROR;
            free_atom_tree(list);
            return NULL;
        }
        tokenizer->iter++;
        atom_t *arg = parse_comp(tokenizer);
        if (tokenizer->error == PARSE_ERROR)
            return NULL;
        prev_arg->next = arg;
        prev_arg = arg;
    }
}

atom_t *parse_dictionary(struct t_tokenizer *tokenizer) {
printd("PARSE_DICTIONARY START\n");
    struct t_token *token = *tokenizer->iter;
    if (token == NULL || token->type != T_OCURLY) {
        tokenizer->error = PARSE_ERROR;
        printf("PARSE_DICTIONARY PARSE_ERROR\n");
        return NULL;
    }
    tokenizer->iter++;
    atom_t *dictionary = new_atom(strdup("dict"), A_DICTIONARY);
    token = *tokenizer->iter;
    if (token->type == T_CCURLY) {
        tokenizer->iter++;
        return dictionary;
    }
    atom_t *key = parse_comp(tokenizer);
    if (tokenizer->error == PARSE_ERROR)
        return NULL;
    add_child_atom(dictionary, key);
    token = *tokenizer->iter;
    if (token->type != T_COLUMN) {
        printf("PARSE_DICTIONARY COLUMN ERROR\n");
        tokenizer->error = PARSE_ERROR;
        free_atom_tree(dictionary);
        return NULL;
    }
    tokenizer->iter++;
    atom_t *value = parse_comp(tokenizer);
    if (tokenizer->error == PARSE_ERROR) {
        free_atom_tree(dictionary);
        return NULL;
    }
    add_child_atom(dictionary, value);
    atom_t *prev_arg = dictionary->child->next;
    while (TRUE) {
        token = *tokenizer->iter;
        if (token->type == T_CCURLY) {
            tokenizer->iter++;
            return dictionary;
        }
        if (token->type != T_COMMA) {
            printf("PARSE_DICTIONARY COMMA ERROR\n");
            tokenizer->error = PARSE_ERROR;
            free_atom_tree(dictionary);
            return NULL;
        }
        tokenizer->iter++;
        atom_t *key = parse_comp(tokenizer);
        if (tokenizer->error == PARSE_ERROR) {
            free_atom_tree(dictionary);
            return NULL;
        }
        prev_arg->next = key;
        prev_arg = key;
        token = *tokenizer->iter;
        if (token->type != T_COLUMN) {
            printf("PARSE_DICTIONARY COLUMN2 ERROR\n");
            tokenizer->error = PARSE_ERROR;
            free_atom_tree(dictionary);
            return NULL;
        }
        tokenizer->iter++;
        atom_t *value = parse_comp(tokenizer);
        if (tokenizer->error == PARSE_ERROR) {
            free_atom_tree(dictionary);
            return NULL;
        }
        prev_arg->next = value;
        prev_arg = value;
    }
}

atom_t *parse_call(struct t_tokenizer *tokenizer, atom_t *first_arg) {
printd("PARSE_CALL\n");
    atom_t *callparams = parse_callparams(tokenizer);
    if (tokenizer->error == PARSE_ERROR)
        return NULL;
printd("PARSE_CALL_end\n");
    struct t_token *cphar = *tokenizer->iter;
    if (cphar->type != T_CPHAR) {
printf("PARSE_CALL CPHAR ERR\n");
        tokenizer->error == PARSE_ERROR;
        if (callparams)
            free_atom_tree(callparams);
        return NULL;
    }
    tokenizer->iter++;
    atom_t *call_func = new_atom(strdup(first_arg->value), A_FUNCCALL);
    if (callparams)
        add_child_atom(call_func, callparams);
    return call_func;
}

atom_t *parse_atom(struct t_tokenizer *tokenizer) {
printd("PARSE_ATOM START\n");
    struct t_token *first_arg = *tokenizer->iter;
    if (first_arg == NULL) {
        tokenizer->error = PARSE_ERROR;
        printf("PARSE_ATOM_ERROR %s\n", first_arg->value);
        return NULL;
    }
    if (first_arg->type == T_NUMBER) {
// TODO strtoint
        atom_t *var = new_atom(strdup(first_arg->value), A_INTEGER);
        tokenizer->iter++;
        return var;
    } else if (first_arg->type == T_STRING) {
// TODO strtoint
        atom_t *var = new_atom(strdup(first_arg->value), A_STRING);
        tokenizer->iter++;
        return var;
    } else if (first_arg->type == T_IDENTIFIER) {
        atom_t *var = new_atom(strdup(first_arg->value), A_VAR);
        struct t_token *token = *tokenizer->iter;
// TODO don't forget adding obracket for slices
        tokenizer->iter++;
        return var;
    } else if (first_arg->type == T_OBRACKET) {
        atom_t *list = parse_list(tokenizer);
        return list;
    } else if (first_arg->type == T_OCURLY) {
        atom_t *dict = parse_dictionary(tokenizer);
        return dict;
    } else if (first_arg->type == T_OPHAR) {
        tokenizer->iter++;
        atom_t *phar = parse_comp(tokenizer);
        if ((*tokenizer->iter)->type != T_CPHAR) {
            tokenizer->error = PARSE_ERROR;
            printf("PARSE_ATOM ERROR NO CPHAR %s\n", (*tokenizer->iter)->value);
            return NULL;
        }
        tokenizer->iter++;
        return phar;
    }
    tokenizer->error = PARSE_ERROR;
    printf("PARSE_ATOM_ERROR %s\n", first_arg->value);
    return NULL;
}

atom_t *parse_trailer(struct t_tokenizer *tokenizer) {
    struct t_token *token = *tokenizer->iter;
    if (token == NULL) {
        return NULL;
    }
    if (token->type == T_DOTCHAR) {
        atom_t *accessor = new_atom(strdup("."), A_ACCESSOR);
        tokenizer->iter++;
        token = *tokenizer->iter;
        if (token->type != T_IDENTIFIER) {
            printf("PARSE TRAILER NOT IDENTIFIER %s\n", token->value);
            free_atom_tree(accessor);
            tokenizer->error = PARSE_ERROR;
            return NULL;
        }
        atom_t *var = new_atom(strdup(token->value), A_VAR);
        add_child_atom(accessor, var);
        tokenizer->iter++;
        return accessor;
    } else if (token->type == T_OPHAR) {
        tokenizer->iter++;
        atom_t *callparams = parse_callparams(tokenizer);
        if (tokenizer->error == PARSE_ERROR)
            return NULL;
        atom_t *funccall = new_atom(strdup("()call"), A_FUNCCALL);
        if (callparams)
            add_child_atom(funccall, callparams);
        tokenizer->iter++;
        return funccall;
    } else if (token->type == T_OBRACKET) {
        tokenizer->iter++;
        atom_t *slice = parse_slice(tokenizer);
        if (tokenizer->error == PARSE_ERROR)
            return NULL;
        tokenizer->iter++;
        return slice;
    } else {
        printd("PARSE TRAILER %s\n", token_type_name(token->type));
        return NULL;
    }
}

atom_t *parse_power(struct t_tokenizer *tokenizer) {
    atom_t *atom = parse_atom(tokenizer);
    if (tokenizer->error == PARSE_ERROR)
        return NULL;
    atom_t *trailer = NULL;
    atom_t *top_trailer = parse_trailer(tokenizer);
    if (tokenizer->error == PARSE_ERROR) {
        free_atom_tree(atom);
        return NULL;
    }
    if (top_trailer == NULL)
        return atom;
    if (top_trailer->type == A_ACCESSOR || (top_trailer->type == A_FUNCCALL && strcmp(top_trailer->value, "slicecall"))) {
        add_child_atom(top_trailer, atom);
        switch_children_atom(top_trailer);
    } else {
// SLICE
        atom_t *accessor = new_atom(strdup("."), A_ACCESSOR);
        add_child_atom(accessor, atom);
        atom_t *getitem = new_atom(strdup("__getitem__"), A_VAR);
        add_child_atom(accessor, getitem);
        atom_t *funccall = new_atom(strdup("()call"), A_FUNCCALL);
        add_child_atom(funccall, accessor);
        atom_t *params = new_atom(strdup("params"), A_PARAMS);
        add_child_atom(funccall, params);
        add_child_atom(params, top_trailer);
        top_trailer = funccall;
    }
    while (trailer = parse_trailer(tokenizer)) {
        if (tokenizer->error == PARSE_ERROR) {
            printf("PARSE_POWER ERROR\n");
            free_atom_tree(top_trailer);
            return NULL;
        }
        if (trailer->type == A_ACCESSOR) {
            add_child_atom(trailer, top_trailer);
            switch_children_atom(trailer);
            top_trailer = trailer;
// hacky but... :)
        } else if (trailer->type == A_FUNCCALL && strcmp(trailer->value, "slicecall")) {
            add_child_atom(trailer, top_trailer);
            switch_children_atom(trailer);
            top_trailer = trailer;
        } else {
// SLICE
            atom_t *accessor = new_atom(strdup("."), A_ACCESSOR);
            add_child_atom(accessor, top_trailer);
            atom_t *getitem = new_atom(strdup("__getitem__"), A_VAR);
            add_child_atom(accessor, getitem);
            atom_t *funccall = new_atom(strdup("()call"), A_FUNCCALL);
            add_child_atom(funccall, accessor);
            atom_t *params = new_atom(strdup("params"), A_PARAMS);
            add_child_atom(funccall, params);
            add_child_atom(params, trailer);
            top_trailer = funccall;
        }
    }
    return top_trailer;
}

atom_t *parse_term(struct t_tokenizer *tokenizer) {
    atom_t *power = parse_power(tokenizer);
    if (tokenizer->error == PARSE_ERROR)
        return NULL;
    struct t_token *token;
    atom_t *top_muldiv = NULL;
token = *tokenizer->iter;
    while ((token = *tokenizer->iter) != NULL && (token->type == T_MULTIPLY || token->type == T_DIVIDE)) {
        atom_t *muldiv = new_atom(strdup("()call"), A_FUNCCALL);
        tokenizer->iter++;
        atom_t *sec_power = parse_power(tokenizer);
        if (tokenizer->error == PARSE_ERROR) {
            free_atom_tree(top_muldiv != NULL? top_muldiv:power);
            return NULL;
        }
        if (top_muldiv == NULL) {
            atom_t *accessor = new_atom(strdup("."), A_ACCESSOR);
            add_child_atom(accessor, power);
            add_child_atom(accessor, new_atom(strdup(token->type == T_MULTIPLY ? "__mul__" : "__div__" ), A_VAR));
            add_child_atom(muldiv, accessor);
        } else {
            atom_t *accessor = new_atom(strdup("."), A_ACCESSOR);
            add_child_atom(accessor, top_muldiv);
            add_child_atom(accessor, new_atom(strdup(token->type == T_MULTIPLY ? "__mul__" : "__div__" ), A_VAR));
            add_child_atom(muldiv, accessor);
        }
        atom_t *params = new_atom(strdup("params"), A_PARAMS);
        add_child_atom(params, sec_power);
        add_child_atom(muldiv, params);
        top_muldiv = muldiv;
    }
    return top_muldiv != NULL? top_muldiv : power;
}

atom_t *parse_arith(struct t_tokenizer *tokenizer) {
    atom_t *term = parse_term(tokenizer);
    if (tokenizer->error == PARSE_ERROR)
        return NULL;
    struct t_token *token;
    atom_t *top_arith = NULL;
    while ((token = *tokenizer->iter)!= NULL && (token->type == T_PLUS || token->type == T_MINUS)) {
        atom_t *arith = new_atom(strdup("()call"), A_FUNCCALL);
        tokenizer->iter++;
        atom_t *sec_term = parse_term(tokenizer);
        if (tokenizer->error == PARSE_ERROR) {
            free_atom_tree(top_arith != NULL? top_arith:term);
            return NULL;
        }
        if (top_arith == NULL) {
            atom_t *accessor = new_atom(strdup("."), A_ACCESSOR);
            add_child_atom(accessor, term);
            add_child_atom(accessor, new_atom(strdup(token->type == T_PLUS ? "__add__" : "__sub__" ), A_VAR));
            add_child_atom(arith, accessor);
        } else {
            atom_t *accessor = new_atom(strdup("."), A_ACCESSOR);
            add_child_atom(accessor, top_arith);
            add_child_atom(accessor, new_atom(strdup(token->type == T_PLUS ? "__add__" : "__sub__" ), A_VAR));
            add_child_atom(arith, accessor);
        }
        atom_t *params = new_atom(strdup("params"), A_PARAMS);
        add_child_atom(params, sec_term);
        add_child_atom(arith, params);
        top_arith = arith;
    }
    return top_arith != NULL? top_arith : term;
}

atom_t *parse_shift(struct t_tokenizer *tokenizer) {
    atom_t *arith = parse_arith(tokenizer);
    if (tokenizer->error == PARSE_ERROR)
        return NULL;
    struct t_token *token;
    atom_t *top_shift = NULL;
    while ((token = *tokenizer->iter)!= NULL && (token->type == T_RSHIFT || token->type == T_LSHIFT)) {
        atom_t *shift = new_atom(strdup("()call"), A_FUNCCALL);
        tokenizer->iter++;
        atom_t *sec_arith = parse_arith(tokenizer);
        if (tokenizer->error == PARSE_ERROR) {
            free_atom_tree(top_shift != NULL? top_shift:arith);
            return NULL;
        }
        if (top_shift == NULL) {
            atom_t *accessor = new_atom(strdup("."), A_ACCESSOR);
            add_child_atom(accessor, arith);
            add_child_atom(accessor, new_atom(strdup(token->type == T_RSHIFT ? "__rshift__" : "__lshift__" ), A_VAR));
            add_child_atom(shift, accessor);
        } else {
            atom_t *accessor = new_atom(strdup("."), A_ACCESSOR);
            add_child_atom(accessor, top_shift);
            add_child_atom(accessor, new_atom(strdup(token->type == T_LSHIFT ? "__rshift__" : "__lshift__" ), A_VAR));
            add_child_atom(shift, accessor);
        }
        atom_t *params = new_atom(strdup("params"), A_PARAMS);
        add_child_atom(params, sec_arith);
        add_child_atom(shift, params);
        top_shift = shift;
    }
    return top_shift != NULL? top_shift : arith;
}

atom_t *parse_expr(struct t_tokenizer *tokenizer) {
    atom_t *expr = parse_shift(tokenizer);
    if (tokenizer->error == PARSE_ERROR)
        return NULL;
    printd("PARSE_EXPR_RETURNS atom: %s:\n", expr?expr->value:"__");
    return expr;
}

atom_t *parse_comp(struct t_tokenizer *tokenizer) {
    atom_t *shift = parse_shift(tokenizer);
    if (tokenizer->error == PARSE_ERROR)
        return NULL;
    struct t_token *token;
    atom_t *top_comp = NULL;
    while ((token = *tokenizer->iter)!= NULL && (token->type == T_GT || token->type == T_LT || token->type == T_EQUALSEQUALS)) {
        tokenizer->iter++;
char buff[1024];
        atom_t *equals_accessor = new_atom(strdup("."), A_ACCESSOR);
        atom_t *equals = new_atom(strdup("__eq__"), A_VAR);
        atom_t *equals_call = new_atom(strdup("()call"), A_FUNCCALL);
        atom_t *comp = new_atom(strdup("()call"), A_FUNCCALL);
        atom_t *sec_shift = parse_shift(tokenizer);
        if (tokenizer->error == PARSE_ERROR) {
            free_atom_tree(top_comp != NULL? top_comp:shift);
            return NULL;
        }
        atom_t *accessor = new_atom(strdup("."), A_ACCESSOR);
        if (top_comp == NULL)
            add_child_atom(accessor, shift);
        else
            add_child_atom(accessor, top_comp);
        atom_t *cmp = new_atom(strdup("__cmp__"), A_VAR);
        add_child_atom(accessor, cmp);
        add_child_atom(comp, accessor);
        atom_t *params = new_atom(strdup("params"), A_PARAMS);
        add_child_atom(params, sec_shift);
        add_child_atom(comp, params);

        add_child_atom(equals_accessor, comp);
        add_child_atom(equals_accessor, equals);

        add_child_atom(equals_call, equals_accessor);
        atom_t *equals_params = new_atom(strdup("params"), A_PARAMS);
        atom_t *comp_result = new_atom(strdup(token->type == T_GT? "1":token->type == T_LT?"-1":"0"), A_INTEGER);
        add_child_atom(equals_params, comp_result);
        add_child_atom(equals_call, equals_params);
        top_comp = equals_call;
    }
    return top_comp != NULL? top_comp : shift;
}

atom_t *parse_params(struct t_tokenizer *tokenizer) {
    atom_t * params = new_atom(strdup("params"), A_PARAMS);
    struct t_token * ophar = *tokenizer->iter;
    if (ophar == NULL || ophar->type != T_OPHAR) {
        tokenizer->error = PARSE_ERROR;
        free_atom_tree(params);
        printf("PARSE_PARAMS OPHAR ERR\n");
        return NULL;
    }
    tokenizer->iter++;
    struct t_token *token;
    while((token = *(tokenizer->iter)) && token->type != T_CPHAR) {
        if (token == NULL || token->type != T_IDENTIFIER) {
            tokenizer->error = PARSE_ERROR;
            free_atom_tree(params);
            printf("PARSE_PARAMS IDENTIRIER ERR\n");
            return NULL;
        }
        atom_t *param = new_atom(strdup(token->value), A_VAR);
        add_child_atom(params, param);
        tokenizer->iter++;
        token = *tokenizer->iter;
        if (token == NULL || (token->type != T_COMMA && token->type != T_CPHAR)) {
            tokenizer->error = PARSE_ERROR;
            printf("PARSE_PARAMS CPHAR ERR\n");
            return NULL;
        }
        if (token->type == T_COMMA)
            tokenizer->iter++;
    }
    tokenizer->iter++;
    return params;
}

int block_has_yield(atom_t *atom) {
    atom_t *child = atom->child;
    if (atom->type == A_FUNCDEF || atom->type == A_GENFUNCDEF)
        return FALSE;
    int children_has = FALSE;
    while(child) {
        children_has |= block_has_yield(child);
        child = child->next;
    }
    return atom->type == A_YIELD | children_has;
}

atom_t *parse_func(struct t_tokenizer *tokenizer, int current_indent) {
    struct t_token * func_name = *tokenizer->iter;
    if (func_name == NULL || func_name->type != T_IDENTIFIER) {
        printf("PARSE_FUNC IDENTIFIER ERR\n");
        tokenizer->error = PARSE_ERROR;
        return NULL;
    }
    tokenizer->iter++;
    atom_t *params = parse_params(tokenizer);
    if (params == NULL || tokenizer->error == PARSE_ERROR) {
        printf("PARSE_FUNC IDENTIFIER ERR\n");
        return NULL;
    }
    struct t_token *column = *tokenizer->iter;
    if (column == NULL || column->type != T_COLUMN) {
        printf("PARSE_FUNC COLUMN ERR\n");
        free_atom_tree(params);
        tokenizer->error = PARSE_ERROR;
        return NULL;
    }
    tokenizer->iter++;
    struct t_token *eol = *tokenizer->iter;
    if (eol == NULL || eol->type != T_EOL) {
        printf("PARSE_FUNC EOL ERR\n");
        free_atom_tree(params);
        tokenizer->error = PARSE_ERROR;
        return NULL;
    }
    tokenizer->iter++;
    atom_t *block = parse_block(tokenizer, current_indent);
    if (tokenizer->error == PARSE_ERROR) {
        free_atom_tree(params);
        return NULL;
    }
    atom_t *funcdef;
    if (block_has_yield(block) == TRUE)
        funcdef = new_atom(strdup(func_name->value), A_GENFUNCDEF);
    else
        funcdef = new_atom(strdup(func_name->value), A_FUNCDEF);
    add_child_atom(funcdef, params);
    add_child_atom(funcdef, block);
    return funcdef;
}

atom_t *parse_class(struct t_tokenizer *, int);
atom_t *parse_if(struct t_tokenizer *, int);
atom_t *parse_for(struct t_tokenizer *, int);
atom_t *parse_while(struct t_tokenizer *, int);
atom_t *parse_stmt(struct t_tokenizer *tokenizer, int current_indent) {
    printd("PARSE_STMT_ASSG %s:\n", (*tokenizer->iter)->value);
    struct t_token *token = *tokenizer->iter;
    if (token == NULL || token->type != T_IDENTIFIER) {
        printf("PARSE_STMT ASSG IDENTIFIER ERR\n");
        if (token != NULL)
            printf("%s:%s\n", token_type_name(token->type), token->value);
        tokenizer->error = PARSE_ERROR;
        return NULL;
    }
    atom_t *stmt = NULL;
    if (!strncmp(token->value, "return", 5)) {
        printd("Have seen a return\n");
        tokenizer->iter++;
        stmt = parse_return(tokenizer);
    } else if (!strncmp(token->value, "yield", 5)) {
        printd("Have seen a yield\n");
        tokenizer->iter++;
        stmt = parse_yield(tokenizer);
    } else if (!strcmp(token->value, "for")) {
        tokenizer->iter++;
        stmt = parse_for(tokenizer, current_indent);
    } else if (!strcmp(token->value, "while")) {
        tokenizer->iter++;
        stmt = parse_while(tokenizer, current_indent);
    } else if (!strcmp(token->value, "if")) {
        tokenizer->iter++;
        stmt = parse_if(tokenizer, current_indent);
    } else if (!strcmp(token->value, "def")) {
        tokenizer->iter++;
        stmt = parse_func(tokenizer, current_indent);
    } else if (!strcmp(token->value, "class")) {
        tokenizer->iter++;
        stmt = parse_class(tokenizer, current_indent);
    } else {
        struct t_token **prev_iter = tokenizer->iter;
        atom_t *var = parse_var(tokenizer, NULL);
        if (tokenizer->error == PARSE_ERROR)
            return NULL;
        struct t_token *token = *tokenizer->iter;
        if (token->type == T_OBRACKET) {
            tokenizer->iter++;
            atom_t *slice = parse_slice(tokenizer);
            if (tokenizer->error == PARSE_ERROR)
               return NULL;
            tokenizer->iter++;
            token = *tokenizer->iter;
            if (token->type != T_EQUALS) {
                printf("PARSE_STMT NOT EQUALS OR OPHAR setitem |%s|\n", token->value);
                tokenizer->error = PARSE_ERROR;
                if(var)
                    free_atom_tree(var);
                free_atom_tree(slice);
                return NULL;
            }
            tokenizer->iter++;
            atom_t *expr = parse_comp(tokenizer);
            if (expr == NULL) {
                if(var)
                    free_atom_tree(var);
                free_atom_tree(slice);
                return NULL;
            }
            stmt = new_atom(strdup("call()"), A_FUNCCALL);
            atom_t *accessor = new_atom(strdup("."), A_ACCESSOR);
            add_child_atom(accessor, var);
            atom_t *setitem = new_atom(strdup("__setitem__"), A_VAR);
            add_child_atom(accessor, setitem);
            add_child_atom(stmt, accessor);
            atom_t *params = new_atom(strdup("params"), A_PARAMS);
            add_child_atom(params, slice);
            add_child_atom(params, expr);
            add_child_atom(stmt, params);
        } else if (token->type == T_EQUALS) {
            tokenizer->iter++;
            stmt = new_atom(strdup("="), A_ASSIGNMENT);
            add_child_atom(stmt, var);
            atom_t *expr = parse_comp(tokenizer);
            if (expr == NULL) {
                if(var)
                    free_atom_tree(var);
                return NULL;
            }
            add_child_atom(stmt, expr);
        } else if (token->type == T_OPHAR) {
            tokenizer->iter = prev_iter;
            stmt = parse_comp(tokenizer);
// TODO ??? design mem leak stuff
            if (stmt == NULL) {
                return NULL;
            }
        } else {
            printf("PARSE_STMT NOT EQUALS OR OPHAR |%s|\n", token->value);
            tokenizer->error = PARSE_ERROR;
            if (var)
                free_atom_tree(var);
            return NULL;
        }
        token = *tokenizer->iter;
        if (token != NULL && token->type != T_EOL) {
            tokenizer->error = PARSE_ERROR;
            if (token != NULL)
                printf("PARSE_STMT_NOT_EOL ERROR |%s|%s\n", token->value, token_type_name(token->type));
            else
                printf("PARSE_STMT NOT EOL TOKEN NULL ERROR");
            free_atom_tree(var);
            return NULL;
        }
    }
    if (tokenizer->error == PARSE_ERROR)
        return NULL;
    struct t_token *eol = *tokenizer->iter;
    if (eol != NULL && eol->type == T_EOL)
        tokenizer->iter++;
    if ((*tokenizer->iter) != NULL) {
        printd("PARSE_STMT_END %s:\n", (*tokenizer->iter)?(*tokenizer->iter)->value:"__");
        printd("PARSE_STMT_RETURNS atom: %s:\n", stmt?stmt->value:"__");
    }
    return stmt;
}

atom_t * parse_block(struct t_tokenizer *tokenizer, int previous_indent) {
    atom_t *block = new_atom(strdup("block"), A_BLOCK);
    int start_count = 0;
    struct t_token *indent = *tokenizer->iter;
    if (indent == NULL)
        return block;
    if (indent->type == T_INDENT) {
printd("INDENT FOUND start\n");
        start_count = *indent->value;
        tokenizer->iter++;}
    if (start_count <= previous_indent || token == NULL) {
printf("PARSE_BLOCK_INDENT_PARSE_ERROR_ start %d prev %d\n", start_count, previous_indent);
        free_atom_tree(block);
        tokenizer->error = PARSE_ERROR;
        return NULL;
    }
    int count = start_count;
    while (count == start_count) {
        atom_t *stmt = parse_stmt(tokenizer, start_count);
        if (tokenizer->error == PARSE_ERROR) {
            free_atom_tree(block);
            return NULL;
        }
printd("statement parse returned to parse_block\n");
if (*tokenizer->iter)
printd("%s\n", (*tokenizer->iter)->value);
        if (stmt == NULL || tokenizer->error == PARSE_ERROR)
            return block;
        add_child_atom(block, stmt);
        if ((*tokenizer->iter) == NULL) {
            return block;
        }
        indent = *tokenizer->iter;
        if (indent != NULL && indent->type == T_INDENT) {
            start_count = *indent->value;
            if (count == start_count)
                tokenizer->iter++;
        } else
            start_count = 0;
    }
    return block;
}

atom_t *parse_if_else_if(struct t_tokenizer *tokenizer, int current_indent) {
    struct t_token *token = *tokenizer->iter;
    if (token == NULL || token->type != T_COLUMN) {
        tokenizer->error = PARSE_ERROR;
        printf("PARSE_IF ERROR NO COLUMN");
        return NULL;
    }
    tokenizer->iter++;
    token = *tokenizer->iter;
    if (token == NULL || token->type != T_EOL) {
        tokenizer->error = PARSE_ERROR;
        printf("PARSE_IF ERROR NO EOL");
        return NULL;
    }
    tokenizer->iter++;
    atom_t *block = parse_block(tokenizer, current_indent);
    return block;
}

atom_t *parse_if(struct t_tokenizer *tokenizer, int current_indent) {
    atom_t *if_clause = new_atom(strdup("if"), A_IF);
    atom_t *expr = parse_comp(tokenizer);
    if (tokenizer->error == PARSE_ERROR) {
        free_atom_tree(if_clause);
        return NULL;
    }
    atom_t *block = parse_if_else_if(tokenizer, current_indent);
    if (tokenizer->error == PARSE_ERROR) {
        free_atom_tree(if_clause);
        return NULL;
    }
    add_child_atom(if_clause, expr);
    add_child_atom(if_clause, block);
    while (TRUE) {
        struct t_token *indent = *tokenizer->iter;
        if (indent == NULL)
            return if_clause;
        int clause_indent = 0;
        if (indent->type == T_INDENT) {
            clause_indent = *(indent->value);
            tokenizer->iter++;
        }
        if (clause_indent != current_indent) {
            return if_clause;
        }

        struct t_token *else_if = *tokenizer->iter;
        if (else_if == NULL || else_if->type != T_IDENTIFIER || (strcmp(else_if->value, "else") && strcmp(else_if->value, "elif"))) {
            tokenizer->iter--;
            printd("only if, returning\n");
            return if_clause;
        }
        tokenizer->iter++;
        else_if = *tokenizer->iter;
        if (else_if->type == T_COLUMN) {
            atom_t *block = parse_if_else_if(tokenizer, current_indent);
            if (tokenizer->error == PARSE_ERROR) {
                free_atom_tree(if_clause);
                return NULL;
            }
            add_child_atom(if_clause, block);
        } else {
            atom_t *expr = parse_comp(tokenizer);
            if (tokenizer->error == PARSE_ERROR) {
                free_atom_tree(if_clause);
                return NULL;
            }
            atom_t *block = parse_if_else_if(tokenizer, current_indent);
            if (tokenizer->error == PARSE_ERROR) {
                free_atom_tree(if_clause);
                return NULL;
            }
            add_child_atom(if_clause, expr);
            add_child_atom(if_clause, block);
        }
    }
}

atom_t *parse_for(struct t_tokenizer *tokenizer, int current_indent) {
    struct t_token *var_name = *tokenizer->iter;
    if (var_name == NULL || var_name->type != T_IDENTIFIER) {
        tokenizer->error = PARSE_ERROR;
        printf("PARSE_FOR var_name\n");
        return NULL;
    }
    tokenizer->iter++;
    struct t_token *in = *tokenizer->iter;
    if (in == NULL || var_name->type != T_IDENTIFIER || strcmp(in->value, "in")) {
        printf("PARSE_FOR in\n");
        tokenizer->error = PARSE_ERROR;
        return NULL;
    }
    tokenizer->iter++;
    atom_t *expr = parse_comp(tokenizer);
    if (expr == NULL) {
        printf("PARSE_FOR expr\n");
        tokenizer->error = PARSE_ERROR;
        return NULL;
    }
    if ((*tokenizer->iter)->type == T_CPHAR)
        tokenizer->iter++;
    struct t_token *column = *tokenizer->iter;
    if (column == NULL || column->type != T_COLUMN) {
        printf("PARSE_FOR column\n");
        free_atom_tree(expr);
        tokenizer->error = PARSE_ERROR;
        return NULL;
    }
    tokenizer->iter++;
    struct t_token *eol = *tokenizer->iter;
    if (eol == NULL || eol->type != T_EOL) {
        printf("PARSE_FOR not eol\n");
        free_atom_tree(expr);
        tokenizer->error = PARSE_ERROR;
        return NULL;
    }
    tokenizer->iter++;
    atom_t *for_loop = new_atom(strdup("FOR"), A_FOR);
    atom_t *var = new_atom(strdup(var_name->value), A_VAR);
    add_child_atom(for_loop, var);
    add_child_atom(for_loop, expr);
    atom_t *block = parse_block(tokenizer, current_indent);
    if (tokenizer->error == PARSE_ERROR) {
        free_atom_tree(for_loop);
    }
    add_child_atom(for_loop, block);
    return for_loop;
}

atom_t *parse_while(struct t_tokenizer *tokenizer, int current_indent) {
    struct t_token *var_name = *tokenizer->iter;
    atom_t *expr = parse_comp(tokenizer);
    if (expr == NULL || tokenizer->error == PARSE_ERROR) {
        printf("PARSE_FOR expr\n");
        tokenizer->error = PARSE_ERROR;
        return NULL;
    }
    struct t_token *column = *tokenizer->iter;
    if (column == NULL || column->type != T_COLUMN) {
        printf("PARSE_FOR column\n");
        free_atom_tree(expr);
        tokenizer->error = PARSE_ERROR;
        return NULL;
    }
    tokenizer->iter++;
    struct t_token *eol = *tokenizer->iter;
    if (eol == NULL || eol->type != T_EOL) {
        printf("PARSE_FOR not eol\n");
        free_atom_tree(expr);
        tokenizer->error = PARSE_ERROR;
        return NULL;
    }
    tokenizer->iter++;
    atom_t *while_loop = new_atom(strdup("WHILE"), A_WHILE);
    add_child_atom(while_loop, expr);
    atom_t *block = parse_block(tokenizer, current_indent);
    if (tokenizer->error == PARSE_ERROR) {
        free_atom_tree(while_loop);
    }
    add_child_atom(while_loop, block);
    return while_loop;
}

atom_t *parse_class(struct t_tokenizer *tokenizer, int current_indent) {
   //TODO TODO  INDENT!?!
    struct t_token *class_name = *tokenizer->iter;
    if (class_name == NULL || class_name->type != T_IDENTIFIER) {
        tokenizer->error = PARSE_ERROR;
        printf("PARSE_CLASS var_name\n");
        return NULL;
    }
    tokenizer->iter++;
    atom_t *class = new_atom(strdup(class_name->value), A_CLASS);
    atom_t *inherits = parse_params(tokenizer);
    if (tokenizer->error == PARSE_ERROR) {
        free_atom_tree(class);
        return NULL;
    }
    add_child_atom(class, inherits);
    printf("|%s|\n", (*tokenizer->iter)->value);
    struct t_token *column = *tokenizer->iter;
    if (column == NULL || column->type != T_COLUMN) {
        printf("PARSE_CLASS column ERR\n");
        free_atom_tree(class);
        tokenizer->error = PARSE_ERROR;
        return NULL;
    }
    tokenizer->iter++;
    if ((*tokenizer->iter)->type == T_EOL)
        tokenizer->iter++;
    struct t_token *indent = *tokenizer->iter;
    if (indent->type != T_INDENT) {
    printf("PARSE_CLASS_NO_INDENT_PARSE_ERROR_ \n");
        free_atom_tree(class);
        tokenizer->error = PARSE_ERROR;
        return NULL;
    }
    int start_count = *indent->value;
    int count = start_count;
    while (count == start_count) {
        indent = *tokenizer->iter;
        if (indent->type != T_INDENT) {
        printf("PARSE_CLASS_NO_INDENT_PARSE_ERROR_ got %s\n", indent->value);
            free_atom_tree(class);
            tokenizer->error = PARSE_ERROR;
            return NULL;
        }
        start_count = *indent->value;
       tokenizer->iter++;
        if (start_count <= current_indent || token == NULL) {
    printf("PARSE_CLASS_INDENT_PARSE_ERROR_ start %d prev %d\n", start_count, current_indent);
            free_atom_tree(class);
            tokenizer->error = PARSE_ERROR;
            return NULL;
        }
        struct t_token *token = *tokenizer->iter;
        if (token == NULL || token->type != T_IDENTIFIER) {
            printf("PARSE_CLASS ASSG IDENTIFIER ERR\n");
            free_atom_tree(class);
            tokenizer->error = PARSE_ERROR;
            return NULL;
        }
        atom_t *field = NULL;
        if (!strcmp(token->value, "def")) {
            tokenizer->iter++;
            field = parse_func(tokenizer, current_indent);
            if (tokenizer->error == PARSE_ERROR) {
                free_atom_tree(class);
                return NULL;
            }
            printd("PARSED CLASS METHOD\n");
            add_child_atom(class, field);
        } else {
            struct t_token **prev_iter = tokenizer->iter;
            field = parse_var(tokenizer, NULL);
            if (tokenizer->error == PARSE_ERROR) {
                free_atom_tree(class);
                return NULL;
            }
            add_child_atom(class, field);
            struct t_token *token = *tokenizer->iter;
            if (token->type != T_EQUALS) {
                printf("PARSE_CLASS ERR ATOM NOT EQUALS %s\n", token->value);
                free_atom_tree(class);
                tokenizer->error = PARSE_ERROR;
                return NULL;
            }
            tokenizer->iter++;
            atom_t *expr = parse_comp(tokenizer);
            if (tokenizer->error == PARSE_ERROR) {
                free_atom_tree(class);
                return NULL;
            }
            add_child_atom(field, expr);
            printd("PARSED CLASS FIELD\n");
        }
        indent = (*tokenizer->iter);
        if (indent == NULL || indent->type != T_INDENT)
            break;
        count = *indent->value;
    }
    return class;
}


struct t_token* new_token() {
    return malloc(sizeof(struct t_token));
}

int tokenize_stream(FILE *fp, atom_tree_t* root, struct t_tokenizer *tokenizer) {
    int c;
    char buffer[64];
    buffer[0] = '\0';
    int type;

    int i = 0;
    tokenizer->tokens = malloc(sizeof(struct t_token *) * 30);
    tokenizer->current_line = 0;

    struct t_token **token_iter = tokenizer->tokens;
    while (PARSE_ERROR != (type = token(tokenizer, buffer, fp)) && type != T_INITIAL) {
        printd("%s\n", buffer);
        tokenizer->tokens = realloc(tokenizer->tokens, sizeof(struct t_token *) * (i + 2));
        assert(tokenizer->tokens);
        struct t_token *token = new_token();
        token->value = strdup(buffer);
        token->type = type;
        token->line = tokenizer->current_line;
        tokenizer->tokens[i] = token;
        i++;
    }
    if (type == PARSE_ERROR) {
        tokenizer->error = PARSE_ERROR;
        return 0;
    }
    tokenizer->tokens[i] = NULL;
    tokenizer->iter = tokenizer->tokens;
    while (*tokenizer->iter != NULL) {
        print_token(*tokenizer->iter);
        tokenizer->iter++;
    }
    tokenizer->iter = tokenizer->tokens;
    struct t_token **new_tokens = (struct t_token **)malloc(sizeof(struct t_token *) * (i + 2));
    struct t_token **new_tokens_iter = new_tokens;
    struct t_token **tokens = tokenizer->tokens;
    while (*tokens != NULL) {
        // Replacing spaces in the beginning of statements with indents
        int indent_count = 0;
        while (*tokens != NULL && (*tokens)->type == T_SPACE) {
            free(*tokens);
            tokens++;
            indent_count++;
        }
        // Getting rid of blank lines
        if ((*tokens)->type == T_EOL) {
            free(*tokens);
            tokens++;
            continue;
        }
        if (indent_count > 0) {
            int *indent_count_p = malloc(sizeof(int));
            *indent_count_p = indent_count;
            struct t_token * token = new_token();
            // (:
            token->value = indent_count_p;
            token->type = T_INDENT;
            *new_tokens_iter = token;
            new_tokens_iter++;
        }
        // Copying the tokens related with the statement without the spaces
        while ((*tokens) != NULL && (*tokens)->type != T_EOL) {
            if ((*tokens)->type != T_SPACE) {
                *new_tokens_iter = *tokens;
                new_tokens_iter++;
            }
            tokens++;
        }
        // Getting the EOL
        if ((*tokens) != NULL && (*tokens)->type == T_EOL) {
            *new_tokens_iter = *tokens;
            new_tokens_iter++;
            tokens++;
        }
    }
    *new_tokens_iter = NULL;
    free(tokenizer->tokens);
    new_tokens = realloc(new_tokens, sizeof(struct t_token*) * (new_tokens_iter-new_tokens+2));
    tokenizer->iter = new_tokens;
    tokenizer->tokens = new_tokens;
    // this while-statement assigns into c, and then checks against EOF:
    while (*tokenizer->iter != NULL) {
        print_token(*tokenizer->iter);
        tokenizer->iter++;
    }
    tokenizer->iter = tokenizer->tokens;
    return 1; 
}

void free_tokenizer(struct t_tokenizer *tokenizer) {
    printd("Freeing tokenizer\n");
    tokenizer->iter = tokenizer->tokens;
    while (*(tokenizer->iter) != NULL) {
        struct t_token *token = *tokenizer->iter;
        printd("Freeing token %p", token->value);
        print_token(token);
        free(token->value);
        free(token);
        tokenizer->iter++;
    }
    printd("Freeing tokenizer struct\n");
    free(tokenizer);
    printd("Freed tokenizer struct\n");
}

void free_atom_tree(atom_t *atom) {
    printd("freeing atom\n");
    if (atom->next)
        free_atom_tree(atom->next);
    if (atom->child)
        free_atom_tree(atom->child);
    free(atom->value);
    free(atom);
}

