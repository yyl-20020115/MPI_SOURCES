#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"

#define MAX_N 50
#define PI    3.1415926535897932
#define EPS   10E-8
#define V_TAG 99
#define P_TAG 100
#define Q_TAG 101
#define R_TAG 102
#define S_TAG 103
#define S_TAG2 104

typedef enum { FALSE, TRUE }
BOOL;

typedef struct
{
	double r;
	double i;
} complex_t;

complex_t p[MAX_N], q[MAX_N], s[2 * MAX_N], r[2 * MAX_N];
complex_t w[2 * MAX_N];
int variableNum;
double transTime = 0, totalTime = 0, beginTime;
MPI_Status status;

void comp_add(complex_t* result, const complex_t* c1, const complex_t* c2)
{
	result->r = c1->r + c2->r;
	result->i = c1->i + c2->i;
}


void comp_multiply(complex_t* result, const complex_t* c1, const complex_t* c2)
{
	result->r = c1->r * c2->r - c1->i * c2->i;
	result->i = c1->r * c2->i + c2->r * c1->i;
}


/*
 * Function:    shuffle
 * Description: �ƶ�f�д�beginPos��endPosλ�õ�Ԫ�أ�ʹ֮��λ����ż
 *              �������С�����˵��:��������f��beginPos=2, endPos=5
 *              ��shuffle���������н��Ϊf[2..5]�������У����к����
 *              λ�ö�Ӧ��ԭf��Ԫ��Ϊ: f[2],f[4],f[3],f[5]
 * Parameters:  fΪ�����������׵�ַ
 *              beginPos, endPosΪ�������±귶Χ
 */
void shuffle(complex_t* f, int beginPos, int endPos)
{
	int i;
	complex_t temp[2 * MAX_N];

	for (i = beginPos; i <= endPos; i++)
	{
		temp[i] = f[i];
	}

	int j = beginPos;
	for (i = beginPos; i <= endPos; i += 2)
	{
		f[j] = temp[i];
		j++;
	}
	for (i = beginPos + 1; i <= endPos; i += 2)
	{
		f[j] = temp[i];
		j++;
	}
}


/*
 * Function:		evaluate
 * Description:	�Ը�������f����FFT����IFFT(��x����)���������Ϊy��
 * 			����leftPos �� rightPos֮��Ľ��Ԫ��
 * Parameters:	f : ԭʼ���������׵�ַ
 * 			beginPos : ԭʼ����������f�еĵ�һ���±�
 * 			endPos : ԭʼ����������f�е����һ���±�
 * 			x : ��ŵ�λ�������飬��Ԫ��Ϊw,w^2,w^3...
 * 			y : �������
 * 			leftPos : ��������������y��Ƭ�ϵ���ʼ�±�
 * 			rightPos : ��������������y��Ƭ�ϵ���ֹ�±�
 * 			totalLength : y�ĳ���
 */
void evaluate(complex_t* f, int beginPos, int endPos,
	const complex_t* x, complex_t* y,
	int leftPos, int rightPos, int totalLength)
{
	int i;
	if ((beginPos > endPos) || (leftPos > rightPos))
	{
		printf("Error in use Polynomial!\n");
		exit(-1);
	}
	else if (beginPos == endPos)
	{
		for (i = leftPos; i <= rightPos; i++)
		{
			y[i] = f[beginPos];
		}
	}
	else if (beginPos + 1 == endPos)
	{
		for (i = leftPos; i <= rightPos; i++)
		{
			complex_t temp;
			comp_multiply(&temp, &f[endPos], &x[i]);
			comp_add(&y[i], &f[beginPos], &temp);
		}
	}
	else
	{
		complex_t tempX[2 * MAX_N], tempY1[2 * MAX_N], tempY2[2 * MAX_N];
		int midPos = (beginPos + endPos) / 2;

		shuffle(f, beginPos, endPos);

		for (i = leftPos; i <= rightPos; i++)
		{
			comp_multiply(&tempX[i], &x[i], &x[i]);
		}

		evaluate(f, beginPos, midPos, tempX, tempY1,
			leftPos, rightPos, totalLength);
		evaluate(f, midPos + 1, endPos, tempX, tempY2,
			leftPos, rightPos, totalLength);

		for (i = leftPos; i <= rightPos; i++)
		{
			complex_t temp;
			comp_multiply(&temp, &x[i], &tempY2[i]);
			comp_add(&y[i], &tempY1[i], &temp);
		}
	}
}


/*
 * Function:    print
 * Description: ��ӡ����Ԫ�ص�ʵ��
 * Parameters:  fΪ����ӡ������׵�ַ
 *              fLengthΪ����ĳ���
 */
