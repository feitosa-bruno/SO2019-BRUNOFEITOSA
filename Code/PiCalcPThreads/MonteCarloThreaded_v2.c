// Método Monte Carlo com Threads

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <gmp.h>

// Precisão/Limite
#define PRECISION		8192
// #define LIMIT 			10
// #define LIMIT 			1000000
// #define LIMIT 			100000000
#define LIMIT 			1000000000
// OBS: Manter limite divisível por 4 (terminando em 00)

// Número de Threads
#define NUM_THREADS 2

// Passo para Laço Iterativo
#define STEP		NUM_THREADS

typedef struct MCData {
	unsigned int	matchCount;
	unsigned int	limit;
	unsigned int 	step;
	unsigned int	start;
} MCData;

struct thread_data {
	int thread_id;
	MCData mcData;
};

struct thread_data threadDataArray[NUM_THREADS];


// Buffer para Números Aleatórios
struct drand48_data randBuffer;

void printf_piMCt_gmp(void);
void *loopIterationThread(void *threadarg);
void loopIteration(MCData * mcData);
bool testDistance(mpf_t x2, mpf_t y2);
void getSquared(mpf_t z, double random_d);
void piFromCounter(mpf_t result, unsigned int counter, unsigned int limit);
void printHeartbeat(unsigned int counter);

int main(int argc, char* argv[]) {
	// Precisão padrão para todos os BigNums
	mpf_set_default_prec(PRECISION);

	printf("Iniciando...\n");
	printf_piMCt_gmp();
	printf("Terminado\n");

	return 0;
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
	// Contador de Acertos para Todas as Threads
	unsigned int matchCountTotal = 0;
	
	// Threads
	pthread_t threads[NUM_THREADS - 1]; // Uma thread é o proprio programa principal
	pthread_attr_t attr;
	void *status;

	pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	for (int k = 0; k < NUM_THREADS; k++){
		threadDataArray[k].thread_id			= k;
		threadDataArray[k].mcData.matchCount 	= 0;
		threadDataArray[k].mcData.limit			= LIMIT;
		threadDataArray[k].mcData.step			= STEP;
		threadDataArray[k].mcData.start			= k;
	}
	
	// Lança Threads
	for (int t = 1; t < NUM_THREADS; t++) {	// Thread 0 roda no próprio corpo
		pthread_create(
			&threads[t], &attr, loopIterationThread, (void *)&threadDataArray[t]
		);
	}

	// Calcula Thread 0 (no programa principal)
	loopIteration(&threadDataArray[0].mcData);

	for (int t = 1; t < NUM_THREADS; t++) {	// Thread 0 roda no próprio corpo
		pthread_join(threads[t], &status);	// então só junciona thread 1 em diante
	}

	mpf_t result;

	mpf_init(result);
	
	for (int k = 0; k < NUM_THREADS; k++) {
		matchCountTotal += threadDataArray[k].mcData.matchCount;
	}

	piFromCounter(result, matchCountTotal, LIMIT);

	gmp_printf("\nPI: %.*Ff (Calculado)\n", 6, result);

	// // Cálculo de Erro
	// mpf_sub(delta, res, mp_pi);

	// // Saída de Resultados
	// printResult(res, mp_pi, delta);
}

void loopIteration(MCData * mcData) {
	unsigned int start = mcData->start;
	unsigned int limit = mcData->limit;
	unsigned int step = mcData->step;
	struct drand48_data randBuffer;
	double random;
	mpf_t x, y;

	srand48_r(time(NULL), &randBuffer);

	mpf_init(x);
	mpf_init(y);

	// Cada Thread inicia em uma posição diferente e
	// tem um passo que pula as ações feitas por outras threads
	// Ex.: thread[0] calcula com i = (0, 2, 4, 6...)
	// 		thread[1] calcula com i = (1, 3, 5, 7...)
	for (int i = start; i < limit; i += step) {
		drand48_r(&randBuffer, &random);
		getSquared(x, random);
		drand48_r(&randBuffer, &random);
		getSquared(y, random);
		if (testDistance(x, y)) {
			mcData->matchCount++;
		}
		// printf("%d \n",i);
	}
}

void *loopIterationThread(void *threadarg) {
	// Somente o cabeçalho muda entre a thread e não-thread
	struct thread_data *argument;
	argument = (struct thread_data *) threadarg;
	long taskid		= argument->thread_id;
	MCData *		mcData;
	mcData			= &argument->mcData;

	unsigned int start = mcData->start;
	unsigned int limit = mcData->limit;
	unsigned int step = mcData->step;
	struct drand48_data randBuffer;
	double random;
	mpf_t x, y;

	srand48_r(time(NULL) + taskid, &randBuffer);

	mpf_init(x);
	mpf_init(y);

	// Cada Thread inicia em uma posição diferente e
	// tem um passo que pula as ações feitas por outras threads
	// Ex.: thread[0] calcula com i = (0, 2, 4, 6...)
	// 		thread[1] calcula com i = (1, 3, 5, 7...)
	for (int i = start; i < limit; i += step) {
		drand48_r(&randBuffer, &random);
		getSquared(x, random);
		drand48_r(&randBuffer, &random);
		getSquared(y, random);
		if (testDistance(x, y)) {
			mcData->matchCount++;
		}
		// printf("%d \n",i);
	}
	
	pthread_exit((void *) threadarg);
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

void getSquared(mpf_t z, double random_d) {
	// printf("%lf\n", random_d);
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
