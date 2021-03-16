#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<omp.h>

void Hello(void);
void Trap(double a, double b, int n, double* global_result_p);
double Local_Trap(double a, double b, int n);
double Pi(int n, int thread_count);

double f(double x)
{
	return x * x + 2.0 * x - 1.5;
}

double f(int i)
{
	int j, start = i * (i + 1) / 2, finish = start + i;
	double return_val = 0.0;

	for (j = start; j <= finish; j++)
	{
		return_val += sin(j);
	}
	return return_val;
}

int main(int argc, char* argv[])
{
	double global_result = 0.0;
	double a, b;
	int n;
	int thread_count;

	thread_count = strtol(argv[1], NULL, 10);
	printf("Enter a, b, and n\n");
	scanf("%lf %lf %d", &a, &b, &n);
//#	pragma omp parallel num_threads(thread_count)
//	Trap(a, b, n, &global_result);

//#	pragma omp parallel num_threads(thread_count)
//	{
//		double my_result = Local_Trap(a, b, n);
//#	pragma omp critical
//		global_result += my_result;
//	}

#pragma omp parallel num_threads(thread_count) reduction(+: global_result)
	global_result += Local_Trap(a, b, n);

	printf("With n = %d trapezoids, our estimate\n", n);
	printf("of the integral from %f to %f = %.14e\n", a, b, global_result);
//#pragma omp parallel num_threads(thread_count)
//	Hello();

	system("pause");

	return 0;
}

void Hello(void)
{
	int my_rank = omp_get_thread_num();
	int thread_count = omp_get_num_threads();

	printf("Hello from thread %d of %d\n", my_rank, thread_count);
}

void Trap(double a, double b, int n, double* global_result_p)
{
	double h, x, my_result;
	double local_a, local_b;
	int i, local_n;
	int my_rank = omp_get_thread_num();
	int thread_count = omp_get_num_threads();

	h = (b - a) / n;
	local_n = n / thread_count;
	local_a = a + my_rank * local_n * h;
	local_b = local_a + local_n * h;
	my_result = (f(local_a) + f(local_b)) / 2.0;
	for (i = 1; i < local_n - 1; i++)
	{
		x = local_a + i * h;
		my_result += f(x);
	}
	my_result = my_result * h;

#pragma omp critical
	* global_result_p += my_result;
}

double Local_Trap(double a, double b, int n)
{
	double h, x, my_result;
	double local_a, local_b;
	int i, local_n;
	int my_rank = omp_get_thread_num();
	int thread_count = omp_get_num_threads();

	h = (b - a) / n;
	local_n = n / thread_count;
	local_a = a + my_rank * local_n * h;
	local_b = local_a + local_n * h;
	my_result = (f(local_a) + f(local_b)) / 2.0;
	for (i = 1; i < local_n - 1; i++)
	{
		x = local_a + i * h;
		my_result += f(x);
	}
	my_result = my_result * h;

	return my_result;
}

double Pi(int n, int thread_count)
{
	double pi_approx = 0.0;
	double factor = 1.0;
	double sum = 0.0;
	int k;
#pragma omp parallel for num_threads(thread_count) default(none) reduction(+: sum) private(k, factor) shared(n)
	for (k = 0; k < n; k++)
	{
		factor = (k % 2 == 0) ? 1.0 : -1.0;
		sum += factor / (2 * k + 1);
	}

	pi_approx = 4.0 * sum;

	return pi_approx;
}

void Sort1(int a[], int n, int thread_count)
{
	int phase, i, tmp;
	for (phase = 0; phase < n; phase++)
	{
		if (phase % 2 == 0)
		{
# pragma omp parallel for num_threads(thread_count) default(none) shared(a, n) private(i, tmp)
			for (i = 1; i < n; i += 2)
			{
				if (a[i - 1] > a[i])
				{
					tmp = a[i - 1];
					a[i - 1] = a[i];
					a[i] = tmp;
				}
			}
		}
		else
		{
#pragma omp parallel for num_threads(thread_count) default(none) shared(a, n) private(i, tmp)
			for (i = 1; i < n - 1; i += 2)
			{
				if (a[i] > a[i + 1])
				{
					tmp = a[i + 1];
					a[i + 1] = a[i];
					a[i] = tmp;
				}
			}
		}
	}
}


void Sort2(int a[], int n, int thread_count)
{
	int phase, i, tmp;
#pragma omp parallel num_threads(thread_count) default(none) shared(a, n) private(i, tmp, phase)
	{
		for (phase = 0; phase < n; phase++)
		{
			if (phase % 2 == 0)
			{
#pragma omp for
				for (i = 1; i < n; i += 2)
				{
					if (a[i - 1] > a[i])
					{
						tmp = a[i - 1];
						a[i - 1] = a[i];
						a[i] = tmp;
					}
				}
			}
			else
			{
#pragma omp for
				for (i = 1; i < n - 1; i += 2)
				{
					if (a[i] > a[i + 1])
					{
						tmp = a[i + 1];
						a[i + 1] = a[i];
						a[i] = tmp;
					}
				}
			}
		}
	}
}

// 循环调度
/*
1. static: 轮转分配
2. dynamic: 运行完请求下一块
3. guided: 请求的下一块会变小
4. runtime: 调用OMP_SCHEDULE定义的值

系统开销：guided > dynamic > static
*/
void Test1(int n, int thread_count)
{
	double sum = 0.0;
#pragma omp parallel for num_threads(thread_count) reduction(+:sum) schedule(static,1)
	for (int i = 0; i <= n; i++)
	{
		sum += f(i);
	}
}