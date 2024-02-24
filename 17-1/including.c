#include <mpi.h>
#include <stdio.h>

int n;
double xtemp[20], ytemp[20];
double x, y;
int s, mys;
int group_size, my_rank;

/*�ж��������߶��Ƿ��н���*/
int cal_inter(int number, int i, double x, double y)
{
	double x1, y1, x2, y2, temp;
	int result;
	result = 0;

	if (number + i * group_size >= n)
		return result;
	x1 = xtemp[number + i * group_size];
	y1 = ytemp[number + i * group_size];
	x2 = xtemp[(number + 1 + i * group_size) % n];
	y2 = ytemp[(number + 1 + i * group_size) % n];

	if (y1 > y2)
	{
		temp = x1;
		x1 = x2;
		x2 = temp;
		temp = y1;
		y1 = y2;
		y2 = temp;
	}

	/*�ж���ֱ�ߵ����*/
	if (x1 == x2)
	{
		if ((x > x1) && (y <= y2) && (y > y1))
			result = 1;
		else
			result = 0;
		/*������ֱ���ϣ�Ӧ�ö�result��һ���ȽϵĴ��ֵ��������100*/
		if ((x == x1) && ((y - y1) * (y2 - y) >= 0))
			result = 100;
	}
	else
	{
		/*����ֱ�ߣ���ˮƽ��*/
		if (y1 != y2)
		{
			temp = x2 + (y - y2) * (x2 - x1) / (y2 - y1);
			/*����պ��ڱ��ϣ��Ҳ�Ϊ�¶���*/
			if ((temp < x) && (y <= y2) && (y > y1))
				result = 1;
			else
				result = 0;

			/*���ڱ��ϣ�Ӧ�ö�result��һ���ȽϵĴ��ֵ��������100*/
			if ((temp == x) && ((y - y2) * (y1 - y) >= 0))
				result = 100;
		}
		else
		{
			/*����ˮƽ���ϣ�Ӧ�ö�result��һ���ȽϵĴ��ֵ��������100*/
			if ((y == y1) && ((x1 - x) * (x - x2) >= 0)) result = 100;
		}
	}
	return result;
}

main(int argc, char* argv[])
{
	int i;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &group_size);

	/*����������������ʼ��*/
	mys = 0;

	/*���������������ζ����Ҫ�жϵĵ������*/
	if (my_rank == 0)
	{
		printf("�������ĸ���:");
		scanf("%d", &n);
		printf("��������������\n");
		for (i = 0; i < n; i++)
		{
			printf("%d:", i);
			scanf("%lf", &xtemp[i]);
			scanf("%lf", &ytemp[i]);
		}
		printf("������Ҫ�жϵ������\n");
		int ret = scanf("%lf %lf", &x, &y);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	/*�Ѷ���εĶ�����������������Ҫ�б�ĵ�����겥�͸����н���*/
	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&x, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Bcast(&y, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Bcast(xtemp, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Bcast(ytemp, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	MPI_Barrier(MPI_COMM_WORLD);
	/*ÿһ������������n/group_size�����ϵ���������*/
	for (i = 0; i < n / group_size + 1; i++)
	{
		mys += cal_inter(my_rank, i, x, y);
	}

	MPI_Barrier(MPI_COMM_WORLD);

	/*��mys��ֵ��Լ��s*/
	MPI_Reduce(&mys, &s, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

	/*����sֵȷ��������*/
	if (my_rank == 0)
	{
		if (s >= 100)
			printf("vertex p is in polygon\n");
		else
			if (s % 2 == 1)
				printf("vertex p is in polygon\n");
			else
				printf("vertex p is out of polygon\n");
	}
	MPI_Finalize();
}

