#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "radix.h"

static char *reverse(char *c)
{
    int len = strlen(c), i;
    char *str = malloc(len + 1);
    for (i = 0; i < len; i++)
        str[i] = c[len - i - 1];
    str[len] = '\0';
    return str;
}

static int count_comparisons(rxt_node *root)
{
    if (!root || root->color) return 0;
    return root->pos + count_comparisons(root->left) + count_comparisons(root->right);
}

static void print_in_order(rxt_node *root)
{
    if (!root) return;
    if (root->color) {
        if (2 == root->color) root = root->value;
        printf("%s: %s\n", root->key, (char*)root->value);
        return;
    }
    print_in_order(root->left);
    print_in_order(root->right);
}

static void print_value(char *key, rxt_node *root)
{
    char *value = rxt_get(key, root);
    if (value)
        printf("%s: %s\n", key, value);
    else
        printf("%s: NOT FOUND\n", key);
}

int main(int argc, char **argv)
{
    rxt_node *root = rxt_init();

#define INSERT(val) rxt_put(#val, reverse(#val), root)
#define RGET(val) print_value(val, root)
#define RDEL(val) rxt_delete(#val, root)

#if 0
// this test set is somewhat broken
leaf b1010 = BINLEAF("\xa", 5),
     b1011 = BINLEAF("\xb", 5),
     b10   = BINLEAF("\x2", 2),
     b110  = BINLEAF("\x6", 6),
     b1    = BINLEAF("\x1", 2);

INSERT(b1010);
INSERT(b1011);
INSERT(b10);
INSERT(b110);
INSERT(b1);
print(&root);
#endif

#if 0
    INSERT(gosh);
    INSERT(foo);
    INSERT(foobar);
    INSERT(fooquux);
    INSERT(bar);
    INSERT(fooa);
    INSERT(fooquick);
    INSERT(foospace);
    //INSERT(foobar);
    print(&root);
    RGET("foospace");
    RGET("foobar");
    RGET("foo");
    RGET("fooquux");
    RGET("fooquux");
    RGET("gosh");
    RGET("fooquick");
    RGET("bar");
    RGET("omgwtfbbq");
#endif

#if 1
    INSERT(romane);
    INSERT(romanus);
    INSERT(rubicon);
    INSERT(romulus);
    INSERT(romulus);
    INSERT(rubens);
    INSERT(ruber);
    INSERT(rubicundus);
    RDEL(rubicon);
    RDEL(romane);
    RDEL(romulus);
    RDEL(rubens);
    rxt_print(&root); printf("-------------\n");

    RGET("romane");
    RGET("romanus");
    RGET("romulus");
    RGET("rubicon");
    RGET("rubens");
    RGET("ruber");
    RGET("rubicundus");
#endif

    printf("---------------\n");
    //print_in_order(&root);
    printf("COMPARISONS: %d\n", count_comparisons(root));

    rxt_free(root);

#undef INSERT
#undef RGET
#undef RDEL

}
