#include "stdio.h"
#include "stdlib.h"
#include "mpi.h"

#define a(x,y) a[x*M+y]
#define A(x,y) A[x*M+y]
#define B(x,y) B[x*M+y]
#define floatsize sizeof(float)
#define intsize sizeof(int)

int M, N;
float* A, * B;
double starttime;
double time1;
double time2;
int my_rank;
int p;
int m;
float* a;
float* f;
MPI_Status status;

/*
 * ��������fatal
 * ���ܣ�����������ش���ʱ��ֹ������������Ϣ��
 * ���룺messageΪ�洢������Ϣ���ַ����飻
 */
void fatal(char* message)
{
	printf("%s\n", message);
	exit(1);
}

/*
 * ��������Environment_Finalize
 * ���ܣ����������ռ�õ��ڴ�ռ䣻
 * ���룺a��f��Ϊ���渡���������飬�ֱ����ڴ�����и�Ԫ�غ�
 *       �����̽�Ҫ��������ݣ�
 */
void Environment_Finalize(float* a, float* f)
{
	free(a);
	free(f);
}


/*
 * ��������matrix_init
 * ���ܣ������ʼ������������洢�ռ䣬�������ļ��ж������ݣ�
 * ���룺�����������
 * ������޷���ֵ��
 */
void matrix_init()
{
	int i, j;
	FILE* fdA;
	if (my_rank == 0)
	{
		/* starttime��¼��ʼ�������ݵ�ʱ�� */
		starttime = MPI_Wtime();
		fdA = fopen("dataIn.txt", "r");
		fscanf(fdA, "%d %d", &M, &N);
		/* ��������Ƿ����򱨴� */
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
		B = (float*)malloc(floatsize * M * M);
	}
	/* �㲥M(�����ά��)�����н��� */
	MPI_Bcast(&M, 1, MPI_INT, 0, MPI_COMM_WORLD);
}

/*
 * ��������maxtrix_partition
 * ���ܣ����󻮷֡�0�Ž��̶Ծ���A�����н��滮�֣������ֺ���Ӿ���
 *       �ֱ��͸�1��p-1�Ž��̡�
 */
void matrix_partition()
{
	int i, j, i1, i2;
	/* ������Ϊ����Ԫ�ؽ������ͺͽ��ջ����� */
	f = (float*)malloc(sizeof(float) * M);
	/* �����������̵��Ӿ����СΪm*M */
	a = (float*)malloc(sizeof(float) * m * M);
	if (a == NULL || f == NULL)
		fatal("allocate error\n");
	/* 0�Ž��̿�����Ӧ���ݵ����Ӿ���a */
	if (my_rank == 0)
	{
		for (i = 0; i < m; i++)
			for (j = 0; j < M; j++)
				a(i, j) = A(i * p, j);
		/* 0�Ž��̲����н��滮�ֽ�����A����Ϊm*M��p���Ӿ������η��͸�1��p-1�Ž��� */
		for (i = 0; i < m * p; i++)
			if ((i % p) != 0)
			{
				i1 = i % p;
				i2 = i / p + 1;
				if (i < M)
					MPI_Send(&A(i, 0), M, MPI_FLOAT, i1, i2, MPI_COMM_WORLD);
				else
					MPI_Send(&A(0, 0), M, MPI_FLOAT, i1, i2, MPI_COMM_WORLD);
			}
	}
	else
	{                                             /* �������̽��ո��Ե��Ӿ������� */
		for (i = 0; i < m; i++)
			MPI_Recv(&a(i, 0), M, MPI_FLOAT, 0, i + 1, MPI_COMM_WORLD, &status);
	}
}

/*
 * ��������broadcast_j
 * ���ܣ����Ϊj�Ľ��̹㲥����Ԫ��
 * ���룺iΪÿ���������ڴ�����Ӿ�����кţ�
 *       jΪ��Ԫ�����ڵĽ��̵ı�ţ�
 *       vΪ��Ԫ��(�ھ���A��)���кţ�
 */
void broadcast_j(int i, int j, int v)
{
	int k;
	/* j�Ž��̹㲥����Ԫ�� */
	if (my_rank == j)
	{
		a(i, v) = 1 / a(i, v);
		for (k = 0; k < M; k++)
		{
			if (k != v)
				a(i, k) = a(i, k) * a(i, v);             /* �������и�Ԫ�� */
			f[k] = a(i, k);
		}
		/* ���任�������Ԫ��(��������f��)�㲥�����н����� */
		MPI_Bcast(f, M, MPI_FLOAT, my_rank, MPI_COMM_WORLD);
	}
	else
	{
		/* ������̽��չ㲥��������Ԫ�ش�������f�� */
		MPI_Bcast(f, M, MPI_FLOAT, j, MPI_COMM_WORLD);
	}
}


