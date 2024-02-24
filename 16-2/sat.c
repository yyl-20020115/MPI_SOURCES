/* FILE : sat.c*/
#include <stdio.h>
#include <stdlib.h>
/*�����й�ʱ��ĺ���*/
#include <time.h>  
/*mpi�⺯��*/
#include "mpi.h"  
#include <math.h>

/*������Ŀ*/
#define NVARS 		100
/*�Ӿ���Ŀ*/
#define NCLAUSES 	3
/*ÿ���Ӿ�ĳ���*/
#define LENGTH_CLAUSE 	3
/*fgetsһ����������ַ���*/
#define	MAXLINE		60
#define	TRUE		1
#define	FALSE		0
/*�Ȳ��ǿ������Ҳ���ǲ��������*/
#define	UNCERTAIN	-1
/*����ֵΪ��*/
#define	NOVALUE		-1
#define	DEPTH		5
/*����pow(x,y)*/
int Pow(int x, int y);
#define	COUNT		Pow(2,DEPTH)
#define	TASKTAG		11 


/**************/
/*��Ҫ���ݽṹ*/
/**************/
/*����*/
struct VARIABLE {
	/*������ֵΪ��,��ֵΪ��,��ֵΪ����*/
	int value;
};
/*����*/
struct LITERAL {
	/*ȡֵΪVAR����VAR��NOVALUE */
/* VARȡֵ�ӣ���NVARS*/
	int value;
};
/*�Ӿ�*/
struct CLAUSE {
	struct LITERAL lit[LENGTH_CLAUSE];
	int num_unassigned;
	/*������״̬ȡֵTRUE��FALSE��UNCERTAIN */
	int state_of_satisfied;
};
/*��ȡ��ʽ*/
struct CNF {
	struct VARIABLE var[NVARS];
	struct CLAUSE clause[NCLAUSES];
	int num_uncertain;
	/*���ȡֵΪTRUE��FALSE��UNCERTAIN*/
	int result;
};


/************/
/*��һ������*/
/************/
/*����һ���Ӿ��Ƿ������*/
int DavisPutnam();
/*������һ����ֵʱ���½ṹ�ãΣ�*/
void UpdateCNF();
/*���ããΣ��Ƿ�ֻ�е�һ�־�*/
int HasUnitClause();
/*���ããΣ��Ƿ��д�����*/
int HasPureLiteral();
/*�������ļ������ݳ�ʼ���ãΣ�*/
void InitCNF();
/*��ʧ�ܼǺŻָ��ãΣ�*/
void Reverse();
/*����һ�������ľ���ֵ*/
int Abs();
/*���ãΣƴ����һ����������*/
void PackCNF();
/*��һ�����������ͷŵ�һ���ãΣ�*/
void UnpackCNF();
/*����������¸ããΣ�*/
void SetValue(); /*�ӣ���ȡ�ã�λ*/
unsigned GetBits(); /*�Ӹ���������Ȼ����㲢���ͽ��*/
void SlaveWork();
/*�ӣãΣ���ѡ����*/
void PickVars();
/*�������Ƿ��ڻ�����*/
int NotInBuf();

