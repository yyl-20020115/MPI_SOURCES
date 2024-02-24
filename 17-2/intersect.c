#include <stdio.h>
#include <mpi.h>
//���̺źʹ�������Ŀ
int my_rank, group_size;
//��һ������εĶ˵�
double pointx1[20], pointy1[20];
//�ڶ�������εĶ˵�
double pointx2[20], pointy2[20];
//����еı���
int n_poly, m_poly;
//�жϵĽ��
int result, localresult;
/*
 *������:if_not_parallel
 *����:����ֱ�߲���ֱ�������,�ж��Ƿ��ཻ
 *����:(x1,y1),(x2,y2)���߶�һ�������˵������
 *     (x3,y3),(x4,y4)���߶ζ��������˵������
 *���:��������ֵ�ж������߶��Ƿ��ཻ
 */
int if_intersect_np(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4)
{
	double x;
	x = ((x1 * y2 - x2 * y1) / (x2 - x1) + (x4 * y3 - x3 * y4) / (x4 - x3)) / ((y2 - y1) / (x2 - x1) - (y4 - y3) / (x4 - x3));
	if (((x1 - x) * (x - x2) >= 0) && ((x3 - x) * (x - x4) >= 0))
		return 1;
	else
		return 0;
}
/*
 *������:if_intersect_1v
 *����:�߶�һ�Ǵ�ֱ��,�ж��߶ζ���һ�Ƿ��ཻ
 *����:(x1,y1),(x2,y2)���߶�һ�������˵������
 *     (x3,y3),(x4,y4)���߶ζ��������˵������
 *���:��������ֵ�ж������߶��Ƿ��ཻ
 */
int if_intersect_1v(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4)
{
	double y;
	if (x3 == x4)
	{
		//���߶�ƽ�У�����һ��ֱ����
		if (x3 != x1)
			return 0;
		else if ((y4 < y1) || (y2 < y3))
			return 0;
		else
			return 1;
	}
	//�߶�һ��ֱ,�߶ζ�����ֱ
	else
	{
		//�߶�һ����ֱ�����߶ζ�����ֱ�ߵĽ���������
		y = (y4 - y3) / (x4 - x3) * (x1 - x4) + y4;
		if ((y >= y1) && (y <= y2) && (y <= y4) && (y >= y3) && ((x4 - x1) * (x3 - x1) <= 0))
			return 1;
		else
			return 0;
	}
}
/*
 *������:if_intersect
 *����:�ж��߶�һ(x1,y1),(x2,y2)���߶ζ�(x3,y3),(x4,y4)�Ƿ��ཻ
 *����:(x1,y1),(x2,y2)���߶�һ�������˵������
 *     (x3,y3),(x4,y4)���߶ζ��������˵������
 *���:��������ֵ�ж������߶��Ƿ��ཻ
 */
int if_intersect(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4)
{
	double temp;
	int result;
	//�������Ļ����ڶ�����
	if (y1 > y2)
	{
		temp = x1;
		x1 = x2;
		x2 = temp;
		temp = y1;
		y1 = y2;
		y2 = temp;
	}
	if (y3 > y4)
	{
		temp = x3;
		x3 = x4;
		x4 = temp;
		temp = y3;
		y3 = y4;
		y4 = temp;
	}
	//�߶�һΪ��ֱ�߶�
	if (x1 == x2)
	{
		result = if_intersect_1v(x1, y1, x2, y2, x3, y3, x4, y4);
	}
	else
	{
		/*�߶�һ����ֱ���߶ζ���ֱ*/
		if (x3 == x4)
			//�����߶�һ�߶ζ�λ�ã��ݹ���ñ������ж����߶��Ƿ��ཻ
			result = if_intersect(x3, y3, x4, y4, x1, y1, x2, y2);
		//�߶�һ�߶ζ��Բ���ֱ
		else
		{
			//�����߶��д���ƽ����x���
			if ((y1 == y2) || (y3 == y4))
				//�����߶�һ�߶ζ���x,yλ�ã��ݹ���ñ������ж����߶��Ƿ��ཻ
				result = if_intersect(y1, x1, y2, x2, y3, x3, y4, x4);
			//�߶�һ�߶ζ�ƽ��
			else if ((y4 - y3) * (x2 - x1) == (x4 - x3) * (y2 - y1))
			{
				//���߶�����ֱ����y���ϵĽؾ���ͬ�������߶���ͬһֱ����
				if ((x4 * y3 - x3 * y4) * (x2 - x1) == (x2 * y1 - x1 * y2) * (x4 - x3))
				{
					//���߶���һ��ֱ���ϣ������ཻ
					if ((y4 < y1) || (y2 < y3))
						result = 0;
					//���߶���һ��ֱ���ϣ����ཻ
					else
						result = 1;
				}
				//���߶β���ͬһֱ����
				else
					result = 0;
			}
			//���߶β�ƽ��
			else
			{
				result = if_intersect_np(x1, y1, x2, y2, x3, y3, x4, y4);
			}
		}
	}
	return result;
}
/*
 *������:Broadcast
 *����:����������ε����ݴ�����������
 *����:��
 *���:��
 */
