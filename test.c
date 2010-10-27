#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "radix.h"

static char *reverse(char *c)
{
    //in-place reversal
    int len = strlen(c), i;
    char *str = malloc(len + 1);
    strcpy(str, c);
    for (i = 0; i < len/2; i++) {
        int tmp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = tmp;
    }
    str[len] = '\0';
    return str;
}

static inline int keylen(char *str)
{
    return 8 * (strlen(str) + 1) - 1;
}

#define NEWLEAF(str) { .color = 1, \
                       .key = str, \
                       .value = reverse(str), \
                       .keylen = keylen(str)  }

#define BINLEAF(val, bits) { \
    .color = 1, \
    .key = val, \
    .keylen = bits }

static int count_comparisons(node *root)
{
    if (!root || root->color) return 0;
    return root->pos + count_comparisons(root->left) + count_comparisons(root->right);
}

static void print_in_order(node *root)
{
    if (!root) return;
    if (root->color) {
        printf("%s: %s\n", root->key, (char*)root->value);
        return;
    }
    print_in_order(root->left);
    print_in_order(root->right);
}

static void print_value(char *key, node *root)
{
    char *value = rdx_get(key, root);
    if (value)
        printf("%s: %s\n", key, value);
    else
        printf("%s: NOT FOUND\n", key);
}

int main(int argc, char **argv)
{
    node root;
    root.left = root.right = NULL;
    root.parent = root.value = NULL;
    root.color = 0;

#define INSERT(val) insert(&val, &root)
#define RGET(val) print_value(val, &root)

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
    leaf fooquux = NEWLEAF("fooquux"),
         foobar = NEWLEAF("foobar"),
         foo = NEWLEAF("foo"),
         foospace = NEWLEAF("foospace"),
         fooquick = NEWLEAF("fooquick"),
         bar = NEWLEAF("bar"),
         fooa = NEWLEAF("fooa"),
         gosh = NEWLEAF("gosh");

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
    leaf romane = NEWLEAF("romane"),
         romanus = NEWLEAF("romanus"),
         romulus = NEWLEAF("romulus"),
         rubens = NEWLEAF("rubens"),
         ruber = NEWLEAF("ruber"),
         rubicon = NEWLEAF("rubicon"),
         rubicundus = NEWLEAF("rubicundus");
    insert(&romane, &root);
    insert(&romanus, &root);
    insert(&romulus, &root);
    insert(&rubens, &root);
    insert(&ruber, &root);
    insert(&rubicon, &root);
    insert(&rubicundus, &root);

    print(&root);
    RGET("romane");
    RGET("romulus");
    RGET("rubens");
    RGET("ruber");
    RGET("rubicon");
    RGET("romanus");
#endif

    print_in_order(&root);
    printf("COMPARISONS: %d\n", count_comparisons(&root));

#undef INSERT
#undef RGET

}
