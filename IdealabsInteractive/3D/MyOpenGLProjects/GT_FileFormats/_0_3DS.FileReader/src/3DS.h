
#ifndef _3DS_H_
#define _3DS_H_

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <stdarg.h>
#include <math.h>
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
	unsigned short int	chunkID;			//The Chunks ID
	unsigned int		chunkLength;		//The Chunks Length
	unsigned int		bytesRead;			//Amount of bytes read within the Chunk
};

//This is a 3 point class, used to store the vertices of our Model.
class CVector3 {
	public:
		float x, y, z;
};

//This is a 2 point class, used to store the UV texture coordinates.
class CVector2 {
	public:
		float x, y;
};

// This is our face structure.  This is is used for indexing into the vertex 
// and texture coordinate arrays.  From this information we know which vertices
// from our vertex array go to which face, along with the correct texture coordinates.
struct tFace {
	int vertexIndex[3];		//This stores the vertex indexes that make up this triangle face.
	int coordIndex[3];		//This stores the texture coordinate index to be applied to this face.
};

//This holds the material information. It may be a texture map or a colour.
// Some of these are not used, but I left them because you will want to eventually
// read in the UV tile ratio and the UV tile offset for some models.
struct tMaterial {
	char materialName[255];			//The Material Name
	char textureFileName[255];		//The texture file name, if this is a texture map
	BYTE colour[3];					//The colour of the object (R, G, B)
	int materialID;					//The texture ID
	float uTile;					//U tiling of the texture
	float vTile;					//V tiling of the texture
	float uOffset;					//U tiling offset of the texture
	float vOffset;					//V tiling offset of the texture
};

//This holds the information for a single OBJECT in our MODEL/SCENE.
//You should eventually turn into a robust class that 
//has loading/drawing/querying functions like:
//LoadModel(...); DrawObject(...); DrawModel(...); DestroyModel(...);
struct t3DObject {
	char objectName[255];			//The name of the Object
		
	int numOfVertices;				//The number of vertices in the MODEL
	int numOfFaces;					//The number of faces in the MODEL
	int numOfTextureVertex;			//The number of texture Coordinates
			
	bool bHasTexture;				//Does this object has a texture?
	int materialID;					//If yes, this is the texture ID to use, which is the index to out texture array
	
	tFace		*pFaces;			//The objects faces
	CVector3	*pVertices;			//The objects vertices
	CVector3	*pNormals;			//The objects normals
	CVector2	*pTexVertices;		//The objects texture`s UV coordinates
};

///This holds our MODEL/SCENE information.
//This should also turn into a robust class.
// We use STL's (Standard Template Library) vector class to ease our link list burdens. :)
struct t3DModel {
	int numOfObjects;						//The number of Objects in our MODEL/SCENE
	int numOfMaterials;						//The number of materials used by the Objects in our MODEL/SCENE
	
	vector<t3DObject>		pObjects;		//List of the Objects
	vector<tMaterial>		pMaterials;	//List of the Materials (textures & colours).	
};


class CReader3DS {
	public:
		FILE *mFilePointer;
		
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

		void displayErrorMessage(const char *errMsg, ...);

		void CReader3DS::computeNormals(t3DModel *pModel);
};

//This global buffer is used to read past unwanted data.

#endif