/*������*/
int main(int argc, char** argv)
{
	struct CNF  Cnf;  /*��ȡ��ʽCNF*/
	int length = 2 + NVARS + NCLAUSES * (2 + LENGTH_CLAUSE) + 1;  	/*����������*/
	int* buf = (int*)malloc(sizeof(int) * length);  /*LENGHT_CLAUSE*NCLAUSE������*/
	MPI_Status status;  /*MPI--״̬*/
	int i, j, myrank, size, BufResult;
	char* CnfFile;  /*����CNF�ļ���*/
	FILE* in;  /*�����ļ�*/

	MPI_Init(&argc, &argv);  /*MPI--��ʼ��*/
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);  /*MPI--�εں�*/
	MPI_Comm_size(MPI_COMM_WORLD, &size);  /*MPI--��Ŀ*/

	/*root processor read data and initiate Cnf,then pack it to buff*/
	if (myrank == 0)  /*����0(������)*/
	{
		if (argc < 2)  /*��֤����������*/
		{
			printf("You forgot to enter a parametes!\n");  /*������Ŀ����*/
			MPI_Abort(MPI_COMM_WORLD, 99);  /*MPI--�˳�*/
		}
		CnfFile = argv[1];  /*��ȡCNF�ļ���*/
		if ((in = fopen(CnfFile, "rt")) == NULL)	/*read file*/
		{
			printf("cannot open the result file\n");  /*�ļ������ڻ�û����Ӧ������*/
			MPI_Abort(MPI_COMM_WORLD, 99);  /*MPI--�˳� */
		}

		if (myrank == 0) printf("NVARS=%d NCLAUSES=%d LENGTH_CLAUSE=%d\n", NVARS, NCLAUSES, LENGTH_CLAUSE);
		InitCNF(&Cnf, in);  /*�������ļ��ж�ȡ���ݲ���ʼ��CNF*/
		PackCNF(Cnf, buf);  /*��CNF��������ͻ���--����MPI�㲥*/
	}

	/*broadcast buf to all the processors*/
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Bcast(buf, length, MPI_INT, 0, MPI_COMM_WORLD);  /*MPI--�㲥CNF��*/
	MPI_Barrier(MPI_COMM_WORLD);

	/*if it is root processor,distribute the task and wait result from slaves*/
	if (myrank == 0)  /*����0(������)*/
	{
		double start, end;
		start = MPI_Wtime();  /*MPI--ʱ��(��ʼʱ��)*/
		/*send the first bulk of task*/
		for (i = 0; i < size - 1; i++)  /*��ÿһ���ӽ��̶�������Ϣ*/
			MPI_Send(&i, 1, MPI_INT, i + 1, TASKTAG, MPI_COMM_WORLD);  /*����CNF��Ϣ���ӽ���*/
		while (1)  /*һֱִ��*/
		{
			MPI_Recv(&BufResult, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);  /*�Ӵӽ��̽��ս��*/
			if (BufResult == TRUE)  /*SAT�㷨�ɹ� */
			{
				end = MPI_Wtime();  /*MPI--ʱ��(����ʱ��)*/
				i = COUNT + 10;
				printf("\nSatisfied! time%f\n", end - start);  /*����ʱ��*/
				/*send i as the end signal*/
				for (j = 0; j < size - 1; j++)
					MPI_Send(&i, 1, MPI_INT, j + 1, TASKTAG, MPI_COMM_WORLD);  /*��ӽ��̷�����ֹ�ź�*/
				break;  /*�˳�ִ��*/
			}
			else if (i > COUNT)  /*SAT�㷨������ģ*/
			{
				printf("\nUnsatisfied!\n");  /*SAT�㷨ʧ��*/
				/*send i as the terminal signal*/
				for (j = 0; j < size - 1; j++)
					MPI_Send(&i, 1, MPI_INT, j + 1, TASKTAG, MPI_COMM_WORLD);  /*��ӽ��̷�����ֹ�ź�*/
				break;  /*�˳�ִ��*/
			}
			else
			{
				int dest = status.MPI_SOURCE;  /*ȷ����Ϣ����Ŀ�ĵ�*/
				MPI_Send(&i, 1, MPI_INT, dest, TASKTAG, MPI_COMM_WORLD);  /*��ӽ��̷�����ӦCNF��*/
				i++;  /*��һ������*/
			}
		}
	}
	else  /*��������(�ӽ���)*/
	{
		unsigned	BufTask;  /*������*/
		UnpackCNF(&Cnf, buf);  /*�������е�CNF���*/
		while (1)  /*һֱִ��*/
		{
			MPI_Recv(&BufTask, 1, MPI_INT, 0, TASKTAG, MPI_COMM_WORLD, &status);  /*�ӽ���0����CNF����Ϣ*/
			if (BufTask <= COUNT)  /*δԽ��*/
				SlaveWork(Cnf, BufTask, &BufResult);  /*��0���̽�������,����,���������0����*/
			else break;  /*�������*/
		}
	}
	MPI_Finalize();  /*MPI--����*/

	free(buf);
	return(0);
}


