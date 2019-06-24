// Método Gauss-Legendre

#include <stdio.h>
#include <stdbool.h>
#include <gmp.h>

// #define PRECISION	256
// #define	LIMIT		600			// Limite para precisão de 32 bytes

// #define PRECISION	512
// #define LIMIT 		1000		// Limite para precisão de 64 bytes

// #define PRECISION	1024
// #define LIMIT 		2000		// Limite para precisão de 128 bytes

#define PRECISION		8192
#define LIMIT 			10		// Limite para precisão de 1KB /1024 Bytes/ 8192 bits

// #define PRECISION	8388608		// 1MB de Precisão quebra a biblioteca

// Nível de Vocalização
#define VOCAL false
#define DEBUG false


void printf_piGL_gmp(void);
void printResult(mpf_t result, mpf_t target, mpf_t delta, mpf_t accuracy, mpf_t tn_);
void printDebugHearbeat(unsigned int counter, mpf_t accuracy, mpf_t tn_);
void printHeartbeat(unsigned int counter);

int main(int argc, char* argv[]) {

	// Precisão padrão para todos os BigNums
	mpf_set_default_prec(PRECISION);

	if (VOCAL) printf("Iniciando...\n");
	printf_piGL_gmp();
	if (VOCAL) printf("Terminado\n");

	return 0;
}

void printHeartbeat(unsigned int counter) {
	int remove = 0;
	remove = printf("i: %10u", counter);
	for (remove; remove > 0; remove--) printf("\b");
}

void printDebugHearbeat(unsigned int counter, mpf_t accuracy, mpf_t tn_) {
	int remove = 0;
	remove  = gmp_printf("%10u; ", counter);
	remove += gmp_printf("AC: %.*Fe; ", 6, accuracy);
	remove += gmp_printf("tn: %.*Fe", 6, tn_);
	for (remove; remove > 0; remove--) gmp_printf("\b");
}

void printDebug(
	unsigned int counter,
	mpf_t an,
	mpf_t bn,
	mpf_t tn,
	mpf_t pn,
	mpf_t aux,
	mpf_t pi) {
	gmp_printf("%2u; ", counter);
	gmp_printf("%.*Fe; ", 6, an);
	gmp_printf("%.*Fe; ", 6, bn);
	gmp_printf("%.*Fe; ", 6, tn);
	gmp_printf("%.*Fe; ", 6, pn);
	gmp_printf("%.*Fe; ", 6, aux);
	gmp_printf("%.*Fe;\n", 50, pi);
}

void printResult(
	mpf_t result,
	mpf_t target,
	mpf_t delta,
	mpf_t accuracy,
	mpf_t tn_) {
	if (VOCAL) gmp_printf("\nPI: %.*Ff (Calculado)\n", 6, result);
	if (VOCAL) gmp_printf("PI: %.*Ff (Esperado)\n", 6, target);
	if (VOCAL) gmp_printf("ER: %.*Ff (Erro)\n", 6, delta);
	if (VOCAL) gmp_printf("AC: %.*Fe (Acurácia an - bn)\n", 6, accuracy);
	if (VOCAL) gmp_printf("tn: %.*Fe (tn)\n", 6, tn_);
	if (!VOCAL) gmp_printf("%.*Ff\n", 6, result);
}

void printf_piGL_gmp(void) {
	// PI Referência (9 casas decimais, segundo Wolfram)
	mpf_t mp_pi;
	mpf_init(mp_pi);
	mpf_set_d(mp_pi, 3.141592654);

	// Erro
	mpf_t delta;
	mpf_init(delta);

	// Resultado
	mpf_t res;
	mpf_init (res);

	// Variável Auxiliar
	mpf_t aux;
	mpf_init (aux);
	mpf_sqrt_ui(aux, 2);		// aux = sqrt(2)

	// Valores Iniciais / Iteração Atual
	mpf_t an;
	mpf_init (an);
	mpf_set_ui(an, 1);			// 1
	mpf_t bn;
	mpf_init (bn);
	mpf_div_ui(bn, aux, 2);		// sqrtl(2) / 2
	mpf_t tn;
	mpf_init (tn);
	mpf_set_d(tn, 0.25);		// 0.25
	mpf_t pn;
	mpf_init (pn);
	mpf_set_ui(pn, 1);			// 1
	mpf_t piC;
	mpf_init (piC);

	// Variáveis de Iteração Seguinte
	mpf_t an_, bn_, tn_, pn_;
	mpf_init (an_);
	mpf_init (bn_);
	mpf_init (tn_);
	mpf_init (pn_);

	// Iterador e Limite
	unsigned int i = 0;
	unsigned int lim = LIMIT;

	mpf_add(res, an, bn);		// (an_ + bn_)
	mpf_mul(res, res, res);		// (an_ + bn_)*(an_ + bn_)
	mpf_div_ui(res, res, 4);	// (an_ + bn_)*(an_ + bn_)/(4)
	mpf_div(res, res, tn);		// (an_ + bn_)*(an_ + bn_)/(4*tn_)
	mpf_sub(aux, an, bn);		// Acurácia (an - bn)
	printDebug(i, an, bn, tn, pn, aux, res);

	for (i = 0; i < lim; i++) {
		// Iteração
		// a_n+1
		mpf_add(an_, an, bn);		// an_ = an + bn
		mpf_div_ui(an_, an_, 2);	// an_ = (an + bn)/2
		// b_n+1
		mpf_mul(bn_, an, bn);		// bn_ = an * bn
		mpf_sqrt(bn_, bn_);			// bn_ = sqrt(an * bn)
		// t_n+1
		mpf_sub(aux, an, an_);		// aux = an - an_
		mpf_mul(aux, aux, aux);		// aux = (an - an_) * (an - an_)
		mpf_mul(aux, pn, aux);		// aux = pn * (an - an_) * (an - an_)
		mpf_sub(tn_, tn, aux);		// tn_ = tn - pn * (an - an_) * (an - an_)
		// p_n+1
		mpf_mul_ui(pn_, pn, 2);		// pn_ = pn * 2
		// Debug (an - bn)
		mpf_sub(aux, an_, bn_);		// Acurácia (an - bn)

		// Atualização para Iteração Seguinte
		mpf_set(an, an_);
		mpf_set(bn, bn_);
		mpf_set(tn, tn_);
		mpf_set(pn, pn_);

		mpf_add(res, an, bn);		// (an_ + bn_)
		mpf_mul(res, res, res);		// (an_ + bn_)*(an_ + bn_)
		mpf_div_ui(res, res, 4);	// (an_ + bn_)*(an_ + bn_)/(4)
		mpf_div(res, res, tn);		// (an_ + bn_)*(an_ + bn_)/(4*tn_)
		mpf_sub(aux, an, bn);		// Acurácia (an - bn)
		printDebug(i+1, an, bn, tn, pn, aux, res);
	}
	// Finalização
	mpf_add(res, an_, bn_);		// (an_ + bn_)
	mpf_mul(res, res, res);		// (an_ + bn_)*(an_ + bn_)
	mpf_div_ui(res, res, 4);	// (an_ + bn_)*(an_ + bn_)/(4)
	mpf_div(res, res, tn_);		// (an_ + bn_)*(an_ + bn_)/(4*tn_)
	mpf_sub(aux, an_, bn_);		// Acurácia (an - bn)

	// Cálculo de Erro
	mpf_sub(delta, res, mp_pi);

	// Saída de Resultados
	printResult(res, mp_pi, delta, aux, tn_);
}
