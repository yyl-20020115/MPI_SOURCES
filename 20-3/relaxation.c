#include "stdio.h"
#include "stdlib.h"
#include "mpi.h"
#include "math.h"
#define E 0.0001
#define a(x,y) a[x*size+y]
#define b(x) b[x]
#define v(x) v[x]
#define A(x,y) A[x*size+y]
#define B(x) B[x]
#define V(x) V[x]
#define intsize sizeof(int)
#define floatsize sizeof(float)
#define charsize sizeof(char)

int size, N;                                       /*size Ϊϵ������Ĵ�С��NΪsize��1*/
int m;
float* B;
float* A;
float* V;                                         /* store the value x */
double starttime;
double time1;
double time2;
int my_rank;
int p;
MPI_Status status;
FILE* fdA, * fdB, * fdB1;

void Environment_Finalize(float* a, float* b, float* v)
{
	free(a);
	free(b);
	free(v);
}


/*
 * ������: main
 * ���룺argcΪ�����в���������
 *       argvΪÿ�������в�����ɵ��ַ������顣
 * ���������0���������������
 */
int main(int argc, char** argv)
{
	int i, j, my_rank, group_size;
	int k;
	float* sum;
	float* b;
	float* v;
	float* a;
	float* differ;
	float temp, w;
	int iteration, total, loop;
	int r, q;
	loop = 0;
	total = 0;
	w = 0.9;

	MPI_Init(&argc, &argv);                        /* �������� */
	MPI_Comm_size(MPI_COMM_WORLD, &group_size);    /* �ҽ����� */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);       /* ���Լ���id */
	p = group_size;

	if (my_rank == 0)
	{
		starttime = MPI_Wtime();                    /* ��ʼ��ʱ */

		fdA = fopen("dataIn.txt", "r");              /* ���������ϵ������A��b */
		fscanf(fdA, "%d %d", &size, &N);

		/*size Ҫ�� size=N+1*/
		if (size != N - 1)
		{
			printf("the input is wrong\n");
			exit(1);
		}

		/*����ϵ������A����b����x�Ŀռ�*/
		A = (float*)malloc(floatsize * size * size);
		B = (float*)malloc(floatsize * size);
		V = (float*)malloc(floatsize * size);

		/*���A��b��x*/
		for (i = 0; i < size; i++)
		{
			for (j = 0; j < size; j++)
			{
				int ret = fscanf(fdA, "%f", A + i * size + j);
			}
			int ret = fscanf(fdA, "%f", B + i);
		}

		for (i = 0; i < size; i++)
		{
			int ret = fscanf(fdA, "%f", V + i);
		}

		fclose(fdA);

		/*��ӡA��b��x*/
		printf("Input of file \"dataIn.txt\"\n");
		printf("%d\t%d\n", size, N);
		for (i = 0; i < size; i++)
		{
			for (j = 0; j < size; j++) printf("%f\t", A(i, j));
			printf("%f\n", B(i));
		}
		printf("\n");
		for (i = 0; i < size; i++)
		{
			printf("%f\t", V(i));
		}
		printf("\n\n");
		printf("\nOutput of result\n");

	}                                             /* end of if (my_rank==0) */

	MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);  /* �㲥size�����н���*/

	m = size / p; if (size % p != 0) m++;                  /* mΪÿ�����̱��������� */

	/*ÿ�����̷���ϵ���ȵĿռ�*/
	v = (float*)malloc(floatsize * size);
	a = (float*)malloc(floatsize * m * size);
	b = (float*)malloc(floatsize * m);
	sum = (float*)malloc(floatsize * m);

	if (a == NULL || b == NULL || v == NULL)
		printf("allocate space  fail!");

	if (my_rank == 0)
	{
		for (i = 0; i < size; i++)
			v(i) = V(i);
	}

	MPI_Bcast(v, size, MPI_FLOAT, 0, MPI_COMM_WORLD); /* ����ʼx�㲥�����н��� */

	/* �����̷���A,b,��x��ֵ*/
	if (my_rank == 0)
	{
		for (i = 0; i < m; i++)
			for (j = 0; j < size; j++)
				a(i, j) = A(i, j);

		for (i = 0; i < m; i++)
			b(i) = B(i);
	}

	/* ��A,b��x��ֵ���䵽���н���,ÿ�����̷���m�� */
	if (my_rank == 0)                               /* ����0�����ݹ㲥��ȥ */
	{
		for (i = 1; i < p; i++)
		{
			MPI_Send(&(A(m * i, 0)), m * size, MPI_FLOAT, i, i, MPI_COMM_WORLD);
			MPI_Send(&(B(m * i)), m, MPI_FLOAT, i, i, MPI_COMM_WORLD);
		}
		free(A); free(B); free(V);
	}
	else                                          /* �������̽�������A,b��x�ĳ�ֵ*/
	{
		MPI_Recv(a, m * size, MPI_FLOAT, 0, my_rank, MPI_COMM_WORLD, &status);
		MPI_Recv(b, m, MPI_FLOAT, 0, my_rank, MPI_COMM_WORLD, &status);
	}

	time1 = MPI_Wtime();                            /* ȡ��ʼ����ʱ�� */

	/* ��ÿ�����Խ����ұߵ�Ԫ�غ� */
	for (i = 0; i < m; i++)                              /* ��ʼ���� */
	{
		sum[i] = 0.0;
		for (j = 0; j < size; j++)
		{
			if (j > (my_rank * m + i))
				sum[i] = sum[i] + a(i, j) * v(j);
		}
	}

	while (total < size)                            /* totalΪ������С��E��x���������������н⼴����x�����ĵ������С��Eʱ��ֹͣ���� */
	{
		iteration = 0;                              /* iterationΪ����process�е�����С��E��x�������� */
		total = 0;

		for (j = 0; j < size; j++)                       /* �����Ե�0,1, ..., n-1��Ϊ���� */

		{
			r = j % m; q = j / m;

			if (my_rank == q)                       /* �������ڵĴ����� */
			{
				temp = v(my_rank * m + r);

				/*sum[j]�ۼ��ϱ���������������µ�x���Щ������к�С�ڵ�ǰ������к�*/
				for (i = 0; i < r; i++)
					sum[r] = sum[r] + a(r, my_rank * m + i) * v(my_rank * m + i);

				/*�����µ�xֵ*/
				v(my_rank * m + r) = (1 - w) * temp + w * (b(r) - sum[r]) / a(r, my_rank * m + r);

				if (fabs(v(my_rank * m + r) - temp) < E)  /*�¾�ֵ��С��E���ۼ�iteration*/
					iteration++;

				/*�㲥�µ�xֵ*/
				MPI_Bcast(&v(my_rank * m + r), 1, MPI_FLOAT, my_rank, MPI_COMM_WORLD);

				sum[r] = 0.0;                       /*��ǰ�д�����Ϻ�sum���㣬Ϊ��һ�μ�����׼��*/
				for (i = 0; i < r; i++)                  /*��process��ǰ���е�sum�ۼ���xֵ�Ϊ��һ�μ����ۼӶԽ�Ԫ����ߵ���*/
					sum[i] = sum[i] + a(i, my_rank * m + r) * v(my_rank * m + r);
			}
			else                                  /*���������ڵĴ�����*/
			{
				/*���չ㲥������xֵ*/
				MPI_Bcast(&v(q * m + r), 1, MPI_FLOAT, q, MPI_COMM_WORLD);
				for (i = 0; i < m; i++)                  /*�ۼ���xֵ��Ӧ����*/
					sum[i] = sum[i] + a(i, q * m + r) * v(q * m + r);
			}
		}

		/* �����д�������iterationֵ�ĺ�total���㲥�����д������� */
		MPI_Allreduce(&iteration, &total, 1, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);

		loop++;

		if (my_rank == 0)                           /* ��ӡÿ�ε�����totalֵ */
			printf("%-2d th total vaule = %d\n", loop, total);
	}                                             /* while */

	time2 = MPI_Wtime();                            /* ȡ����ʱ�� */

	if (my_rank == 0)                               /* ��ӡ��xֵ�Լ����С�����ͼ���ʱ�� */
	{

		for (i = 0; i < size; i++) printf("x[%d] = %f\n", i, v(i));
		printf("\n");
		printf("Iteration num = %d\n", loop);
		printf("Whole running time    = %f seconds\n", time2 - starttime);
		printf("Distribute data time  = %f seconds\n", time1 - starttime);
		printf("Parallel compute time = %f seconds\n", time2 - time1);
	}

	MPI_Barrier(MPI_COMM_WORLD);                  /* ͬ�����н��� */
	MPI_Finalize();                               /* �������� */

	Environment_Finalize(a, b, v);
	return (0);
}
