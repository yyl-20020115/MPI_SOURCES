#include <stdio.h>
#include <mpi.h>
#include <math.h>

int my_rank, group_size, sub_group;
int n_all;
double pointx[30], pointy[30];
double tempx[2], tempy[2], xtemp[4], ytemp[4];
MPI_Status status;
int location[30];

/*���y������С�ĵ�,����ֵ���浽���浽tempx,tempy������*/
void getymin()
{
	int i, index = 0;
	double tempvalue = pointy[0];

	for (i = 1; i < n_all; i++)
	{
		if (pointy[i] < tempvalue)
		{
			tempvalue = pointy[i];
			index = i;
		}
		else
		{
			/*��y������ͬ,��ȡx����С�ߡ�����Ϊ�˱�֤(tempx[0],tempy[0])->(tempx[1],tempy[1])Ϊ��ʱ�뷽��*/
			if (pointy[i] == tempvalue)
				if (pointx[index] > pointx[i])
					index = i;
		}
	}
	tempx[0] = pointx[index];
	tempy[0] = pointy[index];

	/*���y������С�ļ���ֻ��1��,�͸��Ƹõ�*/
	tempx[1] = tempx[0];
	tempy[1] = tempy[0];

	/*���켫���Ƿ����1��*/
	for (i = 0; i < n_all; i++)
		if ((pointy[i] == tempy[1]) && (pointx[i] > tempx[1]))
		{
			/*�������1��,ȡx�������,�������һ�����������*/
			tempx[1] = pointx[i];
			tempy[1] = pointy[i];
		}
	return;
}


/*���y�������ĵ�,���浽tempx,tempy������*/
void getymax()
{
	int i, index = 0;
	double temp = pointy[0];

	for (i = 1; i < n_all; i++)
	{
		if (pointy[i] > temp)
		{
			temp = pointy[i];
			index = i;
		}
		else
		{
			/*��y������ͬ,��ȡx������ߡ�����Ϊ�˱�֤(tempx[0],tempy[0])->(tempx[1],tempy[1])Ϊ��ʱ�뷽��*/
			if (temp == pointy[i])
				if (pointx[i] > pointx[index])
					index = i;
		}
	}
	/*����YMAX��������*/
	tempx[0] = pointx[index];
	tempy[0] = pointy[index];

	/*���y�������ļ���ֻ��1��,�͸��Ƹõ�*/
	tempx[1] = tempx[0];
	tempy[1] = tempy[0];

	/*���켫���Ƿ����1��*/
	for (i = 0; i < n_all; i++)
	{
		if ((pointy[i] == tempy[1]) && (pointx[i] < tempx[1]))
		{
			/*�������1��,ȡx����С��,�������һ�����������*/
			tempx[1] = pointx[i];
			tempy[1] = pointy[i];
		}
	}

	return;
}


/*���x������С�ĵ�,���걣�浽tempx,tempy������*/
void getxmin()
{
	int i, index = 0;
	double temp = pointx[0];

	for (i = 1; i < n_all; i++)
	{
		if (pointx[i] < temp)
		{
			temp = pointx[i];
			index = i;
		}
		else
		{
			/*��x������ͬ,��ȡy������ߡ�����Ϊ�˱�֤(tempx[0],tempy[0])->(tempx[1],tempy[1])Ϊ��ʱ�뷽��*/
			if (pointx[i] == temp)
				if (pointy[index] < pointy[i])
					index = i;
		}
	}
	/*����XMIN��������*/
	tempx[0] = pointx[index];
	tempy[0] = pointy[index];

	/*���x������С�ļ���ֻ��1��,�͸��Ƹõ�*/
	tempx[1] = tempx[0];
	tempy[1] = tempy[0];

	/*���켫���Ƿ����1��*/
	for (i = 0; i < n_all; i++)
	{
		if ((pointx[i] == tempx[1]) && (pointy[i] < tempy[1]))
		{
			/*�������1��,ȡy����С��,�������һ�����������*/
			tempx[1] = pointx[i];
			tempy[1] = pointy[i];
		}
	}
	return;
}


