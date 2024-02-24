#include "math.h"
#include "stdio.h"
#include "stdlib.h"
#include "mpi.h"
#define INTSIZE sizeof(int)
#define CHARSIZE sizeof(char)
#define M(i,j) M[i*n*p+j]
#define a(i,j) a[i*n*p+j]
#define b(i,j) b[i*n+j]
#define c(l,i,j) c[i*n*p+l*n+j]
int* a, * b, * c, * d;
/**������ڽӾ���**/
int* M;
/**��������Ŀ**/
int p;
/**������Ŀ**/
int N;
/**����õ����������Ķ�����Ŀ**/
int n;
/**���������**/
int myid;
MPI_Status status;

/*
 * ������: readmatrix
 * ����: �����ڽӾ���
 * ���룺
 * ���������0���������������;����������
 */
int readmatrix()
{
	int i, j;
	printf("Input the size of matrix:");
	int ret = scanf("%d", &N);
	n = N / p;
	if (N % p != 0)n++;
	/*��M�������ռ�*/
	M = (int*)malloc(INTSIZE * n * p * n * p);
	if (!M)
	{
		error("failed to allocate space for M");
	}/*if*/
	printf("Input the matrix:\n");
	/*�������*/
	for (i = 0; i < N; i++)
	{
		for (j = 0; j < N; j++)
		{
			int ret = scanf("%d", &(M(i, j)));
			if (i == j) M(i, j) = 1;
		}/*for*/
		for (j = N; j < n * p; j++) M(i, j) = 0;
	}/*for*/
	for (i = N; i < n * p; i++)
		for (j = 0; j < n * p; j++)
			if (i == j) M(i, j) = 1;
			else M(i, j) = 0;
	return(0);
} /*readmatrix*/

/*
 * ������: error
 * ����: ��ʾ�����˳�
 * ���룺messageΪҪ��ʾ����Ϣ
 * ������������Ĵ������ź���Ϣ
 *       ����0���������������;����������
 */
int error(message)
char* message;
{
	printf("processor %d:%s\n", myid, message);
	/*�˳�*/
	exit(0);
	return(0);
} /*error*/

/*
 * ������: sendmatrix
 * ����: ��������������ڽӾ�������
 * ���룺
 * ���������0���������������;����������
 */
int sendmatrix()
{
	int i, j;
	for (i = 1; i < p; i++)
	{
		MPI_Send(&(M(n * i, 0)), n * n * p, MPI_INT, i, i, MPI_COMM_WORLD);
		for (j = 0; j < n * p; j++)
			MPI_Send(&(M(j, n * i)), n, MPI_INT, i, i, MPI_COMM_WORLD);
	} /*for*/
	return(0);
} /*sendmatrix*/

/*
 * ������: getmatrix
 * ����: �������������ڽӾ�������
 * ���룺
 * ���������0���������������;����������
 */
int getmatrix()
{
	int i, j;
	if (myid == 0)
	{
		for (i = 0; i < n; i++)
			for (j = 0; j < n * p; j++)
				a(i, j) = M(i, j);
		for (i = 0; i < n * p; i++)
			for (j = 0; j < n; j++)
				b(i, j) = M(i, j);
	}
	else
	{
		MPI_Recv(a, n * n * p, MPI_INT, 0, myid, MPI_COMM_WORLD, &status);
		for (j = 0; j < n * p; j++)
			MPI_Recv(&(b(j, 0)), n, MPI_INT, 0, myid, MPI_COMM_WORLD, &status);
	} /*if*/
	return(0);
}

/*
 * ������: writeback
 * ����: ���о���������̣������������c������
 * ���룺
 * ���������0���������������;����������
 */
