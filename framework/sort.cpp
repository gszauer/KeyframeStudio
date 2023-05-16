#include "sort.h"
#include "../debt/qsort.h"

static void SortInternal_Swap(void* a, void* b, unsigned int bytes) {
	unsigned char* x = (unsigned char*)a;
	unsigned char* y = (unsigned char*)b;
	unsigned char z = 0;
	for (unsigned int b = 0; b < bytes; ++b) {
		z = x[b];
		x[b] = y[b];
		y[b] = z;
	}
}

void QSort(void* base, unsigned int nitems, unsigned int size, int (*compare)(const void*, const void*)) {
	unsigned char* m = (unsigned char*)base;

#define QSRT_LESS(x, y) (compare(m + (x) * size, m + (y) * size) < 0)
#define QSRT_SWAP(i, j) SortInternal_Swap(m + (i) * size, m + (j) * size, size)
	QSORT(nitems, QSRT_LESS, QSRT_SWAP);
#undef QSRT_LESS
#undef QSRT_SWAP
}