/*���x�������ĵ�,������ֵ���汣�浽tempx,tempy������*/
void getxmax()
{
	int i, index = 0;
	double temp = pointx[0];

	for (i = 1; i < n_all; i++)
	{
		if (pointx[i] > temp)
		{
			temp = pointx[i];
			index = i;
		}
		else
		{
			/*��x������ͬ,��ȡy����С�ߡ�����Ϊ�˱�֤(tempx[0],tempy[0])->(tempx[1],tempy[1])Ϊ��ʱ�뷽��*/
			if (temp == pointx[i])
				if (pointy[index] > pointy[i])
					index = i;
		}
	}
	/*����XMAX��������*/
	tempx[0] = pointx[index];
	tempy[0] = pointy[index];

	/*���x�������ļ���ֻ��1��,�͸��Ƹõ�*/
	tempx[1] = tempx[0];
	tempy[1] = tempy[0];

	/*���켫���Ƿ����1��*/
	for (i = 0; i < n_all; i++)
	{
		if ((pointx[i] == tempx[1]) && (pointy[i] > tempy[1]))
		{
			/*�������1��,ȡy�������,�����2�����������*/
			tempx[1] = pointx[i];
			tempy[1] = pointy[i];
		}
	}
	return;
}


/*temp[x],temp[y]��������ֵ��������Ϊ���ж����ֵʱ��������β���������滮��
����ʱ�ֱ���������Ϊ�˵㣬�м��û�������ĵ���*/

/*ȷ����������һ����*/
void getincludedvertex(int tag)
{
	double linea, lineb, dist;
	int i, count = 0;

	/*���ñ����˻���һ����, ����˵������������ڴ���������*/
	if ((tempx[1] == tempx[0]) && (tempy[1] == tempy[0]))
	{
		n_all = 1;
		pointx[0] = tempx[1];
		pointy[0] = tempy[1];
	}
	else
	{
		/*�������˵�����ȷ��ֱ�ߵ�б�ʺͲ���*/
		linea = (tempy[1] - tempy[0]) / (tempx[1] - tempx[0]);
		lineb = tempy[1] - linea * tempx[1];
		/*���ݵ���ֱ�ߵ���һ����ȷ���ö����Ƿ��ڴ���������*/
		for (i = 0; i < n_all; i++)
		{
			dist = linea * pointx[i] + lineb - pointy[i];
			if (tag * dist < 0)
			{
				pointx[count] = pointx[i];
				pointy[count++] = pointy[i];
			}
		}
		/*���������λ���Լ��������˵�*/
		for (i = 0; i < count; i++)
		{
			pointx[count - i] = pointx[count - 1 - i];
			pointy[count - i] = pointy[count - 1 - i];
		}
		/*�������˵��������*/
		pointx[0] = tempx[0];
		pointy[0] = tempy[0];
		pointx[count + 1] = tempx[1];
		pointy[count + 1] = tempy[1];
		count += 2;
		/*����n_allΪҪ������µı����еĵ������*/
		n_all = count;
	}
}


/*ȷ���˵�ĺ�һ�����ţ�������location[]��*/
void nextindex(int i, int x, int y)
{
	double x1, y1, temp, valuemax = 0;
	int j;

	x1 = pointx[i];
	y1 = pointy[i];

	/*��������, ������С���ǵ�������Ϊ�Լ���Nextindexֵ*/
	for (j = 0; j < n_all; j++)
	{
		if (((pointx[j] - x1) * x + (pointy[j] - y1) * y) > 0)
		{
			/*�ɼ��ǵ����һ�����ǵ��������жϼ��Ǵ�С*/
			temp = ((pointx[j] - x1) * x + (pointy[j] - y1) * y) / (sqrt((pointx[j] - x1) * (pointx[j] - x1) + (pointy[j] - y1) * (pointy[j] - y1)));
			if (temp > valuemax)
			{
				location[i] = j;
				valuemax = temp;
			}
		}
	}
	if (valuemax == 0)
		location[i] = -1;
}


