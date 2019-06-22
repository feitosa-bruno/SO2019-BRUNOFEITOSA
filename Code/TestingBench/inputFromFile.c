#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define ARGS_NUM 5

unsigned int limit = -1;

int main(int argc, char *argv[])
{
	char	entries[5][32];
	double	arguments[5];
	bool	argsOK		= true;
	int		wrongArg	= -1;

	for (int i=0; i < ARGS_NUM; i++) {
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
	argsOK = !(limit <= 0);

	if (!argsOK) {
		if (wrongArg == -1) {
			printf("Valor Limite (último argumento) Incorreto\n");
		} else {
			printf("%d° Argumento Incorreto: %s\n", wrongArg, entries[wrongArg-1]);
		}
		return -1;
	}

	for (int i=0; i < ARGS_NUM; i++) {
		printf("args[%d]: %f\n", i, arguments[i]);
	}
	printf("Limite: %u\n", limit);

	return 0;
}