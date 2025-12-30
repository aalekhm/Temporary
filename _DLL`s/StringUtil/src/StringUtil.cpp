
#include <windows.h>
#include "StringUtil.h"

namespace StringUtil {
	EXPORT_DLL bool isWhiteSpace(char c) {
		if(	(	c == ' '
				||
				c == '\t'
				||
				c == '\r'
				||
				c == '\n'
			)
		) {
			return true;
		}

		return false;
	}

	EXPORT_DLL const char* strCat(const char* s1, const char* s2) {
		if(!s1 || !s2)
			return 0;
		
		int len = strlen(s1) + strlen(s2);

		char* outStr = new char[len+1];//(char*)::malloc(len+1);//GlobalAlloc(0, len+1);
		::memset(outStr, 0, len);

		::strcat(outStr, s1);
		::strcat(outStr, s2);
		outStr[len] = '\0';

		return outStr;
	}

	EXPORT_DLL char* rTrim(char* text) {
		char* rTrimText = text;
		int whiteSpacePos = 0;
		int pos = 0;
		
		while(*rTrimText) {
			if(!isWhiteSpace(*rTrimText))
				whiteSpacePos = pos+1;
			
			rTrimText++;
			pos++;
		}

		if(rTrimText > text) {
			char* newText = new char[whiteSpacePos+1];//allocMemory(whiteSpacePos);
			::strncpy(newText, text, whiteSpacePos);
			newText[whiteSpacePos] = '\0';

			delete text;
			//::GlobalFree(text);

			return newText;
		}

		return text;
	}

	EXPORT_DLL char* lTrim(char* text) {
		char* lTrimText = text;
		
		while(*lTrimText) {
			if(!isWhiteSpace(*lTrimText))
				break;

			lTrimText++;
		}
//printf("sssssssssssss %s\n", text);

		if(lTrimText > text) {
//MessageBox(NULL, "00", "00", 1);
			int newTextLen = strlen(lTrimText);
//MessageBox(NULL, "01", "00", 1);
			char* newText = new char[newTextLen+1];//allocMemory(newTextLen);
//MessageBox(NULL, "02", "00", 1);
			::strcpy(newText, lTrimText);
//MessageBox(NULL, "04", "00", 1);
			newText[newTextLen] = '\0';
//MessageBox(NULL, "03", "00", 1);
			delete text;
			text = NULL;
			//::GlobalFree(text);
//printf("sssssssssssss %s\n", newText);
			return newText;
		}

		return text;
	}

	EXPORT_DLL char* trim(char* text) {
		return rTrim(lTrim(text));
	}

	/*
	const char* skipWhiteSpaces(const char* p) {
		if(!p || !*p)
			return 0;

		while(p && *p) {
			if(	*p == ' '
				||
				*p == 0x0d//'\r'
				||
				*p == 0x0a//'\n'
				||
				*p == 0x09//'\t'
			) {
				++p;
			}
			else
				break;
		}

		return p;
	}
	*/

	EXPORT_DLL char* toLower(char* text) {
		/*
		char* ret = allocMemory(strlen(text));
		strcpy(ret, text);

		int i = 0;

		while(text[i]) {
			if(isUpperCase(text[i]))
				ret[i] = text[i] + 32;
			
			i++;
		}
		
		::GlobalFree(text);

		return ret;
		*/
		
		int len = strlen(text);
		char* ret = new char[len+1];//allocMemory(strlen(text)+1);
		strcpy(ret, text);
		
		_strlwr(ret);
		ret[len] = '\0';

		delete text;
		//::GlobalFree(text);

		return ret;
	}

	EXPORT_DLL bool startsWith1(const char* str, const char* startStr, bool isCaseSensitive) {
		if(!str || !*str || !startStr || !*startStr)
			return false;

		const char* tempStr = str;
		if(isCaseSensitive) {
			while(	*tempStr 
					&& 
					*startStr 
					&& 
					*tempStr == *startStr
			) {
				++tempStr;
				++startStr;
			}

			if(*startStr == 0)
				return true;
		}
		else {
			while(	*tempStr 
					&& 
					*startStr 
					&& 
					toLower((char*)*tempStr) == toLower((char*)*startStr)
					||
					*tempStr == *startStr
			) {
				++startStr;
				if(*startStr == 0)
					return true;

				++tempStr;
			}

			//if(*startStr == 0)
			//	return true;
		}
		
		return false;
	}

	EXPORT_DLL bool startsWith(const char* str, const char* startStr){//, bool isCaseSensitive) {
		if(!str || !*str || !startStr || !*startStr)
			return false;
//printf("startsWith00\n");
		char* idx = (char*)strstr(str, startStr);
		if(idx - str == 0) {
//printf("startsWithTRUE\n");
			return true;
		}
//printf("startsWithFALSE\n");
		return false;
	}

	EXPORT_DLL bool endsWith(const char* str, const char* endStr){
		if(!str || !*str || !endStr || !*endStr)
			return false;

		char* revStr = new char[512];
		strcpy(revStr, str);
		revStr = strrev(revStr);

		char* revEndStr = new char[512];
		strcpy(revEndStr, endStr);
		revEndStr = strrev(revEndStr);

		return startsWith(revStr, revEndStr);
	}

	EXPORT_DLL char* stripChars(char* text, char* stripChars) {
		int iLen = strlen(text);
		char* ret = new char[iLen+1];//allocMemory(strlen(text));

		strcpy(ret, text);

		unsigned TEXT_LENGTH = iLen;
		unsigned pos = strcspn(text, stripChars);
		while(pos < TEXT_LENGTH) {
			strcpy(ret + pos, ret + pos + 1);
			pos = strcspn(ret, stripChars);

			TEXT_LENGTH = strlen(ret);
		}
		
		delete text;
		//::GlobalFree(text);

		return ret;
	}


	EXPORT_DLL char* toUpper(char* text) {
		/*
		char* ret = allocMemory(strlen(text));
		strcpy(ret, text);

		int i = 0;

		while(text[i]) {
			if(isLowerCase(text[i]))
				ret[i] = text[i] - 32;
			
			i++;
		}
		
		::GlobalFree(text);

		return ret;
		*/
		
		int len = strlen(text);
		char* ret = new char[len+1];//allocMemory(strlen(text)+1);
		strcpy(ret, text);
		
		_strupr(ret);
		ret[len] = '\0';
		
		delete text;
		//::GlobalFree(text);

		return ret;
	}

	EXPORT_DLL bool isUpperCase(char c) {
		return (c >= 65 && c <= 90);
	}

	EXPORT_DLL bool isLowerCase(char c) {
		return (c >= 97 && c <= 122);
	}

	EXPORT_DLL bool isAlphabet(char c) {
		return (isLowerCase(c) || isUpperCase(c));
	}

	EXPORT_DLL bool isAlphaNumeric(char c) {
		return (isAlphabet(c) || (c >= 48 && c <= 57));
	}

	EXPORT_DLL char* substr(char* inStr, int beginIndex) {
		if(beginIndex < 0 || beginIndex >= strlen(inStr))
			return NULL;

		int len = strlen(inStr) - beginIndex;
		char* subStr = new char[len+1];//allocMemory(len);

		strncpy(subStr, inStr+beginIndex, len);
		subStr[len] = '\0';

		return subStr;
	}

	EXPORT_DLL char* substr(char* inStr, int beginIndex, int endIndex) {
		if(	beginIndex < 0 
			|| 
			beginIndex > endIndex 
			|| 
			beginIndex >= strlen(inStr)
		)
			return NULL;

		int len = endIndex - beginIndex;
		//int len2 = strlen(inStr) - beginIndex;
		//if(len2 < len)
		//	len = len2;

		char* subStr = new char[len+1];//allocMemory(len);

		strncpy(subStr, inStr+beginIndex, len);
		subStr[len] = '\0';

		return subStr;
	}

	EXPORT_DLL int	indexOf(char* inStr, char* searchStr) {
		char* ret = strstr(inStr, searchStr);
		if(ret != NULL)
			return ret - inStr;
		return -1;
	}

	EXPORT_DLL int	indexOf(char* inStr, char* searchStr, int fromIndex) {
		if(fromIndex >= strlen(inStr))
			return -1;

		char* str = &inStr[fromIndex];

		int retValue = indexOf(str, searchStr);
		if(retValue >= 0)
			return retValue + fromIndex;

		return -1;
	}

	EXPORT_DLL char* replace(char* inStr, char oldChar, char newChar) {
		int i = 0;
		char c = *inStr;

		while(c) {
			if(c == oldChar) {
				inStr[i] = newChar;
			}

			c = inStr[++i];
		}

		return inStr;
	}

	EXPORT_DLL char* replace(char* inStr, char* oldStr, char* newStr) {
		unsigned int numOfOccurances = 0;
		
		//scan for the total numbr of occurances of 'oldStr' in 'inStr'
		int ret = indexOf(inStr, oldStr);
		while(ret >= 0) {
			numOfOccurances++;
			ret = indexOf(inStr, oldStr, ret + 1);
		}
		
		if(numOfOccurances > 0) {
				//make sufficient space which might be after replacing 'oldStr' with 'newStr'
				unsigned int totalSize = strlen(inStr) + numOfOccurances*(strlen(newStr) - strlen(oldStr));
				char* retBuff = new char[totalSize+1];//allocMemory(totalSize+1);
				retBuff[totalSize] = '\0';
				
				unsigned int startOffset = 0, len = 0, off = 0;

				//get the next location of the 'oldStr'
				ret = indexOf(inStr, oldStr);
				len = ret - startOffset;
				
				while(ret >= 0) {
					//copy the 'string' that lies in-between
					strncpy(retBuff + startOffset, inStr + off, len);
					off = ret + strlen(oldStr);
					
					//copy/append the 'newStr' 
					strncpy(retBuff + startOffset + len, newStr, strlen(newStr));
					
					//get the next location of the 'oldStr'
					ret = indexOf(inStr, oldStr, ret + 1);
					
					//shift/move the 'startOffset'
					startOffset += (len + strlen(newStr));
					
					//calculate the length of the 'string' that lies in-between the occurances of 'oldStr'
					len = ret - off;
				}
				
				//copy the remaining tail of the 'inStr'
				strncpy(retBuff + startOffset, inStr + off, strlen(inStr + off));
				
				delete inStr;
				//::GlobalFree(inStr);

				inStr = 0;
				return retBuff;
		}

		
		return inStr;
	}

	EXPORT_DLL char charAt(char* inStr, int index) {
		if(	index < 0
			||
			index >= strlen(inStr)
		)
			return NULL;

		return inStr[index];
	}

	EXPORT_DLL int indexOf(char* inStr, char ch) {
		/*//PSUEDO CODE
		int i = 0;
		char c = *inStr;

		while(c) {
			if(c == ch)
				return i;
			
			c = inStr[++i];
		}

		return -1;

		//0R
		//*/

		char* ret = strchr(inStr, ch);
		if(ret != NULL)
			return (ret - inStr);

		return -1;
	}

	EXPORT_DLL int indexOf(char* inStr, char ch, int fromIndex) {
		if(fromIndex >= strlen(inStr))
			return -1;

		char* str = &inStr[fromIndex];

		int retValue = indexOf(str, ch);
		if(retValue >= 0)
			return retValue + fromIndex;

		return -1;
	}

	EXPORT_DLL int lastIndexOf(char* inStr, char ch) {
		/*//PSUEDO CODE
		char* revStr = allocMemory(strlen(inStr));
		strcpy(revStr, inStr);
		revStr = strrev(revStr);
		
		int retValue = indexOf(revStr, ch);

		if(retValue >= 0)
			return strlen(inStr) - retValue - 1;

		return -1;
		
		//0R
		//*/
		
		char* ret = strrchr(inStr, ch);
		if(ret != NULL)
			return (ret - inStr);

		return -1;
	}
	
	EXPORT_DLL int lastIndexOf(char* inStr, char ch, int fromIndex) {
		if(fromIndex < 0)
			return -1;

		int modifiedFromIndex = strlen(inStr) - fromIndex;

		char* revStr = new char[strlen(inStr)+1];//allocMemory(strlen(inStr)+1);
		strcpy(revStr, inStr);
		revStr = strrev(revStr);

		revStr += modifiedFromIndex;
		int retValue = indexOf(revStr, ch);

		if(retValue >= 0) {
			delete revStr;
			//::GlobalFree(revStr);

			return fromIndex - retValue - 1;
		}

		delete revStr;
		//::GlobalFree(revStr);

		return -1;
	}

	EXPORT_DLL int lastIndexOf(char* inStr, char* searchStr) {
		unsigned int pos;
		char* ret = strstr(inStr, searchStr);
		
		while(ret) {
			pos = ret - inStr;

			ret = strstr(ret + 1, searchStr);
		}
		
		return pos;
	}
}