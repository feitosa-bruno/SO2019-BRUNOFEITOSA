// Método Monte Carlo com Threads
// NO GOOD

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <gmp.h>

// Precisão/Limite
#define PRECISION		8192
#define LIMIT 			1000000
// #define LIMIT 			100000000
// #define LIMIT 			1000000000
// OBS: Manter limite divisível por 4 (terminando em 00)

// Número de Threads
#define NUM_THREADS 4

struct thread_data {
	int thread_id;
	bool check;
};

struct thread_data threadDataArray[NUM_THREADS];

// Buffer para Números Aleatórios
struct drand48_data randBuffer;

void printf_piMCt_gmp(void);
void *calcIteration(void *threadarg);
bool testDistance(mpf_t x2, mpf_t y2);
void getSquared(mpf_t z);
void piFromCounter(mpf_t result, unsigned int counter, unsigned int limit);
void printHeartbeat(unsigned int counter);


int main(int argc, char* argv[]) {
	// Precisão padrão para todos os BigNums
	mpf_set_default_prec(PRECISION);

	printf("Iniciando...\n");

	// Inicialização da Semente
	srand48_r(time(NULL), &randBuffer);

	printf("Semente Iniciada.\n");
	printf("Calculando...\n");
	printf_piMCt_gmp();
	printf("Terminado\n");

	return 0;
}

void *calcIteration(void *threadarg) {
	struct thread_data *argument;
	argument = (struct thread_data *) threadarg;
	long taskid = argument->thread_id;

	mpf_t x;
	mpf_init(x);
	mpf_t y;
	mpf_init(y);
	
	getSquared(x);
	getSquared(y);
	if (testDistance(x, y)) {
		argument->check = true;
	} else {
		argument->check = false;
	}

	mpf_clear(x);
	mpf_clear(y);

	pthread_exit((void *) threadarg);
}

void printResult(mpf_t result, mpf_t target, mpf_t delta) {
	gmp_printf("\nPI: %.*Ff (Calculado)\n", 6, result);
	gmp_printf("PI: %.*Ff (Esperado)\n", 6, target);
	gmp_printf("ER: %.*Ff (Erro)\n", 6, delta);
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

void printf_piMCt_gmp(void) {
	// Threads
	pthread_t threads[NUM_THREADS];
	pthread_attr_t attr;
	void *status;

	pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

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

	// Thread 
	bool result[NUM_THREADS];

	// Iterador, Limite e Contador
	unsigned int i = 0;
	unsigned int lim = LIMIT;
	unsigned int count = 0;

	for (i = 0; i < lim; i+=NUM_THREADS) {
		for (int t=0; t < NUM_THREADS; t++) {
			threadDataArray[t].thread_id = t;
			pthread_create(
				&threads[t],
				&attr,
				calcIteration,
				(void *) &threadDataArray[t]
			);
		}
		for (int t=0; t < NUM_THREADS; t++) {
			pthread_join(threads[t], &status);
			if (threadDataArray[t].check) {
				count++;
			}
		}

		// Verificação de que o programa está rodando
		// printHeartbeat(i);
		if ((i % 4 == 0) && i != 0) {
			piFromCounter(res, count, i);
			mpf_sub(delta, res, mp_pi);
			printDebugHeartbeat(i, delta);
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
	// Ao invés de calulcar: resultado = 4*(counter/limite)
	// Usando: resultado = counter/quo; onde: quo = limite/4
	// É a mesma coisa, mas divisão por 4 é trivial/rápida em variáveis inteiras
	unsigned int quo = limit >> 2;	// limit/4
	mpf_set_ui(result, counter);
	mpf_div_ui(result, result, quo);
}