void print(const complex_t* f, int fLength)
{
	BOOL isPrint = FALSE;
	int i;

	/* f[0] */
	if (abs(f[0].r) > EPS)
	{
		printf("%f", f[0].r);
		isPrint = TRUE;
	}

	for (i = 1; i < fLength; i++)
	{
		if (f[i].r > EPS)
		{
			if (isPrint)
				printf(" + ");
			else
				isPrint = TRUE;
			printf("%ft^%d", f[i].r, i);
		}
		else if (f[i].r < -EPS)
		{
			if (isPrint)
				printf(" - ");
			else
				isPrint = TRUE;
			printf("%ft^%d", -f[i].r, i);
		}
	}
	if (isPrint == FALSE)
		printf("0");
	printf("\n");
}


/*
 * Function:    myprint
 * Description: ������ӡ��������Ԫ�أ�����ʵ�����鲿
 * Parameters:  fΪ����ӡ������׵�ַ
 *              fLengthΪ����ĳ���
 */
void myprint(const complex_t* f, int fLength)
{
	int i;
	for (i = 0; i < fLength; i++)
	{
		printf("%f+%fi , ", f[i].r, f[i].i);
	}
	printf("\n");
}


/*
 * Function:   addTransTime
 * Description:�ۼƷ����������ķѵ�ʱ��
 * Parameters: toAddΪ�ۼӵ�ʱ��
 */
void addTransTime(double toAdd)
{
	transTime += toAdd;
}


/*
 * Function:    readFromFile
 * Description:	��dataIn.txt��ȡ����
 */
BOOL readFromFile()
{
	int i;
	FILE* fin = fopen("dataIn.txt", "r");

	if (fin == NULL)
	{
		printf("Cannot find input data file\n"
			"Please create a file \"dataIn.txt\"\n"
			"2\n"
			"1.0  2\n"
			"2.0  -1\n"
		);
		return(FALSE);
	}

	int ret = fscanf(fin, "%d\n", &variableNum);
	if ((variableNum < 1) || (variableNum > MAX_N))
	{
		printf("variableNum out of range!\n");
		return(FALSE);
	}

	for (i = 0; i < variableNum; i++)
	{
		int ret = fscanf(fin, "%lf", &p[i].r);
		p[i].i = 0.0;
	}

	for (i = 0; i < variableNum; i++)
	{
		int ret = fscanf(fin, "%lf", &q[i].r);
		q[i].i = 0.0;
	}

	fclose(fin);

	printf("Read from data file \"dataIn.txt\"\n");
	printf("p(t) = ");
	print(p, variableNum);
	printf("q(t) = ");
	print(q, variableNum);

	return(TRUE);
}


/*
 * Function:    sendOrigData
 * Description: ��ԭʼ���ݷ��͸���������
 * Parameters:  sizeΪ��Ⱥ�н��̵���Ŀ
 */
void sendOrigData(int size)
{
	int i;
	for (i = 1; i < size; i++)
	{
		MPI_Send(&variableNum, 1, MPI_INT, i, V_TAG, MPI_COMM_WORLD);
		MPI_Send(p, variableNum * 2, MPI_DOUBLE, i, P_TAG, MPI_COMM_WORLD);
		MPI_Send(q, variableNum * 2, MPI_DOUBLE, i, Q_TAG, MPI_COMM_WORLD);
	}
}


/*
 * Function:    recvOrigData
 * Description:	����ԭʼ����
 */
void recvOrigData()
{
	MPI_Recv(&variableNum, 1, MPI_INT, 0, V_TAG, MPI_COMM_WORLD, &status);
	MPI_Recv(p, variableNum * 2, MPI_DOUBLE, 0, P_TAG, MPI_COMM_WORLD, &status);
	MPI_Recv(q, variableNum * 2, MPI_DOUBLE, 0, Q_TAG, MPI_COMM_WORLD, &status);
}


