
#include "MD3.h"
#include "main.h"

//using namespace ConsoleLog;

GLuint *texture;

char logText[255];
FILE *mFileConsoleLog;

void log(const char *format, ...) {
	va_list		ap;						// Pointer To List Of Arguments

	va_start(ap, format);					// Parses The String For Variables
	    vsprintf(logText, format, ap);	// And Converts Symbols To Actual Numbers
	va_end(ap);							// Results Are Stored In Text
	
	fwrite(logText, 1, strlen(logText), mFileConsoleLog);
}

bool bitmapLoader(LPCSTR fileName, GLuint textureID) {
	HBITMAP hBMP;
	BITMAP BMP;

	hBMP = (HBITMAP)LoadImage(GetModuleHandle(NULL), fileName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);

	if(!hBMP)
		return false;

	GetObject(hBMP, sizeof(BMP), &BMP);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, BMP.bmWidth, BMP.bmHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, BMP.bmBits);
/*
char sCurText[255];
sprintf(sCurText, "%s = %d x %d", fileName, BMP.bmWidth, BMP.bmHeight);
MessageBox(NULL, sCurText, "W x H", MB_OK);
//*/
	DeleteObject(hBMP);

	return true;
}

CMD3Model::CMD3Model() {
	// Here we initialize all our mesh structures for the character
	memset(&m_Head,  0, sizeof(t3DModel));
	memset(&m_Upper, 0, sizeof(t3DModel));
	memset(&m_Lower, 0, sizeof(t3DModel));
	memset(&m_Weapon, 0, sizeof(t3DModel));

	mModelTextures = new GLuint[MAX_TEXTURES];
	glGenTextures(MAX_TEXTURES, &mModelTextures[0]);
}

CMD3Model::~CMD3Model() {
	// Here we free all of the meshes in our model
	destroyModel(&m_Head);
	destroyModel(&m_Upper);
	destroyModel(&m_Lower);
	destroyModel(&m_Weapon);
}

//This loads our Quake3 model from the given path and character name
bool CMD3Model::loadModel(LPSTR strModelPath, LPSTR strModelName) {
	mFileConsoleLog = fopen("console.log", "wb");

	log("/******************* LOADING MODEL *******************/\r\n\r\n");	

	// This function is where all the character loading is taken care of.  We use
	// our CLoadMD3 class to load the 3 mesh and skins for the character. Since we
	// just have 1 name for the model, we add that to _lower.md3, _upper.md3 and _head.md3
	// to load the correct mesh files.

	char	strLowerModelFileName[255] = {0};	// This stores the file name for the lower.md3 model
	char	strUpperModelFileName[255] = {0};	// This stores the file name for the upper.md3 model
	char	strHeadModelFileName[255] = {0};	// This stores the file name for the head.md3 model
	char	strLowerSkinFileName[255] = {0};	// This stores the file name for the lower.md3 skin
	char	strUpperSkinFileName[255] = {0};	// This stores the file name for the upper.md3 skin
	char	strHeadSkinFileName[255] = {0};		// This stores the file name for the head.md3 skin
	CMD3Loader	md3Loader;

	// Make sure valid path and model names were passed in
	if(!strModelPath || !strModelName)
		return false;
	
	// Store the correct files names for the .md3 and .skin file for each body part.
	// We concatinate this on top of the path name to be loaded from.
	sprintf(strLowerModelFileName, "%s\\%s_lower.md3", strModelPath, strModelName);
	sprintf(strUpperModelFileName, "%s\\%s_upper.md3", strModelPath, strModelName);
	sprintf(strHeadModelFileName, "%s\\%s_head.md3", strModelPath, strModelName);

	// Get the skin file names with their path
	sprintf(strLowerSkinFileName, "%s\\%s_lower.skin", strModelPath, strModelName);
	sprintf(strUpperSkinFileName, "%s\\%s_upper.skin", strModelPath, strModelName);
	sprintf(strHeadSkinFileName, "%s\\%s_head.skin", strModelPath, strModelName);

	// Next we want to load the character meshes.  The CModelMD3 class has member
	// variables for the head, upper and lower body parts.  These are of type t3DModel.
	// Depending on which model we are loading, we pass in those structures to ImportMD3.
	// This returns a true of false to let us know that the file was loaded okay.  The
	// appropriate file name to load is passed in for the last parameter.
	
	// Load the head mesh (*_head.md3) and make sure it loaded properly
	if(!md3Loader.importMD3(&m_Head, strHeadModelFileName)) {
		// Display an error message telling us the file could not be found
		MessageBox(NULL, "Unable to load the HEAD model!", "Error", MB_OK);
		return false;
	}

	// Load the lower mesh (*_lower.md3) and make sure it loaded properly
	if(!md3Loader.importMD3(&m_Lower, strLowerModelFileName)) {
		// Display an error message telling us the file could not be found
		MessageBox(NULL, "Unable to load the LOWER model!", "Error", MB_OK);
		return false;
	}

	// Load the upper mesh (*_upper.md3) and make sure it loaded properly
	if(!md3Loader.importMD3(&m_Upper, strUpperModelFileName)) {
		// Display an error message telling us the file could not be found
		MessageBox(NULL, "Unable to load the UPPER model!", "Error", MB_OK);
		return false;
	}
	
	// Load the upper skin (*_upper.skin) and make sure it loaded properly
	if(!md3Loader.loadSkin(&m_Upper, strUpperSkinFileName)) {
		// Display an error message telling us the file could not be found
		MessageBox(NULL, "Unable to load the UPPER skin!", "Error", MB_OK);
		return false;
	}

	// Load the lower skin (*_lower.skin) and make sure it loaded properly
	if(!md3Loader.loadSkin(&m_Lower, strLowerSkinFileName)) {
		// Display an error message telling us the file could not be found
		MessageBox(NULL, "Unable to load the LOWER skin!", "Error", MB_OK);
		return false;
	}

	// Load the head skin (*_Head.skin) and make sure it loaded properly
	if(!md3Loader.loadSkin(&m_Head, strHeadSkinFileName)) {
		// Display an error message telling us the file could not be found
		MessageBox(NULL, "Unable to load the HEAD skin!", "Error", MB_OK);
		return false;
	}

	// Once the models and skins were loaded, we need to load then textures.
	// We don't do error checking for this because we call CreateTexture() and 
	// it already does error checking.  Most of the time there is only
	// one or two textures that need to be loaded for each character.  There are
	// different skins though for each character.  For instance, you could have a
	// army looking Lara Croft, or the normal look.  You can have multiple types of
	// looks for each model.  Usually it is just color changes though.

	// Load the lower, upper and head textures.  
	loadModelTextures(&m_Lower, strModelPath);
	loadModelTextures(&m_Upper, strModelPath);
	loadModelTextures(&m_Head,  strModelPath);

	// The character data should all be loaded when we get here (except the weapon).
	// Now comes the linking of the body parts.  This makes it so that the legs (lower.md3)
	// are the parent node, then the torso (upper.md3) is a child node of the legs.  Finally,
	// the head is a child node of the upper body.  What I mean by node, is that you can
	// think of the model having 3 bones and 2 joints.  When you translate the legs you want
	// the whole body to follow because they are inseparable (unless a magic trick goes wrong).
	// The same goes for the head, it should go wherever the body goes.  When we draw the
	// lower body, we then recursively draw all of it's children, which happen to be just the
	// upper body.  Then we draw the upper body's children, which is just the head.  So, to
	// sum this all up, to set each body part's children, we need to link them together.
	// For more information on tags, refer to the Quick Notes and the functions below.

	// Link the lower body to the upper body when the tag "tag_torso" is found in our tag array
	linkModel(&m_Lower, &m_Upper, "tag_torso");

	// Link the upper body to the head when the tag "tag_head" is found in our tag array
	linkModel(&m_Upper, &m_Head, "tag_head");

	// The character was loaded correctly so return true
	return true;
}