/*����������*/
void output(int rank)
{
	int j = 0, index, temp;
	double xtem, ytem;
	int flag = 0;

	printf("������ǵ�%d���ֵĵ�\n", rank / sub_group);
	if (rank == 0)
		flag = 1;
	if (flag == 0)
		if (!((pointx[n_all - 1] == tempx[0]) &&
			(pointy[n_all - 1] == tempy[0])))
			flag = 1;

	/*��nextindex����,ʹ����ĵ㰴��ʱ�뷽��,�Ա�֤���������������˳������*/
	index = location[j];
	while (location[index] != -1)
	{
		temp = location[index];
		location[index] = j;
		j = index;
		index = temp;
	}
	location[index] = j;
	location[0] = -1;

	/*��������˳���������*/
	j = n_all - 1;
	while (location[j] != -1)
	{
		if (!((j == n_all - 1) && (flag == 0)))
		{
			printf("%.2lf,%.2lf\n", pointx[j], pointy[j]);
			xtem = pointx[j];
			ytem = pointy[j];
		}
		j = location[j];
	}

	/*�ж��ǲ����غ�*/
	if (!((j == n_all - 1) && (flag == 0)))
	{
		printf("%.2lf,%.2lf\n", pointx[j], pointy[j]);
		xtem = pointx[j];
		ytem = pointy[j];
	}

	if ((rank == 3 * sub_group) && !((xtem == tempx[1]) && (ytem == tempy[1])))
		printf("%.2lf,%.2lf\n", tempx[1], tempy[1]);
	return;
}


