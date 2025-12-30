#include <stdio.h>
#include "CCString.h"

void main() {
	printf("Hello World!!!\n");

	CCString* pCString1 = new CCString("				    This function or variable may be unsafe.");
	*pCString1 += "  LINK as: D:\\_Projects\\_DLL`s\\CString\\Debug\\CString.exe not found or not built by the last incremental link as; performing full link as.  ";

	CCString pCString2("Aalekh------------ .");//Allocated on Stack
	*pCString1 += &pCString2;

	CCString* pCString3 = new CCString("+++++++++	aalekh			  ");
	*pCString1 += pCString3;

	//*pCString1 += 'A';

	CCString pCString4;//Allocated on Stack

	pCString1->toLower();
	printf("pCString = %s\n", pCString1->c_str());
	printf("pCString4 = %s\n", pCString4.c_str());
	printf("pCString1 charAt = %c\n", pCString1->charAt(pCString1->length()-1));
	printf("pCString1 posOf ':' = %d\n", pCString1->indexOf(':'));
	printf("pCString1 lastPosOf ':' = %d\n", pCString1->lastIndexOf(':'));
	printf("pCString1 posOf 'A' = %d\n", pCString1->indexOf('A'));
	printf("pCString1 lastPosOf 'A' = %d\n", pCString1->lastIndexOf('A'));

	printf("pCString1 posOf ':' from 47 = %d\n", pCString1->indexOf(':', 48));
	printf("pCString1 lastPosOf ':' = %d\n", pCString1->lastIndexOf(':', pCString1->length() - 49));

	printf("pCString1 %s\n", pCString1->c_str());
	printf("pCString1 lastPosOf \"unsafe\" = %d ===== %d\n", pCString1->length(), pCString1->lastIndexOf("link"));

	printf("pCString1 lastPosOf \"unsafe\" = %d ===== %d\n", pCString1->length(), pCString1->lastIndexOf("link", 142));

	pCString1->lTrim();
	printf("pCString1.lTrim() = %s of len = %d\n", pCString1->c_str(), pCString1->length());

	pCString1->trim();
	printf("pCString1.trim() = %s of len = %d\n", pCString1->c_str(), pCString1->length());

	char* substr1 = pCString1->substr_c_str(10);
	printf("pCString1->substr_c_str(10) = %s of len = %d\n", substr1, strlen(substr1));
	SAFE_DELETE_ARRAY(substr1);

	char* substr2 = pCString1->substr_c_str(10, 20);
	printf("pCString1->substr_c_str(10, 20) = %s of len = %d\n", substr2, strlen(substr2));
	SAFE_DELETE_ARRAY(substr2);

	CCString* substr3 = pCString1->substr(10);
	printf("pCString1->substr(10) = %s of len = %d\n", substr3->c_str(), substr3->length());
	SAFE_DELETE(substr3);

	CCString* substr4 = pCString1->substr(10, 20);
	printf("pCString1->substr(10, 20) = %s of len = %d\n", substr4->c_str(), substr4->length());
	SAFE_DELETE(substr4);

	//pCString1->replace('e', 'z');
	//printf("pCString1->replace('e', 'z'); = %s of len = %d\n", pCString1->c_str(), pCString1->length());
	
	pCString1->replace("link as", "REPLACED STRING");
	printf("pCString1->replace(\"link\", \"ABCDE\"); = %s of len = %d\n", pCString1->c_str(), pCString1->length());

	printf("pCString1->startsWith(\"this \"); = %d\n", pCString1->startsWith("this function"));
	printf("pCString1->endsWith(\"aalekh\"); = %d\n", pCString1->endsWith("aalekh"));

	pCString1->stripChars("aalekh");
	printf("pCString1->stripChars(\"aalekh\"); = %s\n", pCString1->c_str());

	*pCString3 = "This is assigned String";

	*pCString3 = pCString1;

	char c = pCString2[0];
	pCString2[0] = 'B';
	printf("pCString1[0]; = %c, %c\n", c, pCString2[0]);

	SAFE_DELETE(pCString1);//Allocated on Heap
	SAFE_DELETE(pCString3);//Allocated on Heap
}