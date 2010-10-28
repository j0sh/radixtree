/*
 * a proper radix tree implementation.
 * works on strings of bytes.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "radix.h"

#ifdef MSB_FIRST
static inline int count_bits(char *k1, char *k2, int count)
{
    int mask = 128;
    while (~(*k1 ^ *k2) & mask && --count) {
        mask >>= 1;
        if (0 == mask) {
            mask = 128;
            k1 += 1;
            k2 += 1;
        }
    }
    return count;
}
#else
static inline int count_bits(char *k1, char *k2, int count)
{
    int mask = 1;
    while (~(*k1 ^ *k2) & mask && --count) {
        mask <<= 1;
        if (256 == mask) {
            mask = 1;
            k1 += 1;
            k2 += 1;
        }
    }
    return count;
}
#endif

static int count_common_bits(char *k1, char *k2, int max)
{
    int count = max;
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

    return max - count_bits(k1, k2, count);
}

#ifdef MSB_FIRST
static inline int shift(int i)
{
    return 128 >> (i & 7);
}
#else
static inline int shift(int i)
{
    return 1 << (i & 7);
}
#endif

static inline int get_bit_at(char *k, int i)
{
    // for 0 === i mod 8, look at the LSB.
    // zero based index, so for 1 == i mod 8,
    // look at second bit. slightly unintuitive
    int bytes = i >> 3;
    int mask = shift(i);
    k += bytes;
    return *k & mask;
}

static inline int rdx_min(int a, int b)
{
    return a > b ? b : a;
}

static int insert_leaf(rxt_node *newleaf, rxt_node *sibling, rxt_node *parent)
{
    int idx, bit, max_len;
    rxt_node *inner = malloc(sizeof(rxt_node));

    if (!inner) return -1;
    inner->color = 0;
    inner->value = NULL;
    inner->parent = parent;

    max_len = rdx_min(newleaf->pos, sibling->pos);
    idx  = count_common_bits(newleaf->key, sibling->key, max_len);
    bit = get_bit_at(newleaf->key, idx);

    if (!parent) {
        parent = sibling;
        // do an in-place transform. TODO ascii art
        inner->left = parent->left;
        inner->right = parent->right;
        inner->key = parent->key;
        inner->pos = parent->pos;
        parent->pos = idx;
        parent->left->parent = inner;
        parent->right->parent = inner;
        newleaf->parent = parent;
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
        return insert_leaf(newleaf, parent, parent->parent);
    } else {
        // otherwise, add newleaf as a child of inner
        inner->pos = idx;
        inner->key = newleaf->key;
        newleaf->parent = inner;
        sibling->parent = inner;

        if (bit) {
            inner->right = newleaf;
            inner->left = sibling;
        } else {
            inner->right = sibling;
            inner->left = newleaf;
        }

        // now find out which branch of parent to assign inner
        if (parent->left == sibling)
            parent->left = inner;
        else if (parent->right == sibling)
            parent->right = inner;
        else {
            fprintf(stderr, "inappropriate child %s/%s found in parent when inserting leaf %s (expected %s)\n", parent->left->key, parent->right->key, newleaf->key, sibling->key);
            return -1;
        }
    }
    return 0;
}

// color: 1 for leaf, 0 for inner
static int insert_internal(rxt_node *newleaf, rxt_node *n)
{
    // FIRST: check for common bits
    rxt_node *left = n->left, *right = n->right;
    int bits   = count_common_bits(newleaf->key, left->key,
                                   rdx_min(newleaf->pos, left->pos));
    int bits2  = count_common_bits(newleaf->key, right->key,
                                   rdx_min(newleaf->pos, right->pos));

    if (rdx_min(bits, bits2) < n->pos) {
        if (bits >= bits2)
            return insert_leaf(newleaf, n->left, n);
        return insert_leaf(newleaf, n->right, n);
    }

    if (bits >= bits2) {
         if (left->color)
            return insert_leaf(newleaf, n->left, n);
        insert_internal(newleaf, left);
    } else {
        if (right->color)
            return insert_leaf(newleaf, n->right, n);
        insert_internal(newleaf, right);
    }

    return -1; // this should never happen
}

static inline int keylen(char *str)
{
    return 8 * (strlen(str) + 1) - 1;
}

int rxt_put(char *key, void *value, rxt_node *n)
{
#define NEWLEAF(nl, k, v) \
    nl = malloc(sizeof(rxt_node)); \
    if (!nl) return -1; \
    nl->key = k; \
    nl->pos = keylen(k); \
    nl->value = v; \
    nl->color = 1; \
    nl->parent = n; \
    nl->left = NULL; \
    nl->right = NULL

    rxt_node *newleaf;

    // this special case takes care of the first two entries
    if (!(n->left || n->right)) {
        rxt_node *sib;
        int bits;
        NEWLEAF(newleaf, key, value);
        // create root
        if (!n->value) {
            // attach root
            n->color = 2;
            n->value = newleaf;
            return 0;
        }
        // else convert root to inner and attach leaves
        sib = n->value;
        NEWLEAF(newleaf, key, value);

        // count bits in common
        bits = count_common_bits(key, sib->key,
                    rdx_min(newleaf->pos, sib->pos));


        if (get_bit_at(key, bits)) {
            n->right = newleaf;
            n->left = sib;
        } else {
            n->right = sib;
            n->left = newleaf;
        }
        n->value = NULL;
        n->key = key;
        n->pos = bits;
        n->color = 0;
        return 0;
    }


    NEWLEAF(newleaf, key, value);
    newleaf->parent = NULL; // null for now

    return insert_internal(newleaf, n);

#undef NEWELEAF
}

// prints the tree level by level, for debug purposes.
// [%d/%d] indicates the current id, and the id of the parent.
// id is assigned in the order read from the queue.
void print(rxt_node *root)
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
                printf("%d[%d/%d] , ", n->pos, read, n->parent_id);

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

static rxt_node* get_internal(char *key, rxt_node *root)
{
    if (!root) return NULL;

    if (root->color) {
        if (2 == root->color) root = root->value;
        if (!strncmp(key, root->key, root->pos))
            return root;
        return NULL;
    }

    if (get_bit_at(key, root->pos))
        return get_internal(key, root->right);
    return get_internal(key, root->left);
}

void* rdx_get(char *key, rxt_node *root)
{
    rxt_node *n = get_internal(key, root);
    if (!n) return NULL;
    return n->value;
}
