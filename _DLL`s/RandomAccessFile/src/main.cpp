
#include "RandomAccessFile.h"

void main() {

	RandomAccessFile* raf = new RandomAccessFile();
	raf->openForRead("RandomAccessFile.cpp");

	RandomAccessFile* rafW = new RandomAccessFile();
	rafW->openForWrite("RandomAccessFileWritten.cpp");

	while(!raf->isEOF()) {
		char* sLine = (char*)raf->readLine();
		printf("%s\n", sLine);

		rafW->writeLine(sLine);
	}

	raf->close();
	rafW->close();
}