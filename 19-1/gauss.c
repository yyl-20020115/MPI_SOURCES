#include "stdio.h"
#include "stdlib.h"
#include "mpi.h"
#include "math.h"
#define a(x,y) a[x*M+y]
#define b(x) b[x]
#define A(x,y) A[x*M+y]
#define B(x) B[x]
#define floatsize sizeof(float)
#define intsize sizeof(int)
int M;
int N;
int m;
float* A;
float* B;
double starttime;
double time1;
double time2;
int my_rank;
int p;
int l;
MPI_Status status;

void fatal(char* message)
{
	printf("%s\n", message);
	exit(1);
}


void Environment_Finalize(float* a, float* b, float* x, float* f)
{
	free(a);
	free(b);
	free(x);
	free(f);
}


int main(int argc, char** argv)
{
	int i, j, t, k, my_rank, group_size;
	int i1, i2;
	int v, w;
	float temp;
	int tem;
	float* sum;
	float* f;
	float lmax;
	float* a;
	float* b;
	float* x;
	int* shift;
	FILE* fdA, * fdB;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &group_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	p = group_size;

	if (my_rank == 0)
	{
		starttime = MPI_Wtime();

		fdA = fopen("dataIn.txt", "r");
		int ret = fscanf(fdA, "%d %d", &M, &N);
		if (M != N - 1)
		{
			printf("the input is wrong\n");
			exit(1);
		}

		A = (float*)malloc(floatsize * M * M);
		B = (float*)malloc(floatsize * M);

		for (i = 0; i < M; i++)
		{
			for (j = 0; j < M; j++)
			{
				int ret = fscanf(fdA, "%f", A + i * M + j);
			}
			int ret = fscanf(fdA, "%f", B + i);
		}
		fclose(fdA);
	}

	MPI_Bcast(&M, 1, MPI_INT, 0, MPI_COMM_WORLD);     /* 0�Ŵ������M�㲥�����д���� */
	m = M / p;
	if (M % p != 0) m++;

	f = (float*)malloc(sizeof(float) * (M + 1));        /* �������Ϊ����Ԫ�ؽ������ͺͽ��ջ�����(M+1) */
	a = (float*)malloc(sizeof(float) * m * M);          /* ����������������Ӿ����СΪm*M */
	b = (float*)malloc(sizeof(float) * m);            /* �����������������������СΪm */
	sum = (float*)malloc(sizeof(float) * m);
	x = (float*)malloc(sizeof(float) * M);
	shift = (int*)malloc(sizeof(int) * M);

	if (a == NULL || b == NULL || f == NULL || sum == NULL || x == NULL || shift == NULL)
		fatal("allocate error\n");

	for (i = 0; i < M; i++)
		shift[i] = i;

	/*
	 0�Ŵ���������н��滮�ֽ�����A����Ϊ��СΪm*M��p���Ӿ���,��B����Ϊ��С
	 Ϊm��p�������������η��͸�1��p-1�Ŵ����
	*/
	if (my_rank == 0)
	{
		for (i = 0; i < m; i++)
			for (j = 0; j < M; j++)
				a(i, j) = A(i * p, j);

		for (i = 0; i < m; i++)
			b(i) = B(i * p);
	}

	if (my_rank == 0)
	{
		for (i = 0; i < M; i++)
			if ((i % p) != 0)
			{
				i1 = i % p;
				i2 = i / p + 1;

				MPI_Send(&A(i, 0), M, MPI_FLOAT, i1, i2, MPI_COMM_WORLD);
				MPI_Send(&B(i), 1, MPI_FLOAT, i1, i2, MPI_COMM_WORLD);
			}
	}                                             /*  my_rank==0 */
	else                                          /*  my_rank !=0 */
	{
		for (i = 0; i < m; i++)
		{
			MPI_Recv(&a(i, 0), M, MPI_FLOAT, 0, i + 1, MPI_COMM_WORLD, &status);
			MPI_Recv(&b(i), 1, MPI_FLOAT, 0, i + 1, MPI_COMM_WORLD, &status);
		}
	}

	time1 = MPI_Wtime();                            /* ��ʼ��ʱ */

	for (i = 0; i < m; i++)                              /* ��ȥ */
		for (j = 0; j < p; j++)
		{
			if (my_rank == j)                           /* j�Ŵ��������㲥����Ԫ�� */
			{
				v = i * p + j;                              /* ��Ԫ����ԭϵ������A�е��кź��к�Ϊv */
				lmax = a(i, v);
				l = v;

				for (k = v + 1; k < M; k++)                    /* ��ͬ�е�Ԫ���������Ԫ����ȷ�����Ԫ���ڵ���l */
					if (fabs(a(i, k)) > lmax)
					{
						lmax = a(i, k);
						l = k;
					}

				if (l != v)                             /* �н��� */
				{
					for (t = 0; t < m; t++)
					{
						temp = a(t, v);
						a(t, v) = a(t, l);
						a(t, l) = temp;
					}

					tem = shift[v];
					shift[v] = shift[l];
					shift[l] = tem;
				}

				for (k = v + 1; k < M; k++)                    /* ��һ�� */
					a(i, k) = a(i, k) / a(i, v);

				b(i) = b(i) / a(i, v);
				a(i, v) = 1;

				for (k = v + 1; k < M; k++)
					f[k] = a(i, k);
				f[M] = b(i);

				/* ���͹�һ��������� */
				MPI_Bcast(&f[0], M + 1, MPI_FLOAT, my_rank, MPI_COMM_WORLD);
				/* ������������Ԫ�����ڵ��к� */
				MPI_Bcast(&l, 1, MPI_INT, my_rank, MPI_COMM_WORLD);
			}
			else
			{
				v = i * p + j;
				MPI_Bcast(&f[0], M + 1, MPI_FLOAT, j, MPI_COMM_WORLD);
				MPI_Bcast(&l, 1, MPI_INT, j, MPI_COMM_WORLD);

				if (l != v)
				{
					for (t = 0; t < m; t++)
					{
						temp = a(t, v);
						a(t, v) = a(t, l);
						a(t, l) = temp;
					}

					tem = shift[v];
					shift[v] = shift[l];
					shift[l] = tem;
				}
			}

			if (my_rank <= j)
				for (k = i + 1; k < m; k++)
				{
					for (w = v + 1; w < M; w++)
						a(k, w) = a(k, w) - f[w] * a(k, v);
					b(k) = b(k) - f[M] * a(k, v);
				}

			if (my_rank > j)
				for (k = i; k < m; k++)
				{
					for (w = v + 1; w < M; w++)
						a(k, w) = a(k, w) - f[w] * a(k, v);
					b(k) = b(k) - f[M] * a(k, v);
				}
		}                                             /* for i j */

	for (i = 0; i < m; i++)
		sum[i] = 0.0;

	for (i = m - 1; i >= 0; i--)                           /* �ش� */
		for (j = p - 1; j >= 0; j--)
			if (my_rank == j)
			{
				x[i * p + j] = (b(i) - sum[i]) / a(i, i * p + j);

				MPI_Bcast(&x[i * p + j], 1, MPI_FLOAT, my_rank, MPI_COMM_WORLD);

				for (k = 0; k < i; k++)
					sum[k] = sum[k] + a(k, i * p + j) * x[i * p + j];
			}
			else
			{
				MPI_Bcast(&x[i * p + j], 1, MPI_FLOAT, j, MPI_COMM_WORLD);

				if (my_rank > j)
					for (k = 0; k < i; k++)
						sum[k] = sum[k] + a(k, i * p + j) * x[i * p + j];

				if (my_rank < j)
					for (k = 0; k <= i; k++)
						sum[k] = sum[k] + a(k, i * p + j) * x[i * p + j];
			}

	if (my_rank != 0)
		for (i = 0; i < m; i++)
			MPI_Send(&x[i * p + my_rank], 1, MPI_FLOAT, 0, i, MPI_COMM_WORLD);
	else
		for (i = 1; i < p; i++)
			for (j = 0; j < m; j++)
				MPI_Recv(&x[j * p + i], 1, MPI_FLOAT, i, j, MPI_COMM_WORLD, &status);

	if (my_rank == 0)
	{
		printf("Input of file \"dataIn.txt\"\n");
		printf("%d\t%d\n", M, N);
		for (i = 0; i < M; i++)
		{
			for (j = 0; j < M; j++) printf("%f\t", A(i, j));
			printf("%f\n", B(i));
		}
		printf("\nOutput of solution\n");
		for (k = 0; k < M; k++)
		{
			for (i = 0; i < M; i++)
			{
				if (shift[i] == k) printf("x[%d]=%f\n", k, x[i]);
			}
		}
	}

	time2 = MPI_Wtime();

	if (my_rank == 0)
	{
		printf("\n");
		printf("Whole running time    = %f seconds\n", time2 - starttime);
		printf("Distribute data time  = %f seconds\n", time1 - starttime);
		printf("Parallel compute time = %f seconds\n", time2 - time1);
	}

	MPI_Finalize();
	Environment_Finalize(a, b, x, f);
	return(0);
}