//This loads the textures for the current model passed in with a directory
void CMD3Model::loadModelTextures(t3DModel *pModel, LPSTR strTexturePath) {

	// This function loads the textures that are assigned to each mesh and it's
	// sub-objects.  For instance, the Lara Croft character has a texture for the body
	// and the face/head, and since she has the head as a sub-object in the lara_upper.md3 model, 
	// the MD3 file needs to contain texture information for each separate object in the mesh.
	// There is another thing to note too...  Some meshes use the same texture map as another 
	// one. We don't want to load 2 of the same texture maps, so we need a way to keep track of
	// which texture is already loaded so that we don't double our texture memory for no reason.
	// This is controlled with a STL vector list of "strings".  Every time we load a texture
	// we add the name of the texture to our list of strings.  Before we load another one,
	// we make sure that the texture map isn't already in our list.  If it is, we assign
	// that texture index to our current models material texture ID.  If it's a new texture,
	// then the new texture is loaded and added to our characters texture array: m_Textures[].

	// Go through all the materials that are assigned to this model
	for(int i = 0; i < pModel->numOfMaterials; i++) {
		
		// Check to see if there is a file name to load in this material
		if(strlen(pModel->pMaterials[i].textureFileName) > 0) {

			// Create a boolean to tell us if we have a new texture to load
			bool bNewTexture = true;

			// Go through all the textures in our string list to see if it's already loaded
			for(unsigned int j = 0; j < vStrTextures.size(); j++) {
				
				// If the texture name is already in our list of texture, don't load it again.
				if((strcmp(pModel->pMaterials[i].textureFileName, vStrTextures[j].c_str()) == 0)) {

					// We don't need to load this texture since it's already loaded
					bNewTexture = false;

					// Assign the texture index to our current material textureID.
					// This ID will them be used as an index into m_Textures[].
					pModel->pMaterials[i].textureID = j;
				}
			}

			// Make sure before going any further that this is a new texture to be loaded
			if(bNewTexture == false) 
				continue;

			char strFullPath[255] = {0};

			// Add the file name and path together so we can load the texture
			sprintf(strFullPath, "%s\\%s", strTexturePath, pModel->pMaterials[i].textureFileName);

			// We pass in a reference to an index into our texture array member variable.
			// The size() function returns the current loaded texture count.  Initially
			// it will be 0 because we haven't added any texture names to our strTextures list.	
			bitmapLoader(strFullPath, mModelTextures[vStrTextures.size()]);

			// Set the texture ID for this material by getting the current loaded texture count
			pModel->pMaterials[i].textureID = vStrTextures.size();

			// Now we increase the loaded texture count by adding the texture name to our
			// list of texture names.  Remember, this is used so we can check if a texture
			// is already loaded before we load 2 of the same textures.  Make sure you
			// understand what an STL vector list is.  We have a tutorial on it if you don't.
			vStrTextures.push_back(pModel->pMaterials[i].textureFileName);
		}
	}
}

void CMD3Model::linkModel(t3DModel *pModel, t3DModel *pLinkedModel, LPSTR strTagName) {
	
	// Make sure we have a valid model, link and tag name, otherwise quit this function
	if(!pModel || !pLinkedModel || !strTagName)
		return;

	// This function is used to link 2 models together at a psuedo joint.  For instance,
	// if we were animating an arm, we would link the top part of the arm to the shoulder,
	// then the forearm to would be linked to the top part of the arm, then the hand to
	// the forearm.  That is what is meant by linking.  That way, when we rotate the
	// arm at the shoulder, the rest of the arm will move with it because they are attached
	// to the same matrix that moves the top of the arm.  You can think of the shoulder
	// as the arm's parent node, and the rest are children that are subject to move to where
	// ever the top part of the arm goes.  That is how bone/skeletal animation works.
	//
	// So, we have an array of tags that have a position, rotation and name.  If we want
	// to link the lower body to the upper body, we would pass in the lower body mesh first,
	// then the upper body mesh, then the tag "tag_torso".  This is a tag that quake set as
	// as a standard name for the joint between the legs and the upper body.  This tag was
	// saved with the lower.md3 model.  We just need to loop through the lower body's tags,
	// and when we find "tag_torso", we link the upper.md3 mesh too that tag index in our
	// pLinks array.  This is an array of pointers to hold the address of the linked model.
	// Quake3 models are set up in a weird way, but usually you would just forget about a
	// separate array for links, you would just have a pointer to a t3DModel in the tag
	// structure, which in retrospect, you wouldn't have a tag array, you would have
	// a bone/joint array.  Stayed tuned for a bone animation tutorial from scratch.  This
	// will show you exactly what I mean if you are confused.
	
	// Go through all of our tags and find which tag contains the strTagName, then link'em
	for(int i = 0; i < pModel->numOfTags; i++) {

		// If this current tag index has the tag name we are looking for
		if(!_stricmp(pModel->pTags[i].tagName, strTagName)) {

			// Link the model's link index to the link (or model/mesh) and return
			pModel->pLinks[i] = pLinkedModel;
			return;
		}
	}
}