void Broadcast()
{
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Bcast(&n_poly, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&m_poly, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(pointx1, n_poly, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Bcast(pointy1, n_poly, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Bcast(pointx2, m_poly, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Bcast(pointy2, m_poly, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);
}
/*
 *������:Judge()
 *����:�ж�����������Ƿ��ཻ
 *����:��
 *���:��
 */
void Judge()
{
	int i, j, k;
	/*�Եڶ�������ε�ÿ���ߣ�����δ��������
	�����ཻ, ���ж��Ƿ����һ�������
	�ı߼��ཻ, ��Ӧ���㷨17.4���裨3��
	*/
	result = 0;
	localresult = 0;
	for (i = 0; (i < m_poly) && (result == 0); i++)
	{
		/*�Դ����������жϵĵ�һ������ε�
		�߼�*/
		for (j = 0; j < n_poly / group_size + 1; j++)
		{
			/*�ۼӵڶ�������ε�ǰ�����һ
			������α߼��ཻ���жϽ�
			��*/
			if (j * group_size + my_rank < n_poly)
			{
				k = j * group_size + my_rank;
				localresult += if_intersect(pointx1[k], pointy1[k], pointx1[(k + 1) % n_poly], pointy1[(k + 1) % n_poly], pointx2[i], pointy2[i], pointx2[(i + 1) % m_poly], pointy2[(i + 1) % m_poly]);
			}
		}
		/*ͳ�Ƶڶ�����������һ���������
		���ཻ, ��Ӧ���㷨17.4����(4)*/
		MPI_Scan(&localresult, &result, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
	}
}
/*
 *������:GetData
 *����:ȡ�ö���εĸ�������
 *����:��
 *���:��
 */
void GetData()
{
	int i;
	//0�Ŵ�����
	if (my_rank == 0)
	{
		//���������������
		printf("please input first polygon\n");
		printf("please input the count of vertex:");
		int ret = scanf("%d", &n_poly);
		printf("please input the vertex\n");
		for (i = 0; i < n_poly; i++)
		{
			ret = scanf("%lf", &pointx1[i]);
			ret = scanf("%lf", &pointy1[i]);
		}
		printf("please input second polygon\n");
		printf("please input the count of vertex:");
		ret = scanf("%d", &m_poly);
		printf("please input the vertex\n");
		for (i = 0; i < m_poly; i++)
		{
			ret = scanf("%lf", &pointx2[i]);
			ret = scanf("%lf", &pointy2[i]);
		}
	}
}
/*
 *������:main
 *����:�ж�����������Ƿ��ཻ
 *����:argcΪ�����в�������
 *     argvΪÿ�������в�����ɵ��ַ������顣
 *���������0���������������
 */
int main(int argc, char* argv[])
{

	/*�������1:��ʼ��*/
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &group_size);
	/*�������2:�������л�ȡ����*/
	GetData();
	/*�������3:�����ݹ㲥��ÿ��������*/
	Broadcast();
	/*�������4:�ж��Ƿ��ཻ*/
	Judge();
	/*�������5:������*/
	MPI_Barrier(MPI_COMM_WORLD);
	if (my_rank == 0)
	{
		if (result != 0)
			printf("two polygons intersect\n");
		else
			printf("two polygons don't intersect\n");
	}
	MPI_Finalize();
}