/******************************************/
/*�ú����������ļ�����CNF���ݲ���Cnf��ֵ
���룺���һ����CNF���ݵ��ļ�
�������ã��һ��ָ��Cnf�ṹ��ָ��*/
/******************************************/
void InitCNF(struct CNF* pCnf, FILE* in)  /*�������ļ��ж�ȡ���ݲ���ʼ��CNF(����:CNFʽָ��,�����ļ�)*/
{
	char prestr[MAXLINE];  /*�ļ������ַ�����*/
	int i, j, temp;

	for (i = 0; i < NVARS; i++)  /*�ڸ����ı�����Ŀ��Χ(100)��*/
		pCnf->var[i].value = UNCERTAIN;  /*��CNFʽ��������ֵ(-1)*/

	for (i = 0; i < NCLAUSES; i++) {  /*�ڸ������Ӿ���Ŀ��Χ(350)��*/
		for (j = 0; j < LENGTH_CLAUSE; j++) {  /*�ڸ������Ӿ䳤�ȷ�Χ(3)��*/
			int ret = fscanf(in, "%d", &pCnf->clause[i].lit[j].value);  /*��CNFʽ���ָ���ֵ*/
			/* printf("%d ",&ilauses[c].lit[l]); */
		}
		int ret = fscanf(in, "%d", &temp);  /*���������ֵ������ʱ������*/
		/* printf("%d \n",temp); */
		pCnf->clause[i].num_unassigned = LENGTH_CLAUSE;  /*�Ӿ���δ��ֵ��ȡΪ�Ӿ䳤��*/
		pCnf->clause[i].state_of_satisfied = UNCERTAIN;  /*�Ӿ�״̬ȡΪ��ȷ��(-1)*/
	}
	pCnf->num_uncertain = NVARS;  /*CNF��δ��ֵ��ȡΪ���������Ŀ(100)*/
	pCnf->result = UNCERTAIN;  /*CNF���ȡΪ��ȷ��(-1)*/
	fclose(in);  /*�رմ����ļ�*/
}


/******************************************/
/*�ú���������������Ҫ�ĺ���
���룺Cnfһ��CNF�ṹ
�����DavisPutnamȡֵTRUE��FALSE*/
/******************************************/
int DavisPutnam(struct CNF  Cnf)  /*�����Ӿ��Ƿ�ΪSAT(����:CNFʽ)*/
{
	int Serial;/*number of the var to be signed*/  /*����ֵ�ı�����*/
	/*test if this Cnf has had the result*/
	if (Cnf.result == TRUE) return TRUE;  /*���CNF��һ���ֵĽ��ȡΪ��,������*/
	if (Cnf.result == FALSE) return FALSE;  /*���CNF��һ���ֵĽ��ȡΪ��,���ؼ�*/
	/*test if Cnf has unit clause*/
	if (HasUnitClause(Cnf, &Serial) == TRUE)  /*CNF���е��Ӿ�*/
	{
		Cnf.var[Abs(Serial)].value = (Serial > 0) ? TRUE : FALSE;  /*CNF������ֵ(0��1)*/
		printf("\nsingle%d", Serial);
		UpdateCNF(&Cnf, Abs(Serial));  /*Ϊ��������ֵʱ����CNF*/
		if (Cnf.result == TRUE) return TRUE;  /*���CNF��һ���ֵĽ��ȡΪ��,������*/
		if (Cnf.result == FALSE) return FALSE;  /*���CNF��һ���ֵĽ��ȡΪ��,���ؼ�*/
		if (DavisPutnam(Cnf) == TRUE) return TRUE;  /*���CNF��һ���ֲ����Ӿ��Ľ��ȡΪ��,������*/
		else return FALSE;  /*�������,���ؼ�*/
	}
	/*test if Cnf has pure literal*/
	else if (HasPureLiteral(Cnf, &Serial) == TRUE)  /*CNF���д�����*/
	{
		Cnf.var[Serial].value = TRUE;  /*CNF������ֵΪ��(1)*/
		printf("\nPure %d", Serial);
		UpdateCNF(&Cnf, Serial);  /*Ϊ��������ֵʱ����CNF*/
		if (Cnf.result == TRUE) return TRUE;  /*���CNF��һ���ֵĽ��ȡΪ��,������*/
		if (Cnf.result == FALSE) return FALSE;  /*���CNF��һ���ֵĽ��ȡΪ��,���ؼ�*/
		if (DavisPutnam(Cnf) == TRUE) return TRUE;  /*���CNF��һ���ֲ����Ӿ��Ľ��ȡΪ��,������*/
		else return FALSE;  /*�������,���ؼ�*/
	}
	/*pick a var without value*/
	else  /*CNF��ֻ���е��������Ҳ����д�����*/
	{
		for (Serial = 0; Serial < NVARS; Serial++)  /*�ڸ����ı�����Ŀ��Χ(100)��*/
			if (Cnf.var[Serial].value == NOVALUE)  break;  /*CNFʽ�б���û��ֵ,�˳�*/
		printf("\ncommon%d", Serial);
		/*first assume this var is TRUE*/
		Cnf.var[Serial].value = TRUE;  /*CNF������ֵΪ��(1)*/
		UpdateCNF(&Cnf, Serial);  /*Ϊ��������ֵʱ����CNF*/
		if (Cnf.result == TRUE) return TRUE;  /*CNFʽ�б���ֵΪ��(1),������(1)*/
		else if (Cnf.result == UNCERTAIN)  /*CNFʽ�б���ֵ��ȷ��*/
			if (DavisPutnam(Cnf) == TRUE) return TRUE;  /*���CNF��һ���ֲ����Ӿ��Ľ��ȡΪ��,������*/
		/*Else try that #Serial is FALSE*/
		Cnf.var[Serial].value = FALSE;  /*CNF������ֵΪ��(1)*/
		/*update Cnf when #Serial is FALSE*/
		Reverse(&Cnf, Serial);  /*��CNF��ʧ���ź��лָ�����*/
		if (Cnf.result == TRUE) return TRUE;  /*���CNF��һ���ֵĽ��ȡΪ��,������*/
		if (Cnf.result == FALSE) 	return FALSE;  /*���CNF��һ���ֵĽ��ȡΪ��,���ؼ�*/
		if (DavisPutnam(Cnf) == TRUE) return TRUE;  /*���CNF��һ���ֲ����Ӿ��Ľ��ȡΪ��,������*/
		return FALSE;  /*�������,���ؼ�(0)*/
	}
}


