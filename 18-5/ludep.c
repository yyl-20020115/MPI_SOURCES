#include "stdio.h"
#include "stdlib.h"
#include "mpi.h"
#define a(x,y) a[x*M+y]
/*A为M*M矩阵*/
#define A(x,y) A[x*M+y]
#define l(x,y) l[x*M+y]
#define u(x,y) u[x*M+y]
#define floatsize sizeof(float)
#define intsize sizeof(int)

int M, N;
int m;
float* A;
int my_rank;
int p;
MPI_Status status;

void fatal(char* message)
{
	printf("%s\n", message);
	exit(1);
}

void Environment_Finalize(float* a, float* f)
{
	free(a);
	free(f);
}

int main(int argc, char** argv)
{
	int i, j, k, my_rank, group_size;
	int i1, i2;
	int v, w;
	float* a, * f, * l, * u;
	FILE* fdA;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &group_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	p = group_size;

	if (my_rank == 0)
	{
		fdA = fopen("dataIn.txt", "r");
		int ret = fscanf(fdA, "%d %d", &M, &N);
		if (M != N)
		{
			puts("The input is error!");
			exit(0);
		}
		A = (float*)malloc(floatsize * M * M);
		for (i = 0; i < M; i++)
			for (j = 0; j < M; j++) {
				int ret = fscanf(fdA, "%f", A + i * M + j);
			}
		fclose(fdA);
	}

	/*0号进程将M广播给所有进程*/
	MPI_Bcast(&M, 1, MPI_INT, 0, MPI_COMM_WORLD);
	m = M / p;
	if (M % p != 0) m++;

	/*分配至各进程的子矩阵大小为m*M*/
	a = (float*)malloc(floatsize * m * M);

	/*各进程为主行元素建立发送和接收缓冲区*/
	f = (float*)malloc(floatsize * M);

	/*0号进程为l和u矩阵分配内存，以分离出经过变换后的A矩阵中的l和u矩阵*/
	if (my_rank == 0)
	{
		l = (float*)malloc(floatsize * M * M);
		u = (float*)malloc(floatsize * M * M);
	}

	/*0号进程采用行交叉划分将矩阵A划分为大小m*M的p块子矩阵，依次发送给1至p-1号进程*/
	if (a == NULL) fatal("allocate error\n");

	if (my_rank == 0)
	{
		for (i = 0; i < m; i++)
			for (j = 0; j < M; j++)
				a(i, j) = A((i * p), j);
		for (i = 0; i < M; i++)
			if ((i % p) != 0)
			{
				i1 = i % p;
				i2 = i / p + 1;
				MPI_Send(&A(i, 0), M, MPI_FLOAT, i1, i2, MPI_COMM_WORLD);
			}
	}
	else
	{
		for (i = 0; i < m; i++)
			MPI_Recv(&a(i, 0), M, MPI_FLOAT, 0, i + 1, MPI_COMM_WORLD, &status);
	}

	for (i = 0; i < m; i++)
		for (j = 0; j < p; j++)
		{
			/*j号进程负责广播主行元素*/
			if (my_rank == j)
			{
				v = i * p + j;
				for (k = v; k < M; k++)
					f[k] = a(i, k);

				MPI_Bcast(f, M, MPI_FLOAT, my_rank, MPI_COMM_WORLD);
			}
			else
			{
				v = i * p + j;
				MPI_Bcast(f, M, MPI_FLOAT, j, MPI_COMM_WORLD);
			}

			/*编号小于my_rank的进程（包括my_rank本身）利用主行对其第i+1,…,m-1行数据做行变换*/
			if (my_rank <= j)
				for (k = i + 1; k < m; k++)
				{
					a(k, v) = a(k, v) / f[v];
					for (w = v + 1; w < M; w++)
						a(k, w) = a(k, w) - f[w] * a(k, v);
				}

			/*编号大于my_rank的进程利用主行对其第i,…,m-1行数据做行变换*/
			if (my_rank > j)
				for (k = i; k < m; k++)
				{
					a(k, v) = a(k, v) / f[v];
					for (w = v + 1; w < M; w++)
						a(k, w) = a(k, w) - f[w] * a(k, v);
				}
		}

	/*0号进程从其余各进程中接收子矩阵a，得到经过变换的矩阵A*/
	if (my_rank == 0)
	{
		for (i = 0; i < m; i++)
			for (j = 0; j < M; j++)
				A(i * p, j) = a(i, j);
	}
	if (my_rank != 0)
	{
		for (i = 0; i < m; i++)
			MPI_Send(&a(i, 0), M, MPI_FLOAT, 0, i, MPI_COMM_WORLD);
	}
	else
	{
		for (i = 1; i < p; i++)
			for (j = 0; j < m; j++)
			{
				MPI_Recv(&a(j, 0), M, MPI_FLOAT, i, j, MPI_COMM_WORLD, &status);
				for (k = 0; k < M; k++)
					A((j * p + i), k) = a(j, k);
			}
	}

	if (my_rank == 0)
	{
		for (i = 0; i < M; i++)
			for (j = 0; j < M; j++)
				u(i, j) = 0.0;
		for (i = 0; i < M; i++)
			for (j = 0; j < M; j++)
				if (i == j)
					l(i, j) = 1.0;
				else
					l(i, j) = 0.0;
		for (i = 0; i < M; i++)
			for (j = 0; j < M; j++)
				if (i > j)
					l(i, j) = A(i, j);
				else
					u(i, j) = A(i, j);
		printf("Input of file \"dataIn.txt\"\n");
		printf("%d\t %d\n", M, N);
		for (i = 0; i < M; i++)
		{
			for (j = 0; j < N; j++)
				printf("%f\t", A(i, j));
			printf("\n");
		}
		printf("\nOutput of LU operation\n");
		printf("Matrix L:\n");
		for (i = 0; i < M; i++)
		{
			for (j = 0; j < M; j++)
				printf("%f\t", l(i, j));
			printf("\n");
		}
		printf("Matrix U:\n");
		for (i = 0; i < M; i++)
		{
			for (j = 0; j < M; j++)
				printf("%f\t", u(i, j));
			printf("\n");
		}
	}
	MPI_Finalize();
	Environment_Finalize(a, f);
	return(0);
}
