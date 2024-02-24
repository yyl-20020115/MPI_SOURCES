#include "stdio.h"
#include "stdlib.h"
#include "mpi.h"

#define intsize sizeof(int)
#define floatsize sizeof(float)
#define charsize sizeof(char)
#define A(x,y) A[x*K+y]
#define B(x,y) B[x*N+y]
#define C(x,y) C[x*N+y]
#define a(x,y) a[x*K+y]
#define b(x,y) b[x*n+y]
#define buffer(x,y) buffer[x*n+y] /* �˺������򻯶Ա��Ϊ�����Ĵ������ڵĻ���ռ�ķ��� */
#define c(l,x,y) c[x*N+y+l*n]

float* a, * b, * c, * buffer;
int s;
float* A, * B, * C;            /* A[M,K],B[P,N].��ȷ�������KӦ�õ���P,�����޷����о������ */
int M, N, K, P;
int m, n;
int myid;
int p;                     /* ���湤��վ��Ⱥ�д�������Ŀ��Ҳ��ͨ���Ӵ�С */
FILE* dataFile;            /* ���ڶ�ȡ�����ļ����ݺͽ����������������ļ�����ʱ�ļ�ָ�� */
MPI_Status status;
double time1;
double starttime, endtime;

/*
 * ������: readData
 * ���ܣ�  �˺�����rankIDΪ0�Ľ��̵��ã������dataIn.txt�ļ��ж���
 *         A[M,K],B[P,N]������˾�������ݣ���Ϊ�������C[M,N]����ռ䡣
 *         ����C[N,N]=A[M,K]*B[P,N]
 * ���룺  ��
 * ����ֵ����
 */
void readData()
{
	int i, j;
	starttime = MPI_Wtime();

	dataFile = fopen("dataIn.txt", "r");
	int ret = fscanf(dataFile, "%d%d", &M, &K);              /* ��ȡ����A���У�����M,K */
	A = (float*)malloc(floatsize * M * K);             /* Ϊ����A����ռ� */
	for (i = 0; i < M; i++)                        /* �������A�ĸ�Ԫ�� */
	{
		for (j = 0; j < K; j++)
		{
			int ret = fscanf(dataFile, "%f", A + i * K + j);
		}
	}

	ret = fscanf(dataFile, "%d%d", &P, &N);              /* ��ȡ����B���У�����P,N */
	if (K != P)                                     /* KӦ�õ���P,��������޷���� */
	{
		printf("the input is wrong\n");
		exit(1);
	}
	B = (float*)malloc(floatsize * K * N);             /* Ϊ����B����ռ� */
	if (B == 0) return;
	for (i = 0; i < K; i++)                        /* ���ļ��ж������B�ĸ�Ԫ�� */
	{
		for (j = 0; j < N; j++)
		{
			int ret = fscanf(dataFile, "%f", B + i * N + j);
		}
	}
	fclose(dataFile);

	printf("Input of file \"dataIn.txt\"\n");
	printf("%d\t %d\n", M, K);                     /* ���A�����ά�� */
	for (i = 0; i < M; i++)                              /* ���A��������� */
	{
		for (j = 0; j < K; j++) printf("%f\t", A(i, j));
		printf("\n");
	}
	printf("%d\t %d\n", K, N);                     /* ���B�����ά�� */
	for (i = 0; i < K; i++)                              /* ���B��������� */
	{
		for (j = 0; j < N; j++) printf("%f\t", B(i, j));
		printf("\n");
	}

	C = (float*)malloc(floatsize * M * N);             /* Ϊ�������C[M,N]����ռ� */

}

/*
 * ������: gcd
 * ���ܣ�  �˺��������������������Ĳ�����group_size���������
 * ���룺  M,N:Ҫ�������������������
 *         group_size�������ӱ���С�ڴ˲������˲��������û�ָ����ͨ���Ӵ�С
 * ����ֵ��M��N�Ĳ�����group_size���������
 */
int gcd(int M, int N, int group_size)
{
	int i;
	for (i = M; i > 0; i--)
	{
		if ((M % i == 0) && (N % i == 0) && (i <= group_size))
			return i;

	}
	return 1;
}