//This loads a Quake3 weapon model from the given path and weapon name
bool CMD3Model::loadWeapon(LPSTR strPath, LPSTR strModel) {
	
	char strWeaponModelFileName[255] = {0};		// This stores the file name for the weapon model
	char strWeaponShaderFileName[255] = {0};	// This stores the file name for the weapon shader.
	CMD3Loader md3Loader;

	// Make sure the path and model were valid, otherwise return false
	if(!strPath || !strModel)
		return false;

	// Concatenate the path and model name together
	sprintf(strWeaponModelFileName, "%s\\%s.md3", strPath, strModel);

	// Next we want to load the weapon mesh.  The CModelMD3 class has member
	// variables for the weapon model and all it's sub-objects.  This is of type t3DModel.
	// We pass in a reference to this model in to ImportMD3 to save the data read.
	// This returns a true of false to let us know that the weapon was loaded okay.  The
	// appropriate file name to load is passed in for the last parameter.

	// Load the weapon mesh (*.md3) and make sure it loaded properly
	if(!md3Loader.importMD3(&m_Weapon, strWeaponModelFileName)) {

		// Display the error message that we couldn't find the weapon MD3 file and return false
		MessageBox(NULL, "Unable to load the WEAPON model!", "Error", MB_OK);
		return false;
	}

	// Unlike the other .MD3 files, a weapon has a .shader file attached with it, not a
	// .skin file.  The shader file has it's own scripting language to describe behaviors
	// of the weapon.  All we care about for this tutorial is it's normal texture maps.
	// There are other texture maps in the shader file that mention the ammo and sphere maps,
	// but we don't care about them for our purposes.  I gutted the shader file to just store
	// the texture maps.  The order these are in the file is very important.  The first
	// texture refers to the first object in the weapon mesh, the second texture refers
	// to the second object in the weapon mesh, and so on.  I didn't want to write a complex
	// .shader loader because there is a TON of things to keep track of.  It's a whole
	// scripting language for goodness sakes! :)  Keep this in mind when downloading new guns.

	// Add the path, file name and .shader extension together to get the file name and path
	sprintf(strWeaponShaderFileName, "%s\\%s.shader", strPath, strModel);

	// Load our textures associated with the gun from the weapon shader file
	if(!md3Loader.loadShader(&m_Weapon, strWeaponShaderFileName)) {

		// Display the error message that we couldn't find the shader file and return false
		MessageBox(NULL, "Unable to load the SHADER file!", "Error", MB_OK);
		return false;
	}

	// We should have the textures needed for each weapon part loaded from the weapon's
	// shader, so let's load them in the given path.
	loadModelTextures(&m_Weapon, strPath);

	// Just like when we loaded the character mesh files, we need to link the weapon to
	// our character.  The upper body mesh (upper.md3) holds a tag for the weapon.
	// This way, where ever the weapon hand moves, the gun will move with it.

	// Link the weapon to the model's hand that has the weapon tag
	linkModel(&m_Upper, &m_Weapon, "tag_weapon");

	// The weapon loaded okay, so let's return true to reflect this
	return true;
}

//This frees our Quake3 model and all it's associated data
void CMD3Model::destroyModel(t3DModel *pModel) {
	// To free a model, we need to go through every sub-object and delete
	// their model data.  Since there is only one array of tags and links stored
	// for the model and all of it's objects, we need to only free the model's 
	// tags and links once.

	// Go through all the objects in the model
	for(int i = 0; i < pModel->numOfObjects; i++) {
		// Free the faces, normals, vertices, and texture coordinates.
		if(pModel->pObjects[i].pFaces)			delete [] pModel->pObjects[i].pFaces;
		if(pModel->pObjects[i].pVertices)		delete [] pModel->pObjects[i].pVertices;
		if(pModel->pObjects[i].pTexVertices)	delete [] pModel->pObjects[i].pTexVertices;
		if(pModel->pObjects[i].pNormals)		delete [] pModel->pObjects[i].pNormals;
		
		// Free the tags associated with this model
		if(pModel->pTags)
			delete [] pModel->pTags;

		// Free the links associated with this model (We use free because we used malloc())
		if(pModel->pLinks)
			free(pModel->pLinks);
	}
}

//This recursively draws all the character nodes, starting with the legs
void CMD3Model::drawModel() {
	// This is the function that is called by the client (you) when using the 
	// CModelMD3 class object.  You will notice that we rotate the model by
	// -90 degrees along the x-axis.  This is because most modelers have z up
	// so we need to compensate for this.  Usually I would just switch the
	// z and y values when loading in the vertices, but the rotations that
	// are stored in the tags (joint info) are a matrix, which makes it hard
	// to change those to reflect Y up.  I didn't want to mess with that so
	// this 1 rotate will fix this problem.

	// Rotate the model to compensate for the z up orientation that the model was saved
	glRotatef(-90, 1, 0, 0);
	
	// You might be thinking to draw the model we would just call RenderModel()
	// 4 times for each body part and the gun right?  That sounds logical, but since
	// we are doing a bone/joint animation... and the models need to be linked together,
	// we can't do that.  It totally would ignore the tags.  Instead, we start at the
	// root model, which is the legs.  The legs drawn first, then we go through each of
	// the legs linked tags (just the upper body) and then it with the tag's rotation
	// and translation values.  I ignored the rotation in this tutorial since we aren't
	// doing any animation.  I didn't want to overwhelm you with quaternions just yet :)
	// Normally in skeletal animation, the root body part is the hip area.  Then the legs
	// bones are created as children to the torso.  The upper body is also a child to
	// the torso.  Since the legs are one whole mesh, this works out somewhat the same way.
	// This wouldn't work if the feet and legs weren't connected in the same mesh because
	// the feet rotations and positioning don't directly effect the position and rotation
	// of the upper body, the hips do.  If that makes sense...  That is why the root starts
	// at the hips and moves down the legs, and also branches out to the upper body and
	// out to the arms.

	// Draw the first link, which is the lower body.  This will then recursively go
	// through the models attached to this model and drawn them.
	drawLink(&m_Lower);
}

