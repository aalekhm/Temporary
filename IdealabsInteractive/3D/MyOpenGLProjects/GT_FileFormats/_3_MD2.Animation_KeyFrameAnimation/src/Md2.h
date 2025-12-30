
#ifndef _MD2_H_
#define _MD2_H_

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "main.h"
using namespace std;	//for vector

typedef unsigned char byte;

//These are the needed defines for the max values when loading .MD2 files
#define MD2_MAX_TRIANGLES			4096
#define MD2_MAX_VERTICES			2048
#define MD2_TEX_COORINATES			2048
#define MD2_MAX_FRAMES				512
#define MD2_MAX_SKINS				32
#define MD2_MAX_FRAME_SIZE			(MD2_MAX_VERTICES * 4 + 128)

//This stores the speed of the Animation between each key frame
#define kAnimationSpeed				5.0f

//This holds the Header Info which is read at the beginning of the '.md2' file
struct tMD2Header {
	int magic;						//Magic number to identify the file - 'IPD2'
	int version;					//The version number of the file (must be 8)
	int skinWidth;					//The Skin Width in pixels
	int skinHeight;					//The Skin Height in pixels
	int frameSize;					//This specifies the size in bytes of each frame.
	int numOfSkins;					//Tells us the number of textures available for this Model
	int numOfVertices;				//Total number of Vertices in the Model
	int numOfTexCoordinates;		//Is the number of atexture coordinates stored in the file
	int numOfTriangles;				//Is the number of Triangles in the Model
	int numOfGlCmds;				//Gives the number of OpenGL commands
									//The GL command list is an array of integers that allows 
									//us to render the model using only triangle fans and 
									//triangle strip (GL_TRIANGLE_STRIP and GL_TRIANGLE_FAN), 
									//instead of classic triangles (GL_TRIANGLES). GL commands are very powerful. 
									//It is easy to get a rendering about 10 or 15 fps faster!
	int numOfFrames;				//The number of Animation Frames that holds the Model. In fact, each of them 
									//are refered to as keyframes, which are frames taken from discrete time intervals 
									//because it would be impossible to hold 200 or 300 frames per animation!

	int offsetSkinsStart;
	int offsetTexCoordStart;
	int offsetTrianglesStart;
	int offsetFramesStart;
	int offsetGLCommandsStart;
	int offsetEnd;
};


//This is used to store that vertices that are read in for the current Frame
/*
You may have noticed that compressedVertex[3] contains vertex' (x,y,z) coordinates 
and because of the unsigned char type, these coordinates can only range 
from 0 to 255. In fact these 3D coordinates are compressed (3 bytes 
instead of 12 if we would use float or vec3_t). To uncompress it, we'll 
use other data proper to each frame. lightnormalindex is an index to a 
precalculated normal table. Normal vectors will be used for the lighting.
*/
struct tMD2AliasVertex {
	byte compressedVertex[3];					// compressed vertex (x, y, z) coordinates
	byte lightNormalIndex;						// index to a normal vector for the lighting
};

//This stores the normals and vertices for the Frames
struct tMD2Vertex {
	float vertex[3];
	float normal[3];
};

// This stores the indices into the vertex and texture coordinate arrays
struct tMD2Face {
	short vertexIndices[3];
	short textureIndices[3];
};

/*
Like for vertices, data is compresed. Here we use short (2 bytes) 
instead of float (4 bytes) for storing texture coordinates. But to 
use them, we must convert them to float because texture coordinates 
range from 0.0 to 1.0, and if we kept short values, we could have 
only 0 or 1 and any intermediate value! So how to uncompress them? 
It's quite simple. Divide the short value by the texture size
*/
struct tMD2TexCoord {
	short u;
	short v;
};

//This store the Animation scale, translate and name information for a frame, plus vertices
struct tMD2AliasFrame {				
	float scale[3];								//Scale values
	float translate[3];							//Translate values
	char frameName[16];							//Frame Name
	tMD2AliasVertex aliasVertex[1];				//First vertex of this frame
};

//This stores the FRAME vertices after they have been 'Transformed'
struct tMD2Frame {
	char frameName[16];
	tMD2Vertex *pVertices;
};

//This stores the Skin Name
typedef char tMD2SkinName[64];

class CReaderMD2 {
	public:
		CReaderMD2();

		//This is called whenever u need to load an '.md2' file
		bool importMD2(t3DModel *pModel, char *fileNameMD2);

		FILE *mFilePointer, *mFileConsoleLog;
		char logText[255];

	private:
		void readMD2Data();

		void readSkins();
		void readTexCoords();
		void readFaces();
		void readFrames();

		void convertDataStructures(t3DModel *pModel);
		void parseAnimations(t3DModel *pModel);

		
		
		
		tMD2Header		m_pMD2Header;
		tMD2SkinName	*m_pSkins;
		tMD2TexCoord	*m_pTexCoords;
		tMD2Face		*m_pFaces;
		tMD2Frame		*m_pFrames;
};

#endif