#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <mpi.h>

typedef struct {
	int pedlen;
	int pednum;
	int psuffixlen;
}pntype;

/*
void gen_string(int stringlen,char *string,int seed)
���룺�����ɵ��ı�������: int stringlen;

�������ƥ����ı���: char *string;

���ܣ����ɴ�ƥ����ı���

void gen_pattern(char *text,int *patlen,char *pattern,int seed)

���룺��ƥ���ı� char *text;

	  �����ɵ�ƥ�䴮����: int *patlen;

��������ɵ�ƥ�䴮: char *pattern;

���ܣ����ı�����ѡ��������һ����Ϊ��ƥ���ģʽ��

int Min(int x,int y,int z)
���룺���������� int x,int y,int z;

���������������ѡ����С��һ�� int Min();

���ܣ�������������ѡ����С��һ��

void ModifiedDP(char *T,char *P,int textlen,int patternlen,int k,int *D)
���룺��ƥ����ı���: char *T;
	  ƥ�䴮:char *P;
	  �ı�����:int textlen;
	  ƥ�䴮��:int patternlen

�����ģʽ��P��ǰ׺�Ӵ�p1��pi���ı���T������
	  ��tj��β���Ӵ�֮�����С������������С�༭���룩int *D;

���ܣ�����λ�����Ķ�̬�滮���ƴ�ƥ���㷨

void Next(char *P,int patlen,int *nextval,pntype *pped)
���룺ƥ�䴮 char *P;
	  ƥ�䴮��: int patlen;

�����int *nextval;

���ܣ������봮�������ڷ���������next�Լ�nextvalֵ

void Search(char *T,char *P,int textlen,int patlen,int *nextval,
pntype *pped,int k,int *D,int SubpatternStart,char *pattern,int patternlen,int *VerifyCount)
���룺ԭ�ı���: char *T;
	  ƥ�䴮: char *P;
	  ԭ�ı�����: int textlen;
	  ƥ�䴮��: int patlen;
	  ����ƥ���������������: int k ;
	  ��С�༭����: int *D;
�������ȷƥ���
���ܣ�����λ�����Ĺ��˽��ƴ�ƥ���㷨

int SubPatternLength(int patlen,int k,int num)
���룺 ƥ�䴮��: int patlen;
	   ����ƥ���������������: int k ;
�������ģʽ����
���ܣ���ģʽ������Ϊ2k+1����ģʽ��

*/
/*���ɴ�ƥ����ı���*/
void gen_string(int stringlen, char* string, int seed)
{
	int i, num;

	srand(seed * 100);
	for (i = 0; i < stringlen; i++) {
		num = rand() % 26;
		string[i] = 'a' + num;
	}
	string[stringlen] = '\0';
}

/*���ı�����ѡ��������һ����Ϊ��ƥ���ģʽ��*/
void gen_pattern(char* text, int* patlen, char** pattern, int seed)
{
	int start;

	if ((*pattern = (char*)malloc((*patlen + 1) * sizeof(char))) == NULL) {
		printf("Error alloc memory.\n");
		exit(1);
	}
	srand(seed * 100);
	start = rand() % strlen(text) / 3;
	strncpy(*pattern, text + start, *patlen);
	*(*pattern + *patlen) = '\0';
}

/*������������ѡ����С��һ��*/
int Min(int x, int y, int z)
{
	int minimum;

	minimum = (x < y ? x : y);
	minimum = (minimum < z ? minimum : z);
	return minimum;
}

/*����λ�����Ķ�̬�滮���ƴ�ƥ���㷨*/
void ModifiedDP(char* T, char* P, int textlen, int patternlen, int k, int* D)
{
	int i, j;
	int* D1, * D2;

	if (((D1 = (int*)malloc((textlen + 1) * sizeof(int))) == NULL) ||
		((D2 = (int*)malloc((textlen + 1) * sizeof(int))) == NULL)) {
		printf("Malloc error.\n");
		exit(1);
	}
	for (j = 0; j <= textlen; j++) {
		D1[j] = 0;
		D2[j] = 0;
	}
	for (i = 1; i <= patternlen; i++) {
		D[0] = i;
		for (j = 1; j <= textlen; j++) {
			if (P[i - 1] == T[j - 1])
				D[j] = Min(D2[j] + 1, D[j - 1] + 1, D2[j - 1]);
			else if (i - 2 >= 0 && j - 2 >= 0 && P[i - 1] == T[j - 2] && P[i - 2] == T[j - 1])
				D[j] = Min(D2[j] + 1, D[j - 1] + 1, D1[j - 2] + 1);
			else
				D[j] = Min(D2[j] + 1, D[j - 1] + 1, D2[j - 1] + 1);
		}
		for (j = 0; j <= textlen; j++) {
			D1[j] = D2[j];
			D2[j] = D[j];
		}
	}
	free(D1);
	free(D2);
}