//This draws the current mesh with an effected matrix stack from the last mesh
void CMD3Model::drawLink(t3DModel *pModel) {
	// This function is our recursive function that handles the bone animation
	// so to speak.  We first draw the model that is passed in (first the legs),
	// then go through all of it's tags and draw them.  Notice that when we
	// draw the model that is linked to the current model a new matrix scope
	// is created with glPushMatrix() and glPopMatrix().  This is because each tag
	// has a rotation and translation operation assigned to it.  For instance, when
	// Lara does her back flip death animation, the legs send a rotation and translation 
	// to the rest of the body to be rotated along with the legs as they flip backwards.  
	// If you didn't do this, Lara's body and head would stay in the same place as the
	// legs did a back flipped and landed on the floor.  Of course, this would look really
	// stupid.  A 270-degree rotation to the rest of the body is done for that animation.
	// Keep in mind, the legs mesh is NEVER translated or rotated.  It only rotates and
	// translates the upper parts of the body.  All the rotation and translation of the
	// legs is done in the canned animation that was created in the modeling program.
	// Keep in mind that I ignore the rotation value for that is given in the tag since
	// it doesn't really matter for a static model.  Also, since the rotation is given
	// in a 3x3 rotation matrix, it would be a bit more code that could make you frustrated.

	// Draw the current model passed in (Initially the legs)
	renderModel(pModel);

	// Now we need to go through all of this models tags and draw them.
	for(int i = 0; i < pModel->numOfTags; i++) {
		// Get the current link from the models array of links (Pointers to models)
		t3DModel *pLinkedModel = pModel->pLinks[i];

		// If this link has a valid address, let's draw it!
		if(pLinkedModel) {
//char logText[255];
//sprintf(logText, "NUM_OF_TAGS %d", pModel->numOfTags);
//MessageBox(NULL, logText, "aaaaaaaa", MB_OK);

			// Let's grab the translation for this new model that will be drawn 
			CVector3 vTranslatePosition = pModel->pTags[i].vTranslatePosition;

			// Start a new matrix scope
			glPushMatrix();
				
				// Translate the new model to be drawn to it's position
				glTranslatef(vTranslatePosition.x, vTranslatePosition.y, vTranslatePosition.z);

				// Recursively draw the next model that is linked to the current one.
				// This could either be a body part or a gun that is attached to
				// the hand of the upper body model.
				drawLink(pLinkedModel);

			// End the current matrix scope
			glPopMatrix();
		}
	}
}

void CMD3Model::renderModel(t3DModel *pModel) {
	// This function actually does the rendering to OpenGL.  If you have checked out
	// our other file loading tutorials, it looks pretty much the same as those.  I
	// left out the normals though.  You can go to any other loading and copy the code
	// from those. Usually the Quake models creating the lighting effect in their textures
	// anyway.  

	// Make sure we have valid objects just in case. (size() is in the STL vector class)
	if(pModel->pObjects.size() <= 0)
		return;

	// Go through all of the objects stored in this model
	for(int i = 0; i < pModel->numOfObjects; i++) {
		// Get the current object that we are displaying
		t3DObject *currentObject = &pModel->pObjects[i];

		// If the object has a texture assigned to it, let's bind it to the model.
		// This isn't really necessary since all models have textures, but I left this
		// in here to keep to the same standard as the rest of the model loaders.
		if(currentObject->bHasTexture) {
			
			//Turn Texture Mapping On
			glEnable(GL_TEXTURE_2D);

			// Grab the texture index from the materialID index into our material list
			int textureID = pModel->pMaterials[currentObject->materialID].textureID;

			// Bind the texture index that we got from the material textureID
			glBindTexture(GL_TEXTURE_2D, mModelTextures[textureID]);
		}
		else {
			
			//Turn Texture Mapping Off
			glDisable(GL_TEXTURE_2D);
		}

		// Start drawing our model triangles
		glBegin(GL_TRIANGLES);
			// Go through all of the faces (polygons) of the object and draw them
			for(int j = 0; j < currentObject->numOfFaces; j++) {
				
				// Go through each vertex of the triangle and draw it.
				for(int whichVertex = 0; whichVertex < 3; whichVertex++) {

					// Get the index for the current point in the face list
					int index = currentObject->pFaces[j].vertexIndex[whichVertex];

					// Make sure there is texture coordinates for this (%99.9 likelyhood)
					if(currentObject->pTexVertices) {
						
						// Assign the texture coordinate to this vertex
						glTexCoord2f(	currentObject->pTexVertices[index].x,
										currentObject->pTexVertices[index].y
									);
					}

					// Get the vertex that we are dealing with.  This code will change
					// a bunch when we doing our key frame animation in the next .MD3 tutorial.
					CVector3 vPoint1 = currentObject->pVertices[index];

					// Render the current vertex
					glVertex3f(	vPoint1.x,
								vPoint1.y,
								vPoint1.z
							);
				}
			}

		// Stop drawing polygons
		glEnd();
	}
}

/////////////////////////////////////////////////////CMD3Loader///////////////////////////////////////////////////////////
CMD3Loader::CMD3Loader() {
	// Here we initialize our structures to 0
	memset(&mMD3Header, 0, sizeof(tMD3Header));

	// Set the pointers to null
	m_pSkins = NULL;
	m_pTexCoords = NULL;
	m_pFaces = NULL;
	m_pBones = NULL;
}

//This is called by the client to open the .Md3 file, read it, then clean up
bool CMD3Loader::importMD3(t3DModel *pModel, LPSTR strFileName) {
	log("\t/******************* IMPORTING MODEL = %s\r\n\r\n", strFileName);

	// This function handles the entire model loading for the .md3 models.
	// What happens is we load the file header, make sure it's a valid MD3 model,
	// then load the rest of the data, then call our CleanUp() function.

	// Open the MD3 file in binary
	mFilePointer = fopen(strFileName, "rb");
	
	// Make sure we have a valid file pointer (we found the file)
	if(!mFilePointer) {
		// Display an error message and don't load anything if no file was found
		sprintf(logText, "Unable to find the file: %s!", strFileName);
		MessageBox(NULL, logText, "Error", MB_OK);

		return false;
	}

	// Now that we know the file was found and it's all cool, let's read in
	// the header of the file.  If it has the correct 4 character ID and version number,
	// we can continue to load the rest of the data, otherwise we need to print an error.

	// Read the header data and store it in our m_Header member variable
	fread(&mMD3Header, 1, sizeof(tMD3Header), mFilePointer);
	
	log("\t\t/**************** MD3 MODEL HEADER ****************/\r\n");
		log("\t\t\t fileID[4]\t\t\t= %s\r\n", mMD3Header.fileID);
		log("\t\t\t version;\t\t\t= %d\r\n", mMD3Header.version);
		log("\t\t\t fileName[68]\t\t\t= %s\r\n", mMD3Header.fileName);
		log("\t\t\t numOfFrames\t\t\t= %d\r\n", mMD3Header.numOfFrames);
		log("\t\t\t numOfTags\t\t\t= %d\r\n", mMD3Header.numOfTags);
		log("\t\t\t numOfMeshes\t\t\t= %d\r\n", mMD3Header.numOfMeshes);
		log("\t\t\t numMaxSkins\t\t\t= %d\r\n", mMD3Header.numMaxSkins);
		log("\t\t\t headerSize\t\t\t= %d\r\n", mMD3Header.headerSize);
		log("\t\t\t tagStart\t\t\t= %d\r\n", mMD3Header.tagStart);
		log("\t\t\t tagEnd\t\t\t\t= %d\r\n", mMD3Header.tagEnd);
		log("\t\t\t fileSize\t\t\t= %d\r\n", mMD3Header.fileSize);
	log("\t\t/**************** END OF MD3 MODEL HEADER ****************/\r\n\r\n");
	// Get the 4 character ID
	char *ID = mMD3Header.fileID;
	// The ID MUST equal "IDP3" and the version MUST be 15, or else it isn't a valid
	// .MD3 file.  This is just the numbers ID Software chose.

	// Make sure the ID == IDP3 and the version is this crazy number '15' or else it's a bad egg
	if(	ID[0] != 'I' 
		||
		ID[1] != 'D' 
		||
		ID[2] != 'P' 
		||
		ID[3] != '3' 
		||
		mMD3Header.version != 15
	) {
		// Display an error message for bad file format, then stop loading
		sprintf(logText, "Invalid file format (Version not 15): %s!", strFileName);
		MessageBox(NULL, logText, "Error", MB_OK);
		log("Invalid file format (Version not 15): %s!", strFileName);
		return false;	
	}

	// Read in the model and animation data
	readMD3Data(pModel);
	
	// Clean up after everything
	cleanUp();
	
	log("\t/**************** FINISHED IMPORTING ****************/ \r\n\r\n");
	// Return a success
	return true;
}

