#ifndef RADIXTREE
#define RADIXTREE

typedef struct rxt_node {
    int color;
    char *key; // only stores pos and beyond
    int level; // tree level; for debug only
    int parent_id; //for debug only
    int pos;
    void *value;
    struct rxt_node *parent;
    struct rxt_node *left;
    struct rxt_node *right;
}rxt_node;

void rxt_print(rxt_node*); // only for debugging small trees
int rxt_put(char*, void *, rxt_node*);
void* rxt_get(char*, rxt_node*);
void* rxt_delete(char*, rxt_node*);
void rxt_init(rxt_node *root);

#endif // RADIXTREE