/******************************************/
/*�ú������Cnf�Ƿ��е�Ԫ�־�
���룺Cnfһ��CNF�ṹ��Serial�����ı��
�����HasUnitClauseȡֵΪTRUE��FALSE*/
/******************************************/
int HasUnitClause(struct CNF  Cnf, int* Serial)  /*����CNF�Ƿ��е��Ӿ�(����:CNFʽ,ָ�������ָ��)*/
{
	int i, j, k;
	for (i = 0; i < NCLAUSES; i++)  /*�ڸ������Ӿ���Ŀ��Χ(350)��*/
		if (Cnf.clause[i].num_unassigned == 1)  /*�Ӿ���δ��ֵ��Ϊ1*/
		{
			for (j = 0; j < LENGTH_CLAUSE; j++)  /*�ڸ������Ӿ䳤�ȷ�Χ(3)��*/
			{
				k = Cnf.clause[i].lit[j].value;  /*k��ʱ����Ӿ�����Ӧ�����ֵ�ֵ*/
				if (Cnf.var[Abs(k)].value == NOVALUE)  /*CNF����Ӧ����û��ֵ*/
				{
					*Serial = k;  /*��Ӧ������ֵ*/
					return TRUE;  /*������(1)*/
				}
			}
		}
	*Serial = 0;  /*����ָ���ʽ��*/
	return FALSE;  /*�������,���ؼ�(0)*/
}


