#include "stdio.h"
#include "stdlib.h"
#include "mpi.h"
#include "math.h"

#define E 0.0001
#define a(x,y) a[x*m+y]
#define b(x,y) b[x*m+y]
#define A(x,y) A[x*size+y]
#define B(x,y) B[x*size+y]
#define intsize sizeof(int)
#define floatsize sizeof(float)
#define charsize sizeof(char)

int size, N;                                       /* size:�����������;N:����������� */
int m;                                            /* �����ӷ���ĳߴ� */
int t;                                            /* ���̻��ֵķָ��� */
float* A, * B;                                     /* A:����ԭ����;B:����ת�ú�ľ��� */
double starttime;                                 /* ���濪ʼʱ�� */
double time1;                                     /* ����ַ����ݵĽ���ʱ�� */
double time2;                                     /* �������еĽ���ʱ�� */
int my_rank;                                      /* ���浱ǰ���̵Ľ��̺� */
int p;                                            /* ��������� */
MPI_Status status;                                /* ����MPI״̬ */
FILE* fdA;                                        /* �����ļ� */

/* ���н���ǰ,���ñ������ͷ��ڴ�ռ� */
void Environment_Finalize(float* a, float* b)
{
	free(a);
	free(b);
}

int main(int argc, char** argv)
{
	int i, j, k, my_rank, group_size;
	float* a, * b;
	int u, v;
	float temp;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &group_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	p = group_size;

	/* �����������(rank=0�Ľ���),����ж��ļ��Ĳ���,
	   ����ת�õľ�������ڴ�,���浽ȫ�ֱ���A��
	*/
	if (my_rank == 0)
	{
		starttime = MPI_Wtime();
		fdA = fopen("dataIn.txt", "r");
		/* ������������������,�����浽size��N�� */
		int ret = fscanf(fdA, "%d %d", &size, &N);
		/* �ж��Ƿ��Ƿ���,�������,�����˳� */
		if (size != N)
		{
			puts("The input is error!");
			exit(0);
		}
		A = (float*)malloc(floatsize * size * size);
		B = (float*)malloc(floatsize * size * size);
		/* �����������ֵ����,���浽A�� */
		for (i = 0; i < size; i++)
		{
			for (j = 0; j < size; j++) fscanf(fdA, "%f", A + i * size + j);
		}
		fclose(fdA);
	}
	/* �㲥����ĳߴ� */
	MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);

	/* ������̻��ֵ���Ŀ */
	t = (int)sqrt(p);
	if (t > size)
		t = size;
	if (size % t != 0)
		for (;;)
		{
			t--;
			if (size % t == 0)
				break;
		}
	/* ���ʵ�����õĴ��������� */
	p = t * t;
	/* ÿ���ӷ���ĳߴ� */
	m = size / t;

	/* a�����ӷ���,b����ʱ����,��������������������͸���Ľ��̵��ӷ��� */
	a = (float*)malloc(floatsize * m * m);
	b = (float*)malloc(floatsize * m * m);

	if (a == NULL || b == NULL)
		printf("allocate space  fail!");

	/* ��������,����Լ����ӷ���(�����Ͻǵ��ӷ���) */
	if (my_rank == 0)
	{
		for (i = 0; i < m; i++)
			for (j = 0; j < m; j++)
				a(i, j) = A(i, j);
	}

	/* ���������������̷������� */
	if (my_rank == 0)
	{
		for (i = 1; i < p; i++)
		{
			v = i / t;                                /* �ӷ�����к� */
			u = i % t;                                /* �ӷ�����к� */

			for (j = v * m; j < (v + 1) * m; j++)
				for (k = u * m; k < (u + 1) * m; k++)
					b((j % m), (k % m)) = A(j, k);        /* ���ӷ����ݴ���b�� */

			/* ���ӷ����͵���Ӧ�Ľ��� */
			MPI_Send(b, m * m, MPI_FLOAT, i, i, MPI_COMM_WORLD);
		}
	}
	else if (my_rank < p)                           /* ����������,�������̽������� */
		MPI_Recv(a, m * m, MPI_FLOAT, 0, my_rank, MPI_COMM_WORLD, &status);

	time1 = MPI_Wtime();

	/* �������ǵ��ӷ�����д��� */
	if ((my_rank / t) > (my_rank % t) && my_rank < p)
	{
		v = my_rank / t;                              /* �к� */
		u = my_rank % t;                              /* �к� */

		/* �����ӷ���λ����Ӧ������λ�õĽ��� */
		MPI_Send(a, m * m, MPI_FLOAT, (u * t + v), (u * t + v), MPI_COMM_WORLD);
		/* ����Ӧ������λ�õĽ��̽������� */
		MPI_Recv(a, m * m, MPI_FLOAT, (u * t + v), my_rank, MPI_COMM_WORLD, &status);
	}

	/* �������ǵ��ӷ�����д��� */
	if ((my_rank / t) < (my_rank % t) && my_rank < p)
	{
		v = my_rank / t;                              /* �к� */
		u = my_rank % t;                              /* �к� */
		/* ���ӷ���Ԫ�ظ��Ƶ�b */
		for (i = 0; i < m; i++)
			for (j = 0; j < m; j++)
				b(i, j) = a(i, j);

		/* ����Ӧ������λ�õĽ��̽������� */
		MPI_Recv(a, m * m, MPI_FLOAT, (u * t + v), my_rank, MPI_COMM_WORLD, &status);
		/* �ӷ����͵�λ����Ӧ������λ�õĽ��� */
		MPI_Send(b, m * m, MPI_FLOAT, (u * t + v), (u * t + v), MPI_COMM_WORLD);
	}

	/* ��ÿһ���ӷ������ת�� */
	for (i = 1; i < m; i++)
		for (j = 0; j < i; j++)
		{
			temp = a(i, j);
			a(i, j) = a(j, i);
			a(j, i) = temp;
		}

	/* �����̿�ʼ��ת�õĽ���������
	   �Ƚ������̵Ľ����ϵ�B�����Ͻ�
	*/
	if (my_rank == 0)
	{
		for (i = 0; i < m; i++)
			for (j = 0; j < m; j++)
				B(i, j) = a(i, j);
	}
	/* �����̴��������̽��ս��,��ϵ�B����Ӧλ�� */
	if (my_rank == 0)
	{
		for (i = 1; i < p; i++)
		{
			/* ���������̽��ս�� */
			MPI_Recv(a, m * m, MPI_FLOAT, i, i, MPI_COMM_WORLD, &status);

			v = i / t;                                /* ������к� */
			u = i % t;                                /* ������к� */

			for (j = v * m; j < (v + 1) * m; j++)
				for (k = u * m; k < (u + 1) * m; k++)
					B(j, k) = a((j % m), (k % m));        /* �����ϵ�B����Ӧλ�� */
		}
	}
	else if (my_rank < p)                            /* �������̷��ͽ���������� */
		MPI_Send(a, m * m, MPI_FLOAT, 0, my_rank, MPI_COMM_WORLD);

	/* �������̴�ӡ������ */
	if (my_rank == 0)
	{
		printf("Input of file \"dataIn.txt\"\n");
		printf("%d\t%d\n", size, size);
		for (i = 0; i < size; i++)
		{
			for (j = 0; j < size; j++) printf("%f\t", A(i, j));
			printf("\n");
		}
		printf("\nOutput of Matrix AT\n");
		for (i = 0; i < size; i++)
		{
			for (j = 0; j < size; j++) printf("%f\t", B(i, j));
			printf("\n");
		}
	}
	time2 = MPI_Wtime();
	/* �������̴�ӡʱ����Ϣ */
	if (my_rank == 0)
	{
		printf("\n");
		printf("Whole running time    = %f seconds\n", time2 - starttime);
		printf("Distribute data time  = %f seconds\n", time1 - starttime);
		printf("Parallel compute time = %f seconds\n", time2 - time1);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
	Environment_Finalize(a, b);
	return(0);
}