bool IsInString(string strString, string strSubString)
{
	// Make sure both of these strings are valid, return false if any are empty
	if(strString.length() <= 0 || strSubString.length() <= 0) return false;

	// grab the starting index where the sub string is in the original string
	unsigned int index = strString.find(strSubString);

	// Make sure the index returned was valid
	if(index >= 0 && index < strString.length())
		return true;

	// The sub string does not exist in strString.
	return false;
}

//This loads the texture information for the model from the *.skin file
bool CMD3Loader::loadSkin(t3DModel *pModel, LPSTR skinFileName) {
	// Make sure valid data was passed in
	if(!pModel || !skinFileName) return false;

	// This function is used to load a .skin file for the .md3 model associated
	// with it.  The .skin file stores the textures that need to go with each
	// object and subject in the .md3 files.  For instance, in our Lara Croft model,
	// her upper body model links to 2 texture; one for her body and the other for
	// her face/head.  The .skin file for the lara_upper.md3 model has 2 textures:
	//
	// u_torso,models/players/laracroft/default.bmp
	// u_head,models/players/laracroft/default_h.bmp
	//
	// Notice the first word, then a comma.  This word is the name of the object
	// in the .md3 file.  Remember, each .md3 file can have many sub-objects.
	// The next bit of text is the Quake3 path into the .pk3 file where the 
	// texture for that model is stored  Since we don't use the Quake3 path
	// because we aren't making Quake, I just grab the texture name at the
	// end of the string and disregard the rest.  of course, later this is
	// concatenated to the original MODEL_PATH that we passed into load our character.
	// So, for the torso object it's clear that default.bmp is assigned to it, where
	// as the head model with the pony tail, is assigned to default_h.bmp.  Simple enough.
	// What this function does is go through all the lines of the .skin file, and then
	// goes through all of the sub-objects in the .md3 file to see if their name is
	// in that line as a sub string.  We use our cool IsInString() function for that.
	// If it IS in that line, then we know that we need to grab it's texture file at
	// the end of the line.  I just parse backwards until I find the last '/' character,
	// then copy all the characters from that index + 1 on (I.E. "default.bmp").
	// Remember, it's important to note that I changed the texture files from .tga
	// files to .bmp files because that is what all of our tutorials use.  That way
	// you don't have to sift through tons of image loading code.  You can write or
	// get your own if you really want to use the .tga format.

	// Open the skin file
	ifstream fin(skinFileName);

	// Make sure the file was opened
	if(fin.fail()) {
		// Display the error message and return false
		MessageBox(NULL, "Unable to load skin!", "Error", MB_OK);
		return false;	
	}

	// These 2 variables are for reading in each line from the file, then storing
	// the index of where the bitmap name starts after the last '/' character.
	string strLine = "";
	int textureNameStart = 0;

	// Go through every line in the .skin file
	while(getline(fin, strLine)) {
		// Loop through all of our objects to test if their name is in this line
		for(int i = 0; i < pModel->numOfObjects; i++) {
			// Check if the name of this object appears in this line from the skin file
			// Check if the name of this object appears in this line from the skin file
			if( IsInString(strLine, pModel->pObjects[i].objectName) ) {
				// To abstract the texture name, we loop through the string, starting
				// at the end of it until we find a '/' character, then save that index + 1.
				for(int j = strLine.length() - 1; j > 0; j--)
				{
					// If this character is a '/', save the index + 1
					if(strLine[j] == '/')
					{
						// Save the index + 1 (the start of the texture name) and break
						textureNameStart = j + 1;
						break;
					}
				}

				// Create a local material info structure
				tMaterial material;

				// Copy the name of the file into our texture file name variable.
				// Notice that with string we can pass in the address of an index
				// and it will only pass in the characters from that point on. Cool huh?
				// So now the strFile name should hold something like ("bitmap_name.bmp")
				strcpy(material.textureFileName, &strLine[textureNameStart]);
				
				// The tile or scale for the UV's is 1 to 1 
				material.uTile = material.vTile = 1;

				// Store the material ID for this object and set the texture boolean to true
				pModel->pObjects[i].materialID = pModel->numOfMaterials;
				pModel->pObjects[i].bHasTexture = true;

				// Here we increase the number of materials for the model
				pModel->numOfMaterials++;

				// Add the local material info structure to our model's material list
				pModel->pMaterials.push_back(material);
			}
		}
	}

	// Close the file and return a success
	fin.close();
	return true;
}

