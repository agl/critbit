#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
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

typedef struct {
	unsigned int a;
	unsigned long long b;
} Item;
typedef struct {
	Item key;
	double value;
} ItemEntry;

static void test_perf(long N) {
	critbitn_tree tree = {0, sizeof(Item), sizeof(double)};
	Item item;
	ItemEntry entry;
	clock_t start;
	void *p;
	entry.key = item;
	//printf("\nN = %ld\n", N);

	// insert elem[i] as key, with elem[i+1] as value
	start = clock();
	for (unsigned i = 0; i < N; ++i) {
		entry.key.a = i % 1023;
		entry.key.b = i ^ (i << 6);
		entry.value = 0.234;
		if (critbitn_insert(&tree, &entry) != 2) abort();;
	}
	//printf("insert %lgs\n", ((double) (clock() - start)) / CLOCKS_PER_SEC);

	start = clock();
	for (unsigned i = 0; i < N; ++i) {
		entry.key.a = i % 1023;
		entry.key.b = i ^ (i << 6);
		p = critbitn_lookup(&tree, &entry.key);
		if (p == NULL) abort();
		if (*(double *)p != 0.234) abort();
		entry.value = (i % 10) + 0.432;
		if (critbitn_insert(&tree, &entry) != 1) abort();
	}
	//printf("update %lgs\n", ((double) (clock() - start)) / CLOCKS_PER_SEC);

	start = clock();
	for (unsigned i = 0; i < N; ++i) {
		item.a = i % 1023;
		item.b = i ^ (i << 6);
		p = critbitn_lookup(&tree, &item);
		if (p == NULL) abort();
		if (*(double *)p != (i % 10) + 0.432) abort();
	}
	//printf("lookup %lgs\n", ((double) (clock() - start)) / CLOCKS_PER_SEC);

	start = clock();
	critbitn_clear(&tree);
	//printf("clear %lgs\n", ((double) (clock() - start)) / CLOCKS_PER_SEC);
}

static void test_perf1(long N) {
	critbitn_tree trees[1024]; // {0, sizeof(Item), sizeof(double)};
	Item item;
	ItemEntry entry;
	void *p;
	clock_t start;
	entry.key = item;
	printf("\nN = %ld\n", N);
	for (unsigned i = 0; i < 1024; ++i) {
		trees[i].root = 0;
		trees[i].keylen = sizeof(Item);
		trees[i].valuelen = sizeof(double);
	}

	// insert elem[i] as key, with elem[i+1] as value
	start = clock();
	for (unsigned i = 0; i < N; ++i) {
		entry.key.a = i % 1023;
		entry.key.b = i ^ (i << 6);
		entry.value = 0.234;
		if (critbitn_insert(&trees[entry.key.a], &entry) != 2) abort();;
	}
	printf("insert %lgs\n", ((double) (clock() - start)) / CLOCKS_PER_SEC);

	start = clock();
	for (unsigned i = 0; i < N; ++i) {
		entry.key.a = i % 1023;
		entry.key.b = i ^ (i << 6);
		p = critbitn_lookup(&trees[entry.key.a], &entry.key);
		if (p == NULL) abort();
		if (*(double *)p != 0.234) abort();
		entry.value = (i % 10) + 0.432;
		if (critbitn_insert(&trees[entry.key.a], &entry) != 1) abort();
	}
	printf("update %lgs\n", ((double) (clock() - start)) / CLOCKS_PER_SEC);

	start = clock();
	for (unsigned i = 0; i < N; ++i) {
		item.a = i % 1023;
		item.b = i ^ (i << 6);
		p = critbitn_lookup(&trees[item.a], &item);
		if (p == NULL) abort();
		if (*(double *)p != (i % 10) + 0.432) abort();
	}
	printf("lookup %lgs\n", ((double) (clock() - start)) / CLOCKS_PER_SEC);

	start = clock();
	for (unsigned i = 0; i < 1024; ++i)
		critbitn_clear(&trees[i]);
	printf("clear %lgs\n", ((double) (clock() - start)) / CLOCKS_PER_SEC);
}

int main() {
	long N = 1553301;
	//for (int a=0; a<1000; ++a) test_perf(10000);
	test_perf(755330 / 4);
	test_perf(755330 / 2);
	test_perf(755330);
	test_perf(1553301);
	test_perf(2 * 1553301);
	return 0;

	test_lookup();
	test_update();
	test_delete();
	test_allprefixed();
	//test_perf(N);
	printf("Succes.\n");
	return 0;
}
