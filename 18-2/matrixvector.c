/*
 * �����ܣ��ó����м������������ĳ˻����������������������ô�״���ֵ��㷨
 * ����˵����matrix[width,width]   �洢���ļ��ж�ȡ�����ڼ���ľ����Ԫ�أ�����Ԫ��Ϊ�����ȸ�����
 *           vec[width]    �洢���ļ��ж�ȡ�����ڼ����������Ԫ�أ�����Ԫ��Ϊ�����ȸ�����
 *           width         �洢���ļ��ж�ȡ��δ֪����Ŀ�ȣ��������ĳ��ȣ�������̬��
 *                                  ����ͷָ��matrix��������ͷָ��vec����ռ䣬      ���ͱ���
 *           col            Ĭ�ϵ�ֵӦ��Ϊwidth��1��������Ǳ���f           ���ͱ���
 *           starttime��finishprepare��finishcompute��finishall
 *                         �����ĸ�ȫ�ֱ���������¼�ĸ�ʱ���                        ˫���ȸ�����
 *           status         MPI������Ϣʱ�õ��ı���      MPI_statuc ����
 *           rom_per_pro    ��¼ÿһ������ʵ����Ҫ���������               ���α���
 *           fin            �����ļ�ָ��
 *           i,j            ����ѭ������������Ϣ����ʱ��tag       ���α���
 *           my_rank        �洢��ǰ���̵ı�ʶ                    ���ͱ���
 *           group_size     �洢��ǰ�ܵĽ��̵�����                ���ͱ���
 *           vecin[width]   ��ÿһ�������������ݴ�vec�ı���       Ԫ��Ϊ�����ȸ�����
 *           ma[rom_per_pro,width]
 *                          ÿһ�������������ݴ汻������ǲ��ֵľ����Ԫ��  Ԫ��Ϊ�����ȸ�����
 *           result[rom_per_pro]
 *                          ÿһ��������������¼�������ı���    Ԫ��Ϊ�����ȸ�����
 *           sum            ����������õ����м����              �����ȸ�����
 *           p              ������������                           ���ͱ���
 *           realp          ʵ���õ��Ĵ���������Ŀ                 ���ͱ���
 */
#include "stdio.h"
#include "stdlib.h"
#include "mpi.h"
#include "math.h"

#define matrix(x,y) matrix[x*width+y]
#define vec(x) vec[x]
#define vecin(x) vecin[x]
#define result(x) result[x]
#define ma(x,y) ma[x*width+y]
#define intsize sizeof(int)
#define floatsize sizeof(float)

int realp, p;
int width, col, row_per_pro;
float* matrix;
float* vec;
double starttime, finishprepare, finishcompute, finishall;
MPI_Status status;
FILE* fin;

/*
 ������: free_all
 ���ܣ��ͷ�ÿ�������ж�̬��������������ռ�
 ���룺����˫���ȸ�����ָ�����ma��result��vecin
 ���������Ϊ��
*/
void free_all(float* ma, float* result, float* vecin)
{
	free(ma);
	free(result);
	free(vecin);
}

