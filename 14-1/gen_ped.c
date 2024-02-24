#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

int main(int argc, char* argv[])
{
	int strlen = 0, pedlen = 0, suffixlen = 0, num = 0, i = 0, j = 0;
	char* string = 0;
	FILE* fp = 0;

	strlen = atoi(argv[1]);
	pedlen = atoi(argv[2]);
	srand(atoi(argv[3]));

	string = (char*)malloc(strlen * sizeof(char));
	if (string == NULL) {
		printf("malloc error\n");
		return 1;
	}

	for (i = 0; i < pedlen; i++) {
		num = rand() % 26;
		string[i] = 'a' + num;
	}

	for (j = 1; j < (int)(strlen / pedlen); j++)
		strncpy(string + j * pedlen, string, pedlen);

	if ((suffixlen = strlen % pedlen) != 0)
		strncpy(string + j * pedlen, string, suffixlen);

	if ((fp = fopen(argv[4], "w")) != NULL) {
		fprintf(fp, "%s", string);
		fclose(fp);
	}
	else {
		printf("file open error\n");
		return 1;
	}

	return 0;
}
