#include "parse.h"
#include "interpret.h"

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
    char buff[1024];
    buff[0]='\0';
    char *input = NULL;
    int count = 1;
    while (fgets(buff, 1023, stdin) != NULL) {
        count += sizeof(char *) * 1023;
        char *extended_input = realloc(input, count + 1);
        if (input == NULL)
            extended_input[0] = '\0';
        input = extended_input;
        strncat(input, buff, 1023);
        // i know this is slow, but no need for optimization since it can't get faster than user's typing :)
        if (input[strlen(input)-1] == '\n' && strncmp(buff, "class ", 6) && strncmp(buff, "def ", 4) && buff[0] != ' ')
            break;
        buff[0]='\0';
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
        atom_tree_t *tree = malloc(sizeof(atom_tree_t));
        g_array_append_val(trees, tree);
        stream = fmemopen(input, strlen(input), "r");
        struct t_tokenizer *tokenizer = new_tokenizer();
        int success = tokenize_stream(stream, tree, tokenizer);
        assert(tokenizer->error != PARSE_ERROR);
        // fclose(stream);
        free(input);
        tree->root = parse_block(tokenizer, -1);
        assert(tokenizer->error != PARSE_ERROR);
        free_tokenizer(tokenizer);
        // this is for showing what we get as the ast tree, we won't have this when it is finished
        char buff[10000];
        buff[0] = '\0';
        print_atom(tree->root, buff, 0, FALSE);
        printf("%s\n", buff);
        printd("interpreting\n", buff);
        interpret_block(tree->root, interpreter.globals, 0);
        assert(interpreter.error != RUN_ERROR);
        g_hash_table_foreach(interpreter.globals, print_var_each, NULL);
    }
    atom_tree_t *tree;
    for (int i=0; tree = g_array_index(trees, atom_tree_t *, i); i++)
        free_atom_tree(tree->root);
    g_array_free(trees, TRUE);
}
