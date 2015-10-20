#ifndef RADIXTREE
#define RADIXTREE

#define RADIXTREE_KEYSIZE 128

typedef struct rxt_node {
    int color;
    char *key;
    int ksize;
    void *value;
    int pos; // bit index of the key to compare at (critical position)
    long keycache[RADIXTREE_KEYSIZE/sizeof(long)];
    int level; // tree level; for debug only
    int parent_id; //for debug only
    struct rxt_node *parent;
    struct rxt_node *left;
    struct rxt_node *right;
}rxt_node;

int rxt_put(char*, void *, rxt_node*);
void* rxt_get(char*, rxt_node*);
void* rxt_delete(char*, rxt_node*);

int rxt_put2(void *key, int ksize, void *value, rxt_node *n);
void* rxt_get2(void*, int ksize, rxt_node*);
void* rxt_delete2(void*, int ksize, rxt_node*);

void rxt_free(rxt_node *);
rxt_node *rxt_init();

#endif // RADIXTREE
