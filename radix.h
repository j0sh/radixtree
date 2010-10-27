#ifndef RADIXTREE
#define RADIXTREE

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

void print(node*);
int rxt_put(char*, void *, node*);
void* rdx_get(char*, node*);

#endif // RADIXTREE
