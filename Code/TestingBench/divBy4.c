#include <stdio.h>

int main(int argc, char *argv[])
{
	unsigned int x = 10000;

	printf("X:   %u\n", x);
	printf("X/4: %u\n", x >> 2);

	return 0;
}