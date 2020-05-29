#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "common.h"

// Interleave with simple shifts and bit operations.
unsigned short interleave_obvious(unsigned short in) {
	unsigned short l = in & 0xFF;
	unsigned short h = in >> 8;
	return
		  (h << 8 & 0x8000)
		| (h << 7 & 0x2000)
		| (h << 6 & 0x0800)
		| (h << 5 & 0x0200)
		| (h << 4 & 0x0080)
		| (h << 3 & 0x0020)
		| (h << 2 & 0x0008)
		| (h << 1 & 0x0002)
		| (l << 7 & 0x4000)
		| (l << 6 & 0x1000)
		| (l << 5 & 0x0400)
		| (l << 4 & 0x0100)
		| (l << 3 & 0x0040)
		| (l << 2 & 0x0010)
		| (l << 1 & 0x0004)
		| (l & 0x0001);
}

// Interleave by multiplication.
unsigned short interleave_multiply(unsigned short in) {
	unsigned long long l = in & 0xFF;
	unsigned long long h = in >> 8;
	return (((((l * 0x0101010101010101ULL) & 0x8040201008040201ULL)
		 * 0x0102040810204081ULL) >> 49) & 0x5555)
	| (((((h * 0x0101010101010101ULL) & 0x8040201008040201ULL)
		 * 0x0102040810204081ULL) >> 48) & 0xAAAA);
}

// Interleave by magic numbers.
unsigned short interleave_magic(unsigned short in) {
	unsigned int r0 = in;
	unsigned int r1 = (r0 | (r0 << 8)) & 0x00FF00FF;
	unsigned int r2 = (r1 | (r1 << 4)) & 0x0F0F0F0F;
	unsigned int r3 = (r2 | (r2 << 2)) & 0x33333333;
	unsigned int r4 = (r3 | (r3 << 1)) & 0x55555555;

	return (r4 | (r4 >> 15)) & 0xFFFF;
}

// Verify the checksum. Partly as a consitency check, partly to make it harder
// for the optimiser to delete the code we're trying to benchmark.
void checksum(char* name, int32_t s) {
	if (s != 0xFFCE0000) {
		printf("Checksum failed in benchmark_%s: %08X", name, s);
		exit(1);
	}
}

clock_t benchmark_obvious(int iterations) {
	clock_t clock0 = clock();
	int32_t s = 0;
	for (int r = 0; r < iterations; r++) {
		for (int i = 0; i < 65536; i++) {
				s += interleave_obvious(i);
		}
	}
	checksum("obvious", s);
	return clock() - clock0;
}
clock_t benchmark_multiply(int iterations) {
	clock_t clock0 = clock();
	int32_t s = 0;
	for (int r = 0; r < iterations; r++) {
		for (int i = 0; i < 65536; i++) {
				s += interleave_multiply(i);
		}
	}
	checksum("multiply", s);
	return clock() - clock0;
}
clock_t benchmark_magic(int iterations) {
	clock_t clock0 = clock();
	int32_t s = 0;
	for (int r = 0; r < iterations; r++) {
		for (int i = 0; i < 65536; i++) {
				s += interleave_magic(i);
		}
	}
	checksum("magic", s);
	return clock() - clock0;
}

unsigned short interleave_table[65536];
void init_interleave_table() {
	for (int i = 0; i < 65536; i++) {
			interleave_table[i] = interleave_obvious(i);
	}
}

clock_t benchmark_table(int iterations) {
	clock_t clock0 = clock();
	int32_t s = 0;
	for (int r = 0; r < iterations; r++) {
		for (int i = 0; i < 65536; i++) {
				s += interleave_table[i];
		}
	}
	checksum("table", s);
	return clock() - clock0;
}

bool test() {
	bool r = true;
	for (int i = 0; i < 65536; i++) {
		unsigned short r1 = interleave_obvious(i);
		unsigned short r2 = interleave_multiply(i);
		unsigned short r3 = interleave_magic(i);
		if (r1 != r2 || r1 != r3) {
			printf("Failure at %04X: %04X %04X %04X\n", i, r1, r2, r3);
			r = false;
		}
	}
	return r;
}

int main() {
	if (!test()) {
		exit(1);
	}
	init_interleave_table();
	int iterations = 100;
	benchmark("interleave_obvious", iterations, &benchmark_obvious);
	benchmark("interleave_table", iterations, &benchmark_table);
	benchmark("interleave_multiply", iterations, &benchmark_multiply);
	benchmark("interleave_magic", iterations, &benchmark_magic);
}

