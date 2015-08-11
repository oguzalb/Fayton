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

void test_interpret_block(char *code, atom_tree_t *tree) {
    FILE *stream;
    stream = fmemopen(code, strlen(code), "r");
    struct t_tokenizer *tokenizer = new_tokenizer();
    int success = tokenize_stream(stream, tree, tokenizer);
    assert(tokenizer->error != PARSE_ERROR);
    tree->root = parse_block(tokenizer, -1);
    free_tokenizer(tokenizer);
    assert(tokenizer->error != PARSE_ERROR);
    char buff[2048];
    buff[0] = '\0';
    print_atom(tree->root, buff, 0, FALSE);
    printf("%s\n", buff);
    printd("creating interpreter\n", buff);
    interpreter_t * interpreter = new_interpreter();
    printd("interpreting\n", buff);
    interpret_block(interpreter, tree->root, interpreter->globals, 0);
    assert(interpreter->error != RUN_ERROR);
    g_hash_table_foreach(interpreter->globals, print_var_each, NULL);
    free_atom_tree(tree->root);
}

int main() {
    atom_tree_t tree;
    test_interpret_block(
"a = int(1)\n\
b = a + 3", &tree);
    test_interpret_block(
"for i in range(1, 10):\n\
    print(i)", &tree);
    test_interpret_block(
"def add(a, b):\n\
    return a + b\n\
c = add(1,2)", &tree);
    test_interpret_block(
"for i in [1, 2, 3, 4]:\n\
    print(i)", &tree);
    test_interpret_block(
"a = {1:5, 2:6, 3:7, 4:8}", &tree);
    test_interpret_block(
"a = {1:5, 2:6, 3:7, 4:8}\n\
for i in a.keys():\n\
    print(i)", &tree);
    test_interpret_block(
"class Foo(object):\n\
    def __add__(self, other):\n\
        return self.value + other\n\
    value = 10\n\
foo = Foo()\n\
print(foo+4)", &tree);
    test_interpret_block(
"l = [\"one\", \"two\", \"three\", 4]\n\
for i in l:\n\
    print(i)\n", &tree);
    test_interpret_block(
"l = [1, 2, 3, 4, 5]\n\
print(sum(l))", &tree);
    test_interpret_block(
"class MyThread(Thread):\n\
    def run(self):\n\
        print(255)\n\
thread = MyThread()\n\
thread.run()\n", &tree);
    test_interpret_block("print(1==2)", &tree);
    test_interpret_block(
"def recsum(n, sum):\n\
    if n < 1:\n\
        return sum\n\
    else:\n\
        return recsum(n-1, sum+n)\n\
print(recsum(10, 0))", &tree);
    return 0;
}

