// Método Bailey-Borwein-Plouffe

#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <gmp.h>

// Precisão/Limite
#define PRECISION		8192
#define LIMIT 			15000

// Número de Threads
#define NUM_THREADS	2
// São listadas 2 threads, mas 1 roda em paralelo, e 0 roda no programa principal

// Passo para Laço Iterativo
#define STEP		NUM_THREADS

// Nível de Vocalização
#define VOCAL true
#define DEBUG true

typedef struct BBPData {
	mpf_t			partialSum;
	unsigned int	limit;
	unsigned int 	step;
	unsigned int	start;
} BBPData;

struct thread_data {
	int thread_id;
	BBPData bbpData;
};

struct thread_data threadDataArray[NUM_THREADS];


void printf_piBBPt_gmp(void);
void *loopIterationThread(void *threadarg);
void loopIteration(BBPData * bbpData);
void fourWaySum(mpf_t sum, mpf_t pk_1, mpf_t pk_2,  mpf_t pk_3,  mpf_t pk_4);
void BBPak(mpf_t ak, unsigned int k);
void BBPpk1(mpf_t pk_1, unsigned int k);
void BBPpk2(mpf_t pk_2, unsigned int k);
void BBPpk3(mpf_t pk_3, unsigned int k);
void BBPpk4(mpf_t pk_4, unsigned int k);
void printDebugHearbeat(unsigned int counter, mpf_t delta);
void printHeartbeat(unsigned int counter);

int main(int argc, char* argv[]) {

	// Precisão padrão para todos os BigNums
	mpf_set_default_prec(PRECISION);

	if (VOCAL) printf("Iniciando...\n");
	printf_piBBPt_gmp();
	if (VOCAL) printf("Terminado\n");

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
	if (VOCAL) gmp_printf("\nPI: %.*Ff (Calculado)\n", 6, result);
	if (VOCAL) gmp_printf("PI: %.*Ff (Esperado)\n", 6, target);
	if (VOCAL) gmp_printf("ER: %.*Ff (Erro)\n", 6, delta);
	if (!VOCAL) gmp_printf("\n%.*Ff\n", 6, result);
}

void printf_piBBPt_gmp(void) {
	// Threads
	pthread_t threads[NUM_THREADS - 1]; // Uma thread é o proprio programa principal
	pthread_attr_t attr;
	void *status;

	pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	for (int k = 0; k < NUM_THREADS; k++){
		threadDataArray[k].thread_id = k;
		threadDataArray[k].bbpData.limit	= LIMIT;
		threadDataArray[k].bbpData.step		= STEP;
		threadDataArray[k].bbpData.start	= k;
		mpf_init(threadDataArray[k].bbpData.partialSum);
		mpf_set_ui(threadDataArray[k].bbpData.partialSum, 0);
	}
	
	// Lança Threads
	for (int t = 1; t < NUM_THREADS; t++) {	// Thread 0 roda no próprio corpo
		pthread_create(
			&threads[t], &attr, loopIterationThread, (void *)&threadDataArray[t]
		);
	}

	// Calcula Thread 0 (no programa principal)
	loopIteration(&threadDataArray[0].bbpData);

	for (int t = 1; t < NUM_THREADS; t++) {	// Thread 0 roda no próprio corpo
		pthread_join(threads[t], &status);	// então só junciona thread 1 em diante
	}

	mpf_t result;

	mpf_init(result);
	mpf_add(
		result,
		threadDataArray[0].bbpData.partialSum,
		threadDataArray[1].bbpData.partialSum
	);
	// gmp_printf("\n0: %.*Ff (parcial)\n", 6, threadDataArray[0].bbpData.partialSum);
	// gmp_printf("\n1: %.*Ff (parcial)\n", 6, threadDataArray[1].bbpData.partialSum);

	if (VOCAL) gmp_printf("\nPI: %.*Ff (Calculado)\n", 6, result);
	// if (VOCAL) gmp_printf("PI: %.*Ff (Esperado)\n", 6, target);
	// if (VOCAL) gmp_printf("ER: %.*Ff (Erro)\n", 6, delta);
	if (!VOCAL) gmp_printf("\n%.*Ff\n", 6, result);

	// // PI Referência (9 casas decimais, segundo Wolfram)
	// mpf_t mp_pi;
	// mpf_init(mp_pi);
	// mpf_set_d(mp_pi, 3.141592654);

	// // Saída de Resultados
	// printResult(res, mp_pi, delta);
}