/******************************************/
/*�ú������Cnf�Ƿ��д�����
���룺Cnfһ��CNF�ṹ��Serial�����ı��
�����HasPureLiteralȡֵTRUE��FALSE*/
/******************************************/
int HasPureLiteral(struct CNF  Cnf, int* Serial)  /*����CNF�Ƿ��д�����(����:CNFʽ,ָ�������ָ��)*/
{
	int i, j, k, in_flag = FALSE;  /*��ǳ�ʼ��Ϊ��*/
	for (i = 0; i < NVARS; i++)  /*�ڸ����ı�����Ŀ��Χ(100)��*/
		if (Cnf.var[i].value == NOVALUE)  /*CNF����Ӧ����û��ֵ*/
		{
			for (j = 0; j < NCLAUSES; j++)  /*�ڸ������Ӿ���Ŀ��Χ(350)��*/
			{
				for (k = 0; k < LENGTH_CLAUSE; k++)  /*�ڸ������Ӿ䳤�ȷ�Χ(3)��*/
					if (Cnf.clause[j].lit[k].value == -i) break;  /*�Ӿ�����Ӧ�����ֵ�ֵΪ��Ӧ�ĸ�ֵ,�˳�*/
					else if (Cnf.clause[j].lit[k].value == i) in_flag = TRUE;  /*�Ӿ�����Ӧ�����ֵ�ֵΪ��Ӧ����ֵ,�����Ϊ��*/
				if (k < LENGTH_CLAUSE) break;  /*�Ӿ䳤��С�������ֵ(3),�˳�ѭ��*/
			}
			if (in_flag && j == NCLAUSES)  /*���Ϊ�����ڸ������Ӿ���Ŀ��Χ(350)��*/
			{
				*Serial = i;  /*��Ӧ������ֵ*/
				return TRUE;  /*������(1)*/
			}
		}
	*Serial = 0;  /*����ָ���ʽ��*/
	return FALSE;  /*�������,���ؼ�(0)*/
}


/******************************************/
/*�ú������ݱ���NO.Serial����Cnf��ֵ
���룺Cnfһ��CNF�ṹ��Serial�������*/
/******************************************/
void UpdateCNF(struct CNF* Cnf, int Serial)  /*Ϊ��������ֵʱ����CNF(����:CNFʽ,���ͱ���)*/
{
	int i, j;
	Cnf->num_uncertain--;  /*CNFʽ��δ��ֵ����1*/
	for (i = 0; i < NCLAUSES; i++)  /*�ڸ������Ӿ���Ŀ��Χ(350)��*/
		for (j = 0; j < LENGTH_CLAUSE; j++)  /*�ڸ������Ӿ䳤�ȷ�Χ(3)��*/
			if (Cnf->clause[i].lit[j].value == Serial || Cnf->clause[i].lit[j].value == -Serial)  /*�Ӿ�����Ӧ�����ֵ�ֵΪ��Ӧ�ı�����ֵ��ֵ*/
			{
				Cnf->clause[i].num_unassigned--;  /*�Ӿ���δ��ֵ����1*/
				if (Cnf->clause[i].lit[j].value == Abs(Serial) && Cnf->var[Abs(Serial)].value == TRUE  /*�Ӿ�����Ӧ�����ֵ�ֵΪ��Ӧ�ı�����ֵ��CNF����Ӧ����Ϊ��(1)*/
					|| Cnf->clause[i].lit[j].value == -Abs(Serial) && Cnf->var[Abs(Serial)].value == FALSE)  /*�Ӿ�����Ӧ�����ֵ�ֵΪ��Ӧ�ı�����ֵ��CNF����Ӧ����Ϊ��(0)*/
				{
					/*printf("this is result %d\n",Cnf->result);*/
					Cnf->clause[i].state_of_satisfied = TRUE;  /*�Ӿ���Ϊ��������*/
				}
				else if (Cnf->clause[i].num_unassigned == 0)  /*�Ӿ���δ��ֵ����Ϊ0*/
					if (Cnf->clause[i].state_of_satisfied == UNCERTAIN)  /*�Ӿ��Ƿ�����������ȷ��*/
					{
						Cnf->clause[i].state_of_satisfied = FALSE;  /*�Ӿ���Ϊ����������*/
						Cnf->result = FALSE;  /*CNF�����Ϊ�٣�0��*/
					}
			}
	for (i = 0; i < NCLAUSES; i++)  /*�ڸ������Ӿ���Ŀ��Χ(350)��*/
		if (Cnf->clause[i].state_of_satisfied == FALSE)  /*�Ӿ䲻��������*/
		{
			Cnf->result = FALSE;  /*CNF�����Ϊ�٣�0��*/
			return;
		}
		else if (Cnf->clause[i].state_of_satisfied == UNCERTAIN)  /*�Ӿ��Ƿ�����������ȷ�� */
		{
			Cnf->result = UNCERTAIN;  /*�Ӿ��Ƿ�����������Ϊ��ȷ��*/
			return;
		}
	Cnf->result = TRUE;  /*CNF�����Ϊ�棨1��*/
}


