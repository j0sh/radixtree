/*
 * a proper radix tree implementation.
 * works on strings of bytes.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t u8;

// THIS TYPE OF SHIT LEADS TO A LOT OF MEMORY FRAGMENTATION.
// TODO: ideal allocator? Possibly ARENA, and simulate
// features of an HIERARCHICAL allocator

typedef struct {
    char color;
    char *key;
    int level; // tree level; for debug only
    int parent_id; // for debug only
    int keylen; // size in bits
    void *value;
    struct node *parent;
}leaf;

typedef struct node {
    int color;
    char *key; // only stores pos and beyond
    int level; // tree level; for debug only
    int parent_id; //for debug only
    int pos;
    void *value;
    struct node *parent;
    void *left;
    void *right;
}node;

static int count_common_bits(char *k1, char *k2, int max)
{
    int count = max, mask = 1;
    // Look at the MSB first;
    // XXX SIMD-ify?
    while (*k1 == *k2 && count >= sizeof(int) * 8) {
        int *i1 = (int*)k1, *i2 = (int*)k2;
        if (*i1 == *i2) {
            k1 += sizeof(int);
            k2 += sizeof(int);
            count -= sizeof(int) * 8;
        } else break;
    }
    while (*k1 == *k2 && count >= 8) {
        k1++;
        k2++;
        count -= 8;
    }
    while (~(*k1 ^ *k2) & mask && --count) {
        mask <<= 1;
        if (128 == mask) {
            mask = 1;
            k1 += 1;
            k2 += 1;
        }
    }
    return max - count;
}

static inline int get_bit_at(char *k, int i)
{
    // for 0 === i mod 8, look at the LSB.
    // zero based index, so for 1 == i mod 8,
    // look at second bit. slightly unintuitive
    int bytes = i >> 3;
    int mask = 1 << (i & 7);
    k += bytes;
    return *k & mask;
}

static inline int rdx_min(int a, int b)
{
    return a > b ? b : a;
}

static int insert_leaf(leaf *newleaf, leaf *sibling, node *parent)
{
    int idx, bit, max_len;
    node *inner = malloc(sizeof(node));

    if (!inner) return -1;
    inner->color = 0;
    inner->value = NULL;
    inner->parent = parent;

    max_len = rdx_min(newleaf->keylen, sibling->keylen);
    idx  = count_common_bits(newleaf->key, sibling->key, max_len);
    bit = get_bit_at(newleaf->key, idx);

    if (!parent) {
        parent = (node*)sibling;
        // do an in-place transform. TODO ascii art
        printf("inserting %s in empty parent\n", newleaf->key);
        inner->left = parent->left;
        inner->right = parent->right;
        inner->key = parent->key;
        inner->pos = parent->pos;
        inner->parent = parent;
        parent->pos = idx;
        if (bit) {
            parent->right = newleaf;
            parent->left = inner;
        } else {
            parent->right = inner;
            parent->left = newleaf;
        }
        return 0;
    }

    if (idx < parent->pos) {
        // in this case, set inner to the two children of parent.
        return insert_leaf(newleaf, (leaf*)parent, parent->parent);
    } else {
        // otherwise, add newleaf as a child of inner
        leaf *target = sibling;
        inner->pos = idx;
        inner->key = newleaf->key;
        newleaf->parent = parent;

        // idx == maxlen indicates a prefix.
        // ensure it is on the wrong side.
        // why it has to be newleaf, i have no fucking clue
        if (idx == max_len && max_len == newleaf->keylen) {
            leaf *tmp = newleaf;
            printf("why not: new %s, sib %s\n", newleaf->key, sibling->key);
            newleaf = sibling;
            sibling = tmp;
            target = newleaf;
        }
        printf("inseting %s in populated node\n", newleaf->key);
        if (bit) {
            inner->right = newleaf;
            inner->left = sibling;
        } else {
            inner->right = sibling;
            inner->left = newleaf;
        }

        // now find out which branch of parent to assign inner
        if (parent->left == target)
            parent->left = inner;
        else if (parent->right == target)
            parent->right = inner;
        else {
            fprintf(stderr, "inappropriate child %s found in parent when inserting leaf %s (expected %s)\n", ((node*)parent->right)->key, newleaf->key, sibling->key);
            return -1;
        }
    }
    return 0;
}

// color: 1 for leaf, 0 for inner

int insert(leaf *newleaf, node *n)
{
    int mask, nklen = newleaf->keylen, bits, bits2;
    char *key = newleaf->key;
    // this special case takes care of the first two entries
    if (!(n->left || n->right)) {
        char *oldkey;
        int max_len;
        // create root
        if (!n->value) {
            // attach root
            n->value = newleaf;
            newleaf->parent = n;
            return 0;
        }
        // XXX BROKEN WHEN INSERTING A PREFIX

        // else convert root to inner and attach leaves
        oldkey = ((leaf*)n->value)->key;
        max_len = rdx_min(nklen, ((leaf*)n->value)->keylen);

        // count bits in common
        bits = count_common_bits(key, oldkey, max_len);

        if (bits == max_len) {
            // special case for prefixes: reverse them
            leaf *tmp = newleaf;
            newleaf = n->value;
            n->value = tmp;
        }

        if (get_bit_at(key, bits)) {
            n->right = newleaf;
            n->left = n->value;
        } else {
            n->right = n->value;
            n->left = newleaf;
        }
        newleaf->parent = n;
        n->value = NULL;
        n->key = key;
        n->pos = bits;
        return 0;
    }

    // FIRST: check for common bits
    node *lefty = n->left, *righty = n->right;
    if (lefty)
        bits = count_common_bits(key, lefty->key, rdx_min(nklen, lefty->pos)); else
        bits = -1;
    if (righty)
        bits2 = count_common_bits(key, righty->key, rdx_min(nklen, righty->pos));
    else
        bits2 = -1;

    if (bits >= bits2) {
         if (lefty->color)
            return insert_leaf(newleaf, n->left, n);
        insert(newleaf, lefty);
    } else {
        if (righty->color)
            return insert_leaf(newleaf, n->right, n);
        insert(newleaf, righty);
    }

    return 0;
}

// prints the tree level by level, for debug purposes.
// [%d/%d] indicates the current id, and the id of the parent.
// id is assigned in the order read from the queue.
void print(node *root)
{
    int i, write = 0, read = 0, prev_level = -1;
    node *queue[100];
    for (i = 0; i < 100; i++)queue[i] = NULL;
    root->parent_id = read;
    root->level = 0;
    queue[write++] = root;

    while (read != write) {
        node *n = queue[read++];
        // insert linebreak if needed
        if (prev_level != n->level) {
            printf("\n\nlevel %d:\n", n->level);
            prev_level = n->level;
        }
        if (n->color)
            printf("%s[%d/%d] , ", n->key, read, n->parent_id);
        else {
            if (n->value)
                printf("%d (%s)[%d/%d] ,", n->pos, ((leaf*)n->value)->key, read, n->parent_id);
            else
                printf("%d[%d/%d] , ", n->pos, read, n->parent_id);

            if (n->left) {
                node *left = n->left;
                left->level = n->level + 1;
                left->parent_id = read;
                queue[write++] = left;
            }
            if (n->right) {
                node *right = n->right;
                right->level = n->level + 1;
                right->parent_id = read;
                queue[write++] = right;
            }
        }
    }
    printf("\n");
}

static int get_internal(char *key, node *root, int len)
{
    if (!root) {
        printf("%s not found.\n", key);
        return 0;
    }
    if (root->color) {
        printf("found %s:%s\n", key, (char*)((leaf*)root)->value);
        return 0;
    }

    // recall that in the case of a prefix, we install it in the opposite
    if ((len != root->pos && get_bit_at(key, root->pos)) ||
        (len == root->pos && !get_bit_at(key, root->pos))) {
        get_internal(key, root->right, len);
    } else {
        get_internal(key, root->left, len);
    }
    return 0;
}

static inline int keylen(char *str)
{
    return 8 * strlen(str) - 1;
}

int rdx_get(char *key, node *root)
{
    return get_internal(key, root, keylen(key));
}

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

// count number of leading zeroes

#define NEWLEAF(str) { .color = 1, \
                       .key = str, \
                       .value = reverse(str), \
                       .keylen = keylen(str)  }

#define BINLEAF(val, bits) { \
    .color = 1, \
    .key = val, \
    .keylen = bits }

int main(int argc, char **argv)
{
    node root;
    root.left = root.right = NULL;
    root.parent = root.value = NULL;
    root.color = 0;

#define INSERT(val) insert(&val, &root)

#if 0
// this test set is somewhat broken
leaf b1010 = BINLEAF("\xa", 5),
     b1011 = BINLEAF("\xb", 5),
     b10   = BINLEAF("\x2", 2),
     b110  = BINLEAF("\x6", 6),
     b1    = BINLEAF("\x1", 2);

insert(&b1010, &root);
insert(&b1011, &root);
insert(&b10, &root);
insert(&b110, &root);
insert(&b1, &root);
#endif

#if 1
    leaf fooquux = NEWLEAF("fooquux"),
         foobar = NEWLEAF("foobar"),
         foo = NEWLEAF("foo"),
         foospace = NEWLEAF("foospace"),
         fooquick = NEWLEAF("fooquick"),
         bar = NEWLEAF("bar"),
         fooa = NEWLEAF("fooa"),
         gosh = NEWLEAF("gosh");

    INSERT(fooquick);
    INSERT(fooquux);
    INSERT(gosh);
    INSERT(foo);
    INSERT(bar);
    //INSERT(foobar);
    //INSERT(foospace);
    //INSERT(foobar);
#endif

#if 0
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
#endif

    print(&root);
    rdx_get("foospace", &root);
    rdx_get("foo", &root);
    rdx_get("fooquux", &root);
    rdx_get("bar", &root);
#undef INSERT

}
