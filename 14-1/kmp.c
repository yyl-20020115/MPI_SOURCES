#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <mpi.h>
#define  MAX(m,n)    (m>n?m:n)

typedef struct {
	int pedlen;
	int psuffixlen;
	int pednum;
}pntype;


/*��ģʽ���������ڷ�������������Ӧ��new��newvalֵ*/
void Next(char* W, int patlen, int* nextval, pntype* pped)
{
	int i, j, plen;
	int* next;

	if ((next = (int*)malloc((patlen + 1) * sizeof(int))) == NULL) {
		printf("no enough memory\n");
		exit(1);
		return;
	}

	/*����next��nextval*/
	next[0] = nextval[0] = -1;
	j = 1;
	while (j <= patlen) {
		i = next[j - 1];
		while (i != (-1) && W[i] != W[j - 1]) i = next[i];
		next[j] = i + 1;

		if (j != patlen) {
			if (W[j] != W[i + 1])
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

/*�Ľ���KMP�㷨*/
void kmp(char* T, char* W, int textlen, int patlen, int* nextval, pntype* pped, int prefix_flag, int matched_num, int* match, int* prefixlen)
{
	int i, j;

	i = matched_num;
	j = matched_num;

	while (i < textlen)
	{
		if ((prefix_flag == 1) && ((patlen - j) > (textlen - i))) {
			break;
		}

		while (j != (-1) && W[j] != T[i])  j = nextval[j];

		if (j == (patlen - 1)) {
			match[i - (patlen - 1)] = 1;
			if (pped->pednum + pped->psuffixlen == 1)
				j = -1;
			else
				j = patlen - 1 - pped->pedlen;
		}
		j++;
		i++;
	}
	(*prefixlen) = j;
}

/*�ع�ģʽ���Լ�next����*/
void Rebuild_info(int patlen, pntype* pped, int* nextval, char* W)
{
	int i;
	if (pped->pednum == 1)
		memcpy(W + pped->pedlen, W, pped->psuffixlen);
	else {
		memcpy(W + pped->pedlen, W, pped->pedlen);
		for (i = 3; i <= pped->pednum; i++) {
			memcpy(W + (i - 1) * pped->pedlen, W, pped->pedlen);
			memcpy(nextval + (i - 1) * pped->pedlen, nextval + pped->pedlen, pped->pedlen * sizeof(int));
		}

		if (pped->psuffixlen != 0) {
			memcpy(W + (i - 1) * pped->pedlen, W, pped->psuffixlen);
			memcpy(nextval + (i - 1) * pped->pedlen, nextval + pped->pedlen, pped->psuffixlen * sizeof(int));
		}
	}
}

/*�����ı���*/
void gen_string(int strlen, int pedlen, char* string, int seed)
{
	int suffixlen, num, i, j;

	srand(seed);
	for (i = 0; i < pedlen; i++) {
		num = rand() % 26;
		string[i] = 'a' + num;
	}
	for (j = 1; j < (int)(strlen / pedlen); j++)
		strncpy(string + j * pedlen, string, pedlen);
	if ((suffixlen = strlen % pedlen) != 0)
		strncpy(string + j * pedlen, string, suffixlen);
}

/*���ļ�����ģʽ����Ϣ*/
void GetFile(char* filename, char** place, int* number)
{
	FILE* fp = 0;
	struct stat statbuf = { 0 };

	if ((fp = fopen(filename, "rb")) == NULL) {
		printf("Error open file %s\n", filename);
		exit(0);
		return;
	}

	fstat(fileno(fp), &statbuf);
	if (((*place) = (char*)malloc(sizeof(char) * statbuf.st_size)) == 0) {
		printf("Error alloc memory\n");
		exit(1);
		return;
	}

	if (fread((*place), 1, statbuf.st_size, fp) != statbuf.st_size) {
		printf("Error in reading num\n");
		exit(0);
		return;
	}
	fclose(fp);
	(*number) = statbuf.st_size;
}

/*��ӡ���в�����Ϣ*/
void PrintFile_info(char* filename, char* T, int id)
{
	FILE* fp = 0;
	int i = 0;

	if ((fp = fopen(filename, "a")) == NULL) {
		printf("Error open file %s\n", filename);
		exit(0);
		return;
	}

	fprintf(fp, "The Text on node %d is %s .\n", id, T);

	fclose(fp);
}


/*��ӡƥ����*/
void PrintFile_res(char* filename, int* t, int len, int init, int id)
{
	FILE* fp = 0;
	int i = 0;

	if ((fp = fopen(filename, "a")) == NULL) {
		printf("Error open file %s\n", filename);
		exit(0);
		return;
	}

	fprintf(fp, "This is the match result on node %d \n", id);
	for (i = 0; i <= len - 1; i++)
		if (t[i] == 1)
			fprintf(fp, "(%d)  +\n", i + init);
		else
			fprintf(fp, "(%d)  -\n", i + init);
	fclose(fp);
}

void main(int argc, char* argv[])
{
	char* T = 0, * W = 0;
	int* nextval = 0, * match = 0;
	int	textlen = 0, patlen = 0, pedlen = 0, nextlen_send = 0;
	pntype pped = { 0 };
	int	i = 0, myid = 0, numprocs = 0, prefixlen = 0, ready = 0;
	MPI_Status  status;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);

	nextlen_send = 0;
	ready = 1;
	/*�����ı�������*/
	textlen = atoi(argv[1]);
	textlen = textlen / numprocs;
	pedlen = atoi(argv[2]);
	if ((T = (char*)malloc(textlen * sizeof(char))) == NULL) {
		printf("no enough memory\n");
		exit(1);
	}
	gen_string(textlen, pedlen, T, myid);

	if (myid == 0) {
		PrintFile_info("match_result", T, myid);
		if (numprocs > 1)
			MPI_Send(&ready, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
	}
	else {
		MPI_Recv(&ready, 1, MPI_INT, myid - 1, myid - 1, MPI_COMM_WORLD, &status);
		PrintFile_info("match_result", T, myid);
		if (myid != numprocs - 1)
			MPI_Send(&ready, 1, MPI_INT, myid + 1, myid, MPI_COMM_WORLD);
	}

	printf("\n");

	if ((match = (int*)malloc(textlen * sizeof(int))) == NULL) {
		printf("no enough memory\n");
		exit(1);
	}

	/*������0����ģʽ��������¼���в���*/
	if (myid == 0) {
		printf("processor num = %d \n", numprocs);
		printf("textlen = %d\n", textlen * numprocs);

		GetFile("pattern.dat", &W, &patlen);
		printf("patlen= %d\n", patlen);

		if ((nextval = (int*)malloc(patlen * sizeof(int))) == NULL) {
			printf("no enough memory\n");
			exit(1);
		}
		/*��ģʽ�����з�������Ӧ���㷨14.6���裨1��*/
		Next(W, patlen, nextval, &pped);
		if (numprocs > 1) {
			if (pped.pednum == 1)
				nextlen_send = patlen;
			else
				nextlen_send = pped.pedlen * 2;
		}
	}

	/*���������������ģʽ������Ϣ����Ӧ���㷨14.6���裨2��*/
	if (numprocs > 1) {
		MPI_Bcast(&patlen, 1, MPI_INT, 0, MPI_COMM_WORLD);
		if (myid != 0)
			if (((nextval = (int*)malloc(patlen * sizeof(int))) == NULL)
				|| ((W = (char*)malloc(patlen * sizeof(char))) == NULL)) {
				printf("no enough memory\n");
				exit(1);
			}

		MPI_Barrier(MPI_COMM_WORLD);
		MPI_Bcast(&pped, 3, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&nextlen_send, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(nextval, nextlen_send, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(W, pped.pedlen, MPI_CHAR, 0, MPI_COMM_WORLD);
	}

	MPI_Barrier(MPI_COMM_WORLD);

	/*�����޸Ĺ���KMP�㷨���оֲ���ƥ�䣬��Ӧ���㷨14.6���裨3��*/
	if (numprocs == 1) {
		kmp(T, W, textlen, patlen, nextval, &pped, 1, 0, match + patlen - 1, &prefixlen);
	}
	else {
		if (myid != 0)
			/*�����������ֱ���ݲ��ִ������Լ�������Ϣ�ع�ģʽ��*/
			Rebuild_info(patlen, &pped, nextval, W);
		if (myid != numprocs - 1)
			kmp(T, W, textlen, patlen, nextval, &pped, 0, 0, match + patlen - 1, &prefixlen);
		else
			kmp(T, W, textlen, patlen, nextval, &pped, 1, 0, match + patlen - 1, &prefixlen);

		MPI_Barrier(MPI_COMM_WORLD);

		/*�������������жμ�ƥ�䣬��Ӧ���㷨14.6���裨4��*/
		if (myid < numprocs - 1)
			MPI_Send(&prefixlen, 1, MPI_INT, myid + 1, 99, MPI_COMM_WORLD);

		if (myid > 0)
			MPI_Recv(&prefixlen, 1, MPI_INT, myid - 1, 99, MPI_COMM_WORLD, &status);

		MPI_Barrier(MPI_COMM_WORLD);

		if ((myid > 0) && (prefixlen != 0))
			kmp(T - prefixlen, W, prefixlen + patlen - 1, patlen, nextval, &pped, 1, prefixlen, match + patlen - 1 - prefixlen, &prefixlen);

		MPI_Barrier(MPI_COMM_WORLD);
	}

	/*���ƥ����*/
	if (myid == 0) {
		PrintFile_res("match_result", match + patlen - 1, textlen - patlen + 1, 0, myid);
		if (numprocs > 1)
			MPI_Send(&ready, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
	}
	else {
		MPI_Recv(&ready, 1, MPI_INT, myid - 1, myid - 1, MPI_COMM_WORLD, &status);
		PrintFile_res("match_result", match, textlen, myid * textlen - patlen + 1, myid);
		if (myid != numprocs - 1)
			MPI_Send(&ready, 1, MPI_INT, myid + 1, myid, MPI_COMM_WORLD);
	}

	free(T);
	free(W);
	free(nextval);
	MPI_Finalize();
}