/*
 * ������: printResult
 * ���ܣ��˺�����rankIDΪ0�Ľ��̵��ã�������A,B,C�����ӡ������û���
 *       ��������ڷַ����ݺͲ��м����ʱ��
 * ���룺��
 * ����ֵ����
 */
void printResult()
{
	int i, j;
	printf("\nOutput of Matrix C = AB\n");
	for (i = 0; i < M; i++)                              /* ���C����Ľ������ */
	{
		for (j = 0; j < N; j++) printf("%f\t", C(i, j));
		printf("\n");
	}

	endtime = MPI_Wtime();
	printf("\n");

	printf("Whole running time    = %f seconds\n", endtime - starttime);
	printf("Distribute data time  = %f seconds\n", time1 - starttime);
	printf("Parallel compute time = %f seconds\n", endtime - time1);

}

/*
 * ������: main
 * ���ܣ������������
 * ���룺argcΪ�����в���������
 *       argvΪÿ�������в�����ɵ��ַ������顣
 * ���������0���������������������ֵ�����������
 */
int main(int argc, char** argv)
{
	int i, j, k, l, group_size, mp1, mm1;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &group_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	p = group_size;

	//����һ�γ������dataIn.txt�ļ��ж���A[M,K],B[P,N]������˾�������ݣ�
	//��Ϊ�������C[M,N]����ռ䡣C[N,N]=A[M,K]*B[P,N]
	//ע����γ���ֻ�б��Ϊ0�Ĵ�������ִ�д˲�����
	if (myid == 0)
	{
		readData();
	}

	if (myid == 0)                                  /* �ɱ��Ϊ0�Ľ��̽�A,B�����������ά��M,K,N���͸������������� */
		for (i = 1; i < p; i++)
		{
			MPI_Send(&M, 1, MPI_INT, i, i, MPI_COMM_WORLD);
			MPI_Send(&K, 1, MPI_INT, i, i, MPI_COMM_WORLD);
			MPI_Send(&N, 1, MPI_INT, i, i, MPI_COMM_WORLD);
		}
	else                                          /* ��ŷ�0�Ľ��̸������A,B�����������ά��M,K,N */
	{
		MPI_Recv(&M, 1, MPI_INT, 0, myid, MPI_COMM_WORLD, &status);
		MPI_Recv(&K, 1, MPI_INT, 0, myid, MPI_COMM_WORLD, &status);
		MPI_Recv(&N, 1, MPI_INT, 0, myid, MPI_COMM_WORLD, &status);
	}

	p = gcd(M, N, group_size);
	m = M / p;                                        /* m���������зֿ��ÿ������� */
	n = N / p;                                        /* m���������зֿ��ÿ������� */

	if (myid < p)
	{

		a = (float*)malloc(floatsize * m * K);         /* a[m,K]�����洢��������ӵ�еľ���A���п� */
		b = (float*)malloc(floatsize * K * n);         /* b[K,n]�����洢��ʱ������ӵ�еľ���B���п� */
		c = (float*)malloc(floatsize * m * N);         /* c[m,N]�����洢������������p-1�εõ����н�� */

		if (myid % 2 != 0)                            /* Ϊ���Ϊ�����Ĵ��������䷢�ͻ���ռ� */
			buffer = (float*)malloc(K * n * floatsize);

		if (a == NULL || b == NULL || c == NULL)            /* �������ռ�������ӡ������Ϣ */
			printf("Allocate space for a,b or c fail!");

		if (myid == 0)                              /* ���Ϊ0�Ĵ�������Ӧ����ӵ�еľ���A,B��Ԫ�ض����Լ���a,b�� */
		{
			for (i = 0; i < m; i++)
				for (j = 0; j < K; j++)
					a(i, j) = A(i, j);

			for (i = 0; i < K; i++)
				for (j = 0; j < n; j++)
					b(i, j) = B(i, j);
		}

		if (myid == 0)                              /* ���Ϊ0�Ĵ������������������ĳ�ʼ���ݷֱ𷢸��������� */
		{
			for (i = 1; i < p; i++)
			{
				MPI_Send(&A(m * i, 0), K * m, MPI_FLOAT, i, i, MPI_COMM_WORLD);

				for (j = 0; j < K; j++)
					MPI_Send(&B(j, n * i), n, MPI_FLOAT, i, i, MPI_COMM_WORLD);
			}

			free(A);
			free(B);                              /* ���ˣ�A,B������������Ѿ���ȫ����ɢ�������������ͷ�A,B��ռ�ռ� */
		}
		else                                      /* ��ŷ�0�Ĵ�������0���������ܸ��Եĳ�ʼ�������� */
		{
			MPI_Recv(a, K * m, MPI_FLOAT, 0, myid, MPI_COMM_WORLD, &status);

			for (j = 0; j < K; j++)
				MPI_Recv(&b(j, 0), n, MPI_FLOAT, 0, myid, MPI_COMM_WORLD, &status);
		}

		if (myid == 0)
			time1 = MPI_Wtime();                    /* ���Ϊ0�Ĵ�������¼��ʼ������˼����ʱ�� */

		for (i = 0; i < p; i++)                         /* һ������p�ּ��� */
		{
			l = (i + myid) % p;

			for (k = 0; k < m; k++)
				for (j = 0; j < n; j++)
					for (c(l, k, j) = 0, s = 0; s < K; s++)
						c(l, k, j) += a(k, s) * b(s, j);

			mm1 = (p + myid - 1) % p;                     /* ���㱾���̵�ǰһ�����̵ı�� */
			mp1 = (myid + 1) % p;                       /* ���㱾���̵ĺ�һ�����̵ı�� */

			if (i != p - 1)
			{
				if (myid % 2 == 0)                     /* ż���Ŵ������ȷ��ͺ���� */
				{
					MPI_Send(b, K * n, MPI_FLOAT, mm1, mm1, MPI_COMM_WORLD);
					MPI_Recv(b, K * n, MPI_FLOAT, mp1, myid, MPI_COMM_WORLD, &status);
				}
				else                              /*�����Ŵ������Ƚ�B���п���ڻ�����buffer�У�Ȼ����ձ����������
													�����������͵�B���п飬����ٽ���������ԭ����B���п鷢�͸����
													����ǰ��Ĵ����� */
				{
					for (k = 0; k < K; k++)
						for (j = 0; j < n; j++)
							buffer(k, j) = b(k, j);
					MPI_Recv(b, K * n, MPI_FLOAT, mp1, myid, MPI_COMM_WORLD, &status);
					MPI_Send(buffer, K * n, MPI_FLOAT, mm1, mm1, MPI_COMM_WORLD);
				}
			}
		}
		if (myid == 0)                              /* ���Ϊ0�Ľ���ֱ�ӽ����������浽�������C�� */
			for (i = 0; i < m; i++)
				for (j = 0; j < N; j++)
					C(i, j) = *(c + i * N + j);

		if (myid != 0)                              /* ��ŷ�0�Ľ�����Ҫ�Ѽ��������͵����Ϊ0�Ĵ�������ȥ */
			MPI_Send(c, m * N, MPI_FLOAT, 0, myid, MPI_COMM_WORLD);
		else                                      /* ���Ϊ0�Ľ��̸�������������̵ļ����������浽�������C�� */
		{
			for (k = 1; k < p; k++)
			{
				MPI_Recv(c, m * N, MPI_FLOAT, k, k, MPI_COMM_WORLD, &status);

				for (i = 0; i < m; i++)
					for (j = 0; j < N; j++)
						C((k * m + i), j) = *(c + i * N + j);
			}
		}

		if (myid == 0)       /* 0�Ŵ���������A,B,C�����ӡ������û�����������ڷַ����ݺͲ��м����ʱ�� */
			printResult();
	}

	MPI_Finalize();

	if (myid < p)            /* �ͷ�������ʱ����ռ� */
	{
		free(a);
		free(b);
		free(c);
		if (myid == 0)       /* ֻ��0�Ž���Ҫ�ͷ�C */
			free(C);
		if (myid % 2 != 0)     /* ֻ�������Ž���Ҫ�ͷ�buffer */
			free(buffer);
	}

	return (0);
}
