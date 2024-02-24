#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

/* ȫ�ֱ������� */
float** A, ** B, ** C;              /* �ܾ���,C = A * B */
float* a, * b, * c, * tmp_a, * tmp_b; /* a��b��c��ֿ飬tmp_a��tmp_b������ */
int dg, dl, dl2, p, sp;            /* dg:�ܾ���ά��;dl:�����ά��;dl2=dl*dl;p:����������;sp��sqrt(p) */
int my_rank, my_row, my_col;      /* my_rank:������ID;(my_row,my_col):�������߼��������� */
MPI_Status status;

/*
 *������: get_index
 *���ܣ��������߼�����������rank�ŵ�ת��
 *���룺���ꡢ�߼�����ά��
 *�����rank��
 */
int get_index(int row, int col, int sp)
{
	return ((row + sp) % sp) * sp + (col + sp) % sp;
}

/*
 *��������random_A_B
 *���ܣ�������ɾ���A��B
 */
void random_A_B()
{
	int i, j;

	srand((unsigned int)time(NULL));     /*�����������*/

	/*�������A,B,����ʼ��C*/
	for (i = 0; i < dg; i++)
		for (j = 0; j < dg; j++)
		{
			A[i][j] = rand();
			B[i][j] = rand();
			C[i][j] = 0.0;
		}
}

/* ��������scatter_A_B
 * ���ܣ�rankΪ0�Ĵ���������������������A��B�������ؿ�
 */
void scatter_A_B()
{
	int i, j, k, l;
	int p_imin, p_imax, p_jmin, p_jmax;

	for (k = 0; k < p; k++)

	{
		/*������Ӧ���������ֵõľ�������ܾ����е����귶Χ*/
		p_jmin = (k % sp) * dl;
		p_jmax = (k % sp + 1) * dl - 1;
		p_imin = (k - (k % sp)) / sp * dl;
		p_imax = ((k - (k % sp)) / sp + 1) * dl - 1;
		l = 0;

		/*rank=0�Ĵ�������A,B�е���Ӧ�鿽��tmp_a,tmp_b��׼������������������*/
		for (i = p_imin; i <= p_imax; i++)
		{
			for (j = p_jmin; j <= p_jmax; j++)
			{
				tmp_a[l] = A[i][j];
				tmp_b[l] = B[i][j];
				l++;
			}
		}

		/*rank=0�Ĵ�����ֱ�ӽ��Լ���Ӧ�ľ�����tmp_a,tmp_b����a,b*/
		if (k == 0)
		{
			memcpy(a, tmp_a, dl2 * sizeof(float));
			memcpy(b, tmp_b, dl2 * sizeof(float));
		}
		else   /*rank=0�Ĵ���������������������tmp_a,tmp_b����صľ����*/
		{
			MPI_Send(tmp_a, dl2, MPI_FLOAT, k, 1, MPI_COMM_WORLD);
			MPI_Send(tmp_b, dl2, MPI_FLOAT, k, 2, MPI_COMM_WORLD);
		}
	}
}

/*
 *������:init_alignment
 *����:����A��B��ʼ��׼
 */
void init_alignment()
{
	/*��A������Ϊ(i,j)�ķֿ�A(i,j)����ѭ���ƶ�i��*/
	MPI_Sendrecv(a, dl2, MPI_FLOAT, get_index(my_row, my_col - my_row, sp), 1,
		tmp_a, dl2, MPI_FLOAT, get_index(my_row, my_col + my_row, sp), 1, MPI_COMM_WORLD, &status);
	memcpy(a, tmp_a, dl2 * sizeof(float));

	/*��B������Ϊ(i,j)�ķֿ�B(i,j)����ѭ���ƶ�j��*/
	MPI_Sendrecv(b, dl2, MPI_FLOAT, get_index(my_row - my_col, my_col, sp), 1,
		tmp_b, dl2, MPI_FLOAT, get_index(my_row + my_col, my_col, sp), 1, MPI_COMM_WORLD, &status);
	memcpy(b, tmp_b, dl2 * sizeof(float));
}

/*
 *��������main_shift
 *���ܣ��ֿ�������ƺ����ƣ�������ֿ�c
 */
void main_shift()
{
	int i, j, k, l;

	for (l = 0; l < sp; l++)
	{
		/*�������ˣ�c+=a*b */
		for (i = 0; i < dl; i++)
			for (j = 0; j < dl; j++)
				for (k = 0; k < dl; k++)
					c[i * dl + j] += a[i * dl + k] * b[k * dl + j];

		/* ���ֿ�a����1λ */
		MPI_Send(a, dl2, MPI_FLOAT, get_index(my_row, my_col - 1, sp), 1, MPI_COMM_WORLD);
		MPI_Recv(a, dl2, MPI_FLOAT, get_index(my_row, my_col + 1, sp), 1, MPI_COMM_WORLD, &status);

		/* ���ֿ�b����1λ */
		MPI_Send(b, dl2, MPI_FLOAT, get_index(my_row - 1, my_col, sp), 1, MPI_COMM_WORLD);
		MPI_Recv(b, dl2, MPI_FLOAT, get_index(my_row + 1, my_col, sp), 1, MPI_COMM_WORLD, &status);
	}
}

/*
 *��������collect_c
 *���ܣ�rankΪ0�Ĵ����������ദ�����ռ��ֿ����c
 */
