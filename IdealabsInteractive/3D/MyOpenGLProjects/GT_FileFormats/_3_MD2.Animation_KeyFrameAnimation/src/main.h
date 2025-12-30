
#ifndef _MAIN_H_
#define _MAIN_H_

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <fstream>
#include <vector>
#include <gl\gl.h>										// Header File For The OpenGL32 Library
#include <gl\glu.h>										// Header File For The GLu32 Library
#include <gl\glaux.h>
#include <crtdbg.h>
using namespace std;

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

// This holds our information for each animation of the Quake model.
// A STL vector list of this structure is created in our t3DModel structure below.
struct tAnimation {
	char animName[255];				//This stores the name of the Animation (eg. Stand, Idle, Run, Walk, Painn, Jump, ... etc)
	int startFrame;					//This stores the first Frame number of this Animation
	int endFrame;					//This stores the last Frame number of this Animation
};

// This is our face structure.  This is is used for indexing into the vertex 
// and texture coordinate arrays.  From this information we know which vertices
// from our vertex array go to which face, along with the correct texture coordinates.
struct tFace {
	int vertexIndex[3];		//This stores the vertex indexes that make up this triangle face.
	int texCoordIndex[3];		//This stores the texture coordinate index to be applied to this face.
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
	int numOfAnimations;					//The number of Animation in this Model ( NEW )
	int currentAnim;						//The current index into the Animations List ( NEW )
	int currentFrame;						//The current Frame of the current Animation ( NEW )

	vector<t3DObject>		pObjects;		//List of the Objects
	vector<tMaterial>		pMaterials;		//List of the Materials (textures & colours).	
	vector<tAnimation>		pAnimations;	//List of the Animations ( NEW )
};


#endif