
#include "StringTokenizer.h"
#include "RandomAccessFile.h"
#include "Token.h"
#include <windows.h>

///*
StringTokenizer::StringTokenizer(const char* data) {
	this->clear();
	
	m_iDataLength = strlen(data);
	m_Data = new char[m_iDataLength+1];
	memset(m_Data, 0, m_iDataLength+1);
	strncpy(m_Data, data, m_iDataLength+1);
	m_Data[m_iDataLength] = '\0';	
}
//*/

StringTokenizer::StringTokenizer(const char* sFileNameWithPath, bool isFile) {
	//printf("In StringTokenizer Constructor\n");
	this->clear();

	RandomAccessFile* rafIn = new RandomAccessFile();
	bool goAhead = rafIn->openForRW(sFileNameWithPath);

	if(goAhead) {
		m_iDataLength = rafIn->getFileLength();
		m_Data = new char[m_iDataLength+1];
		::memset(m_Data, 1, m_iDataLength);
		m_Data[m_iDataLength] = '\0';

		int bytesRead = rafIn->read(m_Data, 0, m_iDataLength);
		m_Data[bytesRead] = '\0';
		rafIn->close();
		
		//printf("%s\n", m_Data);
	}
}

char StringTokenizer::peek(int count) {
	if(m_iPos + count >= m_iDataLength)
		return END_OF_FILE;
	else
		return m_Data[m_iPos + count];
}

Token* StringTokenizer::getNextToken() {
//printf("initRead\n");
	initRead();
//printf("while\n");
	while(true) {
		char ch = peek(0);
//printf("--ch = %c\n", ch);
		if(bUseExplicitSymbols) {
			if(ch == END_OF_FILE) {
				return createToken(_EOF, "");
			}
			else
			if(isSymbol(ch)) {
				
				createToken(SYMBOL);
				consume(1);
				
				initRead();
			}
			else
				consume(1);
		}
		else {
//printf("ch = %c\n", ch);
			if(isSpecialSeperator(ch))
				return createToken(_SPECIAL_SEPERATOR);
			else {
				switch(ch) {
					case END_OF_FILE:
						return createToken(_EOF, "");
					break;
					case ' ':
					case '\t':
						if(!bSkipWhiteSpaces)
							return readWhiteSpace();
						else
							consume(1);
					break;
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
						return readNumber();
					break;
					case '\r':
							initRead();
							consume(1);//Skip

							if(peek(0) == '\n')
								consume(1);

							m_iLine++;
							m_iColumn = 1;

							return createToken(_EOL);
					break;
					case '\n':
							initRead();
							m_iLine++;
							m_iColumn = 1;

							return createToken(_EOL);
					break;
					case '"':
						return readString();
					break;
					case '\'':
						return readCharacter();
					break;
					/*
					case ':':
						if(peek(1) == ':' && peek(2) == '=') {
							initRead();

							consume(1);
							consume(1);
							consume(1);
							return createToken(_GRAMMER_ASSIGNMENT);
						}
						else
							return checkDefault(ch);
					break;
					*/
					default:
						//if(!isSpecialSeperator(ch)) {
							return checkDefault(ch);
						//}
						//else {
							//printf("asdasdasdasdasdasdasd _SPECIAL_SEPERATOR\n");
						//	return createToken(_SPECIAL_SEPERATOR);
						//}
					break;
				}
			}
		}
	}
}

char StringTokenizer::consume(int iHowMany) {
//printf("m_iPos before %d\n", m_iPos);
	char ret;
	m_iPos += iHowMany;	
	m_iColumn += iHowMany;
//printf("m_iPos after %d\n", m_iPos);
	ret = m_Data[m_iPos];

	return ret;
}

Token* StringTokenizer::createToken(TokenKind tokKind, char* value) {
	return new Token(tokKind, value, m_iLine, m_iColumn);
}