void collect_C()
{
	int i, j, i2, j2, k;
	int p_imin, p_imax, p_jmin, p_jmax; /* �ֿ�������ܾ����ж���߽�ֵ */

	/* ��rankΪ0�Ĵ������зֿ����c��������ܾ���C��Ӧλ�� */
	for (i = 0; i < dl; i++)
		for (j = 0; j < dl; j++)
			C[i][j] = c[i * dl + j];

	for (k = 1; k < p; k++)
	{
		/*��rankΪ0�Ĵ�����������������������Ӧ�ķֿ�c*/
		MPI_Recv(c, dl2, MPI_FLOAT, k, 1, MPI_COMM_WORLD, &status);

		p_jmin = (k % sp) * dl;
		p_jmax = (k % sp + 1) * dl - 1;
		p_imin = (k - (k % sp)) / sp * dl;
		p_imax = ((k - (k % sp)) / sp + 1) * dl - 1;

		i2 = 0;
		/*�����յ���c����C�е���Ӧλ��,�Ӷ������C*/
		for (i = p_imin; i <= p_imax; i++)
		{
			j2 = 0;
			for (j = p_jmin; j <= p_jmax; j++)
			{
				C[i][j] = c[i2 * dl + j2];
				j2++;
			}
			i2++;
		}
	}
}

/*��������print
 *���ܣ���ӡ����
 *���룺ָ�����ָ���ָ�룬�ַ���
 */
void print(float** m, char* str)
{
	int i, j;
	printf("%s", str);
	/*��ӡ����m*/
	for (i = 0; i < dg; i++)
	{
		for (j = 0; j < dg; j++)
			printf("%15.0f    ", m[i][j]);
		printf("\n");
	}
	printf("\n");
}

/*
 *��������main
 *���ܣ������̣�Cannon�㷨���������
 *���룺argcΪ�����в���������argvΪÿ�������в�����ɵ��ַ�������
 */
int main(int argc, char* argv[])
{
	int i;

	MPI_Init(&argc, &argv);                  /* ����MPI���� */
	MPI_Comm_size(MPI_COMM_WORLD, &p);       /* ȷ������������ */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); /* ȷ�����ԵĴ�������ʶ�� */

	sp = sqrt(p);

	/* ȷ����������������ȫƽ�����������ӡ������Ϣ�������˳� */
	if (sp * sp != p)
	{
		if (my_rank == 0)
			printf("Number of processors is not a quadratic number!\n");
		MPI_Finalize();
		exit(1);
	}

	if (argc != 2)
	{
		if (my_rank == 0)
			printf("usage: mpirun -np ProcNum cannon MatrixDimension\n");
		MPI_Finalize();
		exit(1);
	}

	dg = atoi(argv[1]);    /* �ܾ���ά�� */
	dl = dg / sp;          /* ����ֿ����ά�� */
	dl2 = dl * dl;

	/* ���㴦�������߼������е����� */
	my_col = my_rank % sp;
	my_row = (my_rank - my_col) / sp;

	/* Ϊa��b��c����ռ� */
	a = (float*)malloc(dl2 * sizeof(float));
	b = (float*)malloc(dl2 * sizeof(float));
	c = (float*)malloc(dl2 * sizeof(float));

	/* ��ʼ��c */
	for (i = 0; i < dl2; i++)
		c[i] = 0.0;

	/* Ϊtmp_a��tmp_b����ռ� */
	tmp_a = (float*)malloc(dl2 * sizeof(float));
	tmp_b = (float*)malloc(dl2 * sizeof(float));

	if (my_rank == 0)
	{
		/* rankΪ0�Ĵ�����ΪA��B��C����ռ� */
		A = (float**)malloc(dg * sizeof(float*));
		B = (float**)malloc(dg * sizeof(float*));
		C = (float**)malloc(dg * sizeof(float*));

		for (i = 0; i < dg; i++)
		{
			A[i] = (float*)malloc(dg * sizeof(float));
			B[i] = (float*)malloc(dg * sizeof(float));
			C[i] = (float*)malloc(dg * sizeof(float));
		}
		random_A_B();     /* rankΪ0�Ĵ��������������A��B���� */
		scatter_A_B();    /* rankΪ0�Ĵ���������������������A��B�������ؿ� */
	}
	else               /* rank��Ϊ0�Ĵ�������������rankΪ0�Ĵ���������Ӧ����ֿ� */
	{
		MPI_Recv(a, dl2, MPI_FLOAT, 0, 1, MPI_COMM_WORLD, &status);
		MPI_Recv(b, dl2, MPI_FLOAT, 0, 2, MPI_COMM_WORLD, &status);
	}

	init_alignment();    /* A��B����ĳ�ʼ��׼ */

	main_shift();        /* �ֿ�������ơ�����, cannon�㷨�������� */

	if (my_rank == 0)
	{
		collect_C();       /* rankΪ0�Ĵ����������ദ�����ռ��ֿ����c */
		print(A, "random matrix A : \n");  /* ��ӡ����A */
		print(B, "random matrix B : \n");  /* ��ӡ����B */
		print(C, "Matrix C = A * B : \n");     /* ��ӡ����C */

	}
	else
	{
		MPI_Send(c, dl2, MPI_FLOAT, 0, 1, MPI_COMM_WORLD); /* rank��Ϊ0�Ĵ�������rankΪ0�Ĵ��������;����c */
	}

	MPI_Barrier(MPI_COMM_WORLD);        /* ͬ�����д����� */
	MPI_Finalize();                     /* ����MPI���� */

	return 0;
}
