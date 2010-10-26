#ifndef RADIXTREE
#define RADIXTREE

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

void print(node*);
int insert(leaf*, node*);
int rdx_get(char*, node*);

#endif // RADIXTREE