/*�����봮�������ڷ���������next�Լ�nextvalֵ*/
void Next(char* P, int patlen, int* nextval, pntype* pped)
{
	int i, j;
	int* next;

	if ((next = (int*)malloc((patlen + 1) * sizeof(int))) == NULL) {
		printf("No enough memory.\n");
		exit(1);
	}
	next[0] = nextval[0] = -1;
	j = 1;
	while (j <= patlen) {
		i = next[j - 1];
		while (i != -1 && P[i] != P[j - 1])
			i = next[i];
		next[j] = i + 1;
		if (j != patlen) {
			if (P[j] != P[i + 1])
				nextval[j] = i + 1;
			else
				nextval[j] = nextval[i + 1];
		}
		j++;
	}
	pped->pedlen = patlen - next[patlen];
	pped->pednum = (int)(patlen / pped->pedlen);
	pped->psuffixlen = patlen % pped->pedlen;
	free(next);
}

/*����λ�����Ĺ��˽��ƴ�ƥ���㷨*/
void Search(char* T, char* P, int textlen, int patlen, int* nextval, pntype* pped, int k, int* D, int SubpatternStart, char* pattern, int patternlen, int* VerifyCount)
{
	int i, j, p;
	int VerifyStart, VerifyEnd, VerifyLength;
	int* TempD;

	if ((TempD = (int*)malloc((patternlen + 2 * k + 1) * sizeof(int))) == NULL) {
		printf("Malloc error.\n");
		exit(1);
	}
	i = 0;
	j = 0;
	while (i < textlen) {
		while ((j != -1) && (P[j] != T[i]))
			j = nextval[j];
		if (j == (patlen - 1)) {
			(*VerifyCount)++;
			VerifyStart = i - patlen - SubpatternStart - k + 1;
			VerifyEnd = i - patlen - SubpatternStart + patternlen + k;
			VerifyStart = (VerifyStart > 0 ? VerifyStart : 0);
			VerifyEnd = (VerifyEnd < textlen - 1 ? VerifyEnd : textlen - 1);
			VerifyLength = VerifyEnd - VerifyStart + 1;
			ModifiedDP(T + VerifyStart, pattern, VerifyLength, patternlen, k, TempD);
			for (p = 1; p <= VerifyLength; p++)
				D[VerifyStart + p] = (D[VerifyStart + p] < TempD[p] ? D[VerifyStart + p] : TempD[p]);

			if (pped->pednum + pped->psuffixlen == 1)
				j = -1;
			else
				j = patlen - 1 - pped->pedlen;
		}
		j++;
		i++;
	}
	free(TempD);
}


/*��ģʽ������Ϊ2k+1����ģʽ��*/
int SubPatternLength(int patlen, int k, int num)
{
	int r;

	r = patlen % (2 * k + 1);
	if (r == 0)
		return(patlen / (2 * k + 1));
	else if (num <= r)
		return(patlen / (2 * k + 1) + 1);
	else
		return(patlen / (2 * k + 1));
}

