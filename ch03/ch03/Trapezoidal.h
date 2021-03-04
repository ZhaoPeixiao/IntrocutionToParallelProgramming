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

void Build_mpi_type(double* a_p, double* b_p, double* n_p, MPI_Datatype* input_mpi_t_p)
{
	int array_of_blocklengths[3] = { 1, 1, 1 };
	MPI_Datatype array_of_types[3] = { MPI_DOUBLE,MPI_DOUBLE, MPI_INT };
	MPI_Aint a_addr, b_addr, n_addr;
	MPI_Aint arrar_of_displacements[3] = { 0 };

	MPI_Get_address(a_p, &a_addr);
	MPI_Get_address(b_p, &b_addr);
	MPI_Get_address(n_p, &n_addr);

	arrar_of_displacements[1] = b_addr - a_addr;
	arrar_of_displacements[2] = n_addr - b_addr;

	MPI_Type_create_struct(3, array_of_blocklengths, arrar_of_displacements, array_of_types, input_mpi_t_p);
	MPI_Type_commit(input_mpi_t_p);
}

void Get_input(int my_rank, int comm_sz, double* a_p, double* b_p, int* n_p)
{
	//int dest;
	//if (my_rank == 0)
	//{
	//	printf("Enter a, b, and n \n");
	//	scanf("%lf %lf %d", a_p, b_p, n_p);
	//	for (dest = 1; dest < comm_sz; dest++)
	//	{
	//		MPI_Send(a_p, 1, MPI_DOUBLE, dest, 0, MPI_COMM_WORLD);
	//		MPI_Send(b_p, 1, MPI_DOUBLE, dest, 0, MPI_COMM_WORLD);
	//		MPI_Send(n_p, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
	//	}
	//}
	//else
	//{
	//	MPI_Recv(a_p, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	//	MPI_Recv(b_p, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	//	MPI_Recv(n_p, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	//}

	MPI_Datatype input_mpi_t;
	Build_mpi_type(a_p, b_p, n_p, &input_mpi_t);
	// MPI_Bcast
	if (my_rank == 0)
	{
		printf("Enter a, b, and n \n");
		scanf("%lf %lf %d", a_p, b_p, n_p);
	}

	//MPI_Bcast(a_p, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	//MPI_Bcast(b_p, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	//MPI_Bcast(n_p, 1, MPI_INT, 0, MPI_COMM_WORLD);

	// 派生数据类型 节约消息发送时间
	MPI_Bcast(a_p, 1, input_mpi_t, 0, MPI_COMM_WORLD);

	MPI_Type_free(&input_mpi_t);
}

int trapezoidal(void)
{
	int my_rank, comm_sz, n = 1024, local_n;
	double a = 0.0, b = 3.0, h, local_a, local_b;
	double local_int, total_int;
	int source;

	MPI_Init(NULL, NULL);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

	//Get_input(my_rank, comm_sz, &a, &b, &n);

	h = (b - a) / n;
	local_n = n / comm_sz;

	local_a = a + my_rank * local_n * h;
	local_b = local_a = local_n * h;
	local_int = Trap(local_a, local_b, local_n, h);

	if (my_rank != 0)
	{
		MPI_Send(&local_int, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
	}
	else
	{
		total_int = local_int;
		for (source = 1; source < comm_sz; source++)
		{
			MPI_Recv(&local_int, 1, MPI_DOUBLE, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			total_int += local_int;
		}
	}

	if (my_rank == 0)
	{
		printf("With n = %d trapezoids, our estimate\n", n);
		printf("of the integral from %f to %f = %.15e\n", a, b, total_int);
	}

	MPI_Finalize();

	return 0;
}