int main(int argc, char* argv[])
{
	int rank, size, i;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if (rank == 0)
	{
		/* 0# ���̴��ļ�dataIn.txt�������ʽp,q�Ľ�����ϵ������ */
		if (!readFromFile())
			exit(-1);

		/* ������Ŀ̫�࣬���ÿ������ƽ�����䲻��һ��Ԫ�أ��쳣�˳� */
		if (size > 2 * variableNum)
		{
			printf("Too many Processors , reduce your -np value\n");
			MPI_Abort(MPI_COMM_WORLD, 1);
		}

		beginTime = MPI_Wtime();

		/* 0#���̰Ѷ���ʽ�Ľ�����p��q���͸��������� */
		sendOrigData(size);

		/* �ۼƴ���ʱ�� */
		addTransTime(MPI_Wtime() - beginTime);
	}
	else                                          /* �������̽��ս���0�����������ݣ�����variableNum������p��q */
	{
		recvOrigData();
	}
	/* ��ʼ������w�����ڽ��и���Ҷ�任 */
	int wLength = 2 * variableNum;
	for (i = 0; i < wLength; i++)
	{
		w[i].r = cos(i * 2 * PI / wLength);
		w[i].i = sin(i * 2 * PI / wLength);
	}

	/* ���ָ������̵Ĺ�����Χ startPos ~ stopPos */
	int everageLength = wLength / size;
	int moreLength = wLength % size;
	int startPos = moreLength + rank * everageLength;
	int stopPos = startPos + everageLength - 1;

	if (rank == 0)
	{
		startPos = 0;
		stopPos = moreLength + everageLength - 1;
	}

	/* ��p��FFT���������Ϊs��ÿ�����̽��������������� */
	/* λ��ΪstartPos �� stopPos��Ԫ�� */
	evaluate(p, 0, variableNum - 1, w, s, startPos, stopPos, wLength);

	/* ��q��FFT���������Ϊr��ÿ�����̽��������������� */
	/* λ��ΪstartPos �� stopPos��Ԫ�� */
	evaluate(q, 0, variableNum - 1, w, r, startPos, stopPos, wLength);

	/* s��r����������������s�У�ͬ����ÿ������ֻ�����Լ���Χ�ڵĲ��� */
	for (i = startPos; i <= stopPos; i++)
	{
		complex_t temp;
		comp_multiply(&temp, &s[i], &r[i]);
		s[i] = temp;
		s[i].r /= wLength * 1.0;
		s[i].i /= wLength * 1.0;
	}

	/* �������̶���s���Լ������������Ĳ��ַ��͸�����0�����ӽ���0���ջ��ܵ�s */
	if (rank > 0)
	{
		MPI_Send(s + startPos, everageLength * 2, MPI_DOUBLE, 0, S_TAG, MPI_COMM_WORLD);
		MPI_Recv(s, wLength * 2, MPI_DOUBLE, 0, S_TAG2, MPI_COMM_WORLD, &status);
	}
	else
	{
		/* ����0����sƬ�ϣ���������̷���������s */
		double tempTime = MPI_Wtime();

		for (i = 1; i < size; i++)
		{
			MPI_Recv(s + moreLength + i * everageLength, everageLength * 2,
				MPI_DOUBLE, i, S_TAG, MPI_COMM_WORLD, &status);
		}

		for (i = 1; i < size; i++)
		{
			MPI_Send(s, wLength * 2,
				MPI_DOUBLE, i,
				S_TAG2, MPI_COMM_WORLD);
		}

		addTransTime(MPI_Wtime() - tempTime);
	}

	/* swap(w[i],w[(wLength-i)%wLength]) */
	/* ��������w���������渵��Ҷ�任 */
	complex_t temp;
	for (i = 1; i < wLength / 2; i++)
	{
		temp = w[i];
		w[i] = w[wLength - i];
		w[wLength - i] = temp;
	}

	/* �������̶�s����FFT�������r����Ӧ���� */
	evaluate(s, 0, wLength - 1, w, r, startPos, stopPos, wLength);

	/* �����̰��Լ�����Ĳ��ֵ�r��Ƭ�Ϸ��͵�����0 */
	if (rank > 0)
	{
		MPI_Send(r + startPos, everageLength * 2, MPI_DOUBLE,
			0, R_TAG, MPI_COMM_WORLD);
	}
	else
	{
		/* ����0���ո���Ƭ�ϵõ�������r����ʱr����������ʽp,q��˵Ľ������ʽ�� */
		double tempTime = MPI_Wtime();

		for (i = 1; i < size; i++)
		{
			MPI_Recv((r + moreLength + i * everageLength), everageLength * 2,
				MPI_DOUBLE, i, R_TAG, MPI_COMM_WORLD, &status);
		}

		totalTime = MPI_Wtime();
		addTransTime(totalTime - tempTime);
		totalTime -= beginTime;

		/* ��������Ϣ�Լ�ʱ��ͳ����Ϣ */
		printf("\nAfter FFT r(t)=p(t)q(t)\n");
		printf("r(t) = ");
		print(r, wLength - 1);
		printf("\nUse prossor size = %d\n", size);
		printf("Total running time = %f(s)\n", totalTime);
		printf("Distribute data time = %f(s)\n", transTime);
		printf("Parallel compute time = %f(s)\n", totalTime - transTime);
	}
	MPI_Finalize();
}
