// Modelo Black-Scholes

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <gmp.h>
#include <mpfr.h>


// Precisão/Limite
#define PRECISION		8192
// #define LIMIT 			1000000
// #define LIMIT 			100000000
// #define LIMIT 			1000000000
// OBS: Manter limite divisível por 4 (terminando em 00)

// Argumentos de Entrada (Excluindo Limite)
#define ARGS_NUM 5

// Método de Arredondamento
#define STDR	MPFR_RNDN

// Número de Threads
#define NUM_THREADS	2
// São listadas 2 threads, mas 1 roda em paralelo, e 0 roda no programa principal

// Passo para Laço Iterativo
#define STEP		NUM_THREADS

// Estado do programa vocalizado
#define VOCAL true
#define DEBUG true

// Limite de Iterações (inicializado inválido, validado após entrada por usuário)
unsigned int limit = -1;

typedef struct BSData {
	mpfr_t 			S;
	mpfr_t 			E;
	mpfr_t 			r;
	mpfr_t 			v;
	mpfr_t 			T;
	mpfr_t 			*trials;
	mpfr_t			mean;
	mpfr_t 			stddev;
	mpfr_t 			confWidth;
	mpfr_t 			confMin;
	mpfr_t 			confMax;
	unsigned int	limit;
	unsigned int 	step;
	unsigned int	start;
	mpfr_t 			aux1;
	mpfr_t 			aux2;
	mpfr_t 			aux3;
	mpfr_t 			aux4;
} BSData;

struct thread_data {
	int		thread_id;
	BSData	bsData;
};

struct thread_data threadDataArray[NUM_THREADS];


void printExecutionArguments(double* argList, unsigned int limit);
void initializeBSData(BSData * bsData, double* argList, unsigned int limit);
bool calcBlackScholes(BSData * bsData);
void loopIteration (BSData * bsData);
void *loopIterationThread (void *threadarg);
void calcTrials(BSData * bsData);
void calcMean(BSData * bsData);
void calcStdDev(BSData * bsData);
void calcIntervals(BSData * bsData);
void printBSData(BSData * bsData);
void printHeartbeat(unsigned int counter);

int main(int argc, char* argv[]) {
	char			entries[ARGS_NUM+1][32];
	double			arguments[ARGS_NUM];
	bool			argsOK		= true;
	int				wrongArg	= -1;
	unsigned int	limit		= 0;
	double * 		argList[ARGS_NUM];
	BSData			bsData;
	
	for (int i = 0; i < ARGS_NUM; i++) {
		scanf("%s", entries[i]);
		arguments[i] = strtod(entries[i], NULL);
		argsOK = !(arguments[i] <= 0.0);
		if (!argsOK) {
			wrongArg = i + 1;
			break;
		}
	}
	scanf("%s", entries[ARGS_NUM]);
	limit = (unsigned int)strtoul(entries[ARGS_NUM], NULL, 0);
	argsOK = !(limit <= 1);

	if (!argsOK) {
		if (wrongArg == -1) {
			printf("Valor Limite (último argumento) Incorreto\n");
		} else {
			printf("%d° Argumento Incorreto: %s\n", wrongArg, entries[wrongArg-1]);
		}
		return -1;
	}

	for (int i=0; i < ARGS_NUM; i++) {
		argList[i] = &arguments[i];
	}
	
	printExecutionArguments(*argList, limit);

	if (VOCAL) printf("Iniciando...\n");
	if (VOCAL) printf("Inicializando Dados...\n");
	initializeBSData(&bsData, *argList, limit);
	if (VOCAL) printf("Dados Inicializados.\n");
	if (VOCAL) printf("Calculando...\n");
	wrongArg = calcBlackScholes(&bsData);
	if (wrongArg) {
		printf("Método Inválido para os Arugmentos de Entrada.\n");
	} else {
		printBSData(&bsData);
	}
	if (VOCAL) printf("Terminado\n");

	return 0;
}

void printExecutionArguments(double* argList, unsigned int limit) {
	printf("Argumentos de Entrada:\n");
	for (int i=0; i<ARGS_NUM; i++) {
		printf("[%d]: %f\n", i, argList[i]);
	}
	printf("Lim: %u\n\n", limit);
}