/******************************************/
/*�����ǽ�һ�������ֵ����һ������ʱ�����Ǳ��뽫Cnfת�õ�������ȡ��״̬
���룺Cnfһ��ָ��CNF�ṹ��ָ�룬Serial�����ı��
�����Cnf*/
/******************************************/
void Reverse(struct CNF* Cnf, int Serial)  /*��CNF��ʧ���ź��лָ�����(����:CNFʽ,���ͱ���)*/
{
	int i, j, k, temp;
	for (i = 0; i < NCLAUSES; i++)  /*�ڸ������Ӿ���Ŀ��Χ(350)��*/
		for (j = 0; j < LENGTH_CLAUSE; j++)  /*�ڸ������Ӿ䳤�ȷ�Χ(3)��*/
			if (Cnf->clause[i].lit[j].value == Serial)  /*�Ӿ�����Ӧ�����ֵ�ֵΪ��Ӧ�ı�����ֵ*/
				/*#Serial is in this clause and it is positive*/
			{
				/*judge if this clause can be satisfied*/
				for (k = 0; k < LENGTH_CLAUSE; k++)  /*�ڸ������Ӿ䳤�ȷ�Χ(3)��*/
					if ((temp = Cnf->clause[i].lit[k].value) == Abs(temp)
						&& Cnf->var[Abs(temp)].value == TRUE  /*�Ӿ�����Ӧ�����ֵ�ֵΪ��Ӧ�ı�����ֵ��CNF����Ӧ����Ϊ��(1)*/
						|| temp == -Abs(temp) && Cnf->var[Abs(temp)].value == FALSE)  /*�Ӿ�����Ӧ�����ֵ�ֵΪ��Ӧ�ı�����ֵ��CNF����Ӧ����Ϊ��(0)*/
						break;
				if (k == LENGTH_CLAUSE)  /*�����Ӿ�ĩβ*/
					/*if k=LENGTH_CLAUSE then this clause can't be satisfied,so the CNF can't be satisfied*/
				{
					Cnf->clause[i].state_of_satisfied = FALSE;  /*�Ӿ��Ƿ�����������Ϊ�٣�0��*/
					Cnf->result = FALSE;  /*CNF�����Ϊ�٣�0��*/
				}
			}
			else if (Cnf->clause[i].lit[j].value == -Serial)/*if*/  /*�Ӿ�����Ӧ�����ֵ�ֵΪ��Ӧ�ı�����ֵ*/
				Cnf->clause[i].state_of_satisfied = TRUE;  /*�Ӿ��Ƿ�����������Ϊ�棨1��*/
	for (i = 0; i < NCLAUSES; i++)  /*�ڸ������Ӿ���Ŀ��Χ(350)��*/
		if (Cnf->clause[i].state_of_satisfied == FALSE)  /*�Ӿ��Ƿ���������Ϊ�٣�0��*/
		{
			Cnf->result = FALSE;  /*CNF�����Ϊ�٣�0��*/
			return;
		}
		else if (Cnf->clause[i].state_of_satisfied == UNCERTAIN)  /*�Ӿ��Ƿ���������Ϊ��ȷ��*/
		{
			Cnf->result = UNCERTAIN;  /*CNF�����Ϊ��ȷ��--�Ժ�ȷ��*/
			return;
		}
	Cnf->result = TRUE;  /*CNF�����Ϊ�棨1��*/
}


/*****************************************/
/*�ú�������һ�������ľ���ֵ*/
/*****************************************/
int Abs(int i)  /*���ر����ľ���ֵ�����������ͱ�����*/
{
	if (i > 0) return i;  /*�������*/
	return -i;  /*�������*/
}


/****************************/
/*�ú�������һ������x��y�η�*/
/****************************/
int Pow(int x, int y)
{
	int i, res = 1;
	for (i = 0; i < y; i++) res = res * x;
	return res;
}


