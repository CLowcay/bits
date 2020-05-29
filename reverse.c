#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "common.h"

// Reverse with simple shifts and bit operations.
unsigned char reverse_obvious(unsigned char in) {
	return
		(in >> 7) | (in << 7)
		| ((in >> 5) & 0x02)
		| ((in << 5) & 0x40)
		| ((in >> 3) & 0x04)
		| ((in << 3) & 0x20)
		| ((in >> 1) & 0x08)
		| ((in << 1) & 0x10);
}

// Reverse by multiplication and modulus.
unsigned char reverse_multiply_mod(unsigned char in) {
	return (in * 0x0202020202ULL & 0x010884422010ULL) % 1023;
}

// Reverse by multiplication.
unsigned char reverse_multiply(unsigned char in) {
	return ((in * 0x80200802ULL) & 0x0884422110ULL) * 0x0101010101ULL >> 32;
}

// Reverse by magic numbers.
unsigned char reverse_magic(unsigned char in) {
	unsigned char in1 = ((in >> 1) & 0x55) | ((in & 0x55) << 1);
	unsigned char in2 = ((in1 >> 2) & 0x33) | ((in1 & 0x33) << 2);
	return ((in2 >> 4) & 0x0F) | ((in2 & 0x0F) << 4);
}

// Reverse by intrinsic.
unsigned char reverse_builtin(unsigned char in) {
	return __builtin_bitreverse8(in);
}

// Verify the checksum. Partly as a consitency check, partly to make it harder
// for the optimiser to delete the code we're trying to benchmark.
void checksum(char* name, int32_t s) {
	if (s != 0x13747800) {
		printf("Checksum failed in benchmark_%s: %08X", name, s);
		exit(1);
	}
}

clock_t benchmark_obvious(int iterations) {
	clock_t clock0 = clock();
	int32_t s = 0;
	for (int r = 0; r < iterations; r++) {
		for (int i = 0; i < 256; i++) {
			s += reverse_obvious(i);
		}
	}
	checksum("obvious", s);
	return clock() - clock0;
}
clock_t benchmark_multiply_mod(int iterations) {
	clock_t clock0 = clock();
	int32_t s = 0;
	for (int r = 0; r < iterations; r++) {
		for (int i = 0; i < 256; i++) {
			s += reverse_multiply_mod(i);
		}
	}
	checksum("multiply_mod", s);
	return clock() - clock0;
}
clock_t benchmark_multiply(int iterations) {
	clock_t clock0 = clock();
	int32_t s = 0;
	for (int r = 0; r < iterations; r++) {
		for (int i = 0; i < 256; i++) {
			s += reverse_multiply(i);
		}
	}
	checksum("multiply", s);
	return clock() - clock0;
}
clock_t benchmark_magic(int iterations) {
	clock_t clock0 = clock();
	int32_t s = 0;
	for (int r = 0; r < iterations; r++) {
		for (int i = 0; i < 256; i++) {
			s += reverse_magic(i);
		}
	}
	checksum("magic", s);
	return clock() - clock0;
}
clock_t benchmark_builtin(int iterations) {
	clock_t clock0 = clock();
	int32_t s = 0;
	for (int r = 0; r < iterations; r++) {
		for (int i = 0; i < 256; i++) {
			s += reverse_builtin(i);
		}
	}
	checksum("builtin", s);
	return clock() - clock0;
}

unsigned char reverse_table[256];
void initReverseTable() {
	for (int i = 0; i < 256; i++) {
		reverse_table[i] = reverse_obvious(i);
	}
}

clock_t benchmark_table(int iterations) {
	clock_t clock0 = clock();
	int32_t s = 0;
	for (int r = 0; r < iterations; r++) {
		for (int i = 0; i < 256; i++) {
			s += reverse_table[i];
		}
	}
	checksum("table", s);
	return clock() - clock0;
}

bool test() {
	bool r = true;
	for (int i = 0; i < 256; i++) {
		unsigned char r1 = reverse_obvious(i);
		unsigned char r2 = reverse_multiply_mod(i);
		unsigned char r3 = reverse_multiply(i);
		unsigned char r4 = reverse_magic(i);
		unsigned char r5 = reverse_builtin(i);
		if (r1 != r2 || r1 != r3 || r1 != r4 || r1 != r5) {
			printf("Failure at %02X: %02X %02X %02X %02X %02X", i, r1, r2, r3, r4, r5);
			r = false;
		}
	}
	return r;
}

int main() {
	if (!test()) {
		exit(1);
	}
	initReverseTable();
	int iterations = 10000;
	benchmark("reverse_obvious", iterations, &benchmark_obvious);
	benchmark("reverse_table", iterations, &benchmark_table);
	benchmark("reverse_multiply_mod", iterations, &benchmark_multiply_mod);
	benchmark("reverse_multiply", iterations, &benchmark_multiply);
	benchmark("reverse_magic", iterations, &benchmark_magic);
	benchmark("reverse_builtin", iterations, &benchmark_builtin);
}

