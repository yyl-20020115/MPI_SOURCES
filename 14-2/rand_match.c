#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include "mpi.h"

#define PiM 21

/*��ʼ��Fp����*/
int f[2][2][2] = { {{1,0},{1,1}},{{1,1},{0,1}} };
int g[2][2][2];
int pdata[] = { 2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,67,71,79,83 };

/*�����ַ���*/
void gen_string(int stringlen, char* String, int seed)
{
	int i, num;

	srand(seed * 100);
	for (i = 0; i < stringlen; i++) {
		num = rand() % 2;
		String[i] = '0' + num;
	}
	String[stringlen] = '\0';
}

/*������������������ѡȡһ������*/
int drawp(int seed) {
	srand(seed * 100);
	return pdata[rand() % PiM];
}

/*���㷨�е�һ������n�����Ĵ��ĳ��ȣ��ڶ�������m��ģʽ���ĳ��ȡ�(Լ��0<m,n<=128)*/
int main(int argc, char* argv[]) {
	int groupsize = 0, myrank = 0, n = 0, m = 0, p = 0, i = 0, j = 0, h = 0, tmp = 0;
	int textlen = 0;
	char* Text = 0, * Pattern = 0;
	int* MATCH = 0;
	int B[128][2][2], C[128][2][2], D[128][2][2], L[128][2][2];
	int fp_pattern[2][2];
	MPI_Status status;


	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &groupsize);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

	n = atoi(argv[1]);
	m = atoi(argv[2]);

	textlen = n / groupsize;

	if (myrank == groupsize - 1)
		textlen = n - textlen * (groupsize - 1);

	if ((Text = (char*)malloc(textlen * sizeof(char) + 1)) == NULL) {
		printf("no enough memory\n");
		exit(1);
		return 1;
	}

	if ((Pattern = (char*)malloc(m * sizeof(char) + 1)) == NULL) {
		printf("no enough memory\n");
		exit(1);
		return 1;
	}

	if ((MATCH = (int*)malloc(textlen * sizeof(int) + 1)) == NULL) {
		printf("no enough memory\n");
		exit(1);
		return 1;
	}

	/*��ʼ��Fp�������Gp*/
	g[0][0][0] = 1; g[0][0][1] = 0; g[0][1][0] = p - 1; g[0][1][1] = 1;
	g[1][0][0] = 1; g[1][0][1] = p - 1; g[1][1][0] = 0; g[1][1][1] = 1;

	/*�������Ĵ���ģʽ��*/
	gen_string(textlen, Text, m * n * myrank);

	/*���ѡ��һ����������Ӧ�㷨14.9���裨2��*/
	if (myrank == 0) {
		p = drawp(n / m);
		gen_string(m, Pattern, p * m * n);
	}

	/*����ѡ�е�������m������ÿ���ڵ�*/
	MPI_Bcast(&p, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(Pattern, m, MPI_CHAR, 0, MPI_COMM_WORLD);

	printf("one node %d n=%d,m=%d,p=%d\n", myrank, n, m, p);
	printf("one node %d Text=%s\n", myrank, Text);
	printf("one node %d Pattern=%s\n", myrank, Pattern);

	MPI_Barrier(MPI_COMM_WORLD);

	/*����ģʽ����ָ�ƺ���ֵ����Ӧ�㷨14.9���裨3��*/
	if (myrank == 0) {
		for (i = 1; i <= m; i++) {
			B[i][0][0] = f[Pattern[i - 1] - '0'][0][0];
			B[i][0][1] = f[Pattern[i - 1] - '0'][0][1];
			B[i][1][0] = f[Pattern[i - 1] - '0'][1][0];
			B[i][1][1] = f[Pattern[i - 1] - '0'][1][1];
		}

		B[0][0][0] = (B[1][0][0] * B[2][0][0] + B[1][0][1] * B[2][1][0]) % p;
		B[0][0][1] = (B[1][0][0] * B[2][0][1] + B[1][0][1] * B[2][1][1]) % p;
		B[0][1][0] = (B[1][1][0] * B[2][0][0] + B[1][1][1] * B[2][1][0]) % p;
		B[0][1][1] = (B[1][1][0] * B[2][0][1] + B[1][1][1] * B[2][1][1]) % p;

		for (j = 1; j <= m - 2; j++) {
			B[j][0][0] = (B[j - 1][0][0] * B[j + 2][0][0] + B[j - 1][0][1] * B[j + 2][1][0]) % p;
			B[j][0][1] = (B[j - 1][0][0] * B[j + 2][0][1] + B[j - 1][0][1] * B[j + 2][1][1]) % p;
			B[j][1][0] = (B[j - 1][1][0] * B[j + 2][0][0] + B[j - 1][1][1] * B[j + 2][1][0]) % p;
			B[j][1][1] = (B[j - 1][1][0] * B[j + 2][0][1] + B[j - 1][1][1] * B[j + 2][1][1]) % p;
		}

		fp_pattern[0][0] = B[m - 2][0][0];
		fp_pattern[0][1] = B[m - 2][0][1];
		fp_pattern[1][0] = B[m - 2][1][0];
		fp_pattern[1][1] = B[m - 2][1][1];
	}

	MPI_Bcast(fp_pattern, 4, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);

	/*����Ni�����������C��*/
	for (i = 1; i <= textlen; i++) {
		B[i][0][0] = f[Text[i - 1] - '0'][0][0];
		B[i][0][1] = f[Text[i - 1] - '0'][0][1];
		B[i][1][0] = f[Text[i - 1] - '0'][1][0];
		B[i][1][1] = f[Text[i - 1] - '0'][1][1];
	}

	C[1][0][0] = B[1][0][0];
	C[1][0][1] = B[1][0][1];
	C[1][1][0] = B[1][1][0];
	C[1][1][1] = B[1][1][1];

	for (i = 2; i <= textlen; i++) {
		C[i][0][0] = (C[i - 1][0][0] * B[i][0][0] + C[i - 1][0][1] * B[i][1][0]) % p;
		C[i][0][1] = (C[i - 1][0][0] * B[i][0][1] + C[i - 1][0][1] * B[i][1][1]) % p;
		C[i][1][0] = (C[i - 1][1][0] * B[i][0][0] + C[i - 1][1][1] * B[i][1][0]) % p;
		C[i][1][1] = (C[i - 1][1][0] * B[i][0][1] + C[i - 1][1][1] * B[i][1][1]) % p;
	}

	if (groupsize > 1) {
		if (myrank == 0) {
			MPI_Send(&C[textlen], 4, MPI_INT, myrank + 1, myrank, MPI_COMM_WORLD);
		}
		else if (myrank == groupsize - 1) {
			MPI_Recv(&C[0], 4, MPI_INT, myrank - 1, myrank - 1, MPI_COMM_WORLD, &status);
		}
		else {
			MPI_Recv(&C[0], 4, MPI_INT, myrank - 1, myrank - 1, MPI_COMM_WORLD, &status);

			i = textlen;

			C[textlen + 1][0][0] = (C[0][0][0] * C[i][0][0] + C[0][0][1] * C[i][1][0]) % p;
			C[textlen + 1][0][1] = (C[0][0][0] * C[i][0][1] + C[0][0][1] * C[i][1][1]) % p;
			C[textlen + 1][1][0] = (C[0][1][0] * C[i][0][0] + C[0][1][1] * C[i][1][0]) % p;
			C[textlen + 1][1][1] = (C[0][1][0] * C[i][0][1] + C[0][1][1] * C[i][1][1]) % p;

			C[i][0][0] = C[textlen + 1][0][0];
			C[i][0][1] = C[textlen + 1][0][1];
			C[i][1][0] = C[textlen + 1][1][0];
			C[i][1][1] = C[textlen + 1][1][1];

			MPI_Send(&C[textlen], 4, MPI_INT, myrank + 1, myrank, MPI_COMM_WORLD);
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);

	if (myrank != 0) {
		for (i = 1; i < textlen; i++) {
			C[textlen + 1][0][0] = (C[0][0][0] * C[i][0][0] + C[0][0][1] * C[i][1][0]) % p;
			C[textlen + 1][0][1] = (C[0][0][0] * C[i][0][1] + C[0][0][1] * C[i][1][1]) % p;
			C[textlen + 1][1][0] = (C[0][1][0] * C[i][0][0] + C[0][1][1] * C[i][1][0]) % p;
			C[textlen + 1][1][1] = (C[0][1][0] * C[i][0][1] + C[0][1][1] * C[i][1][1]) % p;
			C[i][0][0] = C[textlen + 1][0][0];
			C[i][0][1] = C[textlen + 1][0][1];
			C[i][1][0] = C[textlen + 1][1][0];
			C[i][1][1] = C[textlen + 1][1][1];
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);

	if (groupsize > 1) {
		if (myrank == 0) {
			MPI_Recv(&C[textlen + 1], 4 * (m - 1), MPI_INT, myrank + 1, myrank, MPI_COMM_WORLD, &status);
		}
		else if (myrank == groupsize - 1) {
			MPI_Send(&C[1], 4 * (m - 1), MPI_INT, myrank - 1, myrank - 1, MPI_COMM_WORLD);
		}
		else {
			MPI_Recv(&C[textlen + 1], 4 * (m - 1), MPI_INT, myrank + 1, myrank, MPI_COMM_WORLD, &status);
			MPI_Send(&C[1], 4 * (m - 1), MPI_INT, myrank - 1, myrank - 1, MPI_COMM_WORLD);
		}
	}

	/*����Ri�����������D��*/
	for (i = 1; i <= textlen; i++) {
		B[i][0][0] = g[Text[i - 1] - '0'][0][0];
		B[i][0][1] = g[Text[i - 1] - '0'][0][1];
		B[i][1][0] = g[Text[i - 1] - '0'][1][0];
		B[i][1][1] = g[Text[i - 1] - '0'][1][1];
	}

	D[1][0][0] = B[1][0][0];
	D[1][0][1] = B[1][0][1];
	D[1][1][0] = B[1][1][0];
	D[1][1][1] = B[1][1][1];

	for (i = 2; i <= textlen; i++) {
		D[i][0][0] = (B[i][0][0] * D[i - 1][0][0] + B[i][0][1] * D[i - 1][1][0]) % p;
		D[i][0][1] = (B[i][0][0] * D[i - 1][0][1] + B[i][0][1] * D[i - 1][1][1]) % p;
		D[i][1][0] = (B[i][1][0] * D[i - 1][0][0] + B[i][1][1] * D[i - 1][1][0]) % p;
		D[i][1][1] = (B[i][1][0] * D[i - 1][0][1] + B[i][1][1] * D[i - 1][1][1]) % p;
	}

	if (groupsize > 1) {
		if (myrank == 0) {
			MPI_Send(&D[textlen], 4, MPI_INT, myrank + 1, myrank, MPI_COMM_WORLD);
		}
		else if (myrank == groupsize - 1) {
			MPI_Recv(&D[0], 4, MPI_INT, myrank - 1, myrank - 1, MPI_COMM_WORLD, &status);
		}
		else {
			MPI_Recv(&D[0], 4, MPI_INT, myrank - 1, myrank - 1, MPI_COMM_WORLD, &status);

			i = textlen;
			D[textlen + 1][0][0] = (D[i][0][0] * D[0][0][0] + D[i][0][1] * D[0][1][0]) % p;
			D[textlen + 1][0][1] = (D[i][0][0] * D[0][0][1] + D[i][0][1] * D[0][1][1]) % p;
			D[textlen + 1][1][0] = (D[i][1][0] * D[0][0][0] + D[i][1][1] * D[0][1][0]) % p;
			D[textlen + 1][1][1] = (D[i][1][0] * D[0][0][1] + D[i][1][1] * D[0][1][1]) % p;

			D[i][0][0] = D[textlen + 1][0][0];
			D[i][0][1] = D[textlen + 1][0][1];
			D[i][1][0] = D[textlen + 1][1][0];
			D[i][1][1] = D[textlen + 1][1][1];

			MPI_Send(&D[textlen], 4, MPI_INT, myrank + 1, myrank, MPI_COMM_WORLD);
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);

	if (myrank == 0) {
		D[0][0][0] = 1;
		D[0][0][1] = 0;
		D[0][1][0] = 0;
		D[0][1][1] = 1;
	}

	if (myrank != 0) {
		for (i = 1; i < textlen; i++) {
			D[textlen + 1][0][0] = (D[i][0][0] * D[0][0][0] + D[i][0][1] * D[0][1][0]) % p;
			D[textlen + 1][0][1] = (D[i][0][0] * D[0][0][1] + D[i][0][1] * D[0][1][1]) % p;
			D[textlen + 1][1][0] = (D[i][1][0] * D[0][0][0] + D[i][1][1] * D[0][1][0]) % p;
			D[textlen + 1][1][1] = (D[i][1][0] * D[0][0][1] + D[i][1][1] * D[0][1][1]) % p;

			D[i][0][0] = D[textlen + 1][0][0];
			D[i][0][1] = D[textlen + 1][0][1];
			D[i][1][0] = D[textlen + 1][1][0];
			D[i][1][1] = D[textlen + 1][1][1];
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);

	for (i = 1; i <= textlen; i++)
		MATCH[i] = 0;

	/*�����������ֱ������������ı�����Li�����б��Ƿ����ƥ��λ�ã���Ӧ�㷨14.9�Ĳ��裨4��*/
	if (myrank == groupsize - 1) {
		for (i = 1; i <= textlen - m + 1; i++) {
			L[i][0][0] = (D[i - 1][0][0] * C[i + m - 1][0][0] + D[i - 1][0][1] * C[i + m - 1][1][0]) % p;
			L[i][0][1] = (D[i - 1][0][0] * C[i + m - 1][0][1] + D[i - 1][0][1] * C[i + m - 1][1][1]) % p;
			L[i][1][0] = (D[i - 1][1][0] * C[i + m - 1][0][0] + D[i - 1][1][1] * C[i + m - 1][1][0]) % p;
			L[i][1][1] = (D[i - 1][1][0] * C[i + m - 1][0][1] + D[i - 1][1][1] * C[i + m - 1][1][1]) % p;
			if (((L[i][0][0] + p) % p == fp_pattern[0][0])
				&& ((L[i][0][1] + p) % p == fp_pattern[0][1])
				&& ((L[i][1][0] + p) % p == fp_pattern[1][0])
				&& ((L[i][1][1] + p) % p == fp_pattern[1][1])) {
				MATCH[i] = 1;
				printf("on node %d match pos is %d\n", myrank, i);
			}
		}
	}
	else {
		for (i = 1; i <= textlen; i++) {
			L[i][0][0] = (D[i - 1][0][0] * C[i + m - 1][0][0] + D[i - 1][0][1] * C[i + m - 1][1][0]) % p;
			L[i][0][1] = (D[i - 1][0][0] * C[i + m - 1][0][1] + D[i - 1][0][1] * C[i + m - 1][1][1]) % p;
			L[i][1][0] = (D[i - 1][1][0] * C[i + m - 1][0][0] + D[i - 1][1][1] * C[i + m - 1][1][0]) % p;
			L[i][1][1] = (D[i - 1][1][0] * C[i + m - 1][0][1] + D[i - 1][1][1] * C[i + m - 1][1][1]) % p;
			if (((L[i][0][0] + p) % p == fp_pattern[0][0])
				&& ((L[i][0][1] + p) % p == fp_pattern[0][1])
				&& ((L[i][1][0] + p) % p == fp_pattern[1][0])
				&& ((L[i][1][1] + p) % p == fp_pattern[1][1])) {
				MATCH[i] = 1;
				printf("on node %d match pos is %d\n", myrank, i);
			}
		}
	}

	MPI_Finalize();
	return 0;
}
