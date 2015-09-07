#include "parse.h"
#include "interpret.h"

#include <readline/readline.h>
#include <readline/history.h>

FILE *fmemopen (void *buf, size_t size, const char *opentype)
{
    FILE *f;

    assert(strcmp(opentype, "r") == 0);

    f = tmpfile();
    fwrite(buf, 1, size, f);
    rewind(f);

    return f;
}

char *read_input() {
    char *buff = NULL;
    char *input = NULL;
    char *prompt = ">>> ";
    while ((buff = readline(prompt)) != NULL) {
        add_history(buff);
        char *extended_input;
        if (input != NULL) {
            asprintf(&extended_input, "%s%s\n", input, buff);
            free(input);
        } else {
            asprintf(&extended_input, "%s\n", buff);
            prompt = "    ";
        }
        input = extended_input;
        // i know this is slow, but no need for optimization since it can't get faster than user's typing :)
        if (strncmp(buff, "class ", 6)
            && strncmp(buff, "def ", 4)
            && strncmp(buff, "for ", 4)
            // needs to get improved
            && strncmp(buff, "if ", 3)
            && strncmp(buff, "elif ", 5)
            && strncmp(buff, "else ", 5)
            && buff[0] != ' ')
            break;
        free(buff);
    }
    return input;
}

int main() {
    char* input;
    FILE *stream;
    init_interpreter();
    GArray *trees = g_array_new(TRUE, TRUE, sizeof(atom_tree_t *));
    while (input = read_input()) {
        printf("code:%s\n", input);
        if (!strncmp(input, "quit()", 6))
            break;
        atom_tree_t *tree = new_atom_tree();
        g_array_append_val(trees, tree);
        stream = fmemopen(input, strlen(input), "r");
        struct t_tokenizer *tokenizer = new_tokenizer(TRUE);
        int success = tokenize_stream(stream, tree, tokenizer);
        if (tokenizer->error == PARSE_ERROR) {
            printf("Syntax error at line:%d\n", tokenizer->current_line);
            free_tokenizer(tokenizer);
            free(input);
            continue;
        }
        // fclose(stream);
        free(input);
        tree->root = parse_block(tokenizer, -1);
        if (tokenizer->error == PARSE_ERROR) {
            printf("Syntax error at line:%d %s\n", tokenizer->current_line, (*tokenizer->iter)==NULL?"":(*tokenizer->iter)->value);
            free_tokenizer(tokenizer);
            continue;
        }
        free_tokenizer(tokenizer);
        // this is for showing what we get as the ast tree, we won't have this when it is finished
        char *buff = NULL;
        print_atom(tree->root, &buff, 0, FALSE);
        printf("%s\n", buff);
        printd("interpreting\n");
        interpret_block(tree->root, interpreter.globals, 0);
        if (interpreter.error == RUN_ERROR) {
            interpreter.error = 0;
            struct py_thread *main_thread = g_array_index(interpreter.threads, struct py_thread *,0);
            print_stack_trace(main_thread);
            g_array_free(main_thread->stack_trace, FALSE);
            main_thread->stack_trace = g_array_new(TRUE, TRUE, sizeof(char *));
// TODO free also others
        }
        g_hash_table_foreach(interpreter.globals, print_var_each, NULL);
    }
    atom_tree_t *tree;
    for (int i=0; tree = g_array_index(trees, atom_tree_t *, i); i++) {
        if (tree->root != NULL)
            free_atom_tree(tree->root);
        free(tree);
    }
    g_array_free(trees, TRUE);
}
