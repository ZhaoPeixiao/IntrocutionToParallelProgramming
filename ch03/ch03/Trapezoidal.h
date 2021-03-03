#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<string.h>
#include<mpi.h>

double f(double x)
{
	return x * x + 2 * x + 0.5;
}

double Trap(double left_endpt, double right_endpt, int trap_count, double base_len)
{
	double estimate, x;
	int i;

	estimate = (f(left_endpt) + f(right_endpt)) / 2.0;
	for (i = 1; i <= trap_count - 1; i++)
	{
		x = left_endpt + i * base_len;
		estimate += f(x);
	}
	estimate = estimate * base_len;

	return estimate;
}

void Get_input(int my_rank, int comm_sz, double* a_p, double* b_p, int* n_p)
{
	int dest;
	if (my_rank == 0)
	{
		printf("Enter a, b, and n \n");
		scanf("%lf %lf %d", a_p, b_p, n_p);
		for (dest = 1; dest < comm_sz; dest++)
		{
			MPI_Send(a_p, 1, MPI_DOUBLE, dest, 0, MPI_COMM_WORLD);
			MPI_Send(b_p, 1, MPI_DOUBLE, dest, 0, MPI_COMM_WORLD);
			MPI_Send(n_p, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
		}
	}
	else
	{
		MPI_Recv(a_p, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(b_p, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(n_p, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}
}