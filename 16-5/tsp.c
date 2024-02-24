#include <stdio.h>
#include <mpi.h>

/*�����������ֵ*/
#define MAXDISTANCE 999999

/*������������*/
#define MAXPOINT    20

int my_rank, group_size, n;
int point[MAXPOINT];
double dist[MAXPOINT][MAXPOINT];
double maxvalue[MAXPOINT];
int flag = 1;

void sub_tsp(int rank)
{
	int itemp, jtemp, ijtemp, k;
	double temp;
	int i, j;

	for (i = 0; i < n - 2; i++)
		for (j = i + 2; j < n; j++)
		{
			/*�������Ӧ�Ĵ�����*/
			if (my_rank == ((i + j) % group_size))
			{
				/*����Աߣ�i ��j���ĸĽ�Ȩ*/
				temp = dist[point[i]][point[i + 1]] + dist[point[j]][point[j + 1]] - dist[point[i]][point[j]] - dist[point[i + 1]][point[j + 1]];

				/*�ж��ǲ��Ǹ���ĸĽ�Ȩ*/
				if (temp > maxvalue[rank])
				{
					maxvalue[rank] = temp;
					itemp = i;
					jtemp = j;
				}
			}
		}

	/*������ĸĽ�Ȩ����0����Ӧ�ĶԱ�(itemp��jtemp)�������λ�õĵ���������ԭ����HamiltonȦ*/
	if (maxvalue[rank] > 0)
	{
		for (k = itemp + 1; k <= (itemp + 1 + jtemp) / 2; k++)
		{
			ijtemp = point[k];
			point[k] = point[itemp + jtemp + 1 - k];
			point[itemp + jtemp + 1 - k] = ijtemp;
		}
	}

	return;
}


/*�����Ľ�Ȩ*/
int selectmax()
{
	int i, j;
	double temp = 0;
	for (i = 0; i < group_size; i++)
	{
		if (maxvalue[i] > temp)
		{
			j = i;
			temp = maxvalue[i];
		}

	}
	if (temp == 0)
		return -1;
	return j;

}


/*������ŵĻ�·�ͻ�·���ܳ���*/
void output()
{
	int i;
	double sum = 0.0;
	for (i = 0; i < n; i++)
		sum += dist[point[i]][point[i + 1]];

	/*����㷨���н�����ʱ�򣬵õ���HamiltonȦ�ĳ��ȴ��ھ�������ֵ��˵��ԭͼ�в�����Ȧ*/
	if ((sum >= MAXDISTANCE) && (flag == 0))
	{
		printf("ԭͼ�в�����Ȧ!  \n");
		return;
	}

	for (i = 0; i < n; i++)
		printf("%d->", point[i]);

	printf("%d\n", point[n]);
	printf("����ĺ���%.1lf\n", sum);

	return;
}


void main(int argc, char* argv[])
{
	int i, j;
	MPI_Status status;

	/*��������*/
	MPI_Init(&argc, &argv);

	/*���Լ���id�������my_rank ��*/
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	/*�ҽ������������group_size ��*/
	MPI_Comm_size(MPI_COMM_WORLD, &group_size);

	/*�����֮��ľ������*/
	if (my_rank == 0)
	{
		/*�����ĸ����������n��*/
		printf("�������ĸ���:");
		int ret = scanf("%d", &n);

		/*��ĸ������ܴ���MAXPOINT*/
		if (n > MAXPOINT)
		{
			printf("��ĸ������ܴ���%d!  \n", MAXPOINT);
			goto terminal;
		}

		/*�ų�n��0��1��2�����*/
		if (n < 3)
		{
			printf("TSP ������n��%d�������û���壡��  \n", n);
			goto terminal;
		}

		/* ����� i�͵�j֮��� ���룬�����dist[i][j]*/

		for (i = 0; i < n - 1; i++)
		{
			for (j = i + 1; j < n; j++)
			{
				printf("%d<->%d: ", i, j);
				int ret = scanf("%lf", &dist[i][j]);
				dist[j][i] = dist[i][j];
			}
		}

		for (i = 0; i <= n; i++)
		{
			dist[i][i] = 0;
			dist[n][i] = dist[0][i];
			dist[i][n] = dist[i][0];
		}
	}

	/*�Ӹ����������н��̷���n*/
	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

	/*ͬ�����н���*/
	MPI_Barrier(MPI_COMM_WORLD);

	/*�Ӹ����������н��̷��͵�0��������ľ���*/
	for (i = 0; i <= n; i++)
		MPI_Bcast(&dist[i][0], n + 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);

	/*�����ʼ��HamiltonȦ0��>1��>2��> ...... ��>n-1��>0*/

	for (i = 0; i <= n; i++)
		point[i] = i % n;

	/*flag��־���ܲ��ܶ�HamiltonȦ���и���*/
	while (flag == 1)
	{
		/*���ÿ�θ������HamiltonȦ�����ܳ���*/
		if (my_rank == 0)
			output();

		maxvalue[my_rank] = 0;
		sub_tsp(my_rank);

		/*�Ǹ����̽����еĸĽ�Ȩ���ݵ�0������*/
		if (my_rank > 0)
			MPI_Send(&maxvalue[my_rank], 1, MPI_DOUBLE, 0, my_rank, MPI_COMM_WORLD);

		/*�����̽����������̵ĸĽ�Ȩ�������ж����ĸĽ�Ȩ*/

		if (my_rank == 0)
		{
			for (i = 1; i < group_size; i++)
				MPI_Recv(&maxvalue[i], 1, MPI_DOUBLE, i, i, MPI_COMM_WORLD, &status);
			j = selectmax();
		}
		MPI_Barrier(MPI_COMM_WORLD);

		/*�Ӹ����������н��̷��͵����ĸĽ�Ȩ*/
		MPI_Bcast(&j, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Barrier(MPI_COMM_WORLD);

		/*������Ľ�ȨΪ0�����ʾû���κθĽ��������ٶ�HamiltonȦ���и���*/
		if (j == -1)
			flag = 0;
		/*�������Ȩ��Ӧ�Ĵ�����rankֵ�㲥���������У���Ӧ�Ĵ������õ��Ľ���Ȧ*/
		else
			MPI_Bcast(point, n + 1, MPI_INT, j, MPI_COMM_WORLD);
	}

	/*�����������ս��*/
	if (my_rank == 0)
		output();

	/*��������*/
terminal:
	MPI_Finalize();
}