void initializeBSData(BSData * bsData, double * argList, unsigned int limit) {
	// Precisão padrão para todos os BigNums
	mpfr_set_default_prec(PRECISION);

	// Inicialização dos dados a partir dos argumentos
	mpfr_init(bsData->S);
	mpfr_set_d(bsData->S, argList[0], STDR);
	mpfr_init(bsData->E);
	mpfr_set_d(bsData->E, argList[1], STDR);
	mpfr_init(bsData->r);
	mpfr_set_d(bsData->r, argList[2], STDR);
	mpfr_init(bsData->v);
	mpfr_set_d(bsData->v, argList[3], STDR);
	mpfr_init(bsData->T);
	mpfr_set_d(bsData->T, argList[4], STDR);
	bsData->limit = limit;
	
	// Inicialização dos dados restantes
	bsData->trials = malloc(limit * sizeof(mpfr_t));
	for (int i=0; i < limit; i++) mpfr_init(bsData->trials[i]);
	mpfr_init(bsData->mean);
	mpfr_init(bsData->stddev);
	mpfr_init(bsData->confMax);
	mpfr_init(bsData->confMin);
	mpfr_init(bsData->confWidth);
}

bool calcBlackScholes(BSData * bsData) {
	// t = S.exp((r-0.5*v*v)*T).exp(v*sqrt(T)*rndNmb)
	// aux1 = S*exp((r-0.5*v*v)*T);
	// aux2 = v*sqrt(T);
	// t = aux1.exp(aux2.rndNmb)
	// trial[i] = exp(-r.T).max(t-E;0)
	// aux3 = exp(-r.T);
	// trial[i] = aux3.max(t-E;0)
	// if (t < E) trial[i] = aux3.(t-E) else trial[i] = 0
	// t = aux1.exp(aux2.rndNmb)
	// E > aux1.exp(aux2.rndNmb)
	// E/aux1 > exp(aux2.rndNmb)
	// ln(E/aux1) > aux2.rndNmb
	// rndNmb < ln(E/aux1)/aux2
	// aux4 = ln(E/aux1)/aux2
	// Condição de Simplificação
	// Se rndNmb < ln(E/aux1)/aux2, então trial[i] = 0
	// Como 0 <= rndNmb < 1, três condições
	// Se aux4 < 0, então (t-E) sempre é maior que 0 e trial[i] != 0 para todo i
	// Se aux4 > 1, trial[i] = 0 para todo i (método inválido)
	// Se 0 < aux4 < 1, então teste (t - E) é feito

	mpfr_init(bsData->aux1);
	mpfr_init(bsData->aux2);
	mpfr_init(bsData->aux3);
	mpfr_init(bsData->aux4);
	mpfr_set(bsData->aux1, bsData->v, STDR);				// aux1 = v
	mpfr_mul(bsData->aux1, bsData->aux1, bsData->aux1, STDR);// aux1 = v*v
	mpfr_div_ui(bsData->aux1, bsData->aux1, 2, STDR);		// aux1 = v*v/2
	mpfr_sub(bsData->aux1, bsData->r, bsData->aux1, STDR);	// aux1 = r - v*v/2
	mpfr_mul(bsData->aux1, bsData->aux1, bsData->T, STDR);	// aux1 = (r - v*v/2)*T
	mpfr_exp(bsData->aux1, bsData->aux1, STDR);				// aux1 = exp((r - v*v/2)*T)
	mpfr_mul(bsData->aux1, bsData->aux1, bsData->S, STDR);	// aux1 = S*exp((r - v*v/2)*T)
	mpfr_set(bsData->aux2, bsData->T, STDR);				// aux2 = T
	mpfr_sqrt(bsData->aux2, bsData->aux2, STDR);			// aux2 = sqrt(T)
	mpfr_mul(bsData->aux2, bsData->aux2, bsData->v, STDR);	// aux2 = v*sqrt(T)
	mpfr_set(bsData->aux3, bsData->T, STDR);				// aux3 = T
	mpfr_mul(bsData->aux3, bsData->aux3, bsData->r, STDR);	// aux3 = T*r
	mpfr_neg(bsData->aux3, bsData->aux3, STDR);				// aux3 = -T*r
	mpfr_exp(bsData->aux3, bsData->aux3, STDR);				// aux3 = exp(-T*r)
	mpfr_set(bsData->aux4, bsData->E, STDR);				// aux4 = E
	mpfr_div(bsData->aux4, bsData->aux4, bsData->aux1, STDR);//aux4 = E/aux1
	mpfr_log(bsData->aux4, bsData->aux4, STDR);				 // aux4 = ln(E/aux1)
	mpfr_div(bsData->aux4, bsData->aux4, bsData->aux2, STDR);// aux4 = ln(E/aux1)/aux2

	// Debug
	if (VOCAL && DEBUG) mpfr_printf("aux1: %.6RNe\n", bsData->aux1);
	if (VOCAL && DEBUG) mpfr_printf("aux2: %.6RNe\n", bsData->aux2);
	if (VOCAL && DEBUG) mpfr_printf("aux3: %.6RNe\n", bsData->aux3);
	if (VOCAL && DEBUG) mpfr_printf("aux4: %.6RNe\n", bsData->aux4);

	if (mpfr_cmp_ui(bsData->aux4, 1) >= 0) {
		return true;	// Argumentos Inválidos para o Método		
	} else {
		if (VOCAL) printf("Calculando Tentativas...\n");
		calcTrials(bsData);
		if (VOCAL) printf("Calculando Média...\n");
		calcMean(bsData);
		if (VOCAL) printf("Calculando Desvio Padrão...\n");
		calcStdDev(bsData);
		if (VOCAL) printf("Calculando Intervalos...\n");
		calcIntervals(bsData);
		if (VOCAL) printf("Calculos Finalizados.\n");
		return false;	// Execução correta
	}
}