Token* StringTokenizer::createToken(TokenKind tokKind) {
///*
	//int iLen = m_iPos - m_SavePos;
	//char* sTokData = new char[iLen+1];
	//strncpy(sTokData, m_Data+m_SavePos, iLen);
	//sTokData[iLen] = '\0';
	char* sTokData = CCString::substr_c_str(m_Data, m_SavePos, m_iPos);

	onTokenCallback(tokKind, sTokData, m_SaveLine, m_SaveColumn);
//printf("in create token--%s %d %d %d\n", sTokData, strlen(sTokData), m_iLine, m_iColumn);
	return new Token(tokKind, sTokData, m_iLine, m_iColumn);
//*/
	//return new Token();
}

void StringTokenizer::setUseSymbolsOnlyAsTokens(bool bValue) {
	bUseExplicitSymbols = bValue;
}

void StringTokenizer::setData(const char* newData) {
	if(m_Data) {
		m_iPos = m_SavePos = m_iDataLength = 0;
		m_iLine = m_SaveLine = m_iColumn = m_SaveColumn = 1;

		SAFE_DELETE_ARRAY(m_Data);
		
		m_iDataLength = strlen(newData);
		m_Data = new char[m_iDataLength+1];
		::memset(m_Data, 1, m_iDataLength+1);
		strncpy(m_Data, newData, m_iDataLength+1);
		m_Data[m_iDataLength] = '\0';
	}
}

void StringTokenizer::clear() {
	m_iPos = m_SavePos = m_iDataLength = 0;
	m_iLine = m_SaveLine = m_iColumn = m_SaveColumn = 1;

	m_SymbolChars = "=+-/,.*~!@#$%^&(){}[]:;<>?|\\";

	bUseExplicitSymbols = false;
	IGNORE_NONTERMINAL = false;
	//printf("m_SymbolChars len = %d, %s\n", strlen(m_SymbolChars), m_SymbolChars);
}

Token* StringTokenizer::readNumber() {
	
	initRead();
	bool bHasDot = false;

	consume(1);//Read the 1st Digit
	while(true) {
		
		char ch = peek(0);
		if(::isdigit(ch))
			consume(1);
		else 
		if(ch == '.' && !bHasDot) {
			bHasDot = true;
			consume(1);
		}
		else
			break;
	}
	
	return createToken(bHasDot?_FLOAT:_INTEGER);
}

Token* StringTokenizer::readWhiteSpace() {
	initRead();
	consume(1);

	while(true) {
		char ch = peek(0);

		if(isWhiteSpaces(ch))
			consume(1);
		else
			break;
	}

	return createToken(WHITESPACE);
}

Token* StringTokenizer::readString() {
	consume(1);//Read the 1st Digit
	initRead();

	while(true) {
			char ch = peek(0);

			if(ch == '\0')
				break;
			else
			if(ch == '\r') {
				consume(1);
				if(peek(0) == '\n')
					consume(1);

				m_iLine++;
				m_iColumn = 1;
			}
			else
			if(ch == '\n') {
				consume(1);
				m_iLine++;
				m_iColumn = 1;
			}
			else
			if(ch == '"') {
				break;
			}
			else
				consume(1);
	}
	
	Token* tokQuotedString = createToken(QUOTEDSTRING);
	consume(1);
	
	return tokQuotedString;
}

Token* StringTokenizer::readCharacter() {
	consume(1);//Read the 1st Digit
	initRead();

	char ch = peek(0);
	consume(1);
	
	Token* tokCharacter = createToken(CHARACTER);

	ch = peek(0);
	if(ch != '\'') {
		printf("Error in char\n");
	}
	
	consume(1);
	
	return tokCharacter;
}

Token* StringTokenizer::readNonTerminal() {
	consume(1);//Read the 1st Digit
	initRead();
	
	while(true) {
		char ch = peek(0);

		if(ch != '>')
			consume(1);
		else
			break;
	}

	Token* tokNonTerminal = createToken(_NON_TERMINAL);
	consume(1);
	
	return tokNonTerminal;
}

Token* StringTokenizer::readWord() {
	initRead();
	consume(1);

	while(true) {
		char ch = peek(0);

		if(CCString::isAlphabet(ch) || ch == '_' || isdigit(ch))
			consume(1);
		else
			break;
	}
	
	return createToken(_WORD);
}

