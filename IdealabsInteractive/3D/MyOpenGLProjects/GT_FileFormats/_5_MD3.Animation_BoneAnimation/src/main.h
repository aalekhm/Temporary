
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
typedef unsigned char byte;

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define SCREEN_BPP		16

#define MAX_TEXTURES	100

static FILE *mLogFile;

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
	int texCoordIndex[3];	//This stores the texture coordinate index to be applied to this face.
};

//This holds the material information. It may be a texture map or a colour.
// Some of these are not used, but I left them because you will want to eventually
// read in the UV tile ratio and the UV tile offset for some models.
struct tMaterial {
	char materialName[255];			//The Material Name
	char textureFileName[255];		//The texture file name, if this is a texture map
	BYTE colour[3];					//The colour of the object (R, G, B)
	int textureID;					//The texture ID
	float uTile;					//U tiling of the texture
	float vTile;					//V tiling of the texture
	float uOffset;					//U tiling offset of the texture
	float vOffset;					//V tiling offset of the texture
};

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ NEW
// This holds our information for each animation of the Quake model.
// A STL vector list of this structure is created in our t3DModel structure below.
struct tAnimation {
	char animationName[255];			// This stores the name of the animation (I.E. "TORSO_STAND")
	int startFrame;						// This stores the first frame number for this animation
	int endFrame;						// This stores the last frame number for this animation
	int loopingFrames;					// This stores the looping frames for this animation (not used)
	int framesPerSecond;				// This stores the frames per second that this animation runs
};
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ NEW

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
	vector<tMaterial>		pMaterials;		//List of the Materials (textures & colours).	
	
	/*MD3 Specific*/
	int						numOfTags;		// This stores the number of tags in the model
	t3DModel				**pLinks;		// This stores a list of pointers that are linked to this model
	struct tMD3Tag			*pTags;			// This stores all the tags for the model animations

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ NEW
	int numOfAnimations;					// The number of animations in this model 
	int currentAnim;						// The current index into pAnimations list 
	int currentFrame;						// The current frame of the current animation 
	int nextFrame;							// The next frame of animation to interpolate too
	float t;								// The ratio of 0.0f to 1.0f between each key frame
	float lastTime;							// This stores the last time that was stored
	vector<tAnimation> pAnimations;			// This stores the last time that was stored
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ NEW
};


#endif