// Método Monte Carlo

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <gmp.h>

// Precisão/Limite
#define PRECISION		8192
// #define LIMIT 			1000000
// #define LIMIT 			100000000
#define LIMIT 			1000000000
// OBS: Manter limite divisível por 4 (terminando em 00)

// Nível de Vocalização
#define VOCAL false
#define DEBUG false


// Buffer para Números Aleatórios
struct drand48_data randBuffer;

void printf_piMC_gmp(void);
bool testDistance(mpf_t x2, mpf_t y2);
void getSquared(mpf_t z);
void piFromCounter(mpf_t result, unsigned int counter, unsigned int limit);
void printHeartbeat(unsigned int counter);

int main(int argc, char* argv[]) {

	// Precisão padrão para todos os BigNums
	mpf_set_default_prec(PRECISION);

	if (VOCAL) printf("Iniciando...\n");

	// Inicialização da Semente
	srand48_r(time(NULL), &randBuffer);

	if (VOCAL) printf("Semente Iniciada.\n");
	if (VOCAL) printf("Calculando...\n");
	printf_piMC_gmp();
	if (VOCAL) printf("Terminado\n");

	return 0;
}

void printResult(mpf_t result, mpf_t target, mpf_t delta) {
	if (VOCAL) gmp_printf("\nPI: %.*Ff (Calculado)\n", 6, result);
	if (VOCAL) gmp_printf("PI: %.*Ff (Esperado)\n", 6, target);
	if (VOCAL) gmp_printf("ER: %.*Ff (Erro)\n", 6, delta);
	if (!VOCAL) gmp_printf("%.*Ff\n", 6, result);
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

void printf_piMC_gmp(void) {
	// PI Referência (9 casas decimais, segundo Wolfram)
	mpf_t mp_pi;
	mpf_init(mp_pi);
	mpf_set_d(mp_pi, 3.141592654);

	// Erro
	mpf_t delta;
	mpf_init(delta);

	// Resultado
	mpf_t res;
	mpf_init(res);

	// X & Y
	mpf_t x;
	mpf_init(x);
	mpf_t y;
	mpf_init(y);

	// Iterador, Limite e Contador
	unsigned int i = 0;
	unsigned int lim = LIMIT;
	unsigned int count = 0;

	for (i = 0; i < lim; i++) {
		getSquared(x);
		getSquared(y);
		if (testDistance(x,y)) {
			count++;
		}

		// Verificação de que o programa está rodando
		// printHeartbeat(i);
		if ((i % 4 == 0) && i != 0) {
			piFromCounter(res, count, i);
			mpf_sub(delta, res, mp_pi);
			if(DEBUG && VOCAL)	printDebugHeartbeat(i, delta);
		}
	}

	piFromCounter(res, count, lim);

	// Cálculo de Erro
	mpf_sub(delta, res, mp_pi);

	// Saída de Resultados
	printResult(res, mp_pi, delta);
}

bool testDistance(mpf_t x2, mpf_t y2) {
	mpf_t comp;
	mpf_init (comp);
	mpf_add(comp, x2, y2);			// comp = x2 + y2
	mpf_sub_ui(comp, comp, 1);		// comp = x2 + y2 - 1
	int test = mpf_sgn(comp);		// -1 for neg / 0 for 0 / +1 for pos
	mpf_clear(comp);
	return (test <= 0);				// true for comp <= 0; false otherwise
}

void getSquared(mpf_t z) {
	double random_d;
	drand48_r(&randBuffer, &random_d);	// Gera um double aleatório
	mpf_set_d(z, random_d);				// Converte para bignum
	mpf_mul(z, z, z);					// z^2
}

void piFromCounter(mpf_t result, unsigned int counter, unsigned int limit) {
	// Ao invés de calcular: resultado = 4*(counter/limite)
	// Usando: resultado = counter/quo; onde: quo = limite/4
	// É a mesma coisa, mas divisão por 4 é trivial/rápida em variáveis inteiras
	unsigned int quo = limit >> 2;	// limit/4
	mpf_set_ui(result, counter);
	mpf_div_ui(result, result, quo);
}