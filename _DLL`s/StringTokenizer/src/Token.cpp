
#include "Token.h"

char Token::TokenKindS[TOKEN_LAST][255] = {
											"UNKNOWN",
											"_WORD",
											"_INTEGER",
											"_FLOAT",
											"QUOTEDSTRING",
											"WHITESPACE",
											"SYMBOL",
											"CHARACTER",
											"_NON_TERMINAL",
											"_SPECIAL_SEPERATOR",
											"_KEYWORD",
											"_EOL",
											"_EOF",
											"LINE",
											"STATEMENT",
											"COMMENT_SINGLE_LINE",
											"COMMENT_MULTI_LINE",
											"ANY"
										};

Token::Token() {
	this->m_Kind = UNKNOWN;
	this->m_iLine = 0;
	this->m_iColumn = 0;
}

Token::Token(TokenKind kind, char* value, int line, int column) {
	this->m_Kind = kind;
	this->m_iLine = line;
	this->m_iColumn = column;

	int len = strlen(value);

	this->m_Value = new char[len+1];
	memset(this->m_Value, 0, len+1);
	::strcpy(this->m_Value, value);
	this->m_Value[len] = '\0';
}

int Token::getLine() {
	return this->m_iLine;
}

int Token::getColumn() {
	return this->m_iColumn;
}

const char* Token::getValue() {
	return m_Value;
}

TokenKind Token::getKind() {
	return this->m_Kind;
}

const char* Token::getKindS(int iKind) {
	if(iKind >= 0 && iKind < TOKEN_LAST) {
		return TokenKindS[iKind];
	}

	return "UNRECOGNIZED TOKEN";
}

Token::~Token() {
	if(m_Value) {
		delete m_Value;
		m_Value = NULL;
	}
}