/*
 * ��������row_transform
 * ���ܣ������̶����Ӿ���ĸ��н��г����б任
 * ���룺iΪÿ���������ڴ�����Ӿ�����кţ�
 *       jΪ��Ԫ�����ڵĽ��̵ı�ţ�
 *       vΪ��Ԫ��(�ھ���A��)���кţ�
 */
void row_transform(int i, int j, int v)
{
	int k, w;
	/* ��Ų�Ϊj�Ľ����������ж���m�����������б任 */
	if (my_rank != j)
	{
		/* ��������С�������Ԫ�� */
		for (k = 0; k < m; k++)
		{
			for (w = 0; w < M; w++)
				if (w != v)
					a(k, w) = a(k, w) - f[w] * a(k, v);
			a(k, v) = -f[v] * a(k, v);
		}
	}
	/* �����������ݵĽ����������ж�������֮���m-1�����������б任 */
	if (my_rank == j)
	{
		for (k = 0; k < m; k++)
			if (k != i)
			{
				for (w = 0; w < M; w++)
					if (w != v)
						/* �����������ڵĽ����е�����Ԫ�� */
						a(k, w) = a(k, w) - f[w] * a(k, v);
				/* ��������Ԫ�� */
				a(k, v) = -f[v] * a(k, v);
			}
	}
}


/*
 * ��������matrix_inverse
 * ���ܣ��õ�������������󣬽���������B�У�
 */
void matrix_inverse()
{
	int i, j, k;
	/* 0�Ž��̽��Լ��Ӿ���a�еĸ������ݴ���B */
	if (my_rank == 0)
		for (i = 0; i < m; i++)
			for (j = 0; j < M; j++)
				B(i * p, j) = a(i, j);
	/* 0�Ž��̴�����������н����Ӿ���a���õ������任�������B */
	if (my_rank != 0)
	{
		for (i = 0; i < m; i++)
			for (j = 0; j < M; j++)
				MPI_Send(&a(i, j), 1, MPI_FLOAT, 0, my_rank, MPI_COMM_WORLD);
	}
	else
	{
		for (i = 1; i < p; i++)
			for (j = 0; j < m; j++)
				for (k = 0; k < M; k++)
				{
					MPI_Recv(&a(j, k), 1, MPI_FLOAT, i, i, MPI_COMM_WORLD, &status);
					if ((j * p + i) < M)
						B((j * p + i), k) = a(j, k);
				}
	}
}

/*
 * ��������print_result
 * ���ܣ������������ʱ��ͳ�ƣ�
 */
void print_result()
{
	FILE* fdOut;
	int i, j;
	/* 0�Ž��̽�������д��Ŀ���ļ� */
	if (my_rank == 0)
	{
		fdOut = fopen("dataOut.txt", "w");
		fprintf(fdOut, "Input of file \"dataIn.txt\"\n");
		fprintf(fdOut, "%d\t%d\n", M, M);
		for (i = 0; i < M; i++)
		{
			for (j = 0; j < M; j++)
				fprintf(fdOut, "%f\t", A(i, j));
			fprintf(fdOut, "\n");
		}
		fprintf(fdOut, "\nOutput of Matrix A's inversion\n");
		for (i = 0; i < M; i++)
		{
			for (j = 0; j < M; j++)
				fprintf(fdOut, "%f\t", B(i, j));
			fprintf(fdOut, "\n");
		}
		/* 0�Ž��̽�ʱ��ͳ��д��Ŀ���ļ� */
		fprintf(fdOut, "\n");
		fprintf(fdOut, "Whole running time    = %f seconds\n", time2 - starttime);
		fprintf(fdOut, "Distribute data time  = %f seconds\n", time1 - starttime);
		fprintf(fdOut, "Parallel compute time = %f seconds\n", time2 - time1);
		fprintf(fdOut, "Parallel Process number = %d\n", p);
		fclose(fdOut);
	}
}

/*
 * ������: main
 * ���ܣ��������̵��������������������룬���㲢������
 * ���룺argcΪ�����в���������
 *       argvΪÿ�������в�����ɵ��ַ������顣
 * ���������0�����������������
 */
int main(int argc, char** argv)
{
	int i, j, group_size;
	int v;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &group_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	/* pΪ������ */
	p = group_size;
	matrix_init();
	/* mΪÿ��������Ҫ������Ӿ������� */
	m = M / p;
	if (M % p != 0) m++;
	matrix_partition();
	/* time1��¼���㿪ʼ��ʱ�� */
	time1 = MPI_Wtime();
	/* ��������(��ǰ���Ϊj��)�����㲥�Լ��ĵ�i����Ϊ����Ԫ�� */
	for (i = 0; i < m; i++)
		for (j = 0; j < p; j++)
		{
			v = i * p + j;
			if (v < M)
			{
				broadcast_j(i, j, v);
				row_transform(i, j, v);
			}
		}
	matrix_inverse();
	/* time2Ϊ���������ʱ�� */
	time2 = MPI_Wtime();
	print_result();
	MPI_Finalize();
	Environment_Finalize(a, f);
	return(0);
	free(A);
	free(B);
}