//This loads the basic shader texture info associated with the weapon model
bool CMD3Loader::loadShader(t3DModel *pModel, LPSTR shaderFileName) {
	// Make sure valid data was passed in
	if(!pModel || !shaderFileName)
		return false;

	// This function is used to load the .shader file that is associated with
	// the weapon model.  Instead of having a .skin file, weapons use a .shader file
	// because it has it's own scripting language to describe the behavior of the
	// weapon.  There is also many other factors like environment map and sphere map
	// textures, among other things.  Since I am not trying to replicate Quake, I
	// just care about the weapon's texture.  I went through each of the blocks
	// in the shader file and deleted everything except the texture name (of course
	// I changed the .tga files to .bmp for our purposes).  All this file now includes
	// is a texture name on each line.  No parsing needs to be done.  It is important
	// to keep in mind that the order of which these texture are stored in the file
	// is in the same order each sub-object is loaded in the .md3 file.  For instance,
	// the first texture name on the first line of the shader is the texture for
	// the main gun object that is loaded, the second texture is for the second sub-object
	// loaded, and so on. I just want to make sure that you understand that I hacked
	// up the .shader file so I didn't have to parse through a whole language.  This is
	// not a normal .shader file that we are loading.  I only kept the relevant parts.

	// Open the shader file
	ifstream fin(shaderFileName);

	// Make sure the file was opened
	if(fin.fail()) {
		
		// Display the error message and return false
		MessageBox(NULL, "Unable to load shader!", "Error", MB_OK);
		return false;
	}

	// These variables are used to read in a line at a time from the file, and also
	// to store the current line being read so that we can use that as an index for the 
	// textures, in relation to the index of the sub-object loaded in from the weapon model.
	string strLine = "";
	int currentIndex = 0;

	// Go through and read in every line of text from the file
	while(getline(fin, strLine)) {
		
		// Create a local material info structure
		tMaterial material;
			
		// Copy the name of the file into our texture file name variable
		strcpy(material.textureFileName, strLine.c_str());

		// The tile or scale for the UV's is 1 to 1 
		material.uTile = material.vTile = 1;

		// Store the material ID for this object and set the texture boolean to true
		pModel->pObjects[currentIndex].materialID = pModel->numOfMaterials;
		pModel->pObjects[currentIndex].bHasTexture = true;

		// Here we increase the number of materials for the model
		pModel->numOfMaterials++;
		
		// Save the Material in a List
		// Add the local material info structure to our model's material list
		pModel->pMaterials.push_back(material);

		// Here we increase the material index for the next texture (if any)
		currentIndex++;
	}

	// Close the file and return a success
	fin.close();
	return true;
}

//This function reads in all of the model's data, except the animation frames
void CMD3Loader::readMD3Data(t3DModel *pModel) {
	int i = 0;

	// This member function is the BEEF of our whole file.  This is where the
	// main data is loaded.  The frustrating part is that once the data is loaded,
	// you need to do a billion little things just to get the model loaded to the screen
	// in a correct manner.
	
	// Here we allocate memory for the bone information and read the bones in.
	m_pBones = new tMD3Bone[mMD3Header.numOfFrames];
	fread(m_pBones, sizeof(tMD3Bone), mMD3Header.numOfFrames, mFilePointer);
	logBonesInfo(m_pBones, mMD3Header.numOfFrames);

	// Since we don't care about the bone positions, we just free it immediately.
	// It might be cool to display them so you could get a visual of them with the model.

	// Free the unused bones
	delete [] m_pBones;
	
	
	// Next, after the bones are read in, we need to read in the tags.  Below we allocate
	// memory for the tags and then read them in.  For every frame of animation there is
	// an array of tags.
	pModel->pTags = new tMD3Tag[mMD3Header.numOfFrames * mMD3Header.numOfTags];
	fread(pModel->pTags, sizeof(tMD3Tag), mMD3Header.numOfFrames * mMD3Header.numOfTags, mFilePointer);
	logTagsInfo(pModel->pTags, mMD3Header.numOfTags, mMD3Header.numOfFrames);

	// Assign the number of tags to our model
	pModel->numOfTags = mMD3Header.numOfTags;

	// Now we want to initialize our links.  Links are not read in from the .MD3 file, so
	// we need to create them all ourselves.  We use a double array so that we can have an
	// array of pointers.  We don't want to store any information, just pointers to t3DModels.
	// Aa - Links is an pointer to a MODEL
	pModel->pLinks = (t3DModel**) malloc( sizeof(t3DModel) * mMD3Header.numOfTags);
	
	// Initilialize our link pointers to NULL
	for(i = 0; i < mMD3Header.numOfTags; i++)
		pModel->pLinks[i] = NULL;

	// Now comes the loading of the mesh data.  We want to use ftell() to get the current
	// position in the file.  This is then used to seek to the starting position of each of
	// the mesh data arrays.

	// Get the current offset into the file
	long meshOffset = ftell(mFilePointer);

	// Create a local meshHeader that stores the info about the mesh
	tMD3MeshInfo meshHeader;
	
	log("\t\t/**************** MESHES ****************/ \r\n");
	log("\t\tMAX_MESHES = %d\r\n", mMD3Header.numOfMeshes);

	// Go through all of the sub-objects in this mesh
	for(i = 0; i < mMD3Header.numOfMeshes; i++) {
		log("\t\t\tMESHES_NO = %d\r\n", i);
		
		// Seek to the start of this mesh and read in it's header
		fseek(mFilePointer, meshOffset, SEEK_SET);
		fread(&meshHeader, sizeof(tMD3MeshInfo), 1, mFilePointer);
		logMeshesInfo(&meshHeader);

		// Here we allocate all of our memory from the header's information
		m_pSkins		= new tMD3Skin[meshHeader.numOfSkins];
		m_pTexCoords	= new tMD3TexCoord[meshHeader.numOfVertices];
		m_pVertices		= new tMD3Triangle[meshHeader.numOfVertices * meshHeader.numOfMeshFrames];
		m_pFaces		= new tMD3Face[meshHeader.numOfTriangles];

		// Read in the skin information
		fread(m_pSkins, sizeof(tMD3Skin), meshHeader.numOfSkins, mFilePointer);
		logSkinsInfo(m_pSkins, meshHeader.numOfSkins);

		// Seek to the start of the triangle/face data, then read it in
		fseek(mFilePointer, meshOffset + meshHeader.offsetTriangleStart, SEEK_SET);
		fread(m_pFaces, sizeof(tMD3Face), meshHeader.numOfTriangles, mFilePointer);
		logFacesInfo(m_pFaces, meshHeader.numOfTriangles);

		// Seek to the start of the UV coordinate data, then read it in
		fseek(mFilePointer, meshOffset + meshHeader.offsetUVStart, SEEK_SET);
		fread(m_pTexCoords, sizeof(tMD3TexCoord), meshHeader.numOfVertices, mFilePointer);
		logTextureCoordsInfo(m_pTexCoords, meshHeader.numOfVertices);

		// Seek to the start of the vertex/face index information, then read it in.
		fseek(mFilePointer, meshOffset + meshHeader.offsetVertexStart, SEEK_SET);
		fread(m_pVertices, sizeof(tMD3Triangle), meshHeader.numOfVertices * meshHeader.numOfMeshFrames, mFilePointer);
		logVerticesInfo(m_pVertices, meshHeader.numOfMeshFrames, meshHeader.numOfVertices);

		// Now that we have the data loaded into the Quake3 structures, let's convert them to
		// our data types like t3DModel and t3DObject.  That way the rest of our model loading
		// code will be mostly the same as the other model loading tutorials.
		convertToDataStructures(pModel, meshHeader);

		// Free all the memory for this mesh since we just converted it to our structures
		delete [] m_pSkins;
		delete [] m_pFaces;
		delete [] m_pTexCoords;
		delete [] m_pVertices;

		// Increase the offset into the file
		meshOffset += meshHeader.meshSize;
	}

	log("\t\t/**************** END OF MESHES ****************/ \r\n\r\n");
}

