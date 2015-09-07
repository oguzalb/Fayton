#include "parse.h"
atom_t *parse_power(struct t_tokenizer *tokenizer);
atom_t *parse_term(struct t_tokenizer *tokenizer);
atom_t *parse_arith(struct t_tokenizer *tokenizer);
atom_t *parse_shift(struct t_tokenizer *tokenizer);
atom_t *parse_expr(struct t_tokenizer *tokenizer);
// for mac os
FILE *fmemopen (void *buf, size_t size, const char *opentype)
{
    FILE *f;

    assert(strcmp(opentype, "r") == 0);

    f = tmpfile();
    fwrite(buf, 1, size, f);
    rewind(f);

    return f;
}

void test_parse_func_basic(char* code, char *expect, atom_tree_t *tree, atom_t * (func)(struct t_tokenizer *)) {
    printf("%s\n", code);
    FILE *stream;
    stream = fmemopen(code, strlen(code), "r");
    struct t_tokenizer *tokenizer = new_tokenizer(FALSE);
    int success = tokenize_stream(stream, tree, tokenizer);
    assert(tokenizer->error != PARSE_ERROR);
    tree->root = func(tokenizer);
printf("PARSE FINISHED\n");
    assert(tokenizer->error != PARSE_ERROR);
    free_tokenizer(tokenizer);
    char *buff = NULL;
    print_atom(tree->root, &buff, 0, FALSE);
    free_atom_tree(tree->root);
    printf("%s\n", buff);
    assert(!strcmp(buff, expect));
    free(buff);

}
void test_parse_func(char* code, char *expect, atom_tree_t *tree, atom_t * (func)(struct t_tokenizer *, atom_t *)) {
    printf("%s\n", code);
    FILE *stream;
    stream = fmemopen(code, strlen(code), "r");
    struct t_tokenizer *tokenizer = new_tokenizer(FALSE);
    int success = tokenize_stream(stream, tree, tokenizer);
    assert(tokenizer->error != PARSE_ERROR);
    tree->root = func(tokenizer, NULL);
    assert(tokenizer->error != PARSE_ERROR);
    free_tokenizer(tokenizer);
    char *buff = NULL;
    print_atom(tree->root, &buff, 0, FALSE);
    free_atom_tree(tree->root);
    printf("%s\n", buff);
    assert(!strcmp(buff, expect));
    free(buff);
}
void test_parse_expr(char* code, char*expect, atom_tree_t *tree) {
    printf("%s\n", code);
    FILE *stream;
    stream = fmemopen(code, strlen(code), "r");
    struct t_tokenizer *tokenizer = new_tokenizer(FALSE);
    int success = tokenize_stream(stream, tree, tokenizer);
    assert(tokenizer->error != PARSE_ERROR);
    tree->root = parse_expr(tokenizer);
    assert(tokenizer->error != PARSE_ERROR);
    free_tokenizer(tokenizer);
    char *buff = NULL;
    print_atom(tree->root, &buff, 0, FALSE);
    free_atom_tree(tree->root);
    printf("%s\n", buff);
    assert(!strcmp(buff, expect));
    free(buff);
}
void test_parse_block(char* code, char* expect, atom_tree_t *tree) {
    printf("%s\n", code);
    FILE *stream;
    stream = fmemopen(code, strlen(code), "r");
    struct t_tokenizer *tokenizer = new_tokenizer(FALSE);
    int success = tokenize_stream(stream, tree, tokenizer);
    assert(tokenizer->error != PARSE_ERROR);
    tree->root = parse_block(tokenizer, -1);
    assert(tokenizer->error != PARSE_ERROR);
    free_tokenizer(tokenizer);
    char *buff = NULL;
    print_atom(tree->root, &buff, 0, FALSE);
    free_atom_tree(tree->root);
    printf("%s\n", buff);
    assert(!strcmp(buff, expect));
    free(buff);
}