/*
 ������: main
 ���ܣ����ļ��ж���һ�������һ���������������ǵĳ˻�������Ļ������������
 ���룺argcΪ�����в�������
	   argvΪÿ�������в�����ɵ��ַ������顣
 ���������0���������������
*/
int main(int argc, char** argv)
{
	int i, j, my_rank, group_size;
	float sum;
	float* ma;
	float* result;
	float* vecin;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &group_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	p = group_size;

	/*
	 �ɽ���0���matrix,vec�Ķ�̬�ռ����
	 ͬʱ�������;�����뵽vec��matrix�Ŀռ���
	*/
	if (my_rank == 0)
	{
		/*
		 ���³�ʼ���е�ʱ���
		 ���ļ�dataIn.txt������width��colֵ
		 ���������ԣ������жϷ���
		*/
		starttime = MPI_Wtime();
		fin = fopen("dataIn.txt", "r");
		int ret = fscanf(fin, "%d %d", &width, &col);
		if (width != (col - 1))
		{
			printf("The input is wrong,check the input matrix width and the total columns\n\n\n\n");
			exit(1);
		}

		/*
		 �ڽ���0��Ϊmatrix��vec����ռ��dataIn.txt�ж�����Ӧ��ֵ
		 matrix ������width*width��С�ĸ������ռ�
		 vec������width���ĸ������ռ�
		*/
		matrix = (float*)malloc(floatsize * width * width);
		vec = (float*)malloc(floatsize * width);

		for (i = 0; i < width; i++)
		{
			for (j = 0; j < width; j++)
			{
				int ret = fscanf(fin, "%f", matrix + i * width + j);
			}
			int ret = fscanf(fin, "%f", vec + i);
		}

		/* �����ļ�dataIn.txt�ж���Ľ����ʾ����Ļ�� */
		fclose(fin);
		printf("The matrix and the vector has been read from file \"dataIn.txt\" is: %d width\n ", width);
		printf("The element of the matrix is as follows:\n");
		for (i = 0; i < width; i++)
		{
			printf("row %d\t", i);
			for (j = 0; j < width; j++)
				printf("%f\t", matrix(i, j));
			printf("\n");
		}
		printf("the element of the vector is as follows:\n");
		for (i = 0; i < width; i++)
		{
			printf("row %d is %f\n", i, vec(i));
		}

	}

	/* ����0�Ѿ��õ�widthֵ�����������������㲥���������� */
	MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);

	/*
	 ����ÿһ������Ҫ�����������ע�ⲻ������ʱҪʹ��row_per_pro��ֵ��1
	 ����ʵ����Ҫ�����Ľ������������ò��ϵĽ��̽�����ִ��
	 ��ʵ���õ��Ľ�������ÿ��������Ҫ����������������Ļ��
	*/
	row_per_pro = width / p;
	if (width % p != 0)
		row_per_pro++;

	realp = 0;
	while (row_per_pro * realp < width)
		realp++;

	if (my_rank == 0)
		printf("now there is %d processors are working on it and %d rows per processors\n", realp, row_per_pro);

	/* ����Ϊÿһ�������е�����������ָ���������ռ� */
	vecin = (float*)malloc(floatsize * width);
	ma = (float*)malloc(floatsize * row_per_pro * width);
	result = (float*)malloc(floatsize * row_per_pro);

	/* ������0�е�vec������ֵ�㲥���������̵�vecin�����б��� */
	if (my_rank == 0)
	{
		for (i = 0; i < width; i++)
			vecin(i) = vec(i);
		free(vec);
	}
	MPI_Bcast(vecin, width, MPI_FLOAT, 0, MPI_COMM_WORLD);

	/* ������0�е�matrix�ĸ���Ԫ�ذ��������̵ķֹ����д��� */
	if (my_rank == 0)
	{
		for (i = 0; i < row_per_pro; i++)
			for (j = 0; j < width; j++)
				ma(i, j) = matrix(i, j);
	}

	if (my_rank == 0)
	{
		if (width % realp == 0)
		{
			for (i = 1; i < realp; i++)
				MPI_Send(&(matrix(row_per_pro * i, 0)), row_per_pro * width, MPI_FLOAT, i, i, MPI_COMM_WORLD);
		}
		else
		{
			for (i = 1; i < (realp - 1); i++)
				MPI_Send(&(matrix(row_per_pro * i, 0)), row_per_pro * width, MPI_FLOAT, i, i, MPI_COMM_WORLD);
			MPI_Send(&(matrix(row_per_pro * (realp - 1), 0)),
				(width - row_per_pro * (realp - 1)) * width,
				MPI_FLOAT, (realp - 1), (realp - 1), MPI_COMM_WORLD);
		}
		free(matrix);
	}
	else
	{
		if (my_rank < realp)
		{
			if (width % realp == 0)
				MPI_Recv(ma, row_per_pro * width, MPI_FLOAT, 0, my_rank, MPI_COMM_WORLD, &status);
			else
			{
				if (my_rank != realp - 1)
					MPI_Recv(ma, row_per_pro * width, MPI_FLOAT, 0, my_rank, MPI_COMM_WORLD, &status);
				else
					MPI_Recv(ma, (width - row_per_pro * (realp - 1)) * width, MPI_FLOAT, 0, realp - 1, MPI_COMM_WORLD, &status);

			}

		}
	}

	/* ���е����ݶ��Ѿ�������ϣ�ͬ���������̲�����¼���ڵ�ʱ��� */
	MPI_Barrier(MPI_COMM_WORLD);
	finishprepare = MPI_Wtime();

	/* �����������Լ���һ���ֵļ��㹤�� */
	if (my_rank == 0)
	{
		for (i = 0; i < row_per_pro; i++)
		{
			sum = 0.0;
			for (j = 0; j < width; j++)
				sum = sum + ma(i, j) * vecin(j);
			result(i) = sum;
		}
	}
	else
	{
		if (my_rank < (realp - 1))
		{
			for (i = 0; i < row_per_pro; i++)
			{
				sum = 0.0;
				for (j = 0; j < width; j++)
					sum = sum + ma(i, j) * vecin(j);
				result(i) = sum;
			}
		}
		else
		{
			if (my_rank < realp)
			{
				for (i = 0; i < (width - row_per_pro * (realp - 1)); i++)
				{
					sum = 0.0;
					for (j = 0; j < width; j++)
						sum = sum + ma(i, j) * vecin(j);
					result(i) = sum;
				}
			}
		}
	}

	/* ÿһ�����̶�������ϣ����´�ʱ��ʱ��� */
	MPI_Barrier(MPI_COMM_WORLD);

	finishcompute = MPI_Wtime();

	/* ��������ؽ���0 */
	if (my_rank == 0)
	{
		for (i = 0; i < row_per_pro; i++)
			vecin(i) = result(i);
		for (i = 1; i < (realp - 1); i++)
			MPI_Recv(&(vecin(i * row_per_pro)), row_per_pro, MPI_FLOAT, i, i, MPI_COMM_WORLD, &status);
		if (realp != 1)
		{
			MPI_Recv(&(vecin(row_per_pro * (realp - 1))),
				(width - row_per_pro * (realp - 1)),
				MPI_FLOAT, (realp - 1), (realp - 1),
				MPI_COMM_WORLD, &status);
		}

	}
	else
	{
		if (my_rank < realp)
		{
			if (my_rank != (realp - 1))
				MPI_Send(result, row_per_pro, MPI_FLOAT, 0, my_rank, MPI_COMM_WORLD);
			else
				MPI_Send(result, (width - row_per_pro * (realp - 1)), MPI_FLOAT, 0, (realp - 1), MPI_COMM_WORLD);

		}
	}

	/* ������0�ڵ�vecin�ڱ���Ľ����� */
	if (my_rank == 0)
	{
		printf("the result is as follows:\n");
		for (i = 0; i < width; i++)
			printf("row%d %f\n", i, vecin(i));
	}
	MPI_Barrier(MPI_COMM_WORLD);
	finishall = MPI_Wtime();

	/* ��󽫼���ʱ����� */
	if (my_rank == 0)
	{
		printf("\n");
		printf("Whole running time    = %f seconds\n", finishall - starttime);
		printf("prepare time  = %f seconds\n", finishprepare - starttime);
		printf("Parallel compute time = %f seconds\n", finishcompute - finishprepare);

	}

	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
	free_all(ma, result, vecin);
	return (0);
}
