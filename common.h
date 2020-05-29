#ifndef COMMON_H_
#define COMMON_H_

typedef clock_t(*timed_function_t) (int);

double mean(double *x, int n) {
	double sum = 0;
	for (int i = 0; i < n; i++) {
		sum += x[i];
	}
	return sum / n;
}

double variance(double x_mean, double *x, int n) {
	double sum = 0;
	for (int i = 0; i < n; i++) {
		double v = x[i] - x_mean;
		sum += v * v;
	}
	return sum / (n - 1);
}

void benchmark(char* name, int iterations, timed_function_t reverse) {
	double results[100];
	for (int i = 0; i < 100; i++) {
		results[i] = (double) reverse(iterations) / CLOCKS_PER_SEC;
	}

	double r_mean = mean(results, 100);
	double r_var = variance(r_mean, results, 100);
	printf("%s:\n  mean: %.6fs\n  var:  %.6fs²\n", name, r_mean, r_var);
}

#endif

