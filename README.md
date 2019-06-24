Lista dos Programas Neste Projeto
---
| Programa (caminho) | Descrição |
|:--- | ----: |
|\Code\PiCalc\BBP.c | Cálculo do Pi pelo Método BBP (Sequencial)|
|\Code\PiCalc\Gauss-Legendre.c | Cálculo do Pi pelo Método Gauss-Legendre (Sequencial)|
|\Code\PiCalc\MonteCarlo.c | Cálculo do Pi pelo Método Monte Carlo (Sequencial)|
|\Code\PiCalcThreads\BBPThreaded.c | Cálculo do Pi pelo Método BBP (Threaded, Ineficiente)|
|\Code\PiCalcThreads\BBPThreaded_v2.c | Cálculo do Pi pelo Método BBP (Threaded, Eficiente)|
|\Code\PiCalcThreads\Gauss-LegendreThreaded.c | Cálculo do Pi pelo Método Gauss-Legendre (Threaded)|
|\Code\PiCalcThreads\MonteCarloThreaded.c | Cálculo do Pi pelo Método Monte Carlo (Threaded, versão errada)|
|\Code\PiCalcThreads\MonteCarloThreaded_v2.c | Cálculo do Pi pelo Método Monte Carlo (Threaded, versão correta)|
|\Code\ModeloBlackScholes\BlackScholes.c | Cálculo de Opção Call pelo Método Black-Scholes (Sequencial)|
|\Code\ModeloBlackScholesThreads\BlackScholesThreads.c | Cálculo de Opção Call pelo Método Black-Scholes (Threaded)|
|\Code\TestingBench\BBP_debug.c | Cálculo do Pi pelo Método BBP (Sequencial, com saída de todas variáveis)|
|\Code\TestingBench\Gauss-Legendre_debug.c | Cálculo do Pi pelo Método Gauss-Legendre (Sequencial, com saída de todas variáveis)|


Como Utilizar a Pasta de Projeto (com Visual Studio Code)
---
1. Abrir o arquivo SO2019-BRUNOFEITOSA.code-workspace com o Visual Studio Code
2. Abrir um dos Arquivos .c em uma das subpastas de Code
3. Compilar o arquivo com o comando Ctrl+Shift+B
4. Abrir a subpasta onde está o arquivo .c
5. Executar o binário compilado (criado com o mesmo nome do arquivo .c)

Como Utilizar a Pasta de Projeto (sem Visual Studio Code)
---
1. Abrir uma das subpastas de Code
2. Compilar o arquivo .c desejado usando as flags -lgmp -lmpfr -pthread
3. Executar o binário compilado

Notas
---
1. As flags VOCAL e DEBUG (quando presentes) podem ser alternadas entre true e false para melhor visualização da execução do programa
2. Certifique-se de que as bibliotecas GMP e MPFR estão instaladas (GMP deve ser instalada primeiro)