int paramul()
{
	int l, i, j, s;
	/**������������ʾǰһ���ͺ�һ����������ʶ**/
	int last, next;
	for (l = 0; l < p; l++)
	{
		for (i = 0; i < n; i++)
			for (j = 0; j < n; j++)
				for (c(((l + myid) % p), i, j) = 0, s = 0; s < n * p; s++)
					if (a(i, s) && b(s, j))
					{
						c(((l + myid) % p), i, j) = 1; break;
					}
		last = (p + myid - 1) % p;
		next = (myid + 1) % p;
		if (l != p - 1)
		{
			if (myid % 2 == 0)
				/**ż���Ŵ�����ֱ�ӽ�������b**/
			{
				MPI_Send(b, n * n * p, MPI_INT, last, last, MPI_COMM_WORLD);
				MPI_Recv(b, n * n * p, MPI_INT, next, myid, MPI_COMM_WORLD, &status);
			}
			else
			{
				/**�����Ŵ�������Ҫ�Ƚ�ԭ���ݱ��棬��ֹ��ʧ**/
				for (i = 0; i < n * n * p; i++)
					d[i] = b[i];
				MPI_Recv(b, n * n * p, MPI_INT, next, myid, MPI_COMM_WORLD, &status);
				MPI_Send(d, n * n * p, MPI_INT, last, last, MPI_COMM_WORLD);
			}
		}
	}/*for*/
	return(0);
}

/*
 * ������: writeback
 * ����: ��c���󱣴�ľ�����˽��д�ش�����0������M
 * ���룺
 * ���������0���������������;����������
 */
int writeback()
{
	int i;
	if (myid == 0)
	{
		for (i = 0; i < n * n * p; i++)
			M(0, i) = c(0, 0, i);
		for (i = 1; i < p; i++)
		{
			MPI_Recv(&(M(i * n, 0)), n * n * p, MPI_INT, i, i, MPI_COMM_WORLD, &status);
		}/*for*/
	}/*for*/
	else
		MPI_Send(c, n * n * p, MPI_INT, 0, myid, MPI_COMM_WORLD);
	return(0);
}

/*
 * ������: output
 * ����: ������
 * ���룺
 * ���������������M
 *       ����0���������������;����������
 */
int output()
{
	int i, j;
	for (i = 0; i < N; i++)
	{
		for (j = 0; j < N; j++)
			printf("%d ", M(i, j));
		printf("\n");
	}
	return(0);
}
/******************** main ********************/
/*
 * ������: main
 * ����: ����MPI���㣻
 *       ȷ���������ͽ��̱�ʶ����
 *       ���������̺ʹӽ��̳�������⴫�ݱհ����⡣
 * ���룺argcΪ�����в���������
 *       argvΪÿ�������в�����ɵ��ַ������顣
 * ���������0���������������;����������
 */
int main(argc, argv)
int argc;
char** argv;
{

	int i, j;
	int group_size;
	int* temp;
	/*��ʼ��*/
	MPI_Init(&argc, &argv);
	/*ȷ���������еĽ��̸���*/
	MPI_Comm_size(MPI_COMM_WORLD, &group_size);
	/*ȷ���Լ��ڹ������еĽ��̱�ʶ��*/
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	p = group_size;
	/**������0������󲢷��ͣ�����(1)**/
	if (myid == 0)
	{
		/*�����ڽӾ���*/
		readmatrix();
		for (i = 1; i < p; i++)
			MPI_Send(&N, 1, MPI_INT, i, i, MPI_COMM_WORLD);
	}
	else
		MPI_Recv(&N, 1, MPI_INT, 0, myid, MPI_COMM_WORLD, &status);
	n = N / p;
	if (N % p != 0)n++;
	/*����洢�ռ�*/
	a = (int*)malloc(INTSIZE * n * n * p);
	b = (int*)malloc(INTSIZE * n * n * p);
	c = (int*)malloc(INTSIZE * n * n * p);
	d = (int*)malloc(INTSIZE * n * n * p);
	/*����ʧ��*/
	if (a == NULL || b == NULL || c == NULL)
		error("failed to allocate space for a,b and c");
	/**logN�ξ����Գˣ�����(3)**/
	for (i = 0; i <= log(N) / log(2); i++)
	{
		if (myid == 0)printf("loop %d:\n", i);
		if (myid == 0) output();
		/**������0������������ͱ�Ҫ���ݣ�����(3.1)(3.2)**/
		if (myid == 0) sendmatrix();
		/**����(3.3)**/
		getmatrix();
		/**������ˣ�����(3.4)**/
		paramul();
		/**���д�أ�����(3.5)**/
		writeback();
	};
	if (myid == 0)printf("loop %d:\n", i);
	if (myid == 0)output();
	/*����MPI����*/
	MPI_Finalize();
	/*�ͷŴ洢�ռ�*/
	free(a);
	free(b);
	free(c);
	free(M);
	return(0);
} /* main */
