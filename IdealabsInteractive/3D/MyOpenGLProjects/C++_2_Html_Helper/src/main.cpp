
#include <stdio.h>
#include <string.h>

void readLine(FILE* fin, char* str) {
	do {
		fgets(str, 255, fin);
	}while(str[0] == '/' || str[0] == '\n' || str[0] == '\r');

	int len = strlen(str);
	str[len-2] = '\0';
}

void main(int argc, char* args) {
	
	FILE* fin = fopen("list_Of_CC++H_Files.bat", "rb");
	FILE* fout = fopen("convertList.bat", "wb");

	char singleLine[255] = "";
	char writeStr[255] = {0};
	char c = 'A';

	readLine(fin, singleLine);

	while(!feof(fin)) {
		printf("%s", singleLine);
		sprintf(writeStr, "cpphtml.exe %s > %s.html\r\n", singleLine, singleLine);
		fputs(writeStr, fout);
		
		readLine(fin, singleLine);
	}

	fclose(fin);
	fclose(fout);
}