/******************************************/
/*��ΪCNF�ṹ����һ��MPI���ͣ�������Ҫ�㲥����ʱ�����Ǳ��뽫��ѹ����һ����ʽ
���룺Cnfһ��CNF�ṹ
�����һ����������*/
/******************************************/
void PackCNF(struct CNF Cnf, int buf[])  /*��CNF��������ͻ���--����MPI�㲥��������CNFʽ�����ͻ��棩*/
{
	int i = 0, j, k;
	buf[i++] = Cnf.num_uncertain;  /*���ͻ����һ��Ԫ��ΪCNFʽ��ȷ��������*/
	buf[i++] = Cnf.result;  /*���ͻ���ڶ���Ԫ��ΪCNFʽ���*/
	for (j = 0; j < NVARS; j++)  /*�ڸ����ı�����Ŀ��Χ(100)��*/
		buf[i++] = Cnf.var[j].value;  /*�����ͻ�������Ԫ�ظ�CNFʽ��Ӧ������ֵ*/
	for (j = 0; j < NCLAUSES; j++)  /*�ڸ������Ӿ���Ŀ��Χ(350)��*/
	{
		buf[i++] = Cnf.clause[j].num_unassigned;  /*���ͻ����������һ��Ԫ��Ϊ�Ӿ䲻ȷ��������*/
		buf[i++] = Cnf.clause[j].state_of_satisfied;  /*���ͻ���������ڶ���Ԫ��Ϊ�Ӿ�״̬*/
		for (k = 0; k < LENGTH_CLAUSE; k++)  /*�ڸ������Ӿ䳤�ȷ�Χ(3)��*/
			buf[i++] = Cnf.clause[j].lit[k].value;  /*�����ͻ�������Ԫ�ظ��Ӿ���Ӧ���ֵ�ֵ*/
	}
}


/******************************************/
/*�ú�����PackCNF�����ķ�����
���룺bufһ����������
�����pCnfһ��ָ��CNF�ṹ��ָ��*/
/******************************************/
void UnpackCNF(struct CNF* pCnf, int buf[])  /*�������е�CNF�����������CNFʽ�����ͻ��棩*/
{
	int i = 0, j, k;
	pCnf->num_uncertain = buf[i++];  /*CNFʽ��ȷ��������Ϊ���ͻ����һ��Ԫ��*/
	pCnf->result = buf[i++];  /*CNFʽ���Ϊ���ͻ���ڶ���Ԫ��*/
	for (j = 0; j < NVARS; j++)  /*�ڸ����ı�����Ŀ��Χ(100)��*/
		pCnf->var[j].value = buf[i++];  /*��CNFʽ��Ӧ���������ͻ�������Ԫ�ص�ֵ*/
	for (j = 0; j < NCLAUSES; j++)  /*�ڸ������Ӿ���Ŀ��Χ(350)��*/
	{
		pCnf->clause[j].num_unassigned = buf[i++];  /*�Ӿ䲻ȷ��������Ϊ���ͻ����������һ��Ԫ��*/
		pCnf->clause[j].state_of_satisfied = buf[i++];  /*�Ӿ�״̬Ϊ���ͻ���������ڶ���Ԫ��*/
		for (k = 0; k < LENGTH_CLAUSE; k++)  /*�ڸ������Ӿ䳤�ȷ�Χ(3)��*/
			pCnf->clause[j].lit[k].value = buf[i++];  /*���Ӿ���Ӧ���ָ����ͻ�������Ԫ�ص�ֵ*/
	}
}


/******************************************/
/*�ú�����������ת��Ϊ������ֵ
���룺x����
�����pCnfһ��ָ��CNF�ṹ��ָ��*/
/******************************************/
void SetValue(struct CNF* pCnf, unsigned x)  /*��������Ϊ������ֵ������CNF��������ָ��CNFʽ��ָ�룬�޷������ͱ���--��������*/
{
	int i;
	for (i = 0; i < DEPTH; i++)  /*�ڼ�����ȷ�Χ��5����*/
	{
		pCnf->var[i].value = GetBits(x, i);  /*CNFʽ��Ӧ������ֵΪ������*/
		UpdateCNF(pCnf, i);  /*Ϊ��������ֵʱ����CNF*/
		if (pCnf->result == TRUE)  /*CNFʽ���Ϊ��*/
			return;
		else if (pCnf->result == FALSE)  /*CNFʽ���Ϊ��*/
			return;
	}
}


