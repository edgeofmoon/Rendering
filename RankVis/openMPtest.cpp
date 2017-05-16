#include <cstdio>
#include <omp.h>

/*
This source code can be downloaded from supercomputingblog.com
The purpose of this code is to ensure that OpenMP is working and running on multiple threads.
*/

int main(int argc, char* argv[])
{
	// This statement should only print once
	printf("Starting Program!\n");

#pragma omp parallel
	{
		// This statement will run on each thread.
		// If there are 4 threads, this will execute 4 times in total
		printf("Running on multiple threads\n");
	}

	// We're out of the parallelized secion.
	// Therefor, this should execute only once
	printf("Finished!\n");

	char c;
	scanf("%c", &c);
	return 0;
}