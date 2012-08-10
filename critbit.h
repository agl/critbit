#ifndef CRITBIT_H_
#define CRITBIT_H_

typedef struct {
  void *root;
  size_t keylen;
  size_t valuelen;
} critbitn_tree;

void *critbitn_lookup(critbitn_tree *tree, const void *key);
int critbitn_insert(critbitn_tree *tree, const void *key);
int critbitn_delete(critbitn_tree *tree, const void *key);
void critbitn_clear(critbitn_tree *tree);
int critbitn_allprefixed(critbitn_tree *tree, const void *prefix,
		int prefixbits, int (*handle) (const void *, void *), void *arg);
void dump_traverse(void *top, int n);

#endif  // CRITBIT_H_
