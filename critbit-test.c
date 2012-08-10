#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "critbit.h"

typedef struct {
	long key, value;
} Entry;

static void test_lookup() {
	critbitn_tree tree = {0, sizeof(long), sizeof(long)};

	static const long elems[] = {123, 456, 11223, 211, 34567, 8754, 1234, 0};
	long nonelem;
	void *p;

	// insert elem[i] as key, with elem[i+1] as value
	for (unsigned i = 0; elems[i]; ++i) critbitn_insert(&tree, &elems[i]);

	for (unsigned i = 0; elems[i]; ++i) {
		p = critbitn_lookup(&tree, &elems[i]);
		if (p == NULL) abort();
		if (*(long *)p != elems[i+1]) abort();
		nonelem = elems[i] - 1;
		if (critbitn_lookup(&tree, &nonelem) != NULL) abort();
	}

	critbitn_clear(&tree);
}

static void test_update() {
	critbitn_tree tree = {0, sizeof(long), sizeof(long)};
	Entry entry;

	static const long elems[] = {123, 456, 11223, 211, 34567, 8754, 1234, 0};
	long nonelem;
	void *p;

	// insert elem[i] as key, with elem[i+1] as value
	for (unsigned i = 0; elems[i]; ++i)
		if (critbitn_insert(&tree, &elems[i]) != 2) abort();
	for (unsigned i = 0; elems[i]; ++i) {
		entry.key = elems[i];
		entry.value = elems[i] + 2;
		if (critbitn_insert(&tree, &entry) != 1) abort();
	}

	for (unsigned i = 0; elems[i]; ++i) {
		p = critbitn_lookup(&tree, &elems[i]);
		if (p == NULL) abort();
		if (*(long *)p != elems[i] + 2) abort();
		nonelem = elems[i] - 1;
		if (critbitn_lookup(&tree, &nonelem) != NULL) abort();
	}

	critbitn_clear(&tree);
}

static void test_delete() {
	critbitn_tree tree = {0, sizeof(long), sizeof(long)};

	static const long elems[] = {123, 456, 11223, 211, 34567, 8754, 1234, 0};

	for (unsigned i = 1; elems[i]; ++i) {
		critbitn_clear(&tree);

		for (unsigned j = 0; j < i; ++j) critbitn_insert(&tree, &elems[j]);
		for (unsigned j = 0; j < i; ++j) {
			if (critbitn_lookup(&tree, &elems[j]) == NULL) abort();
		}
		for (unsigned j = 0; j < i; ++j) {
			if (1 != critbitn_delete(&tree, &elems[j])) abort();
		}
		for (unsigned j = 0; j < i; ++j) {
			if (critbitn_lookup(&tree, &elems[j]) != NULL) abort();
		}
	}

	critbitn_clear(&tree);
}

static int allprefixed_cb(const void *elem, void *arg) {
	long val = *((long *)elem);
	int n;
	for (n=0; ((long *)arg)[n] != 0; ++n)
		if (n > 20) return 2;
	((long *)arg)[n] = val;
	return 1;
}

static void test_prefixcond(int ret, const long elems[], long retval[],
	long prefix, int prefixbytes) {
	int prefixlen = 8 * prefixbytes, n, cnt = 0; 
	long mask = ((1UL << prefixlen) - 1);
	if (prefixbytes >= sizeof(prefix)) mask = ~0UL;
	printf("prefix=%lx ret=%d\n", prefix, ret);
	for (n=0; elems[n]; n++)
		if (~((elems[n] & mask) ^ prefix) == ~0UL) cnt++;
	for (n=0; n<20 && retval[n]; n++)
		if ((retval[n] & prefix) != prefix) {
			printf("item %ld does not have prefix %ld\n", retval[n], prefix);
			abort();
		}
	if (cnt != n) {
		printf("got %d items; expected %d items\n", n, cnt);
		abort();
	} else
		printf("correct number of items %d\n", n);
}

static void test_allprefixed() {
	critbitn_tree tree = {0, sizeof(long), sizeof(long)};

	static const long elems[] = {0xabab, 0xcdab, 0x1234, 0x134, 0xaabbcc,
		0xfefe00, 0xeebbcc, 0};
	long retval[20] = {0};
	

	for (unsigned i = 0; elems[i]; ++i)
		critbitn_insert(&tree, &elems[i]);
	dump_traverse(tree.root, 0);
	printf("\n");

	long prefix;
	int n, prefixlen;
	
	memset(retval, 0, sizeof(retval));
	prefix = 0; prefixlen = 0;
	n = critbitn_allprefixed(&tree, &prefix, 0, allprefixed_cb, &retval);
	test_prefixcond(n, elems, retval, prefix, prefixlen);

	prefix = 0xab; prefixlen = 1;
	memset(retval, 0, sizeof(retval));
	n = critbitn_allprefixed(&tree, &prefix, prefixlen, allprefixed_cb, &retval);
	test_prefixcond(n, elems, retval, prefix, prefixlen);

	memset(retval, 0, sizeof(retval));
	prefix = 0xbbcc; prefixlen = 2;
	n = critbitn_allprefixed(&tree, &prefix, prefixlen, allprefixed_cb, &retval);
	test_prefixcond(n, elems, retval, prefix, prefixlen);
	
	memset(retval, 0, sizeof(retval));
	prefix = 0x134; prefixlen = sizeof(prefix);
	n = critbitn_allprefixed(&tree, &prefix, prefixlen, allprefixed_cb, &retval);
	test_prefixcond(n, elems, retval, prefix, prefixlen);
	
	critbitn_clear(&tree);
}

int
main() {
	test_lookup();
	test_update();
	test_delete();
	test_allprefixed();
	printf("Succes.\n");
	return 0;
}