void loopIteration(BBPData * bbpData) {
	unsigned int start = bbpData->start;
	unsigned int limit = bbpData->limit;
	unsigned int step = bbpData->step;
	mpf_init(bbpData->partialSum);
	mpf_set_ui(bbpData->partialSum, 0);
	mpf_t ak, pk_1, pk_2, pk_3, pk_4, sum, aux;
	mpf_init(ak);
	mpf_init(pk_1);
	mpf_init(pk_2);
	mpf_init(pk_3);
	mpf_init(pk_4);
	mpf_init(sum);
	mpf_init(aux);

	// Cada Thread inicia em uma posição diferente e
	// tem um passo que pula as ações feitas por outras threads
	// Ex.: thread[0] calcula com i = (0, 2, 4, 6...)
	// 		thread[1] calcula com i = (1, 3, 5, 7...)
	for (int i = start; i < limit; i += step) {
		// printf("%d \n",i);
		BBPak(ak, i);
		BBPpk1(pk_1, i);
		BBPpk2(pk_2, i);
		BBPpk3(pk_3, i);
		BBPpk4(pk_4, i);
		fourWaySum(sum, pk_1, pk_2, pk_3, pk_4);
		mpf_mul(aux, sum, ak);
		mpf_add(bbpData->partialSum, bbpData->partialSum, aux);
	}
}

void *loopIterationThread(void *threadarg) {
	// Somente o cabeçalho muda entre a thread e não-thread
	struct thread_data *argument;
	argument = (struct thread_data *) threadarg;
	long taskid		= argument->thread_id;
	BBPData *		bbpData;
	bbpData			= &argument->bbpData;

	unsigned int start = bbpData->start;
	unsigned int limit = bbpData->limit;
	unsigned int step = bbpData->step;

	mpf_init(bbpData->partialSum);
	mpf_set_ui(bbpData->partialSum, 0);
	mpf_t ak, pk_1, pk_2, pk_3, pk_4, sum, aux;
	mpf_init(ak);
	mpf_init(pk_1);
	mpf_init(pk_2);
	mpf_init(pk_3);
	mpf_init(pk_4);
	mpf_init(sum);
	mpf_init(aux);

	// Cada Thread inicia em uma posição diferente e
	// tem um passo que pula as ações feitas por outras threads
	// Ex.: thread[0] calcula com i = (0, 2, 4, 6...)
	// 		thread[1] calcula com i = (1, 3, 5, 7...)
	for (int i = start; i < limit; i += step) {
		// printf("%d \n",i);
		BBPak(ak, i);
		BBPpk1(pk_1, i);
		BBPpk2(pk_2, i);
		BBPpk3(pk_3, i);
		BBPpk4(pk_4, i);
		fourWaySum(sum, pk_1, pk_2, pk_3, pk_4);
		mpf_mul(aux, sum, ak);
		mpf_add(bbpData->partialSum, bbpData->partialSum, aux);
	}
	
	pthread_exit((void *) threadarg);
}

void fourWaySum(mpf_t sum, mpf_t pk_1, mpf_t pk_2,  mpf_t pk_3,  mpf_t pk_4) {
	// sum = pk_1 + pk_2 + pk_3 + pk_4
	mpf_add(sum, pk_1, pk_2);	// sum = pk_1 + pk_2
	mpf_add(sum, sum, pk_3);	// sum = sum + pk_3
	mpf_add(sum, sum, pk_4);	// sum = sum + pk_4
}

// Produto 1/(16^k)
void BBPak(mpf_t ak, unsigned int k){
	mpf_set_ui(ak, 16);			// 16
	mpf_pow_ui(ak, ak, k);		// 16^k
	mpf_ui_div(ak, 1, ak);	// 1/(16^k)
	// gmp_printf("ak[%ud]: %.*Ff", k, ak);
}

// Primeira Parcela da Soma
void BBPpk1 (mpf_t pk_1, unsigned int k) {
	mpf_set_ui(pk_1, 8);				// 8
	mpf_mul_ui(pk_1, pk_1, k);	// 8*k
	mpf_add_ui(pk_1, pk_1, 1);	// 8*k + 1
	mpf_ui_div(pk_1, 4, pk_1);	// 4/(8*k + 1)
}

// Segunda Parcela da Soma
void BBPpk2 (mpf_t pk_2, unsigned int k) {
	mpf_set_ui(pk_2, 8);				// 8
	mpf_mul_ui(pk_2, pk_2, k);	// 8*k
	mpf_add_ui(pk_2, pk_2, 4);	// 8*k + 4
	mpf_ui_div(pk_2, 2, pk_2);	// 2/(8*k + 4)
	mpf_neg(pk_2, pk_2);			// -2/(8*k + 4)
}

// Terceira Parcela da Soma
void BBPpk3 (mpf_t pk_3, unsigned int k) {
	mpf_set_ui(pk_3, 8);		// 8
	mpf_mul_ui(pk_3, pk_3, k);	// 8*k
	mpf_add_ui(pk_3, pk_3, 5);	// 8*k + 5
	mpf_ui_div(pk_3, 1, pk_3);	// 1/(8*k + 5)
	mpf_neg(pk_3, pk_3);		// -1/(8*k + 5)
}

// Quarta Parcela da Soma
void BBPpk4 (mpf_t pk_4, unsigned int k) {
	mpf_set_ui(pk_4, 8);		// 8
	mpf_mul_ui(pk_4, pk_4, k);	// 8*k
	mpf_add_ui(pk_4, pk_4, 6);	// 8*k + 6
	mpf_ui_div(pk_4, 1, pk_4);	// 1/(8*k + 6)
	mpf_neg(pk_4, pk_4);		// -1/(8*k + 5)
}