Token* StringTokenizer::readSingleLineComment() {
	
	initRead();
	consume(1);
	
	while(true) {
		char ch = peek(0);
//printf("ch == %d %c\n", ch, ch);
		if(ch == '\r' || ch == '\n' || ch == 0)
			break;
		else
			consume(1);
	}

	return createToken(COMMENT_SINGLE_LINE);
}

Token* StringTokenizer::readMultiLineComment() {
	
	initRead();
	consume(1);
	
	while(true) {
		char ch = peek(0);
//printf("ch ===== %d %c\n", ch, ch);
		if(ch == '*' && peek(1) == '/') {
			consume(2);//consume '/'
			break;
		}
		else
			consume(1);
	}

	return createToken(COMMENT_MULTI_LINE);
}

Token* StringTokenizer::checkDefault(char ch) {
	if(CCString::isAlphabet(ch) || ch == '_')
		return readWord();
	else
	if(ch == '/' && peek(1) == '/') {//Single line comment			
		return readSingleLineComment();
	}
	else
	if(ch == '/' && peek(1) == '*') {//Multi line comment			
		return readMultiLineComment();
	}
	else
	if(ch == '-' && ::isdigit(peek(1))) {//Negative number
		return readNumber();
	}
	else
	if(ch == '<' && !IGNORE_NONTERMINAL) {
		return readNonTerminal();
	}
	else
	if(isSymbol(ch)) {
		initRead();
		consume(1);

		return createToken(SYMBOL);
	}
	else {
		initRead();
		consume(1);
		return createToken(UNKNOWN);
	}
}

bool StringTokenizer::isSymbol(char c) {

	for(int i = 0; i < strlen(m_SymbolChars); i++) {
		
		if(c == m_SymbolChars[i])
			return true;
	}

	return false;
}

bool StringTokenizer::isSpecialSeperator(char ch) {
//printf("In isSpecialSeperator\n");
	bool bReturn = false;
	int iSize = vSeperators.size();
//printf("In isSpecialSeperator\n");
	for(int i = 0; i < iSize; i++) {
		
		char* sSeperator = vSeperators[i];
		int iLenCount = strlen(sSeperator);
		int iLength = iLenCount;
//printf("sSeperator = %s\n", sSeperator);
		char c;
		int iPeekCount = 0;
		while(iLenCount > 0) {
			
			c = peek(iPeekCount);
			if(c != CCString::charAt(sSeperator, iPeekCount))
				break;

			iLenCount--;
			iPeekCount++;
		}
		
		c = peek(iPeekCount);//next char

		if(	iLenCount > 0
			||
			(CCString::isAlphabet(c) || ::isdigit(c) || c == '_' || c == '-')
		)
			continue;
		else {
			bReturn = true;
			initRead();
			consume(iLength);
//printf("********************************************************************************************* peek(iPeekCount) === %d %c = %d\n", m_iPos, peek(0), bReturn);
			break;
		}
	}

	return bReturn;
}

void StringTokenizer::setTokenCallback(void (__cdecl *callbackFuncAddr)(int,char *,int,int)) {
	onTokenCallback = callbackFuncAddr;
}

void StringTokenizer::initRead() {
	m_SavePos = m_iPos;
	m_SaveLine = m_iLine;
	m_SaveColumn = m_iColumn;
}

StringTokenizer::~StringTokenizer() {
	this->clear();
	SAFE_DELETE_ARRAY(m_SymbolChars);
	SAFE_DELETE_ARRAY(m_Data);
}

void StringTokenizer::skipWhiteSpaces(bool bSkip) {
	bSkipWhiteSpaces = bSkip;
}

bool StringTokenizer::isWhiteSpaces(char ch) {
	return (ch == '\t' || ch == ' ');
}

void StringTokenizer::ignoreNonTerminals(bool bIgnore) {
	IGNORE_NONTERMINAL = bIgnore;
}