// Método Gauss-Legendre

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
// #include "gmp.h"

char heartbeat[4][3] = {
	{'-', '\b', '\0'},
	{'/', '\b', '\0'},
	{'|', '\b', '\0'},
	{'\\', '\b', '\0'},
};


long double piGL(void);
void printHeartbeat(long int counter);

int main(int argc, char* argv[]) {
	long double output = 0;

	printf("Starting...\n");
	output = piGL();
	printf("Finished!\n");
	printf("PI: %Lf\n", output);

	return 0;
}

void printHeartbeat(long int counter) {
	int pos = counter % 4;
	printf("%s", heartbeat[pos]);
}

long double piGL() {
	// Valores Iniciais / Iteração Atual
	long double an = 1;
	long double bn = sqrtl(2.0) / 2.0;
	long double tn = 0.25;
	long double pn = 1;
	long double aux;
	// Iteração Seguinte
	long double an_, bn_, tn_, pn_;
	// Iterador e Limite
	unsigned int i;
	unsigned int lim = 50000;
	// unsigned int lim = 1000000000;

	for (i = 0; i < lim; i++) {
		// Iteração
		an_ = (an + bn) / 2;
		bn_ = sqrtl(an*bn);
		aux = an - an_;
		tn_ = tn - pn*aux*aux;
		pn_ = 2*pn;
		// Atualização para Iteração Seguinte
		an = an_;
		bn = bn_;
		tn = tn_;
		pn = pn_;

		// Verificação de que o programa está rodando
		printHeartbeat(i);
	}
	aux = (an_ + bn_)*(an_ + bn_)/(4*tn_);
	return aux;
}
