#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[])
{
	double x;
	struct drand48_data *randBuffer;

	srand48_r(time(NULL), randBuffer);

	drand48_r(randBuffer, &x);

	printf("Random number: %f\n", x);

	return 0;
}