
#ifndef _3DS_H_
#define _3DS_H_

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "main.h"
using namespace std;	//for vector

//Primary Chunk at the beginning of the file
#define PRIMARY					0x4D4D

//Main Chunks
#define FILE_VERSION			0x0002		//This gives the version of the .3ds file
#define	OBJECT_INFO				0x3D3D		//This has all the information about the Mesh
#define EDIT_KEY_FRAME			0xB000		//This gives the header for all key frame info

//Sub Chunks for OBJECT_INFO
#define	MESH_VERSION			0x3E3D		//This gives the Mesh version and is found right before the material and object information
#define	MATERIAL				0xAFFF		//This stores the texture info
#define OBJECT					0x4000		//This stores the faces, vertices, etc...

//Sub Chunks for MATERIAL
#define	MATERIAL_NAME			0xA000		//This holds the Material Name
#define	MATERIAL_DIFFUSE		0xA020		//This holds the colour of the Object/Material
#define MATERIAL_MAP			0xA200		//This holds the header of a new Material
#define	MATERIAL_MAP_FILENAME	0xA300		//This holds the filename of the texture

#define	OBJECT_MESH				0x4100		//This lets us know we are reading a new object

//Sub Chunks for OBJECT_MESH
#define	OBJECT_VERTICES			0x4110		//The objects vertices
#define	OBJECT_FACES			0x4120		//The objects faces
#define	OBJECT_MATERIAL			0x4130		//This is found if the object has a material, either a colour or a texture map
#define	OBJECT_UV				0x4140		//The UV texture coordinates

//This holds the Chunk info
struct tCHUNK {
	unsigned /*short */int	chunkID;			//The Chunks ID
	unsigned int		chunkLength;		//The Chunks Length
	unsigned int		bytesRead;			//Amount of bytes read within the Chunk
};

class CReader3DS {
	public:
		FILE *mFilePointer, *mConsoleLogFilePointer;
		char logText[255];

		bool import3DS(t3DModel *pGame3DModel, char* fileName3DS);
		void readChunk(tCHUNK *pChunk);
		void processNextChunk(t3DModel *pModel, tCHUNK *pPreviousChunk);
		void processNextMaterial(t3DModel *pModel, tCHUNK *pPreviousChunk);
		void processNextObjectChunk(t3DModel *pModel, t3DObject *pObject, tCHUNK *pPreviousChunk);
		
		void readColourChunk(tMaterial *pMaterial, tCHUNK *pChunk);
		int readString(char *pBuffer);		//returns number of bytes read.

		void readObjectVertices(t3DObject *pObject, tCHUNK *pPreviousChunk);
		void readObjectFaceIndices(t3DObject *pObject, tCHUNK *currentChunk);
		void readObjectFaceMaterial(t3DModel *pModel, t3DObject *pObject, tCHUNK *pPreviousChunk);
		void readObjectUVCoordinates(t3DObject *pObject, tCHUNK *pPreviousChunk);
		
		void cleanUp();
		void log(char *format, ...);

		void displayErrorMessage(const char *errMsg, ...);
};

//This global buffer is used to read past unwanted data.

#endif