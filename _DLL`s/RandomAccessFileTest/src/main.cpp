
#include <stdlib.h>
#include "RandomAccessFile.h"
#include "StringTokenizer.h"
#include "Token.h"

void onTokenCallback(int iKind, char* sValue, int iLine, int iColumn);

void main(int argc, char* argv[]) {
	
	if(argc < 2) {
		printf("********************************************************\n");
		printf("Usage:\n");
		printf("RandomAccessFile filename.ext:\n");
		printf("********************************************************\n");
	}

	RandomAccessFile* raf = new RandomAccessFile();
	bool bCanRead = raf->openForRead(argv[1]);
	if(bCanRead) {
		
		unsigned long long lFileSize = raf->length();
		printf("\tfile Size = %d\n", lFileSize);

		char* buf = new char[lFileSize];
		int i = 0;
		while( i < lFileSize) {
			buf[i] = '.';
			i++;
		}

		int iBytesRead = raf->read(buf);
		printf("\tBytes read = %d\n", iBytesRead);

		StringTokenizer* strTok = new StringTokenizer("Default");
		strTok->m_SymbolChars = "!@#$%^&*()_-+={}[]|\;':<>,.?/~`\"";
		strTok->setTokenCallback(&onTokenCallback);
		strTok->skipWhiteSpaces(true);
		
		strTok->setData(buf);
		Token* token;

		do {
			token = strTok->getNextToken();
		}while(token->getKind() != _EOF);

	}
	else {
		printf("Could not open file: %s\n", argv[1]);
	}

	raf->close();
}

void onTokenCallback(int iKind, char* sValue, int iLine, int iColumn) {
	
	//printf("Token ==> kind = %d, value = %s at line %d & column %d\n", iKind, sValue, iLine, iColumn);
	printf("tokVal ==> %s\n", sValue);
}