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

void repl_loop() {
    char* input;
    FILE *stream;
    GArray *trees = g_array_new(TRUE, TRUE, sizeof(atom_tree_t *));
    while (input = read_input()) {
        printf("code:%s\n", input);
        if (!strncmp(input, "quit()", 6))
            break;
        atom_tree_t *tree = new_atom_tree();
        g_array_append_val(trees, tree);
        stream = fmemopen(input, strlen(input), "r");
        evaluate_main(stream, tree, TRUE);
        free(input);
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

int interpret_main(char* filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("No file named %s", filename);
        return 1;
    }
// TODO check file etc
    atom_tree_t *tree = new_atom_tree();
    int result = evaluate_main(fp, tree, FALSE);
    fclose(fp);
    free(tree);
    return result;
}

int main(int argc, char* argv[]) {
    init_interpreter();
    if (argc == 1)
        repl_loop();
    else if (argc == 2)
        return interpret_main(argv[1]);
    else {
        printf("Takes an argument or no argument");
        return 0;
    }
    return 0;
}