main(int argc, char* argv[])
{
	int i;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &group_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	/*����������Ҫ4����������������ִ��*/
	if (group_size < 4)
	{
		if (my_rank == 0)
		{
			printf("Need 4 or more processors to run!\n");
		}
		MPI_Finalize();
		exit(0);
	}

	if (my_rank == 0)
	{
		printf("please input all the vertexes!\nfirst is number!\n");
		printf("please input the Number:");
		int ret = scanf("%d", &n_all);
		printf("please input the vertex:\n");
		for (i = 0; i < n_all; i++)
		{
			ret = scanf("%lf", &pointx[i]);
			ret = scanf("%lf", &pointy[i]);
		}
	}
	/*������0��������Ķ�������*/

	sub_group = group_size / 4;

	MPI_Bcast(&n_all, 1, MPI_INT, 0, MPI_COMM_WORLD);
	/*�㲥��������*/
	/*��1�е�����������,���2,3,4�е��������������Ͷ������꣬��Ӧ���㷨17.6����(1.1)*/
	if (my_rank == 0)
	{
		for (i = 1; i < 4; i++)
		{
			MPI_Send(pointx, n_all, MPI_DOUBLE, i * sub_group, i, MPI_COMM_WORLD);
			MPI_Send(pointy, n_all, MPI_DOUBLE, i * sub_group, i, MPI_COMM_WORLD);
		}
	}
	if ((my_rank > 0) && (my_rank < 4 * sub_group) && (my_rank % sub_group == 0))
	{
		MPI_Recv(pointx, n_all, MPI_DOUBLE, 0, my_rank / sub_group, MPI_COMM_WORLD, &status);
		MPI_Recv(pointy, n_all, MPI_DOUBLE, 0, my_rank / sub_group, MPI_COMM_WORLD, &status);
	}

	MPI_Barrier(MPI_COMM_WORLD);

	/*���㼫��,�������ǵ�����ֱ�洢��4��������������*/
	/*��1�е���������������YMIN*/
	if (my_rank == 0)
		getymin();
	/*��2�е���������������XMAX*/
	if (my_rank == sub_group)
		getxmax();
	/*��3�е���������������YMAX*/
	if (my_rank == 2 * sub_group)
		getymax();
	/*��4�е���������������XMIN*/
	if (my_rank == 3 * sub_group)
		getxmin();

	MPI_Barrier(MPI_COMM_WORLD);

	/*�������ɼ�����ɵıߴ洢��ÿһ�е�������������*/
	if ((my_rank > 0) && (my_rank < 4 * sub_group) && (my_rank % sub_group == 0))
	{
		/*������������������Ϣ������0, ��֪��������*/
		MPI_Send(&tempx[1], 1, MPI_DOUBLE, 0, my_rank, MPI_COMM_WORLD);
		MPI_Send(&tempy[1], 1, MPI_DOUBLE, 0, my_rank, MPI_COMM_WORLD);
	}
	if (my_rank == 0)
	{
		for (i = 1; i < 4; i++)
		{
			MPI_Recv(&xtemp[i], 1, MPI_DOUBLE, i * sub_group, i * sub_group, MPI_COMM_WORLD, &status);
			MPI_Recv(&ytemp[i], 1, MPI_DOUBLE, i * sub_group, i * sub_group, MPI_COMM_WORLD, &status);
		}
		xtemp[0] = tempx[1];
		ytemp[0] = tempy[1];
		tempx[1] = xtemp[3];
		tempy[1] = ytemp[3];

		for (i = 1; i < 4; i++)
		{
			/*����0����صļ������귢�͸�������������*/
			MPI_Send(&xtemp[i - 1], 1, MPI_DOUBLE, i * sub_group, i * sub_group, MPI_COMM_WORLD);
			MPI_Send(&ytemp[i - 1], 1, MPI_DOUBLE, i * sub_group, i * sub_group, MPI_COMM_WORLD);
		}
	}
	else
	{
		if ((my_rank < 4 * sub_group) && (my_rank % sub_group == 0))
		{
			/*���������������ս���0����Ϣ,�õ����洢�ɼ��㹹�ɵı�*/
			MPI_Recv(&tempx[1], 1, MPI_DOUBLE, 0, my_rank, MPI_COMM_WORLD, &status);
			MPI_Recv(&tempy[1], 1, MPI_DOUBLE, 0, my_rank, MPI_COMM_WORLD, &status);
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);

	/*ȷ���ı����еĶ��㣬�������ඥ������ĸ���������*/
	/*�ĸ�����������ͬʱ�ж϶����Ƿ����������ڵ����� */

	/*�������ڸ������ڵĵ㣬ÿ�������������õ�Ҫ������µı���*/
	/*����õ����ڴ˷�Χ�ĵ�*/
	if (my_rank == 0)
		getincludedvertex(-1);
	if (my_rank == sub_group)
		getincludedvertex(-1);
	if (my_rank == 2 * sub_group)
		getincludedvertex(1);
	if (my_rank == 3 * sub_group)
		getincludedvertex(1);

	MPI_Barrier(MPI_COMM_WORLD);

	if ((my_rank < 4 * sub_group) && (my_rank % sub_group == 0))
	{
		for (i = 1; i < sub_group; i++)
		{
			/*����������������е����������������µı����еĶ�������*/
			MPI_Send(&n_all, 1, MPI_INT, my_rank + i, i, MPI_COMM_WORLD);
		}
	}
	else if (my_rank < 4 * sub_group)
	{
		/*���������Ӹ��е��������������µı����еĶ�������*/
		MPI_Recv(&n_all, 1, MPI_INT, my_rank / sub_group * sub_group, my_rank % sub_group, MPI_COMM_WORLD, &status);
	}

	MPI_Barrier(MPI_COMM_WORLD);

	if ((my_rank < 4 * sub_group) && (my_rank % sub_group == 0))
	{
		for (i = 1; i < sub_group; i++)
		{
			/*����������������е����������������µı����еĶ�������*/
			MPI_Send(pointx, n_all, MPI_DOUBLE, my_rank + i, i, MPI_COMM_WORLD);
			MPI_Send(pointy, n_all, MPI_DOUBLE, my_rank + i, i, MPI_COMM_WORLD);
		}
	}
	else if (my_rank < 4 * sub_group)
	{
		MPI_Recv(pointx, n_all, MPI_DOUBLE, my_rank / sub_group * sub_group, my_rank % sub_group, MPI_COMM_WORLD, &status);
		MPI_Recv(pointy, n_all, MPI_DOUBLE, my_rank / sub_group * sub_group, my_rank % sub_group, MPI_COMM_WORLD, &status);
	}

	MPI_Barrier(MPI_COMM_WORLD);

	/*ÿһ���ϵĴ�����������ͬһ����ĵ���㼫�ǣ�������С���ǵ�������Ϊ�Լ���Nexindexֵ�����������е��nextindex����*/
	if ((n_all > 1) && (my_rank < 4 * sub_group))
	{
		for (i = my_rank % sub_group; i < n_all - 1; i += sub_group)
		{
			if (my_rank / sub_group == 0)
				nextindex(i, -1, 0);
			if (my_rank / sub_group == 1)
				nextindex(i, 0, -1);
			if (my_rank / sub_group == 2)
				nextindex(i, 1, 0);
			if (my_rank / sub_group == 3)
				nextindex(i, 0, 1);
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);

	/*���������е��nextindex�������͵����Ľ��*/
	if ((my_rank < 4 * sub_group) && (my_rank % sub_group != 0))
	{
		for (i = my_rank % sub_group; i < n_all - 1; i += sub_group)
			MPI_Send(&location[i], 1, MPI_INT, my_rank / sub_group * sub_group, my_rank, MPI_COMM_WORLD);
	}
	else if (my_rank < 4 * sub_group)
	{
		for (i = 1; i < n_all - 1; i++)
		{
			if (i % sub_group != 0)
			{
				MPI_Recv(&location[i], 1, MPI_INT, my_rank + i % sub_group, my_rank + i % sub_group, MPI_COMM_WORLD, &status);
			}
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);
	location[n_all - 1] = -1;

	MPI_Barrier(MPI_COMM_WORLD);

	/*ÿ���������������յ��Nextindex��������Լ��������ϵĵ�)*/
	if (my_rank == 0)
	{
		/*��1����������������*/
		output(my_rank);
		/*������һ���㴫�͸���2�����������ж��ǲ�����ͬ*/
		MPI_Send(&pointx[0], 1, MPI_DOUBLE, sub_group, sub_group, MPI_COMM_WORLD);
		MPI_Send(&pointy[0], 1, MPI_DOUBLE, sub_group, sub_group, MPI_COMM_WORLD);
		/*����ʼ�㴫�͸���4�����������ж��ǲ�����ͬ*/
		MPI_Send(&pointx[n_all - 1], 1, MPI_DOUBLE, 3 * sub_group, 3 * sub_group, MPI_COMM_WORLD);
		MPI_Send(&pointy[n_all - 1], 1, MPI_DOUBLE, 3 * sub_group, 3 * sub_group, MPI_COMM_WORLD);
	}
	if (my_rank == sub_group)
	{
		/*���յ�1�������������͹��������һ����,Ҳ��������ʼ��,���ж��ǲ�����ͬ*/
		MPI_Recv(&tempx[0], 1, MPI_DOUBLE, 0, sub_group, MPI_COMM_WORLD, &status);
		MPI_Recv(&tempy[0], 1, MPI_DOUBLE, 0, sub_group, MPI_COMM_WORLD, &status);
		/*������һ���㴫�͸���3�����������ж��ǲ�����ͬ*/
		MPI_Send(&pointx[0], 1, MPI_DOUBLE, 2 * sub_group, 2 * sub_group, MPI_COMM_WORLD);
		MPI_Send(&pointy[0], 1, MPI_DOUBLE, 2 * sub_group, 2 * sub_group, MPI_COMM_WORLD);
	}
	if (my_rank == 2 * sub_group)
	{

		MPI_Recv(&tempx[0], 1, MPI_DOUBLE, sub_group, 2 * sub_group, MPI_COMM_WORLD, &status);
		MPI_Recv(&tempy[0], 1, MPI_DOUBLE, sub_group, 2 * sub_group, MPI_COMM_WORLD, &status);

		MPI_Send(&pointx[0], 1, MPI_DOUBLE, 3 * sub_group, 3 * sub_group, MPI_COMM_WORLD);
		MPI_Send(&pointy[0], 1, MPI_DOUBLE, 3 * sub_group, 3 * sub_group, MPI_COMM_WORLD);
	}
	if (my_rank == 3 * sub_group)
	{

		MPI_Recv(&tempx[0], 1, MPI_DOUBLE, 2 * sub_group, 3 * sub_group, MPI_COMM_WORLD, &status);
		MPI_Recv(&tempy[0], 1, MPI_DOUBLE, 2 * sub_group, 3 * sub_group, MPI_COMM_WORLD, &status);

		MPI_Recv(&tempx[1], 1, MPI_DOUBLE, 0, 3 * sub_group, MPI_COMM_WORLD, &status);
		MPI_Recv(&tempy[1], 1, MPI_DOUBLE, 0, 3 * sub_group, MPI_COMM_WORLD, &status);
	}

	MPI_Barrier(MPI_COMM_WORLD);

	/*��2����������������*/
	if (my_rank == sub_group)
		output(my_rank);

	MPI_Barrier(MPI_COMM_WORLD);

	/*��3����������������*/
	if (my_rank == 2 * sub_group)
		output(my_rank);

	MPI_Barrier(MPI_COMM_WORLD);

	/*��4����������������*/
	if (my_rank == 3 * sub_group)
		output(my_rank);

	MPI_Barrier(MPI_COMM_WORLD);

	MPI_Finalize();
}