void CMD3Loader::convertToDataStructures(t3DModel *pModel, tMD3MeshInfo meshHeader) {
	int i = 0;

	// This is function takes care of converting all of the Quake3 structures to our
	// structures that we have been using in all of our mode loading tutorials.  You
	// do not need this function if you are going to be using the Quake3 structures.
	// I just wanted to make it modular with the rest of the tutorials so you (me really) 
	// can make a engine out of them with an abstract base class.  Of course, each model
	// has some different data variables inside of the, depending on each format, but that
	// is perfect for some cool inheritance.  Just like in the .MD2 tutorials, we only
	// need to load in the texture coordinates and face information for one frame
	// of the animation (eventually in the next tutorial).  Where, the vertex information
	// needs to be loaded for every new frame, since it's vertex key frame animation 
	// used in .MD3 models.  Half-life models do NOT do this I believe.  It's just
	// pure bone/skeletal animation.  That will be a cool tutorial if the time ever comes.

	// Increase the number of objects (sub-objects) in our model since we are loading a new one
	pModel->numOfObjects++;

	// Create a empty object structure to store the object's info before we add it to our list
	t3DObject currentObject = {0};

	// Copy the name of the object to our object structure
	strcpy(currentObject.objectName, meshHeader.meshName);

	// Assign the vertex, texture coord and face count to our new structure
	currentObject.numOfVertices			= meshHeader.numOfVertices;
	currentObject.numOfTextureVertex	= meshHeader.numOfVertices;
	currentObject.numOfFaces			= meshHeader.numOfTriangles;

	// Allocate memory for the vertices, texture coordinates and face data.
	// Notice that we multiply the number of vertices to be allocated by the
	// number of frames in the mesh.  This is because each frame of animation has a 
	// totally new set of vertices.  This will be used in the next animation tutorial.
	currentObject.pVertices				= new CVector3[currentObject.numOfVertices * meshHeader.numOfMeshFrames];
	currentObject.pTexVertices			= new CVector2[currentObject.numOfVertices];
	currentObject.pFaces				= new tFace[currentObject.numOfFaces];

	// Go through all of the vertices and assign them over to our structure
	for(i = 0; i < currentObject.numOfVertices * meshHeader.numOfMeshFrames; i++) {
		// For some reason, the ratio 64 is what we need to divide the vertices by,
		// otherwise the model is gargantuanly huge!  If you use another ratio, it
		// screws up the model's body part position.  I found this out by just
		// testing different numbers, and I came up with 65.  I looked at someone
		// else's code and noticed they had 64, so I changed it to that.  I have never
		// read any documentation on the model format that justifies this number, but
		// I can't get it to work without it.  Who knows....  Maybe it's different for
		// 3D Studio Max files verses other software?  You be the judge.  I just work here.. :)
		currentObject.pVertices[i].x = m_pVertices[i].vertex[0] / 64.0f;
		currentObject.pVertices[i].y = m_pVertices[i].vertex[1] / 64.0f;
		currentObject.pVertices[i].z = m_pVertices[i].vertex[2] / 64.0f;
	}

	// Go through all of the uv coords and assign them over to our structure
	for(i = 0; i < currentObject.numOfVertices; i++) {
		// Since I changed the images to bitmaps, we need to negate the V ( or y) value.
		// This is because I believe that TARGA (.tga) files, which were originally used
		// with this model, have the pixels flipped horizontally.  If you use other image
		// files and your texture mapping is crazy looking, try deleting this negative.
		currentObject.pTexVertices[i].x = m_pTexCoords[i].texCoords[0];
		currentObject.pTexVertices[i].y = -m_pTexCoords[i].texCoords[1];
	}

	// Go through all of the face data and assign it over to OUR structure
	for(i = 0; i < currentObject.numOfFaces; i++) {

		// Assign the vertex indices to our face data
		currentObject.pFaces[i].vertexIndex[0] = m_pFaces[i].vertexIndexes[0];
		currentObject.pFaces[i].vertexIndex[1] = m_pFaces[i].vertexIndexes[1];
		currentObject.pFaces[i].vertexIndex[2] = m_pFaces[i].vertexIndexes[2];

		// Assign the texture coord indices to our face data (same as the vertex indices)
		currentObject.pFaces[i].texCoordIndex[0] = m_pFaces[i].vertexIndexes[0];
		currentObject.pFaces[i].texCoordIndex[1] = m_pFaces[i].vertexIndexes[1];
		currentObject.pFaces[i].texCoordIndex[2] = m_pFaces[i].vertexIndexes[2];
	}

	// Here we add the current object to our list object list
	pModel->pObjects.push_back(currentObject);
}

void CMD3Loader::cleanUp() {

	// Since we free all of the member variable arrays in the same function as
	// we allocate them, we don't need to do any other clean up other than
	// closing the file pointer, which could probably also be done in the same
	// function.  I left it here so you can add more of your cleanup if you add
	// to this class. 

	// Close the current file pointer
	fclose(mFilePointer);
	mFilePointer = NULL;

	//fclose(mFileConsoleLog);
	//mFileConsoleLog = NULL;
}

void CMD3Loader::logBonesInfo(tMD3Bone *pBones, int MAX_FRAMES) {
	log("\t\t/**************** BONE STRUCTURE ****************/ \r\n");

	for(int i = 0; i < MAX_FRAMES; i++) {
		log("\t\t\tBONE NO - %d of %d\r\n", i, MAX_FRAMES);

		log("\t\t\t\tBounding Box Min.\t\t====> mins[3]\t\t\t= %0.2f\t%0.2f\t%0.2f\r\n", pBones[i].mins[0], m_pBones[i].mins[1], m_pBones[i].mins[2]);
		log("\t\t\t\tBounding Box Max.\t\t====> maxs[3]\t\t\t= %0.2f\t%0.2f\t%0.2f\r\n", m_pBones[i].maxs[0], m_pBones[i].maxs[1], m_pBones[i].maxs[2]);
		log("\t\t\t\tLocal Origin\t\t\t====> position[3]\t\t= %0.2f\t%0.2f\t%0.2f\r\n", m_pBones[i].position[0], m_pBones[i].position[1], m_pBones[i].position[2]);
		log("\t\t\t\tBounding Sphere Radius\t\t====> scale\t\t\t= %0.2f\r\n", m_pBones[i].scale);
		log("\t\t\t\tCreator\t\t\t\t====> creator[16]\t\t= %s\r\n", m_pBones[i].creator);
	}

	log("\t\t/**************** END BONE STRUCTURE ****************/ \r\n\r\n");


}

