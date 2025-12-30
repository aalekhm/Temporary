
#include "StringTokenizer.h"

void onTokenCallback(int iKind, char* sValue, int iLine, int iColumn);

void main() {
	StringTokenizer* st = new StringTokenizer("temp.ms", true);
	st->setTokenCallback(&onTokenCallback);

	Token* token;
	do {
		token = st->getNextToken();
	}while(token->getKind() != _EOF);
}

void onTokenCallback(int iKind, char* sValue, int iLine, int iColumn) {
	printf("Token ==> %s = %d, value = %s at line %d & column %d\n", Token::getKindS(iKind), sValue, iLine, iColumn);
	//printf("Token ==> %d = %d, value = %s at line %d & column %d\n", iKind, sValue, iLine, iColumn);
}
