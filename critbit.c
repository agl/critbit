#define _POSIX_C_SOURCE  200112
#define uint8  uint8_t
#define uint32  uint32_t

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <errno.h>

typedef struct {
	void *child[2];
	uint32 byte;
	uint8 otherbits;
} critbitn_node;

typedef struct {
	void *root;
	size_t keylen;
	size_t valuelen;
} critbitn_tree;

void *critbitn_lookup(critbitn_tree *tree, const void *key) {
	const uint8 *ubytes = (void *) key;
	uint8 *p = tree->root;
	
	// empty tree
	if (!p) return NULL;

	// go on as long as p is an internal node
	while (1 & (intptr_t) p) {
		critbitn_node *q = (void *) (p - 1);

		// condition disabled for fixed, uniform length keys
		// uint8 c = 0; if (q->byte < tree->keylen) c = ubytes[q->byte];
		uint8 c = ubytes[q->byte];
		const int direction = (1 + (q->otherbits | c)) >> 8;

		p = q->child[direction];
	}

	if (0 == memcmp(key, (const void *) p, tree->keylen))
		return p + tree->keylen;
	return NULL;
}

int critbitn_insert(critbitn_tree *tree, const void *key) {
	const uint8 *const ubytes = (void *) key;
	uint8 *p = tree->root;

	if (!p) {
		void *x;
		int a = posix_memalign(&x, sizeof(x), tree->keylen + tree->valuelen);
		if (a) return 0;
		memcpy(x, key, tree->keylen + tree->valuelen);
		tree->root = x;
		return 2;
	}

	while (1 & (intptr_t) p) {
		critbitn_node *q = (void *) (p - 1);

		//uint8 c = 0; if (q->byte < tree->keylen)
		uint8 c = ubytes[q->byte];
		const int direction = (1 + (q->otherbits | c)) >> 8;

		p = q->child[direction];
	}

	uint32 newbyte, newotherbits;

	for (newbyte = 0; newbyte < tree->keylen; ++newbyte) {
		if (p[newbyte] != ubytes[newbyte]) {
			newotherbits = p[newbyte] ^ ubytes[newbyte];
			goto different_byte_found;
		}
	}

	// key exists, update value
	memcpy(&p[tree->keylen], &ubytes[tree->keylen], tree->valuelen);
	return 1;

different_byte_found:

	while (newotherbits & (newotherbits - 1)) newotherbits &= newotherbits - 1;
	newotherbits ^= 255;
	uint8 c = p[newbyte];
	int newdirection = (1 + (newotherbits | c)) >> 8;

	critbitn_node *newnode;
	if (posix_memalign((void **) &newnode, sizeof(void *), sizeof(critbitn_node)))
		return 0;

	void *x;
	if (posix_memalign(&x, sizeof(x), tree->keylen + tree->valuelen)) {
		free(newnode);
		return 0;
	}
	memcpy(x, ubytes, tree->keylen + tree->valuelen);

	newnode->byte = newbyte;
	newnode->otherbits = newotherbits;
	newnode->child[1 - newdirection] = x;

	void **wherep = &tree->root;
	for (;;) {
		uint8 *p = *wherep;
		if (!(1 & (intptr_t) p)) break;
		critbitn_node *q = (void *) (p - 1);
		if (q->byte > newbyte) break;
		if (q->byte == newbyte && q->otherbits > newotherbits) break;
		//uint8 c = 0; if (q->byte < tree->keylen)
		uint8 c = ubytes[q->byte];
		const int direction = (1 + (q->otherbits | c)) >> 8;
		wherep = q->child + direction;
	}

	newnode->child[newdirection] = *wherep;
	*wherep = (void *) (1 + (void *) newnode);

	return 2;
}

