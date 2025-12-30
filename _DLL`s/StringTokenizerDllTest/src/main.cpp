
#include "StringTokenizer.h"
#include "Token.h"
#include "CCString.h"

void onTokenCallback(int iKind, char* sValue, int iLine, int iColumn);

void main(char** argv, int argc) {

	StringTokenizer* st = new StringTokenizer("temp.ms", true);
	st->setTokenCallback(&onTokenCallback);

	Token* token;
	do {
		token = st->getNextToken();
	}while(token->getKind() != _EOF);
	
	CCString str;
	str = "This is a", " concatenated string.";
	printf("dddddddddddd %s of length %d\n", str.c_str(), str.length());
	printf("dddddddddddd %s", CCString::replace(str.c_str(), 'a', '.'));
}

void onTokenCallback(int iKind, char* sValue, int iLine, int iColumn) {
	printf("Token ==> kind = %d, value = %s at line %d & column %d\n", iKind, sValue, iLine, iColumn);
}