int main(int argc, char* argv[])
{
	char* T, * P;
	int alltextlen, textlen, patternlen, k, subpatlen, startpoint;
	int* nextval, * D;
	pntype pped;
	int i, l, myrank, groupsize;
	int VerifyCount = 0;
	int TotalMatchCount = 0, MatchCount = 0;
	MPI_Status status;
	char ch[1];

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &groupsize);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

	alltextlen = atoi(argv[1]);
	patternlen = atoi(argv[2]);
	k = atoi(argv[3]);
	textlen = alltextlen / groupsize;

	if (myrank == 0) {
		if (((T = (char*)malloc((textlen + patternlen + k) * sizeof(char))) == NULL) ||
			((D = (int*)malloc((textlen + patternlen + k) * sizeof(int))) == NULL)) {
			printf("No enough memory.\n");
			exit(1);
		}

		for (i = 0; i <= textlen; i++) D[i] = 10000;

		/*���ɱ����ı�����ģʽ��*/
		gen_string(textlen, T + patternlen + k - 1, myrank);
		gen_pattern(T + patternlen + k - 1, &patternlen, &P, myrank);

		if ((nextval = (int*)malloc((patternlen / (2 * k + 1) + 1) * sizeof(int))) == NULL) {
			printf("No enough memory.\n");
			exit(1);
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);

	if (groupsize > 1) {
		if (myrank != 0) {
			if (myrank == groupsize - 1)
				textlen = alltextlen - (groupsize - 1) * (alltextlen / groupsize);

			if (((T = (char*)malloc((textlen + patternlen + k) * sizeof(char))) == NULL) ||
				((D = (int*)malloc((textlen + patternlen + k) * sizeof(int))) == NULL) ||
				((nextval = (int*)malloc((patternlen / (2 * k + 1) + 1) * sizeof(int))) == NULL) ||
				((P = (char*)malloc((patternlen + 1) * sizeof(char))) == NULL)) {
				printf("No enough memory.\n");
				exit(1);
			}

			for (i = 0; i <= textlen + patternlen + k - 1; i++) D[i] = 10000;
			/*���ɱ����ı���*/
			gen_string(textlen, T + patternlen + k - 1, myrank);
		}

		MPI_Barrier(MPI_COMM_WORLD);

		/*����ģʽ�������д���������Ӧ�㷨14.13���裨2��*/
		MPI_Bcast(P, patternlen + 1, MPI_CHAR, 0, MPI_COMM_WORLD);

		printf("on node %d the text is %s \n", myrank, T + patternlen + k - 1);
		printf("on node %d the pattern is %s \n", myrank, P);

		/*���ͽ��̼�������ݸ����ڽ��̣���Ӧ�㷨14.13���裨3���ͣ�4��*/
		if (myrank == 0) {
			MPI_Send(&T[textlen], patternlen + k - 1, MPI_CHAR, myrank + 1, myrank, MPI_COMM_WORLD);
		}
		else if (myrank == groupsize - 1) {
			MPI_Recv(&T[0], patternlen + k - 1, MPI_CHAR, myrank - 1, myrank - 1, MPI_COMM_WORLD, &status);
		}
		else {
			MPI_Recv(T, patternlen + k - 1, MPI_CHAR, myrank - 1, myrank - 1, MPI_COMM_WORLD, &status);
			MPI_Send(T + textlen, patternlen + k - 1, MPI_CHAR, myrank + 1, myrank, MPI_COMM_WORLD);
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);

	/*�����̷ֱ�Ա�������ִ������λ�����Ĺ��˽��ƴ�ƥ���㷨����Ӧ�㷨14.13���裨5��*/
	startpoint = 0;
	if (myrank == 0)
		for (l = 1; l <= 2 * k + 1; l++) {
			subpatlen = SubPatternLength(patternlen, k, l);
			if (subpatlen == 0) continue;
			Next(P + startpoint, subpatlen, nextval, &pped);
			Search(T + patternlen + k - 1, P + startpoint, textlen, subpatlen, nextval, &pped, k, D, startpoint, P, patternlen, &VerifyCount);
			startpoint = startpoint + subpatlen;
		}
	else
		for (l = 1; l <= 2 * k + 1; l++) {
			subpatlen = SubPatternLength(patternlen, k, l);
			if (subpatlen == 0) continue;
			Next(P + startpoint, subpatlen, nextval, &pped);
			Search(T, P + startpoint, textlen + patternlen + k - 1, subpatlen, nextval, &pped, k, D, startpoint, P, patternlen, &VerifyCount);
			startpoint = startpoint + subpatlen;
		}

	MPI_Barrier(MPI_COMM_WORLD);

	if (myrank == 0)
		for (i = 1; i <= textlen; i++) {
			if (D[i] >= 0 && D[i] <= k)
				MatchCount++;
		}
	else
		for (i = patternlen + k; i <= textlen + patternlen + k - 1; i++) {
			if (D[i] >= 0 && D[i] <= k)
				MatchCount++;
		}

	MPI_Barrier(MPI_COMM_WORLD);
	printf("Total %d matched on node %d \n", MatchCount, myrank);

	free(T);
	free(P);
	free(D);
	free(nextval);
	MPI_Finalize();

	return;
}

