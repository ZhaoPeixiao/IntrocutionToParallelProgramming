#pragma comment(lib, "pthreadVC2.lib")
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

/* Global variable: accessible for all threads*/

int thread_count;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* Hello(void* rank); /* thread function*/

/* mat size*/
const int m = 10;
const int n = 10;
/* construct mat*/
double y[5];
double A[5][5] = { {1.0, 2.0, 3.0, 4.0, 5.0},
					{2.0, 3.0, 4.0, 5.0, 6.0},
					{3.0, 4.0, 5.0, 6.0, 7.0},
					{4.0, 5.0, 6.0, 7.0, 8.0},
					{5.0, 6.0, 7.0, 8.0, 9.0}
				};
double x[5] = {1.0, 1.0, 1.0, 1.0, 1.0};
void* Pth_mat_vect(void* rank); /* calculate mat multi*/

/*calculate pi */
const long pi_n = 1e8;
double sum = 0.0;
int pi_flag = 0;
void* Thread_sum(void* rank);


int main(int argc, char* argv[])
{
	long thread;		/* use long in case of a 64-bit system*/
	pthread_t* thread_handles;

	/* get number of threads from command line*/
	thread_count = strtol(argv[1], NULL, 10);

	thread_handles = malloc(thread_count * sizeof(pthread_t));

	for (thread = 0; thread < thread_count; thread++)
	{
		pthread_create(&thread_handles[thread], NULL, Thread_sum, (void*)thread);
	}

	printf("Hello from main thread, pi_n is %d\n", pi_n);

	for (thread = 0; thread < thread_count; thread++)
	{
		pthread_join(thread_handles[thread], NULL);
	}

	//for (int i = 0; i < 5; i++)
	//{
	//	printf("%lf ", y[i]);
	//}
	printf("pi is %lf", 4.0 * sum);
	printf("\n");

	free(thread_handles);

	system("pause");

	return 0;
}

void* Hello(void* rank)
{
	long my_rank = (long)rank;

	printf("Hello from thread %ld of %d\n", my_rank, thread_count);

	return NULL;
}

void* Pth_mat_vect(void* rank)
{
	long my_rank = (long)rank;
	int i, j;
	int local_m = m / thread_count;
	int my_first_row = my_rank * local_m;
	int my_last_row = (my_rank + 1) * local_m - 1;

	for (i = my_first_row; i <= my_last_row; i++)
	{
		y[i] = 0.0;
		for (j = 0; j < n; j++)
		{
			y[i] += A[i][j] * x[j];
		}
	}
}

void* Thread_sum(void* rank)
{
	long my_rank = (long)rank;
	double factory, my_sum = 0.0;
	long long i;
	long long my_n = pi_n / thread_count;
	long long my_first_i = my_n * my_rank;
	long long my_last_i = my_first_i + my_n;

	if (my_first_i % 2 == 0)
	{
		factory = 1.0;
	}
	else
	{
		factory = -1.0;
	}

	for (i = my_first_i; i < my_last_i; i++, factory = -factory)
	{
		my_sum += factory / (2 * i + 1);
	}

	// Ã¦µÈ´ý
	/*	
	while (pi_flag != my_rank);
	sum += my_sum;
	pi_flag = (pi_flag + 1) % thread_count; 
	*/

	// »¥³âÁ¿
	pthread_mutex_lock(&mutex);
	sum += my_sum;
	pthread_mutex_unlock(&mutex);

	return NULL;
}-