int main () {
    static char buffer[] = "a+b+(c*3)";
    atom_tree_t tree;
    test_parse_func_basic("a", "VAR:a:-1\n", &tree, parse_power);
    test_parse_func_basic("1", "INTEGER:1\n", &tree, parse_power);
    test_parse_func_basic("\"hahaha\"", "STRING:hahaha\n", &tree, parse_power);
    test_parse_func_basic("a.b.c",
"ACCESSOR:.\n\
  ACCESSOR:.\n\
    VAR:a:-1\n\
    VAR:b:-1\n\
  VAR:c:-1\n", &tree, parse_power);
    test_parse_func_basic("a.b.c()",
"FUNCCALL:()call\n\
  ACCESSOR:.\n\
    ACCESSOR:.\n\
      VAR:a:-1\n\
      VAR:b:-1\n\
    VAR:c:-1\n\
  PARAMS:params\n", &tree, parse_power);
    test_parse_func_basic("a.b.c().d()",
"FUNCCALL:()call\n\
  ACCESSOR:.\n\
    FUNCCALL:()call\n\
      ACCESSOR:.\n\
        ACCESSOR:.\n\
          VAR:a:-1\n\
          VAR:b:-1\n\
        VAR:c:-1\n\
      PARAMS:params\n\
    VAR:d:-1\n\
  PARAMS:params\n", &tree, parse_power);
    test_parse_func_basic("a*b*c",
"FUNCCALL:()call\n\
  ACCESSOR:.\n\
    FUNCCALL:()call\n\
      ACCESSOR:.\n\
        VAR:a:-1\n\
        VAR:__mul__:-1\n\
      PARAMS:params\n\
        VAR:b:-1\n\
    VAR:__mul__:-1\n\
  PARAMS:params\n\
    VAR:c:-1\n", &tree, parse_term);
    test_parse_func_basic("a.b()*d()/a",
"FUNCCALL:()call\n\
  ACCESSOR:.\n\
    FUNCCALL:()call\n\
      ACCESSOR:.\n\
        FUNCCALL:()call\n\
          ACCESSOR:.\n\
            VAR:a:-1\n\
            VAR:b:-1\n\
          PARAMS:params\n\
        VAR:__mul__:-1\n\
      PARAMS:params\n\
        FUNCCALL:()call\n\
          VAR:d:-1\n\
          PARAMS:params\n\
    VAR:__div__:-1\n\
  PARAMS:params\n\
    VAR:a:-1\n", &tree, parse_term);
    test_parse_func_basic("a+b",
"FUNCCALL:()call\n\
  ACCESSOR:.\n\
    VAR:a:-1\n\
    VAR:__add__:-1\n\
  PARAMS:params\n\
    VAR:b:-1\n", &tree, parse_arith);
    test_parse_func_basic("a+b*c-3",
"FUNCCALL:()call\n\
  ACCESSOR:.\n\
    FUNCCALL:()call\n\
      ACCESSOR:.\n\
        VAR:a:-1\n\
        VAR:__add__:-1\n\
      PARAMS:params\n\
        FUNCCALL:()call\n\
          ACCESSOR:.\n\
            VAR:b:-1\n\
            VAR:__mul__:-1\n\
          PARAMS:params\n\
            VAR:c:-1\n\
    VAR:__sub__:-1\n\
  PARAMS:params\n\
    INTEGER:3\n", &tree, parse_arith);
    test_parse_func_basic("a<<b+c<<3",
"FUNCCALL:()call\n\
  ACCESSOR:.\n\
    FUNCCALL:()call\n\
      ACCESSOR:.\n\
        VAR:a:-1\n\
        VAR:__lshift__:-1\n\
      PARAMS:params\n\
        FUNCCALL:()call\n\
          ACCESSOR:.\n\
            VAR:b:-1\n\
            VAR:__add__:-1\n\
          PARAMS:params\n\
            VAR:c:-1\n\
    VAR:__rshift__:-1\n\
  PARAMS:params\n\
    INTEGER:3\n", &tree, parse_shift);
    test_parse_func_basic("func(a,1,1)",
"FUNCCALL:()call\n\
  VAR:func:-1\n\
  PARAMS:params\n\
    VAR:a:-1\n\
    INTEGER:1\n\
    INTEGER:1\n", &tree, parse_shift);
    test_parse_func_basic("func(a+1, b+5, (c+3)*4)",
"FUNCCALL:()call\n\
  VAR:func:-1\n\
  PARAMS:params\n\
    FUNCCALL:()call\n\
      ACCESSOR:.\n\
        VAR:a:-1\n\
        VAR:__add__:-1\n\
      PARAMS:params\n\
        INTEGER:1\n\
    FUNCCALL:()call\n\
      ACCESSOR:.\n\
        VAR:b:-1\n\
        VAR:__add__:-1\n\
      PARAMS:params\n\
        INTEGER:5\n\
    FUNCCALL:()call\n\
      ACCESSOR:.\n\
        FUNCCALL:()call\n\
          ACCESSOR:.\n\
            VAR:c:-1\n\
            VAR:__add__:-1\n\
          PARAMS:params\n\
            INTEGER:3\n\
        VAR:__mul__:-1\n\
      PARAMS:params\n\
        INTEGER:4\n", &tree, parse_expr);
    test_parse_func_basic("f1(f2(a+3), 7, f3(a+b*5))",
"FUNCCALL:()call\n\
  VAR:f1:-1\n\
  PARAMS:params\n\
    FUNCCALL:()call\n\
      VAR:f2:-1\n\
      PARAMS:params\n\
        FUNCCALL:()call\n\
          ACCESSOR:.\n\
            VAR:a:-1\n\
            VAR:__add__:-1\n\
          PARAMS:params\n\
            INTEGER:3\n\
    INTEGER:7\n\
    FUNCCALL:()call\n\
      VAR:f3:-1\n\
      PARAMS:params\n\
        FUNCCALL:()call\n\
          ACCESSOR:.\n\
            VAR:a:-1\n\
            VAR:__add__:-1\n\
          PARAMS:params\n\
            FUNCCALL:()call\n\
              ACCESSOR:.\n\
                VAR:b:-1\n\
                VAR:__mul__:-1\n\
              PARAMS:params\n\
                INTEGER:5\n", &tree, parse_expr);
    test_parse_block("a = int(1)\nb = a + 2",
"BLOCK:block\n\
  ASSIGNMENT:=\n\
    VAR:a:-1\n\
    FUNCCALL:()call\n\
      VAR:int:-1\n\
      PARAMS:params\n\
        INTEGER:1\n\
  ASSIGNMENT:=\n\
    VAR:b:-1\n\
    FUNCCALL:()call\n\
      ACCESSOR:.\n\
        VAR:a:-1\n\
        VAR:__add__:-1\n\
      PARAMS:params\n\
        INTEGER:2\n", &tree);
    test_parse_block("for i in range(1,10):\n    print(i)",
"BLOCK:block\n\
  FOR:FOR\n\
    VAR:i:-1\n\
    FUNCCALL:()call\n\
      VAR:range:-1\n\
      PARAMS:params\n\
        INTEGER:1\n\
        INTEGER:10\n\
    BLOCK:block\n\
      FUNCCALL:()call\n\
        VAR:print:-1\n\
        PARAMS:params\n\
          VAR:i:-1\n", &tree);
    test_parse_block("for i, j in range(1,10):\n    print(i, j)",
"BLOCK:block\n\
  FOR:FOR\n\
    TUPLE:tuple\n\
      VAR:i:-1\n\
      VAR:j:-1\n\
    FUNCCALL:()call\n\
      VAR:range:-1\n\
      PARAMS:params\n\
        INTEGER:1\n\
        INTEGER:10\n\
    BLOCK:block\n\
      FUNCCALL:()call\n\
        VAR:print:-1\n\
        PARAMS:params\n\
          VAR:i:-1\n\
          VAR:j:-1\n", &tree);

    test_parse_block(
"def add(a, b):\n\
    return a + b",
"BLOCK:block\n\
  FUNCDEF:add:-1\n\
    FREEVARS:\n\
      a:0\n\
      b:1\n\
    PARAMS:params\n\
      VAR:a:0\n\
      VAR:b:1\n\
    BLOCK:block\n\
      RETURN:return\n\
        FUNCCALL:()call\n\
          ACCESSOR:.\n\
            VAR:a:0\n\
            VAR:__add__:-1\n\
          PARAMS:params\n\
            VAR:b:1\n", &tree);
    test_parse_block("a = [1, 2, 3, 4]", 
"BLOCK:block\n\
  ASSIGNMENT:=\n\
    VAR:a:-1\n\
    LIST:list\n\
      INTEGER:1\n\
      INTEGER:2\n\
      INTEGER:3\n\
      INTEGER:4\n", &tree);
    test_parse_block("a = {1:5, 2:6, 3:7, 4:8}",
"BLOCK:block\n\
  ASSIGNMENT:=\n\
    VAR:a:-1\n\
    DICTIONARY:dict\n\
      INTEGER:1\n\
      INTEGER:5\n\
      INTEGER:2\n\
      INTEGER:6\n\
      INTEGER:3\n\
      INTEGER:7\n\
      INTEGER:4\n\
      INTEGER:8\n", &tree);
    test_parse_block(
"class Hede(object):\n\
    def __add__(self, other):\n\
        return self.value + other\n\
    value = 10\n",
"BLOCK:block\n\
  CLASS:Hede:-1\n\
    PARAMS:params\n\
      VAR:object:-1\n\
    FUNCDEF:__add__:-1\n\
      FREEVARS:\n\
        other:1\n\
        self:0\n\
      PARAMS:params\n\
        VAR:self:0\n\
        VAR:other:1\n\
      BLOCK:block\n\
        RETURN:return\n\
          FUNCCALL:()call\n\
            ACCESSOR:.\n\
              ACCESSOR:.\n\
                VAR:self:0\n\
                VAR:value:-1\n\
              VAR:__add__:-1\n\
            PARAMS:params\n\
              VAR:other:1\n\
    VAR:value:-1\n\
      INTEGER:10\n", &tree);
    test_parse_block(
"a = 5\n\
if a * 3 > 8:\n\
    print(\"a * 3 is greater than 8\")\n\
elif a * 3 < 8:\n\
    print(\"a * 3 is smaller than 8\")\n\
else:\n\
    print(\"a * 3 equals to 9\")\n", 
"BLOCK:block\n\
  ASSIGNMENT:=\n\
    VAR:a:-1\n\
    INTEGER:5\n\
  IF:if\n\
    FUNCCALL:()call\n\
      ACCESSOR:.\n\
        FUNCCALL:()call\n\
          ACCESSOR:.\n\
            FUNCCALL:()call\n\
              ACCESSOR:.\n\
                VAR:a:-1\n\
                VAR:__mul__:-1\n\
              PARAMS:params\n\
                INTEGER:3\n\
            VAR:__cmp__:-1\n\
          PARAMS:params\n\
            INTEGER:8\n\
        VAR:__eq__:-1\n\
      PARAMS:params\n\
        INTEGER:1\n\
    BLOCK:block\n\
      FUNCCALL:()call\n\
        VAR:print:-1\n\
        PARAMS:params\n\
          STRING:a * 3 is greater than 8\n\
    FUNCCALL:()call\n\
      ACCESSOR:.\n\
        FUNCCALL:()call\n\
          ACCESSOR:.\n\
            FUNCCALL:()call\n\
              ACCESSOR:.\n\
                VAR:a:-1\n\
                VAR:__mul__:-1\n\
              PARAMS:params\n\
                INTEGER:3\n\
            VAR:__cmp__:-1\n\
          PARAMS:params\n\
            INTEGER:8\n\
        VAR:__eq__:-1\n\
      PARAMS:params\n\
        INTEGER:-1\n\
    BLOCK:block\n\
      FUNCCALL:()call\n\
        VAR:print:-1\n\
        PARAMS:params\n\
          STRING:a * 3 is smaller than 8\n\
    BLOCK:block\n\
      FUNCCALL:()call\n\
        VAR:print:-1\n\
        PARAMS:params\n\
          STRING:a * 3 equals to 9\n", &tree);
    test_parse_func_basic("l[1:3:2]",
"FUNCCALL:()call\n\
  ACCESSOR:.\n\
    VAR:l:-1\n\
    VAR:__getitem__:-1\n\
  PARAMS:params\n\
    FUNCCALL:slicecall\n\
      VAR:slice:-1\n\
      PARAMS:params\n\
        INTEGER:1\n\
        INTEGER:3\n\
        INTEGER:2\n", &tree, parse_expr); 
    test_parse_func_basic("l[::]",
"FUNCCALL:()call\n\
  ACCESSOR:.\n\
    VAR:l:-1\n\
    VAR:__getitem__:-1\n\
  PARAMS:params\n\
    FUNCCALL:slicecall\n\
      VAR:slice:-1\n\
      PARAMS:params\n\
        VAR:None:-1\n\
        VAR:None:-1\n\
        VAR:None:-1\n", &tree, parse_expr); 

    test_parse_block("d = a.b.c[1:2:3][1]", 
"BLOCK:block\n\
  ASSIGNMENT:=\n\
    VAR:d:-1\n\
    FUNCCALL:()call\n\
      ACCESSOR:.\n\
        FUNCCALL:()call\n\
          ACCESSOR:.\n\
            ACCESSOR:.\n\
              ACCESSOR:.\n\
                VAR:a:-1\n\
                VAR:b:-1\n\
              VAR:c:-1\n\
            VAR:__getitem__:-1\n\
          PARAMS:params\n\
            FUNCCALL:slicecall\n\
              VAR:slice:-1\n\
              PARAMS:params\n\
                INTEGER:1\n\
                INTEGER:2\n\
                INTEGER:3\n\
        VAR:__getitem__:-1\n\
      PARAMS:params\n\
        INTEGER:1\n", &tree);
    test_parse_block("def func():\n\    yield a", 
"BLOCK:block\n\
  GENFUNCDEF:func:-1\n\
    FREEVARS:\n\
    PARAMS:params\n\
    BLOCK:block\n\
      YIELD:yield\n\
        VAR:a:-1\n", &tree);
    test_parse_block("def func():\n\    while True:\n\        yield a", 
"BLOCK:block\n\
  GENFUNCDEF:func:-1\n\
    FREEVARS:\n\
    PARAMS:params\n\
    BLOCK:block\n\
      WHILE:WHILE\n\
        VAR:True:-1\n\
        BLOCK:block\n\
          YIELD:yield\n\
            VAR:a:-1\n", &tree);
    test_parse_block("def func():\n\    def func2():\n\        yield a\n\    return 1",
"BLOCK:block\n\
  FUNCDEF:func:-1\n\
    FREEVARS:\n\
      func2:0\n\
    PARAMS:params\n\
    BLOCK:block\n\
      GENFUNCDEF:func2:0\n\
        FREEVARS:\n\
        PARAMS:params\n\
        BLOCK:block\n\
          YIELD:yield\n\
            VAR:a:-1\n\
      RETURN:return\n\
        INTEGER:1\n", &tree);
    test_parse_block("d[\"num\"] = 5",
"BLOCK:block\n\
  FUNCCALL:call()\n\
    ACCESSOR:.\n\
      VAR:d:-1\n\
      VAR:__setitem__:-1\n\
    PARAMS:params\n\
      STRING:num\n\
      INTEGER:5\n", &tree);
    test_parse_block("print([1,2,3][a.__cmp__(9)])\n",
"BLOCK:block\n\
  FUNCCALL:()call\n\
    VAR:print:-1\n\
    PARAMS:params\n\
      FUNCCALL:()call\n\
        ACCESSOR:.\n\
          LIST:list\n\
            INTEGER:1\n\
            INTEGER:2\n\
            INTEGER:3\n\
          VAR:__getitem__:-1\n\
        PARAMS:params\n\
          FUNCCALL:()call\n\
            ACCESSOR:.\n\
              VAR:a:-1\n\
              VAR:__cmp__:-1\n\
            PARAMS:params\n\
              INTEGER:9\n",&tree);
    test_parse_block(
"def func():\n\
    a = 5\n\
    def func2():\n\
        yield a\n\
        yield b",
"BLOCK:block\n\
  FUNCDEF:func:-1\n\
    FREEVARS:\n\
      func2:1\n\
      a:0\n\
    PARAMS:params\n\
    BLOCK:block\n\
      ASSIGNMENT:=\n\
        VAR:a:0\n\
        INTEGER:5\n\
      GENFUNCDEF:func2:1\n\
        FREEVARS:\n\
          a:0\n\
        PARAMS:params\n\
          CLOSURE:a\n\
        BLOCK:block\n\
          YIELD:yield\n\
            VAR:a:0\n\
          YIELD:yield\n\
            VAR:b:-1\n", &tree);
    test_parse_block(
"if not False:\n\
    print(1)",
"BLOCK:block\n\
  IF:if\n\
    NOT:not\n\
      VAR:False:-1\n\
    BLOCK:block\n\
      FUNCCALL:()call\n\
        VAR:print:-1\n\
        PARAMS:params\n\
          INTEGER:1\n",
&tree);
    test_parse_block(
"class Cat:\n\
    def __init__(self, value):\n\
        self.value = value\n\
c = Cat()\n\
print(c.value)",
"BLOCK:block\n\
  CLASS:Cat:-1\n\
    PARAMS:params\n\
    FUNCDEF:__init__:-1\n\
      FREEVARS:\n\
        value:1\n\
        self:0\n\
      PARAMS:params\n\
        VAR:self:0\n\
        VAR:value:1\n\
      BLOCK:block\n\
        ASSIGNMENT:=\n\
          ACCESSOR:.\n\
            VAR:self:0\n\
            VAR:value:-1\n\
          VAR:value:1\n\
  ASSIGNMENT:=\n\
    VAR:c:-1\n\
    FUNCCALL:()call\n\
      VAR:Cat:-1\n\
      PARAMS:params\n\
  FUNCCALL:()call\n\
    VAR:print:-1\n\
    PARAMS:params\n\
      ACCESSOR:.\n\
        VAR:c:-1\n\
        VAR:value:-1\n",
&tree);
    test_parse_block(
"a = 5\n\
assert a == 5, \"not 5\"\n\
assert a == 4", 
"BLOCK:block\n\
  ASSIGNMENT:=\n\
    VAR:a:-1\n\
    INTEGER:5\n\
  FUNCCALL:()call\n\
    VAR:assert:-1\n\
    PARAMS:params\n\
      FUNCCALL:()call\n\
        ACCESSOR:.\n\
          FUNCCALL:()call\n\
            ACCESSOR:.\n\
              VAR:a:-1\n\
              VAR:__cmp__:-1\n\
            PARAMS:params\n\
              INTEGER:5\n\
          VAR:__eq__:-1\n\
        PARAMS:params\n\
          INTEGER:0\n\
      STRING:not 5\n\
  FUNCCALL:()call\n\
    VAR:assert:-1\n\
    PARAMS:params\n\
      FUNCCALL:()call\n\
        ACCESSOR:.\n\
          FUNCCALL:()call\n\
            ACCESSOR:.\n\
              VAR:a:-1\n\
              VAR:__cmp__:-1\n\
            PARAMS:params\n\
              INTEGER:4\n\
          VAR:__eq__:-1\n\
        PARAMS:params\n\
          INTEGER:0\n",
&tree);
    //fclose(stream);
    return 0;
    //fclose(fp);
}

