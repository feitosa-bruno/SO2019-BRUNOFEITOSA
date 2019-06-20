// Método Gauss-Legendre

#include <stdio.h>
// #include <stdlib.h>
// #include <math.h>
#include <gmp.h>

#define PRECISION 256

void printf_piGL_gmp(void);
void printHeartbeat(unsigned int counter);

int main(int argc, char* argv[]) {

	printf("Starting...\n");
	printf_piGL_gmp();
	printf("Over\n");

	return 0;
}

void printHeartbeat(unsigned int counter) {
	printf("%10u", counter);
	printf("\b\b\b\b\b\b\b\b\b\b");
}

void printf_piGL_gmp(void) {
	// Variável Auxiliar
	mpf_t aux;
	mpf_init2 (aux, PRECISION);
	mpf_sqrt_ui(aux, 2);		// aux = sqrt(2)

	// Valores Iniciais / Iteração Atual
	mpf_t an;
	mpf_init2 (an, PRECISION);
	mpf_set_ui(an, 1);			// 1
	mpf_t bn;
	mpf_init2 (bn, PRECISION);
	mpf_div_ui(bn, aux, 2);		// sqrtl(2) / 2
	mpf_t tn;
	mpf_init2 (tn, PRECISION);
	mpf_set_d(tn, 0.25);		// 0.25
	mpf_t pn;
	mpf_init2 (pn, PRECISION);
	mpf_set_ui(pn, 1);			// 1

	// Iteração Seguinte
	mpf_t an_, bn_, tn_, pn_;
	mpf_init2 (an_, PRECISION);
	mpf_init2 (bn_, PRECISION);
	mpf_init2 (tn_, PRECISION);
	mpf_init2 (pn_, PRECISION);

	// Iterador e Limite
	unsigned int i = 0;
	unsigned int lim = 1000000;
	// unsigned int lim = 1000000000;

	for (i = 0; i < lim; i++) {
		// Iteração
		mpf_add(an_, an, bn);		// an_ = an + bn
		mpf_div_ui(an_, an_, 2);	// an_ = (an + bn)/2
		mpf_mul(bn_, an, bn);		// bn_ = an * bn
		mpf_sqrt(bn_, bn_);			// bn_ = sqrt(an * bn)
		mpf_sub(aux, an, an_);		// aux' = an - an_
		mpf_mul(aux, aux, aux);		// aux = aux' * aux'
		mpf_mul(aux, pn, aux);		// aux = pn * aux' * aux'
		mpf_sub(tn_, tn, aux);		// tn_ = tn - pn * aux' * aux'
		mpf_mul_ui(pn_, pn, 2);		// pn_ = pn * 2

		// Atualização para Iteração Seguinte
		mpf_set(an, an_);
		mpf_set(bn, bn_);
		mpf_set(tn, tn_);
		mpf_set(pn, pn_);

		// Verificação de que o programa está rodando
		printHeartbeat(i);
	}
	// Finalização
	mpf_add(aux, an_, bn_);		// (an_ + bn_)
	mpf_mul(aux, aux, aux);		// (an_ + bn_)*(an_ + bn_)
	mpf_div(aux, aux, tn_);		// (an_ + bn_)*(an_ + bn_)/(tn_)
	mpf_div_ui(aux, aux, 4);	// (an_ + bn_)*(an_ + bn_)/(4*tn_)

	gmp_printf("PI: %.*Ff with %d digits\n", 6, aux, 6);
}