int critbitn_delete(critbitn_tree *tree, const void *key) {
	const uint8 *ubytes = (void *) key;
	uint8 *p = tree->root;
	void **wherep = &tree->root;
	void **whereq = 0;
	critbitn_node *q = 0;
	int direction = 0;

	if (!p) return 0;

	while (1 & (intptr_t) p) {
		whereq = wherep;
		q = (void *) (p - 1);
		//uint8 c = 0; if (q->byte < tree->keylen)
		uint8 c = ubytes[q->byte];
		direction = (1 + (q->otherbits | c)) >> 8;
		wherep = q->child + direction;
		p = *wherep;
	}

	if (0 != memcmp(key, (const void *) p, tree->keylen)) return 0;
	free(p);

	if (!whereq) {
		tree->root = 0;
		return 1;
	}

	*whereq = q->child[1 - direction];
	free(q);

	return 1;
}

static void clear_traverse(void *top) {

	uint8 *p = top;

	if (1 & (intptr_t) p) {
		critbitn_node *q = (void *) (p - 1);
		clear_traverse(q->child[0]);
		clear_traverse(q->child[1]);
		free(q);
	} else {
		free(p);
	}

}

void critbitn_clear(critbitn_tree *tree) {
	if (tree->root) clear_traverse(tree->root);
	tree->root = NULL;
}

static int traverse(uint8 *top,
    	int (*handle) (const void *, void *), void *arg) {

	if (1 & (intptr_t) top) { // internal node
		critbitn_node *q = (void *) (top - 1);
		for (int direction = 0; direction < 2; ++direction)
			switch(traverse(q->child[direction], handle, arg)) {
			case 1:
				break; // continue with right node or done
			case 0:
				return 0; // handle function aborted traversal
			default:
				return -1; // handle function returned incorrect value
			}
		return 1; // done traversing this node
	} else { // leaf node
		return handle((const void *) top, arg);
	}
}

int critbitn_allprefixed(critbitn_tree *tree, const void *prefix,
		int prefixbytes, int (*handle) (const void *, void *), void *arg) {
	const uint8 *ubytes = (void *) prefix;
	uint8 *p = tree->root;
	uint8 *top = p;

	if (!p) return 1; /* S = $\emptyset$ */
	// else if (prefixbytes > tree->keylen) return 3;

	while (1 & (intptr_t) p) {
		critbitn_node *q = (void *) (p - 1);
		uint8 c = 0;
		if (q->byte < prefixbytes) c = ubytes[q->byte];
		const int direction = (1 + (q->otherbits | c)) >> 8;
		p = q->child[direction];
		if (q->byte < prefixbytes) top = p;
	}

	for (size_t i=0; i<prefixbytes; ++i)
		if (p[i] != ubytes[i]) return 1;

	return traverse(top, handle, arg);
}

char* binstr(long x, char *so) {
	/* produce a string with a binary representation of a long integer. */
	char s[8*sizeof(long)+1];
	int  i = 8*sizeof(long);
	s[i--]=0x00;   // terminate string
	do { // fill in array from right to left
		s[i--]=(x & 1) ? '1':'0';  // determine bit
		x>>=1;  // shift right 1 bit
	} while( x > 0);
	i++;   // point to last valid character
	sprintf(so,"%s",s+i); // stick it in the temp string string
	return so;
}

void dump_traverse(void *top, int n) {
	/* dump a representation of the tree with levels marked using indentation.
	*/
	char so[8*sizeof(long)+1];
	uint8 *p = top;
	if (1 & (intptr_t) p) {
		critbitn_node *q = (void *) (p - 1);
		for (int m=0; m<n; ++m) printf("    ");
		printf("node byte=%u, bit=%hhu\n", q->byte,
			__builtin_ffs((unsigned int)~(q->otherbits)));
		dump_traverse(q->child[0], n + 1);
		dump_traverse(q->child[1], n + 1);
	} else {
		for (int m=0; m<n; ++m) printf("    ");
		//printf("leaf: %xd %s\n", *(long *)p, binstr(*(long *)p));
		binstr(*(long *)p, so);
		printf("leaf key: %s value: %ld\n", so, *((long *)p + 1));
	}
}

