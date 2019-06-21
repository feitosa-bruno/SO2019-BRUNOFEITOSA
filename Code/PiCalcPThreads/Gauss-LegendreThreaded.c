// Método Gauss-Legendre

#include <pthread.h>
#include <stdio.h>
#include <gmp.h>

// #define PRECISION	256
// #define	LIMIT		600			// Limite para precisão de 32 bytes

// #define PRECISION	512
// #define LIMIT 		1000		// Limite para precisão de 64 bytes

// #define PRECISION	1024
// #define LIMIT 		2000		// Limite para precisão de 128 bytes

#define PRECISION		8192
#define LIMIT 			15000		// Limite para precisão de 1KB /1024 Bytes/ 8192 bits

// #define PRECISION	8388608		// 1MB de Precisão quebra a biblioteca/memória

// Número de Threads
#define NUM_THREADS 1

struct thread_data {
	int thread_id;
	mpf_t *bn_;
	mpf_t *bn;
	mpf_t *an;
};

struct thread_data threadDataArray[NUM_THREADS];


void *bn_Calc(void *threadarg);
void printf_piGLt_gmp(void);
void printResult(mpf_t result, mpf_t target, mpf_t delta, mpf_t accuracy, mpf_t tn_);
void printDebugHearbeat(unsigned int counter, mpf_t accuracy, mpf_t tn_);
void printHeartbeat(unsigned int counter);

int main(int argc, char* argv[]) {
	// Precisão padrão para todos os BigNums
	mpf_set_default_prec(PRECISION);

	printf("Iniciando...\n");
	printf_piGLt_gmp();
	printf("Terminado\n");

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

void printResult(
	mpf_t result,
	mpf_t target,
	mpf_t delta,
	mpf_t accuracy,
	mpf_t tn_) {
	gmp_printf("\nPI: %.*Ff (Calculado)\n", 6, result);
	gmp_printf("PI: %.*Ff (Esperado)\n", 6, target);
	gmp_printf("ER: %.*Ff (Erro)\n", 6, delta);
	gmp_printf("AC: %.*Fe (Acurácia an - bn)\n", 6, accuracy);
	gmp_printf("tn: %.*Fe (tn)\n", 6, tn_);
}

void printf_piGLt_gmp(void) {
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

	// Variáveis de Iteração Seguinte
	mpf_t an_, bn_, tn_, pn_;
	mpf_init (an_);
	mpf_init (bn_);
	mpf_init (tn_);
	mpf_init (pn_);

	// Iterador e Limite
	unsigned int i = 0;
	unsigned int lim = LIMIT;

	for (i = 0; i < lim; i++) {
		// Iteração
		// b_n+1 (Threaded: Independente das restantes)
		threadDataArray[0].thread_id	= 0;
		threadDataArray[0].bn_ 			= &bn_;
		threadDataArray[0].bn			= &bn;
		threadDataArray[0].an			= &an;
		pthread_create(&threads[0], &attr, bn_Calc, (void *) &threadDataArray[0]);
		// a_n+1
		mpf_add(an_, an, bn);		// an_ = an + bn
		mpf_div_ui(an_, an_, 2);	// an_ = (an + bn)/2
		// t_n+1
		mpf_sub(aux, an, an_);		// aux = an - an_
		mpf_mul(aux, aux, aux);		// aux = (an - an_) * (an - an_)
		mpf_mul(aux, pn, aux);		// aux = pn * (an - an_) * (an - an_)
		mpf_sub(tn_, tn, aux);		// tn_ = tn - pn * (an - an_) * (an - an_)
		// p_n+1
		mpf_mul_ui(pn_, pn, 2);		// pn_ = pn * 2
		
		// Esperar resultado de bn_
		pthread_join(threads[0], &status);

		// Debug (an - bn)
		mpf_sub(aux, an_, bn_);		// Acurácia (an - bn)

		// Atualização para Iteração Seguinte
		mpf_set(an, an_);
		mpf_set(bn, bn_);
		mpf_set(tn, tn_);
		mpf_set(pn, pn_);

		// Verificação de que o programa está rodando
		// printHeartbeat(i);
		printDebugHearbeat(i, aux, tn_);
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

void *bn_Calc(void *threadarg) {
	struct thread_data *argument;
	argument = (struct thread_data *) threadarg;
	long taskid		= argument->thread_id;
	mpf_t *bn_;
	mpf_t *bn;
	mpf_t *an;
	bn_				= argument->bn_;
	bn				= argument->bn;
	an				= argument->an;
	
	// b_n+1
	mpf_mul(*bn_, *an, *bn);		// bn_ = an * bn
	mpf_sqrt(*bn_, *bn_);			// bn_ = sqrt(an * bn)

	pthread_exit((void *) threadarg);
}
