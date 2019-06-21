// Método Gauss-Legendre

#include <stdio.h>
#include <gmp.h>

// #define PRECISION	256
// #define PRECISION	512
// #define PRECISION	1024
#define PRECISION		8192
// #define PRECISION	8388608		// 1MB de Precisão quebra a biblioteca

int main(int argc, char* argv[]) {
	mpf_t test;
	mpf_init2(test, PRECISION);
	mpf_set_d(test, 3.1415926542222222222222222222222);

	printf("Precision: %d bytes\n", PRECISION >> 3);
	printf("sizeof:    %d bytes\n", test->_mp_size);

	return 0;
}