void CMD3Loader::logTagsInfo(tMD3Tag *pTags, int NO_OF_TAGS, int MAX_FRAMES) {
	log("\t\t/**************** TAGS STRUCTURE ****************/ \r\n");

	for(int x = 0; x < NO_OF_TAGS; x++) {
		//log("\t\t\t\t FRAME NO - %d\r\n", i);
		log("\t\t\tTAG_NO = %d of %d\r\n", x, NO_OF_TAGS);

		for(int y = 0; y < MAX_FRAMES; y++) {
			log("\t\t\t\tFRAME_NO = %d of %d\r\n", y, MAX_FRAMES);

			log("\t\t\t\t\tpTags->tagName\t\t\t= %s\r\n", pTags[x*y].tagName);
			log("\t\t\t\t\tpTags->vTranslatePosition\t= %0.2f\t%0.2f\t%0.2f\r\n", pTags[x*y].vTranslatePosition.x, pTags[x*y].vTranslatePosition.y, pTags[x*y].vTranslatePosition.z);

			//log("\t\t\t\t\tpTags->rotationMatrix[3][3]\t= ???\r\n");
			for(int m = 0; m < 3; m++) {
				log("\t\t\t\t\tpTags->rotationMatrix[%d][3]\t= %0.2f \t%0.2f \t%0.2f\r\n", m, pTags[x*y].rotationMatrix[m][0], pTags[x*y].rotationMatrix[m][1], pTags[x*y].rotationMatrix[m][2]);
			}
		}
	}
	log("\t\t/**************** END OF TAGS STRUCTURE ****************/ \r\n\r\n");


}

void CMD3Loader::logMeshesInfo(tMD3MeshInfo *meshHeader) {
	log("\t\t\t/**************** MESHES INFO ****************/ \r\n");

	log("\t\t\t\t meshID[4]\t\t\t= %s\r\n",				meshHeader->meshID);
	log("\t\t\t\t meshName[68]\t\t\t= %s\r\n",			meshHeader->meshName);
	log("\t\t\t\t numOfMeshFrames\t\t= %d\r\n",			meshHeader->numOfMeshFrames);
	log("\t\t\t\t numOfSkins\t\t\t= %d\r\n",			meshHeader->numOfSkins);
	log("\t\t\t\t numOfVertices\t\t\t= %d\r\n",			meshHeader->numOfVertices);
	log("\t\t\t\t numOfTriangles\t\t\t= %d\r\n",		meshHeader->numOfTriangles);
	log("\t\t\t\t offsetTriangleStart\t\t= %d\r\n",		meshHeader->offsetTriangleStart);
	log("\t\t\t\t headerSize\t\t\t= %d\r\n",			meshHeader->headerSize);
	log("\t\t\t\t offsetUVStart\t\t\t= %d\r\n",			meshHeader->offsetUVStart);
	log("\t\t\t\t offsetVertexStart\t\t= %d\r\n",		meshHeader->offsetVertexStart);
	log("\t\t\t\t meshSize\t\t\t= %d\r\n",				meshHeader->meshSize);

	log("\t\t\t/**************** END OF MESHES INFO ****************/ \r\n\r\n");
}
void CMD3Loader::logSkinsInfo(tMD3Skin *pSkins, int MAX_SKINS) {
	log("\t\t\t/**************** SKINS INFO ****************/ \r\n");
	
	for(int i = 0; i < MAX_SKINS; i++) {
		log("\t\t\t\t SKIN - %d of %d\r\n", i, MAX_SKINS);
		log("\t\t\t\t\t skinName[68] = %s\r\n", pSkins[i].skinName);
	}

	log("\t\t\t/**************** END OF SKINS INFO ****************/ \r\n\r\n");
}

void CMD3Loader::logFacesInfo(tMD3Face *pFaces, int MAX_FACES) {
	log("\t\t\t/**************** FACE INFO ****************/ \r\n");
	
	for(int i = 0; i < MAX_FACES; i++) {
		log("\t\t\t\t FACE - %d of %d ====> vertexIndexes[3] 0 1 2 =\t%d\t%d\t%d\r\n", i, MAX_FACES, pFaces[i].vertexIndexes[0], pFaces[i].vertexIndexes[1], pFaces[i].vertexIndexes[2]);
	}

	log("\t\t\t/**************** END OF FACE INFO ****************/ \r\n\r\n");
}

void CMD3Loader::logTextureCoordsInfo(tMD3TexCoord *pTexCoords, int MAX_VERTICES) {
	log("\t\t\t/**************** TEXTURE COORDINATE INDEX INFO ****************/ \r\n");
	
	for(int i = 0; i < MAX_VERTICES; i++) {
		log("\t\t\t\t TEXTURE INDEX - %d of %d ====> texCoords[2] u v =\t%0.2f\t%0.2f\r\n", i, MAX_VERTICES, pTexCoords[i].texCoords[0], -pTexCoords[i].texCoords[1]);
	}

	log("\t\t\t/**************** END OF TEXTURE COORDINATE INDEX INFO ****************/ \r\n\r\n");
}

void CMD3Loader::logVerticesInfo(tMD3Triangle *pTriangles, int MAX_FRAMES, int MAX_VERTICES) {
	log("\t\t\t/**************** VERTICES LIST ****************/ \r\n");
	
	for(int i = 0; i < MAX_FRAMES; i++) {
		log("\t\t\t\t FRAME NO - %d\r\n", i);

		for(int j = 0; j < MAX_VERTICES; j++) {
			log("\t\t\t\t\t VERTEX NO - %d of %d \r\n", j, MAX_VERTICES);
			log("\t\t\t\t\t\t Vertex - x y z\t=\t%0.2f\t%0.2f\t%0.2f\r\n",	pTriangles[j].vertex[0]/64.0f, 
																		pTriangles[j].vertex[1]/64.0f, 
																		pTriangles[j].vertex[2]/64.0f
																	);
			log("\t\t\t\t\t\t Normal - x y\t=\t%d\t%d\r\n", pTriangles[j].normal[0], pTriangles[j].normal[1]);
		}
	}

	log("\t\t\t/**************** END OF VERTICES LIST ****************/ \r\n\r\n");
}


