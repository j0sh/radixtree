#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "radix.h"

// prints the tree level by level, for debug purposes.
// [%d/%d] indicates the current id, and the id of the parent.
// (%s) indicates an inner node's key.
// id is assigned in the order read from the queue.
// XXX only use this for debugging small (<50key) trees!
static void print_tree(rxt_node *root)
{
    int i, write = 0, read = 0, prev_level = -1;
    rxt_node *queue[100];
    for (i = 0; i < 100; i++)queue[i] = NULL;
    root->parent_id = read;
    root->level = 0;
    queue[write++] = root;

    while (read != write) {
        rxt_node *n = queue[read++];
        // insert linebreak if needed
        if (prev_level != n->level) {
            printf("\n\nlevel %d:\n", n->level);
            prev_level = n->level;
        }
        if (n->color)
            printf("%s[%d/%d] , ", n->key, read, n->parent_id);
        else {
            if (n->value)
                printf("%d (%s)[%d/%d] ,", n->pos, ((rxt_node*)n->value)->key, read, n->parent_id);
            else
                printf("%d (%s)[%d/%d] , ", n->pos, n->key, read, n->parent_id);

            if (n->left) {
                rxt_node *left = n->left;
                left->level = n->level + 1;
                left->parent_id = read;
                queue[write++] = left;
            }
            if (n->right) {
                rxt_node *right = n->right;
                right->level = n->level + 1;
                right->parent_id = read;
                queue[write++] = right;
            }
        }
    }
    printf("\n");
}

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
    print_tree(root); printf("-------------\n");

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

    return 0;
}