/******************************************/
/*��x�ұ�ȡ��NO.nλ
���룺xȡ��λ��Դ
�����GetBitsȡֵΪ����*/
/******************************************/
unsigned GetBits(unsigned x, int n)  /*�ӱ�����Ӧλ��ȡֵ���������޷������ͱ��������ͱ���--ȡֵλ�ã�*/
{
	return (x >> n) & ~(~0 << 1);  /*�ӱ�����Ӧλ��ȡֵ*/
}


/******************************************/
/*�ú����ɴӽ���ִ��ִ�У����������Ľ���������ͻظ�����
���룺Cnfһ��CNF�ṹ��������*/
/******************************************/
void SlaveWork(struct CNF Cnf, unsigned i, int BufVar[])  /*�ӽ��̵Ĺ���--��0���̽�������,����,���������0����*/
{
	int BufResult;
	SetValue(&Cnf, i); /*,BufVar);*/  /*��������Ϊ������ֵ������CNF*/
	if (DavisPutnam(Cnf) == TRUE)  /*�Ӿ�ΪSAT*/
	{
		BufResult = TRUE;  /*������Ϊ�棨1��*/
		MPI_Send(&BufResult, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);  /*MPI--���ͻ�������0����*/
	}
	else  /*�Ӿ䲻ΪSAT*/
	{
		BufResult = FALSE;  /*������Ϊ�٣�0��*/
		MPI_Send(&BufResult, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);  /*MPI--���ͻ�������0����*/
	}
}


/******************************************/
/*�ú�����cnf��ѡȡ��Ҫ�ı���
���룺Cnfһ��CNF�ṹ
�����BufVar��������*/
/******************************************/
void PickVars(struct CNF Cnf, int BufVar[])  /*��CNF��ѡȡ��Ӧ������������CNFʽ�����ͻ��棩*/
{
	int i, j, k = 0, l = 0, m;
	for (i = 0; i < DEPTH; i++)  /*�ڼ�����ȷ�Χ��5����*/
		if (HasUnitClause(Cnf, &j) == TRUE && NotInBuf(BufVar, Abs(j))) /*CNF���е��Ӿ���û�б����ڻ�����*/
		{
			/*printf("\nsingle!");*/
			BufVar[i] = Abs(j);  /*���ͻ�������Ӧ��ֵΪ�����Ӿ��ֵ*/
		}
		else if (HasPureLiteral(Cnf, &j) == TRUE && NotInBuf(BufVar, Abs(j)))  /*CNF���д�������û�б����ڻ�����*/
		{
			/*printf("\npure!");*/
			BufVar[i] = Abs(j);  /*���ͻ�������Ӧ��ֵΪ�����Ӿ��ֵ*/
		}
		else
		{
			m = Abs(Cnf.clause[k].lit[l].value);  /*��ʱ������ֵΪ�Ӿ�����Ӧ���ֵ�ֵ*/
			while (!NotInBuf(BufVar, m))  /*����ʱ�����������ͻ�����ʱѭ��*/
			{
				if (l > LENGTH_CLAUSE)  /*�����Ӿ䳤�ȷ�Χ��3��*/
				{
					l = 0;
					k++;
				}
				else l++;  /*�Ӿ��������������1*/
				m = Abs(Cnf.clause[k].lit[l].value);  /*��ʱ������ֵΪ�Ӿ�����Ӧ���ֵ�ֵ*/
				/*printf("\nm=%d",m);*/
			}
			BufVar[i] = m;  /*���ͻ�������ӦԪ�ص�ֵ��Ϊ��ʱ������ֵ*/
		}
}


/******************************************/
/*�ú���������I�Ƿ������������
���룺Buf���������Ļ��壬������
�����NotInBufȡֵ����*/
/******************************************/
int NotInBuf(int Buf[], int i)  /*���Ա����Ƿ��ڻ����У����������ͻ��棬���ͱ�����*/
{
	int j;
	for (j = 0; j < DEPTH; j++)  /*�ڼ�����ȷ�Χ��5����*/
		if (i == Buf[j]) return 0;  /*�����ڻ����У�����0--�ж�ʱΪ��*/
	return 1;  /*�������ڻ����У�����1--�ж�ʱΪ��*/
}
