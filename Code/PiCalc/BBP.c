// Método Bailey-Borwein-Plouffe

#include <stdio.h>
#include <stdbool.h>
#include <gmp.h>

// Precisão/Limite
#define PRECISION		8192
#define LIMIT 			15000

// Nível de Vocalização
#define VOCAL false
#define DEBUG false


void printf_piBBP_gmp(void);
void fourWaySum(mpf_t sum, mpf_t pk_1, mpf_t pk_2,  mpf_t pk_3,  mpf_t pk_4);
void BBPak (mpf_t output, unsigned int k);
void BBPpk1 (mpf_t output, unsigned int k);
void BBPpk2 (mpf_t output, unsigned int k);
void BBPpk3 (mpf_t output, unsigned int k);
void BBPpk4 (mpf_t output, unsigned int k);
void printDebugHearbeat(unsigned int counter, mpf_t delta);
void printHeartbeat(unsigned int counter);

int main(int argc, char* argv[]) {

	// Precisão padrão para todos os BigNums
	mpf_set_default_prec(PRECISION);

	printf("Iniciando...\n");
	printf_piBBP_gmp();
	printf("Terminado\n");

	return 0;
}

void printHeartbeat(unsigned int counter) {
	int remove = 0;
	remove = printf("i: %10u", counter);
	for (remove; remove > 0; remove--) printf("\b");
}

void printDebugHeartbeat(unsigned int counter, mpf_t delta) {
	int remove = 0;
	remove  = gmp_printf("%10u; ", counter);
	remove += gmp_printf("ER: %.*Fe; ", 6, delta);

	for (remove; remove > 0; remove--) printf("\b");
}

void printResult(mpf_t result, mpf_t target, mpf_t delta) {
	if(VOCAL)	gmp_printf("\nPI: %.*Ff (Calculado)\n", 6, result);
	if(VOCAL)	gmp_printf("PI: %.*Ff (Esperado)\n", 6, target);
	if(VOCAL)	gmp_printf("ER: %.*Ff (Erro)\n", 6, delta);
	if(!VOCAL)	gmp_printf("\n%.*Ff\n", 6, result);
}

void printf_piBBP_gmp(void) {
	// PI Referência (9 casas decimais, segundo Wolfram)
	mpf_t mp_pi;
	mpf_init(mp_pi);
	mpf_set_d(mp_pi, 3.141592654);

	// Erro
	mpf_t delta;
	mpf_init(delta);

	// Variável Auxiliar
	mpf_t aux;
	mpf_init(aux);

	// Variável Auxiliar Soma (4 parcelas)
	mpf_t sum;
	mpf_init(sum);

	// Resultado
	mpf_t res;
	mpf_init(res);
	mpf_set_ui(res, 0);				// Somatório Inicializado com 0

	// Parcelas e Produto
	mpf_t ak;
	mpf_init(ak);
	mpf_t pk_1;
	mpf_init (pk_1);
	mpf_t pk_2;
	mpf_init (pk_2);
	mpf_t pk_3;
	mpf_init (pk_3);
	mpf_t pk_4;
	mpf_init (pk_4);

	// Iterador e Limite
	unsigned int i = 0;
	unsigned int lim = LIMIT;

	for (i = 0; i < lim; i++) {
		BBPak(ak, i);				// Produto
		BBPpk1(pk_1, i);			// Parcela 1
		BBPpk2(pk_2, i);			// Parcela 2
		BBPpk3(pk_3, i);			// Parcela 3
		BBPpk4(pk_4, i);			// Parcela 4
		fourWaySum(sum, pk_1, pk_2, pk_3, pk_4);	// Soma das Parcelas
		mpf_mul(aux, ak, sum);		// Parcela da Iteração
		mpf_add(res, res, aux);		// Acumula parcela da iteração

		// Cálculo de Erro
		mpf_sub(delta, res, mp_pi);
	
		// Verificação de que o programa está rodando
		if(!DEBUG && VOCAL)	printHeartbeat(i);
		if(DEBUG && VOCAL)	printDebugHeartbeat(i, delta);
	}

	// Saída de Resultados
	printResult(res, mp_pi, delta);
}

void fourWaySum(mpf_t sum, mpf_t pk_1, mpf_t pk_2,  mpf_t pk_3,  mpf_t pk_4) {
	// sum = pk_1 + pk_2 + pk_3 + pk_4
	mpf_add(sum, pk_1, pk_2);	// sum = pk_1 + pk_2
	mpf_add(sum, sum, pk_3);	// sum = sum + pk_3
	mpf_add(sum, sum, pk_4);	// sum = sum + pk_4
}

// Produto 1/(16^k)
void BBPak (mpf_t output, unsigned int k) {
	mpf_set_ui(output, 16);			// 16
	mpf_pow_ui(output, output, k);	// 16^k
	mpf_ui_div(output, 1, output);	// 1/(16^k)
}

// Primeira Parcela da Soma
void BBPpk1 (mpf_t output, unsigned int k) {
	mpf_set_ui(output, 8);			// 8
	mpf_mul_ui(output, output, k);	// 8*k
	mpf_add_ui(output, output, 1);	// 8*k + 1
	mpf_ui_div(output, 4, output);	// 4/(8*k + 1)
}

// Segunda Parcela da Soma
void BBPpk2 (mpf_t output, unsigned int k) {
	mpf_set_ui(output, 8);			// 8
	mpf_mul_ui(output, output, k);	// 8*k
	mpf_add_ui(output, output, 4);	// 8*k + 4
	mpf_ui_div(output, 2, output);	// 2/(8*k + 4)
	mpf_neg(output, output);		// -2/(8*k + 4)
}

// Terceira Parcela da Soma
void BBPpk3 (mpf_t output, unsigned int k) {
	mpf_set_ui(output, 8);			// 8
	mpf_mul_ui(output, output, k);	// 8*k
	mpf_add_ui(output, output, 5);	// 8*k + 5
	mpf_ui_div(output, 1, output);	// 1/(8*k + 5)
	mpf_neg(output, output);		// -1/(8*k + 5)
}

// Quarta Parcela da Soma
void BBPpk4 (mpf_t output, unsigned int k) {
	mpf_set_ui(output, 8);			// 8
	mpf_mul_ui(output, output, k);	// 8*k
	mpf_add_ui(output, output, 6);	// 8*k + 6
	mpf_ui_div(output, 1, output);	// 1/(8*k + 6)
	mpf_neg(output, output);		// -1/(8*k + 5)
}