void calcTrials(BSData * bsData) {
	// Threads
	pthread_t threads[NUM_THREADS - 1]; // Uma thread é o proprio programa principal
	pthread_attr_t attr;
	void *status;

	pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	for (int k = 0; k < NUM_THREADS; k++){
		threadDataArray[k].thread_id		= k;
		threadDataArray[k].bsData.limit 	= bsData->limit;
		threadDataArray[k].bsData.start		= k;
		threadDataArray[k].bsData.step		= STEP;
		threadDataArray[k].bsData.trials	= bsData->trials;
		mpfr_init(threadDataArray[k].bsData.aux1);
		mpfr_init(threadDataArray[k].bsData.aux2);
		mpfr_init(threadDataArray[k].bsData.aux3);
		mpfr_init(threadDataArray[k].bsData.aux4);
		mpfr_init(threadDataArray[k].bsData.E);
		mpfr_set(threadDataArray[k].bsData.aux1, bsData->aux1, STDR);
		mpfr_set(threadDataArray[k].bsData.aux2, bsData->aux2, STDR);
		mpfr_set(threadDataArray[k].bsData.aux3, bsData->aux3, STDR);
		mpfr_set(threadDataArray[k].bsData.aux4, bsData->aux4, STDR);
		mpfr_set(threadDataArray[k].bsData.E, bsData->E, STDR);
	}
	
	// Lança Threads
	for (int t = 1; t < NUM_THREADS; t++) {	// Thread 0 roda no próprio corpo
		pthread_create(
			&threads[t], &attr, loopIterationThread, (void *)&threadDataArray[t]
		);
	}

	// Calcula Thread 0 (no programa principal)
	loopIteration(&threadDataArray[0].bsData);
	
	for (int t = 1; t < NUM_THREADS; t++) {	// Thread 0 roda no próprio corpo
		pthread_join(threads[t], &status);	// então só junciona thread 1 em diante
	}
}

void loopIteration (BSData * bsData) {
	// trial[i] = aux3.max(t-E;0)
	// t = aux1.exp(aux2.rndNmb)
	// trial[i] = aux3.(aux1.exp(aux2.rndNmb)-E)
	unsigned int 	start	= bsData->start;
	unsigned int 	limit	= bsData->limit;
	unsigned int 	step	= bsData->step;
	double			random_d;
	mpfr_t			*trial;
	mpfr_t			rnd;
	struct drand48_data randBuffer;
	double random;
	
	srand48_r(time(NULL), &randBuffer);

	mpfr_init(rnd);
	
	// printf("%d ; %d ; %d\n", start, step, limit);

	for (int i=start; i < limit; i+=step) {
		// printf("%d\n",i);
		trial = &bsData->trials[i];
		drand48_r(&randBuffer, &random_d);	// Gera um double aleatório
		mpfr_set_d(rnd, random_d, STDR);
		if (mpfr_cmp(rnd, bsData->aux4) <= 0) {	// Verifica se t < E (via aux4)
			mpfr_set_zero(*trial, 1);
		} else {
			// trial[i] = aux3.(aux1.exp(aux2.rndNmb)-E)
			mpfr_set(*trial, rnd, STDR);					// *trial = rndNmb
			mpfr_mul(*trial, *trial, bsData->aux2, STDR);	// *trial = rn*aux2
			mpfr_exp(*trial, *trial, STDR);					// *trial = exp(rn*aux2)
			mpfr_mul(*trial, *trial, bsData->aux1, STDR);	// *trial = aux1*^
			mpfr_sub(*trial, *trial, bsData->E, STDR);		// *trial = ^ - E
			mpfr_mul(*trial, *trial, bsData->aux3, STDR);	// *trial = aux3 * ^
		}
	// printHeartbeat(i);
	// mpfr_printf("Trial[%d]: %.4RNf\n", i, *trial);
	}
}

