// Método Bailey-Borwein-Plouffe

#include <pthread.h>
#include <stdio.h>
#include <gmp.h>

// Precisão/Limite
#define PRECISION		8192
#define LIMIT 			15000

// Número de Threads
#define NUM_THREADS 5

struct thread_data {
	int thread_id;
	mpf_t *result;
	unsigned int k;
};

struct thread_data threadDataArray[NUM_THREADS];


void printf_piBBPt_gmp(void);
void fourWaySum(mpf_t sum, mpf_t pk_1, mpf_t pk_2,  mpf_t pk_3,  mpf_t pk_4);
void *BBPak(void *threadarg);
void *BBPpk1(void *threadarg);
void *BBPpk2(void *threadarg);
void *BBPpk3(void *threadarg);
void *BBPpk4(void *threadarg);
void printDebugHearbeat(unsigned int counter, mpf_t delta);
void printHeartbeat(unsigned int counter);

int main(int argc, char* argv[]) {

	// Precisão padrão para todos os BigNums
	mpf_set_default_prec(PRECISION);

	printf("Iniciando...\n");
	printf_piBBPt_gmp();
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
	gmp_printf("\nPI: %.*Ff (Calculado)\n", 6, result);
	gmp_printf("PI: %.*Ff (Esperado)\n", 6, target);
	gmp_printf("ER: %.*Ff (Erro)\n", 6, delta);
}

void printf_piBBPt_gmp(void) {
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
	mpf_t* mpfList[5];
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
	mpfList[0] = &ak;
	mpfList[1] = &pk_1;
	mpfList[2] = &pk_2;
	mpfList[3] = &pk_3;
	mpfList[4] = &pk_4;

	// Iterador e Limite
	unsigned int i = 0;
	unsigned int lim = LIMIT;

	for (i = 0; i < lim; i++) {
		for (int k=0; k < NUM_THREADS; k++) {
			threadDataArray[k].thread_id	= k;
			threadDataArray[k].result 		= mpfList[k];
			threadDataArray[k].k			= i;
		}
		pthread_create(&threads[0], &attr, BBPak, (void *) &threadDataArray[0]);
		pthread_create(&threads[1], &attr, BBPpk1, (void *) &threadDataArray[1]);
		pthread_create(&threads[2], &attr, BBPpk2, (void *) &threadDataArray[2]);
		pthread_create(&threads[3], &attr, BBPpk3, (void *) &threadDataArray[3]);
		pthread_create(&threads[4], &attr, BBPpk4, (void *) &threadDataArray[4]);
	
		for (int t=0; t < NUM_THREADS; t++) {
			pthread_join(threads[t], &status);
		}
		
		fourWaySum(sum, pk_1, pk_2, pk_3, pk_4);	// Soma das Parcelas
		mpf_mul(aux, ak, sum);		// Parcela da Iteração
		mpf_add(res, res, aux);		// Acumula parcela da iteração

		// Cálculo de Erro
		mpf_sub(delta, res, mp_pi);
	
		// Verificação de que o programa está rodando
		// printHeartbeat(i);
		printDebugHeartbeat(i, delta);
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
void *BBPak (void *threadarg) {
	struct thread_data *argument;
	argument = (struct thread_data *) threadarg;
	long taskid		= argument->thread_id;
	mpf_t *result;
	result			= argument->result;
	unsigned int k	= argument->k;

	mpf_set_ui(*result, 16);			// 16
	mpf_pow_ui(*result, *result, k);	// 16^k
	mpf_ui_div(*result, 1, *result);	// 1/(16^k)

	pthread_exit((void *) threadarg);
}

// Primeira Parcela da Soma
void *BBPpk1 (void *threadarg) {
	struct thread_data *argument;
	argument = (struct thread_data *) threadarg;
	long taskid		= argument->thread_id;
	mpf_t *result;
	result			= argument->result;
	unsigned int k	= argument->k;

	mpf_set_ui(*result, 8);				// 8
	mpf_mul_ui(*result, *result, k);	// 8*k
	mpf_add_ui(*result, *result, 1);	// 8*k + 1
	mpf_ui_div(*result, 4, *result);	// 4/(8*k + 1)

	pthread_exit((void *) threadarg);
}

// Segunda Parcela da Soma
void *BBPpk2 (void *threadarg) {
	struct thread_data *argument;
	argument = (struct thread_data *) threadarg;
	long taskid		= argument->thread_id;
	mpf_t *result;
	result			= argument->result;
	unsigned int k	= argument->k;

	mpf_set_ui(*result, 8);				// 8
	mpf_mul_ui(*result, *result, k);	// 8*k
	mpf_add_ui(*result, *result, 4);	// 8*k + 4
	mpf_ui_div(*result, 2, *result);	// 2/(8*k + 4)
	mpf_neg(*result, *result);			// -2/(8*k + 4)

	pthread_exit((void *) threadarg);
}

// Terceira Parcela da Soma
void *BBPpk3 (void *threadarg) {
	struct thread_data *argument;
	argument = (struct thread_data *) threadarg;
	long taskid		= argument->thread_id;
	mpf_t *result;
	result			= argument->result;
	unsigned int k	= argument->k;

	mpf_set_ui(*result, 8);				// 8
	mpf_mul_ui(*result, *result, k);	// 8*k
	mpf_add_ui(*result, *result, 5);	// 8*k + 5
	mpf_ui_div(*result, 1, *result);	// 1/(8*k + 5)
	mpf_neg(*result, *result);			// -1/(8*k + 5)

	pthread_exit((void *) threadarg);
}

// Quarta Parcela da Soma
void *BBPpk4 (void *threadarg) {
	struct thread_data *argument;
	argument = (struct thread_data *) threadarg;
	long taskid		= argument->thread_id;
	mpf_t *result;
	result			= argument->result;
	unsigned int k	= argument->k;

	mpf_set_ui(*result, 8);				// 8
	mpf_mul_ui(*result, *result, k);	// 8*k
	mpf_add_ui(*result, *result, 6);	// 8*k + 6
	mpf_ui_div(*result, 1, *result);	// 1/(8*k + 6)
	mpf_neg(*result, *result);			// -1/(8*k + 5)

	pthread_exit((void *) threadarg);
}
