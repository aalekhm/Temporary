
#ifndef _MD3_H_
#define _MD3_H_

#include "main.h"



// This file stores all of our structures and classes (besides the modular model ones in main.h)
// in order to read in and display a Quake3 character.  The file format is of type
// .MD3 and comes in many different files for each main body part section.  We convert
// these Quake3 structures to our own structures in Md3.cpp so that we are not dependant
// on their model data structures.  You can do what ever you want, but I like mine :P :)

// This holds the header information that is read in at the beginning of the file
struct tMD3Header{
	char	fileID[4];				// This stores the File ID - Must be 'ID3'
	int		version;				// This stores the file version - Must be 15
	char	fileName[68];			// This stores the name of the File
	int		numOfFrames;			// This stores number of animation frames
	int		numOfTags;				// This stores the tag Count
	int		numOfMeshes;			// This stores the number of sub-objects in the Mesh
	int		numMaxSkins;			// This stores the number of skins for the mesh
	int		headerSize;				// This stores the mesh header size
	int		tagStart;				// This stores the offset into the file for tags
	int		tagEnd;					// This stores the end offset into the file for tags
	int		fileSize;				// This stores the file size
};

// This structure is used to read in the mesh data for the .md3 models
struct tMD3MeshInfo {
	char	meshID[4];				// This stores the mesh ID (We don't care)
	char	meshName[68];			// This stores the mesh name (We do care)
	int		numOfMeshFrames;		// This stores the mesh aniamtion frame count
	int		numOfSkins;				// This stores the mesh skin count
	int		numOfVertices;			// This stores the mesh vertex count
	int		numOfTriangles;			// This stores the mesh face count
	int		offsetTriangleStart;	// This stores the starting offset for the triangles
	int		headerSize;				// This stores the header size for the mesh
	int		offsetUVStart;			// This stores the starting offset for the UV coordinates
	int		offsetVertexStart;		// This stores the starting offset for the vertex indices
	int		meshSize;				// This stores the total mesh size
};

// This is our tag structure for the .MD3 file format.  These are used link other
// models to and the rotate and transate the child models of that model.
struct tMD3Tag {
	char		tagName[64];			// This stores the name of the tag (I.E. "tag_torso")
	CVector3	vTranslatePosition;		// This stores the translation that should be performed
	float		rotationMatrix[3][3];	// This stores the 3x3 rotation matrix for this frame
};

// This stores the bone information (useless as far as I can see...)
struct tMD3Bone {
	float		mins[3];				// This is the min (x, y, z) value for the bone
	float		maxs[3];				// This is the max (x, y, z) value for the bone
	float		position[3];			// This supposedly stores the bone position???
	float		scale;					// This stores the scale of the bone
	char		creator[16];			// The modeler used to create the model (I.E. "3DS Max")
};

// This stores the normals and vertex indices 
struct tMD3Triangle {
	signed short	vertex[3];			// The vertex for this face (scale down by 64.0f)
	unsigned char	normal[2];			// This stores some crazy normal values (not sure...)
};

// This stores the indices into the vertex and texture coordinate arrays
struct tMD3Face {
	int				vertexIndexes[3];
};

// This stores UV coordinates
struct tMD3TexCoord {
	float			texCoords[2];
};

// This stores a skin name (We don't use this, just the name of the model to get the texture)
struct tMD3Skin {
	char			skinName[68];
};

// This class handles all of the main loading code
class CMD3Loader {
	public:
		// This inits the data members
		CMD3Loader();

		// This is the function that you call to load the MD3 model
		bool	importMD3(t3DModel *pModel, char *strFileName);
		
		// This loads a model's .skin file
		bool	loadSkin(t3DModel *pModel, LPSTR skinFileName);

		// This loads a weapon's .shader file
		bool	loadShader(t3DModel *pModel, LPSTR shaderFileName);

	private:
		// This reads in the data from the MD3 file and stores it in the member variables,
		// later to be converted to our cool structures so we don't depend on Quake3 stuff.
		void readMD3Data(t3DModel *pModel);

		// This converts the member variables to our pModel structure, and takes the model
		// to be loaded and the mesh header to get the mesh info.
		void convertToDataStructures(t3DModel *pModel, tMD3MeshInfo meshHeader);
		
		// This frees memory and closes the file
		void cleanUp();
		
		void logBonesInfo(tMD3Bone *pBones, int MAX_FRAMES);
		void logTagsInfo(tMD3Tag *pTags, int NO_OF_TAGS, int MAX_FRAMES);
		void logMeshesInfo(tMD3MeshInfo *meshHeader);
		void logSkinsInfo(tMD3Skin *pSkins, int MAX_SKINS);
		void logFacesInfo(tMD3Face *pFaces, int MAX_FACES);
		void logTextureCoordsInfo(tMD3TexCoord *pTexCoords, int MAX_VERTICES);
		void logVerticesInfo(tMD3Triangle *pTriangles, int MAX_FRAMES, int MAX_TRIANGLES);

		// Member Variables		

		// The file pointer
		FILE *mFilePointer;

		tMD3Header		mMD3Header;				// The header data
		
		tMD3Skin		*m_pSkins;				// The skin name data (not used)
		tMD3TexCoord	*m_pTexCoords;			// The texture coordinates
		tMD3Face		*m_pFaces;				// Face/Triangle data
		tMD3Triangle	*m_pVertices;			// Vertex/UV indices
		tMD3Bone		*m_pBones;				// This stores the bone data (not used)
};

// This is our model class that we use to load and draw and free the Quake3 characters
class CMD3Model {
	public:
		// These our our init and deinit() C++ functions (Constructor/Deconstructor)
		CMD3Model();
		~CMD3Model();
		
		// This loads the model from a path and name prefix.   It takes the path and
		// model name prefix to be added to _upper.md3, _lower.md3 or _head.md3.
		bool loadModel(LPSTR strPath, LPSTR strModel);

		// This loads the weapon and takes the same path and model name to be added to .md3
		bool loadWeapon(LPSTR strPath, LPSTR strModel);

		// This links a model to another model (pLink) so that it's the parent of that child.
		// The strTagName is the tag, or joint, that they will be linked at (I.E. "tag_torso").
		void linkModel(t3DModel *pModel, t3DModel *pLink, LPSTR strTagName);

		// This renders the character to the screen
		void drawModel();

		// This frees the character's data
		void destroyModel(t3DModel *pModel);

	private:
		// This loads the models textures with a given path
		void loadModelTextures(t3DModel *pModel, LPSTR strTexturePath);

		// This recursively draws the character models, starting with the lower.md3 model
		void drawLink(t3DModel *pModel);

		// This a md3 model to the screen (not the whole character)
		void renderModel(t3DModel *pModel);

		// Member Variables

		// This stores the texture array for each of the textures assigned to this model
		GLuint *mModelTextures;	

		// This stores a list of all the names of the textures that have been loaded.  
		// This was created so that we could check to see if a texture that is assigned
		// to a mesh has already been loaded.  If so, then we don't need to load it again
		// and we can assign the textureID to the same textureID as the first loaded texture.
		// You could get rid of this variable by doing something tricky in the texture loading
		// function, but I didn't want to make it too confusing to load the textures.
		vector<string> vStrTextures;

		// These are are models for the character's head and upper and lower body parts
		t3DModel	m_Head;
		t3DModel	m_Upper;
		t3DModel	m_Lower;

		// This store the players weapon model (optional load)
		t3DModel	m_Weapon;
};

#endif