void *loopIterationThread (void *threadarg) {
	// Somente o cabeçalho muda entre a thread e não-thread
	struct thread_data *argument;
	argument = (struct thread_data *) threadarg;
	long taskid		= argument->thread_id;
	BSData *		bsData;
	bsData			= &argument->bsData;

	// trial[i] = aux3.max(t-E;0)
	// t = aux1.exp(aux2.rndNmb)
	// trial[i] = aux3.(aux1.exp(aux2.rndNmb)-E)
	unsigned int 	start	= bsData->start;
	unsigned int 	limit	= bsData->limit;
	unsigned int 	step	= bsData->step;
	double			random_d;
	mpfr_t			*trial;
	mpfr_t			rnd;
	struct drand48_data randBuffer;
	double random;
	
	srand48_r(time(NULL), &randBuffer);

	mpfr_init(rnd);
	
	// printf("%d ; %d ; %d\n", start, step, limit);

	for (int i=start; i < limit; i+=step) {
		// printf("%d\n",i);
		trial = &bsData->trials[i];
		drand48_r(&randBuffer, &random_d);	// Gera um double aleatório
		mpfr_set_d(rnd, random_d, STDR);
		if (mpfr_cmp(rnd, bsData->aux4) <= 0) {	// Verifica se t < E (via aux4)
			mpfr_set_zero(*trial, 1);
		} else {
			// trial[i] = aux3.(aux1.exp(aux2.rndNmb)-E)
			mpfr_set(*trial, rnd, STDR);					// *trial = rndNmb
			mpfr_mul(*trial, *trial, bsData->aux2, STDR);	// *trial = rn*aux2
			mpfr_exp(*trial, *trial, STDR);					// *trial = exp(rn*aux2)
			mpfr_mul(*trial, *trial, bsData->aux1, STDR);	// *trial = aux1*^
			mpfr_sub(*trial, *trial, bsData->E, STDR);		// *trial = ^ - E
			mpfr_mul(*trial, *trial, bsData->aux3, STDR);	// *trial = aux3 * ^
		}
	// mpfr_printf("Trial[%d]: %.4RNf\n", i, *trial);
	// printHeartbeat(i);
	}
	pthread_exit((void *) threadarg);
}

void calcMean(BSData * bsData) {
	// mean += trial[i]*weight
	unsigned int	limit = bsData->limit;
	double			random_d;
	mpfr_t			weight;
	mpfr_t			a;

	mpfr_init(weight);
	mpfr_init(a);
	mpfr_set_ui(weight, 1, STDR);
	mpfr_div_ui(weight, weight, bsData->limit, STDR);
	mpfr_set(a, bsData->trials[0], STDR);
	mpfr_mul(a, a, weight, STDR);
	mpfr_set(bsData->mean, a, STDR);

	for (int i=1; i < limit; i++) {
		mpfr_set(a, bsData->trials[i], STDR);
		mpfr_mul(a, a, weight, STDR);
		mpfr_add(bsData->mean, bsData->mean, a, STDR);
		printHeartbeat(i);
	}
}

void calcStdDev(BSData * bsData) {
	// mean += trial[i]*weight
	unsigned int	limit = bsData->limit;
	mpfr_t			a;

	mpfr_init(a);
	mpfr_set(a, bsData->trials[0], STDR);
	mpfr_sub(a, a, bsData->mean, STDR);
	mpfr_sqr(a, a, STDR);
	mpfr_set(bsData->stddev, a, STDR);

	for (int i=1; i < limit; i++) {
		mpfr_set(a, bsData->trials[i], STDR);
		mpfr_sub(a, a, bsData->mean, STDR);
		mpfr_sqr(a, a, STDR);
		mpfr_add(bsData->stddev, bsData->stddev, a, STDR);
		printHeartbeat(i);
	}
	mpfr_div_ui(bsData->stddev, bsData->stddev, limit-1, STDR);
	mpfr_sqrt(bsData->stddev, bsData->stddev, STDR);
}

void calcIntervals(BSData * bsData) {
	mpfr_set_ui(bsData->confWidth, bsData->limit, STDR);			// confwidth = limit
	mpfr_sqrt(bsData->confWidth, bsData->confWidth, STDR);			// confwidth = sqrt(l)
	mpfr_div_d(bsData->confWidth, bsData->confWidth, 1.96, STDR);	// sqrt(l)/1.96
	mpfr_div(bsData->confWidth, bsData->stddev, bsData->confWidth, STDR);// stddev / ^
	mpfr_sub(bsData->confMin, bsData->mean, bsData->confWidth, STDR);
	mpfr_add(bsData->confMax, bsData->mean, bsData->confWidth, STDR);
}

void printBSData(BSData * bsData) {
	mpfr_printf("Margem Superior: %.4RNf\n", bsData->confMax);
	mpfr_printf("Margem Inferior: %.4RNf\n", bsData->confMin);	
}

void printHeartbeat(unsigned int counter) {
	int remove = 0;
	remove = printf("i: %10u", counter);
	for (remove; remove > 0; remove--) printf("\b");
}
