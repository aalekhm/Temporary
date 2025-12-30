
#include "3DS.h"

int gBuffer[50000] = {0};

char logText[255];
FILE *mFileConsoleLog;

void log(const char *format, ...) {
	va_list		ap;						// Pointer To List Of Arguments

	va_start(ap, format);					// Parses The String For Variables
	    vsprintf(logText, format, ap);	// And Converts Symbols To Actual Numbers
	va_end(ap);							// Results Are Stored In Text
	
	fwrite(logText, 1, strlen(logText), mFileConsoleLog);
}

bool CReader3DS::import3DS(t3DModel *pGame3DModel, char *fileName3DS) {
	char errorMessage[255] = {0};
	
	tCHUNK	pChunk = {0};

	// (1) Open .3DS file
	mFilePointer = fopen(fileName3DS, "rb");
	mFileConsoleLog = fopen("console.txt", "wb");

	if(!mFilePointer) {
		sprintf(errorMessage, "Unable to find file %s", fileName3DS);
		displayErrorMessage(errorMessage);
		return false;
	}

	// (2) Once we have the file open, we need to read the very first data chunk
	// to see if it's a 3DS file.  That way we don't read an invalid file.
	// If it is a 3DS file, then the first chunk ID will be equal to PRIMARY (some hex num)
	
	log("/******************* LOADING MODEL *******************/\r\n");

	// Read the first chuck of the file to see if it's a 3DS file
	readChunk(&pChunk);

	// (3) Make sure the CHUNK_ID we found is PRIMARY, else its not a valid '.3ds' file
	if(pChunk.chunkID != PRIMARY) {
		sprintf(errorMessage, "Invalid .3ds file %s", fileName3DS);
		displayErrorMessage(errorMessage);
		return false;
	}

	// (4) Now we actuallt start reading the data, recursively.
	// Start loading the objects by calling the recursive function
	processNextChunk(pGame3DModel, &pChunk);

	// After we have read the whole 3DS file, we want to calculate our own vertex normals.
	computeNormals(pGame3DModel);


	log("/******************* FINISHED LOADING MODEL *******************/\r\n");

	//Clean up after everything
	cleanUp();

	return true;
}

//This function reads in a chunk ID and it's length in bytes
void CReader3DS::readChunk(tCHUNK *pChunk) {
	//This reads the CHUNK_ID which is 2 bytes.
	//The CHUNK_ID is like PRIMARY, VERION, OBJECT, MATERIAL, etc.
	//It tells what data lies within this chunk section
	pChunk->bytesRead = fread(&pChunk->chunkID, 1, 2, mFilePointer);

	//Then we read the length of the Chunk, which is 4 bytes.
	//This is how we know how much to read in or read past!!!
	pChunk->bytesRead += fread(&pChunk->chunkLength, 1, 4, mFilePointer);
}

//This function reads the main sections of the .3DS file, then dives deeper with recursion
void CReader3DS::processNextChunk(t3DModel *pModel, tCHUNK *pPreviousChunk) {
	t3DObject		newObject = {0};		//This is added to our Object List
	tMaterial		newMaterial = {0};		//This is added to our Material List

	tCHUNK			currentChunk = {0};		//The current Chunk to load
	tCHUNK			tempChunk = {0};		//A temp Chunk for holding data
	
	char errorMessage[255] = {0};

	// Continue to read the sub chunks until we have reached the length.
	// After we read ANYTHING we add the bytes read to the chunk and then check
	// check against the length.

	// Below we check our chunk ID each time we read a new chunk.  Then, if
	// we want to extract the information from that chunk, we do so.
	// If we don't want a chunk, we just read past it.
	while(pPreviousChunk->bytesRead < pPreviousChunk->chunkLength) {
		//Read the next Chunk
		readChunk(&currentChunk);

		//Check the CHUNK_ID
		switch(currentChunk.chunkID) {
			//This holds the version of the .3ds file.
			case FILE_VERSION :
				currentChunk.bytesRead += fread(	gBuffer, 
													1, 
													currentChunk.chunkLength - currentChunk.bytesRead, 
													mFilePointer
												);

				//If the file version is over 3, give a warning that there could be a problem
				if((currentChunk.chunkLength - currentChunk.bytesRead) && gBuffer[0] > 0x03) {
					displayErrorMessage("This 3DS file is over version 3 so it may load incorrectly");
				}
			break;

			//This holds the version of the Mesh.
			case OBJECT_INFO:
				//This holds the version of the Mesh.
				//It also is the head of the MATERIAL & OBJECT chunks.
				//From here we start reading Object and Material Chunk info

				//Read the next Chunk
				readChunk(&tempChunk);
				
				//Read the version of the Mesh
				tempChunk.bytesRead += fread(	gBuffer, 
												1, 
												tempChunk.chunkLength - tempChunk.bytesRead,
												mFilePointer
											);
				//Increas the bytes read by the bytes read from the last Chunk
				currentChunk.bytesRead += tempChunk.bytesRead;

				//Go to the next Chunk,
				//which if the object has a texture will be MATERIAL followed by OBJECT chunk
				processNextChunk(pModel, &currentChunk);
			break;
			//This Chunk is the header of the Material Info Chunks
			//This holds the Material information
			case MATERIAL:

				//Increase the number of Materials
				pModel->numOfMaterials++;

				//Add the new Material to our Material List.
				// If you are unfamiliar with STL's "vector" class, all push_back()
				// does is add a new node onto the list.  I used the vector class
				// so I didn't need to write my own link list functions.  
				pModel->pMaterials.push_back(newMaterial);

				//Proceed to the Material Loading function
				processNextMaterial(pModel, &currentChunk);
			break;
			//This Chunk is the header of the Object Chunk
			//This holds the Object information like its name
			case OBJECT:
				//Increase the number of objects in the MODEL/SCENE
				pModel->numOfObjects++;

				//Add the new Object to our Object List.
				pModel->pObjects.push_back(newObject);

				//Initialize the object and all its data members
				memset(&(pModel->pObjects[pModel->numOfObjects - 1]), 0, sizeof(t3DObject));

				//Get the name of the object and store it, then add the read bytes to our byte counter.
				currentChunk.bytesRead += readString(pModel->pObjects[pModel->numOfObjects - 1].objectName);

				//Now proceed to read in the rest of the Object information
				processNextObjectChunk(pModel, &(pModel->pObjects[pModel->numOfObjects-1]), &currentChunk);
			break;
			case EDIT_KEY_FRAME:

				// Because I wanted to make this a SIMPLE tutorial as possible, I did not include
				// the key frame information.  This chunk is the header for all the animation info.
				// In a later tutorial this will be the subject and explained thoroughly.

				//ProcessNextKeyFrameChunk(pModel, currentChunk);

				// Read past this chunk and add the bytes read to the byte counter
				currentChunk.bytesRead += fread(gBuffer, 1, currentChunk.chunkLength - currentChunk.bytesRead, mFilePointer);
			break;
			default: 
				// If we didn't care about a chunk, then we get here.  We still need
				// to read past the unknown or ignored chunk and add the bytes read to the byte counter.
				currentChunk.bytesRead += fread(gBuffer, 1, currentChunk.chunkLength - currentChunk.bytesRead, mFilePointer);
			break;
		}

		// Add the bytes read from the last chunk to the previous chunk passed in.
		pPreviousChunk->bytesRead += currentChunk.bytesRead;

	}
}

void CReader3DS::processNextMaterial(t3DModel * pModel, tCHUNK *pPreviousChunk) {
	//The current Chunk to work with
	tCHUNK		currentChunk = {0};

	// Continue to read these chunks until we read the end of this sub chunk
	while (pPreviousChunk->bytesRead < pPreviousChunk->chunkLength) {
		//Read the next Chunk
		readChunk(&currentChunk);

		switch(currentChunk.chunkID) {
			//Hold the name of the Material
			case MATERIAL_NAME:
					currentChunk.bytesRead += fread(	pModel->pMaterials[pModel->numOfMaterials - 1].materialName,
														1, 
														currentChunk.chunkLength - currentChunk.bytesRead,
														mFilePointer
													);
			break;
			//Hold the R, G, B colour info of the Material
			case MATERIAL_DIFFUSE:
					readColourChunk(&(pModel->pMaterials[pModel->numOfMaterials - 1]), &currentChunk);
			break;
			//Since its a Material Map, we have more Textures/Materials, so process the next Material.
			case MATERIAL_MAP:
					processNextMaterial(pModel, &currentChunk);
			break;
			//Hold the name of the texture file for our Material
			case MATERIAL_MAP_FILENAME:
					currentChunk.bytesRead += fread(	pModel->pMaterials[pModel->numOfMaterials - 1].textureFileName,
														1, 
														currentChunk.chunkLength - currentChunk.bytesRead,
														mFilePointer
													);
			break;
			//Read past the UNKNOWN Chunks
			default:
					currentChunk.bytesRead += fread(	gBuffer, 
														1, 
														currentChunk.chunkLength - currentChunk.bytesRead,
														mFilePointer
													);
			break;
		}

		//Add the bytes read from this Chunk to the previous Chunk
		pPreviousChunk->bytesRead += currentChunk.bytesRead;
	}
}

void CReader3DS::readColourChunk(tMaterial *pMaterial, tCHUNK *pChunk) {
	tCHUNK tempChunk = {0};

	//Read the Colour info Chunk
	readChunk(&tempChunk);

	//Read in the R, G, B values (3 bytes - 0 through 255)
	tempChunk.bytesRead += fread(	pMaterial->colour,
									1,
									tempChunk.chunkLength - tempChunk.bytesRead,
									mFilePointer
								);

	pChunk->bytesRead += tempChunk.bytesRead;
}

// This function reads in a string of characters
int CReader3DS::readString(char *pBuffer) {
	int index = 0;

	//Read 1 byte of data which is the first letter of the string.
	fread(pBuffer, 1, 1, mFilePointer);
	
	//Loop until we encounter a NULL(0x00)
	while(*(pBuffer + index++) != 0) {
		//Read 1 byte of data.
		fread(pBuffer + index, 1, 1, mFilePointer);
	}

	return strlen(pBuffer) + 1;
}

// This function handles all the information about the objects in the file
void CReader3DS::processNextObjectChunk(t3DModel *pModel, t3DObject *pObject, tCHUNK *pPreviousChunk) {

	//The current Chunk to work with
	tCHUNK		currentChunk = {0};

	// Continue to read these chunks until we read the end of this sub chunk
	while(pPreviousChunk->bytesRead < pPreviousChunk->chunkLength) {
		//Read the next Chunk
		readChunk(&currentChunk);

		switch(currentChunk.chunkID) {
			
			//Tells us that we are reading a new Object
			case OBJECT_MESH:
				// We found a new object, so let's read in it's info using recursion
				processNextObjectChunk(pModel, pObject, &currentChunk);
			break;
			
			//Time to read Object Vertices
			case OBJECT_VERTICES:
				readObjectVertices(pObject, &currentChunk);
			break;
			
			//Time to read Object Face information
			case OBJECT_FACES:
				readObjectFaceIndices(pObject, &currentChunk);
			break;

			//Time to read Object Face Material info
			case OBJECT_MATERIAL:
				// This chunk holds the name of the material that the object has assigned to it.
				// This could either be just a color or a texture map.  This chunk also holds
				// the faces that the texture is assigned to (In the case that there is multiple
				// textures assigned to one object, or it just has a texture on a part of the object.
				// Since most of my game objects just have the texture around the whole object, and 
				// they aren't multitextured, I just want the material name.

				// We now will read the name of the material assigned to this object
				readObjectFaceMaterial(pModel, pObject, &currentChunk);
			break;

			//Time to read UV Texture Coordinates for the Object
			case OBJECT_UV:
				// This chunk holds all of the UV coordinates for our object.  Let's read them in.
				readObjectUVCoordinates(pObject, &currentChunk);
			break;

			default:
				// Read past the ignored or unknown chunks
				currentChunk.bytesRead += fread(	gBuffer, 
													1,
													currentChunk.chunkLength - currentChunk.bytesRead,
													mFilePointer
												);
		}

		// Add the bytes read from the last chunk to the previous chunk passed in.
		pPreviousChunk->bytesRead += currentChunk.bytesRead;
	}
}

void CReader3DS::readObjectVertices(t3DObject *pObject, tCHUNK *pPreviousChunk) {
	// Like most chunks, before we read in the actual vertices, we need
	// to find out how many there are to read in.  Once we have that number
	// we then fread() them into our vertice array.

	// (1) Read in the number of Vertices
	pPreviousChunk->bytesRead += fread(&pObject->numOfVertices, 1, 2, mFilePointer);

	// Allocate memory for the vertices and initialise the vertices structure
	pObject->pVertices = new CVector3[pObject->numOfVertices];
	memset(pObject->pVertices, 0, sizeof(CVector3) * pObject->numOfVertices );

	// (2) Read in the array of Vertices (an array of 3 floats)
	pPreviousChunk->bytesRead += fread(	pObject->pVertices,
										1, 
										pPreviousChunk->chunkLength - pPreviousChunk->bytesRead,
										mFilePointer
									);

	// The coordinate system of OpenGL and 3DS Max are different.
	// Since 3D Studio Max Models with the Z-Axis pointing up (strange and ugly I know! :), 
	// we need to flip the Y values with the Z values in our vertices.  That way it
	// will be normal, with Y pointing up.  Also, because we swap the Y and Z we need to negate 
	// the Z to make it come out correctly.  This is also explained and done in ReadVertices().
	for(int i = 0; i < pObject->numOfVertices; i++) {
		//Temporarily store the 'Y' value
		float fTempY = pObject->pVertices[i].y;
		
		//Shift the 'Z' value to be our new 'Y' value
		pObject->pVertices[i].y = pObject->pVertices[i].z;
		
		//Save the value store in temp('Y' value) to be our new 'Z' value.
		//Also negate it.
		pObject->pVertices[i].z = -fTempY;
	}
}

//This function reads in the indices for the vertex array
void CReader3DS::readObjectFaceIndices(t3DObject *pObject, tCHUNK *pPreviousChunk) {
	//Tells which Face index we are currently reading
	unsigned short index = 0;

	// In order to read in the vertex indices for the object, we need to first
	// read in the number of them, then read them in.  Remember,
	// we only want 3 of the 4 values read in for each face.  The fourth is
	// a visibility flag for 3D Studio Max that doesn't mean anything to us.

	// (1) Read in the number of Faces that are in this object (int)
	pPreviousChunk->bytesRead += fread(&pObject->numOfFaces, 1, 2, mFilePointer);

	//Allow enought memory for the faces and initialize the structure
	pObject->pFaces = new tFace[pObject->numOfFaces];
	memset(pObject->pFaces, 0, sizeof(tFace) * pObject->numOfFaces );

	// (2) Go through all the Faces in this object
	for(int i = 0; i < pObject->numOfFaces; i++) {
		//Next we read in the A, then B, then C value of the face, 
		//but ignore the 4th value. The 4th value is the visibility flag
		//used by 3D Studio Max, we don`t need that value...
		for(int j = 0; j < 4; j++) {
			//Read the vertice index for the Face
			pPreviousChunk->bytesRead += fread(	&index, 
												1, 
												sizeof(index), //unsigned short
												mFilePointer
											);
			if(j < 3) {
				// Store the index in our face structure.
				pObject->pFaces[i].vertexIndex[j] = index;
			}
		}
	}
}

//This function reads in the material name assigned to the object and sets the materialID
void CReader3DS::readObjectFaceMaterial(t3DModel *pModel, t3DObject *pObject, tCHUNK *pPreviousChunk) {
	// This is used to hold the Objects Material name
	char materialName[255] = {0};

	// *What is a material?*  - A material is either the color or the texture map of the object.
	// It can also hold other information like the brightness, shine, etc... Stuff we don't
	// really care about.  We just want the color, or the texture map file name really.

	// Here we read the material name that is assigned to the current object.
	// 'materialName' should now have a string of the material name, like "Material #2" etc..
	pPreviousChunk->bytesRead += readString(materialName);

	// Now that we have a material name, we need to go through all of the materials
	// and check the name against each material.  When we find a material in our material
	// list that matches this name we just read in, then we assign the materialID
	// of the object to that material index.  You will notice that we passed in the
	// model to this function.  This is because we need the number of textures.
	// Yes though, we could have just passed in the model and not the object too.

	// Go through all of the Material Textures
	for(int i = 0; i < pModel->numOfMaterials; i++) {

		// If the material we just read in matches the current texture name
		if(strcmp(materialName, pModel->pMaterials[i].materialName) == 0) {
			
			// Now that we found the material, check if it's a Texture Map & not the Colour.
			// If the 'textureFileName' has a string length of 1 and over it's a texture
			if(strlen(pModel->pMaterials[i].textureFileName) > 0) {
				// Set the Material ID to the current index 'i' and stop checking
				pObject->materialID = i;
				
				//Set the Objects flag to say 'YES' it has a Texture Map to bind.
				pObject->bHasTexture = true;
			}
			break;
		}
		else {
			// Here we check first to see if there is a texture already assigned to this object
			if( ! pObject->bHasTexture) {

				// Set the ID to -1 to show there is no Material binding for this object
				pObject->materialID = -1;
			}
		}
	}

	// Read past the rest of the chunk since we don't care about shared vertices
	// You will notice we subtract the bytes already read in this chunk from the total length.
	pPreviousChunk->bytesRead += fread(gBuffer, 1, pPreviousChunk->chunkLength - pPreviousChunk->bytesRead, mFilePointer);
}

void CReader3DS::readObjectUVCoordinates(t3DObject *pObject, tCHUNK *pPreviousChunk) {
	// In order to read in the UV indices for the object, we need to first
	// read in the amount there are, then read them in.

	// Read in the number of UV coordinates there are (int)
	pPreviousChunk->bytesRead += fread(&pObject->numOfTextureVertex, 1, 2, mFilePointer);

	// Allocate memory to hold the UV coordinates
	pObject->pTexVertices = new CVector2[pObject->numOfTextureVertex];

	// Read in the texture coodinates (an array 2 float)
	pPreviousChunk->bytesRead += fread(	pObject->pTexVertices,
										1,
										pPreviousChunk->chunkLength - pPreviousChunk->bytesRead,
										mFilePointer
									);
}

// *Note* 
//
// Below are some math functions for calculating vertex normals.  We want vertex normals
// because it makes the lighting look really smooth and life like.  You probably already
// have these functions in the rest of your engine, so you can delete these and call
// your own.  I wanted to add them so I could show how to calculate vertex normals.

//////////////////////////////	Math Functions  ////////////////////////////////*

// This computes the magnitude of a normal.   (magnitude = sqrt(x^2 + y^2 + z^2)
#define Mag(Normal) (sqrt(Normal.x*Normal.x + Normal.y*Normal.y + Normal.z*Normal.z))

// This calculates a vector between 2 points and returns the result
CVector3 Vector(CVector3 vPoint1, CVector3 vPoint2)
{
	CVector3 vVector;							// The variable to hold the resultant vector

	vVector.x = vPoint1.x - vPoint2.x;			// Subtract point1 and point2 x's
	vVector.y = vPoint1.y - vPoint2.y;			// Subtract point1 and point2 y's
	vVector.z = vPoint1.z - vPoint2.z;			// Subtract point1 and point2 z's

	return vVector;								// Return the resultant vector
}

// This adds 2 vectors together and returns the result
CVector3 addVector(CVector3 vVector1, CVector3 vVector2)
{
	CVector3 vResult;							// The variable to hold the resultant vector
	
	vResult.x = vVector2.x + vVector1.x;		// Add Vector1 and Vector2 x's
	vResult.y = vVector2.y + vVector1.y;		// Add Vector1 and Vector2 y's
	vResult.z = vVector2.z + vVector1.z;		// Add Vector1 and Vector2 z's

	return vResult;								// Return the resultant vector
}

// This divides a vector by a single number (scalar) and returns the result
CVector3 divideVectorByScaler(CVector3 vVector1, float Scaler)
{
	CVector3 vResult;							// The variable to hold the resultant vector
	
	vResult.x = vVector1.x / Scaler;			// Divide Vector1's x value by the scaler
	vResult.y = vVector1.y / Scaler;			// Divide Vector1's y value by the scaler
	vResult.z = vVector1.z / Scaler;			// Divide Vector1's z value by the scaler

	return vResult;								// Return the resultant vector
}

// This returns the cross product between 2 vectors
CVector3 cross(CVector3 vVector1, CVector3 vVector2)
{
	CVector3 vCross;								// The vector to hold the cross product
												// Get the X value
	vCross.x = ((vVector1.y * vVector2.z) - (vVector1.z * vVector2.y));
												// Get the Y value
	vCross.y = ((vVector1.z * vVector2.x) - (vVector1.x * vVector2.z));
												// Get the Z value
	vCross.z = ((vVector1.x * vVector2.y) - (vVector1.y * vVector2.x));

	return vCross;								// Return the cross product
}

// This returns the normal of a vector
CVector3 normalize(CVector3 vNormal)
{
	double Magnitude;							// This holds the magitude			

	Magnitude = Mag(vNormal);					// Get the magnitude

	vNormal.x /= (float)Magnitude;				// Divide the vector's X by the magnitude
	vNormal.y /= (float)Magnitude;				// Divide the vector's Y by the magnitude
	vNormal.z /= (float)Magnitude;				// Divide the vector's Z by the magnitude

	return vNormal;								// Return the normal
}

/////	This function computes the normals and vertex normals of the objects
/////
///////////////////////////////// COMPUTER NORMALS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void CReader3DS::computeNormals(t3DModel *pModel)
{
	int i = 0;
	CVector3 vVector1, vVector2, vNormal, vPoly[3];

	// If there are no objects, we can skip this part
	if(pModel->numOfObjects <= 0)
		return;

	// What are vertex normals?  And how are they different from other normals?
	// Well, if you find the normal to a triangle, you are finding a "Face Normal".
	// If you give OpenGL a face normal for lighting, it will make your object look
	// really flat and not very round.  If we find the normal for each vertex, it makes
	// the smooth lighting look.  This also covers up blocky looking objects and they appear
	// to have more polygons than they do.    Basically, what you do is first
	// calculate the face normals, then you take the average of all the normals around each
	// vertex.  It's just averaging.  That way you get a better approximation for that vertex.

	// Go through each of the objects to calculate their normals
	for(int index = 0; index < pModel->numOfObjects; index++)
	{
		// Get the current object
		t3DObject *pObject = &(pModel->pObjects[index]);

		// Here we allocate all the memory we need to calculate the normals
		CVector3 *pNormals		= new CVector3 [pObject->numOfFaces];
		CVector3 *pTempNormals	= new CVector3 [pObject->numOfFaces];
		pObject->pNormals		= new CVector3 [pObject->numOfVertices];

		// Go though all of the faces of this object
		for(int i=0; i < pObject->numOfFaces; i++)
		{												
			// To cut down LARGE code, we extract the 3 points of this face
			vPoly[0] = pObject->pVertices[pObject->pFaces[i].vertexIndex[0]];
			vPoly[1] = pObject->pVertices[pObject->pFaces[i].vertexIndex[1]];
			vPoly[2] = pObject->pVertices[pObject->pFaces[i].vertexIndex[2]];

			// Now let's calculate the face normals (Get 2 vectors and find the cross product of those 2)

			vVector1 = Vector(vPoly[0], vPoly[2]);		// Get the vector of the polygon (we just need 2 sides for the normal)
			vVector2 = Vector(vPoly[2], vPoly[1]);		// Get a second vector of the polygon

			vNormal  = cross(vVector1, vVector2);		// Return the cross product of the 2 vectors (normalize vector, but not a unit vector)
			pTempNormals[i] = vNormal;					// Save the un-normalized normal for the vertex normals
			vNormal  = normalize(vNormal);				// Normalize the cross product to give us the polygons normal

			pNormals[i] = vNormal;						// Assign the normal to the list of normals
		}

		//////////////// Now Get The Vertex Normals /////////////////
		
		CVector3 vSum = {0.0, 0.0, 0.0};
		CVector3 vZero = vSum;
		int shared=0;

		for (i = 0; i < pObject->numOfVertices; i++)			// Go through all of the vertices
		{
			for (int j = 0; j < pObject->numOfFaces; j++)	// Go through all of the triangles
			{												// Check if the vertex is shared by another face
				if (pObject->pFaces[j].vertexIndex[0] == i || 
					pObject->pFaces[j].vertexIndex[1] == i || 
					pObject->pFaces[j].vertexIndex[2] == i)
				{
					vSum = addVector(vSum, pTempNormals[j]);// Add the un-normalized normal of the shared face
					shared++;								// Increase the number of shared triangles
				}
			}
			
			// Get the normal by dividing the sum by the shared.  We negate the shared so it has the normals pointing out.
			pObject->pNormals[i] = divideVectorByScaler(vSum, float(-shared));

			// Normalize the normal for the final vertex normal
			pObject->pNormals[i] = normalize(pObject->pNormals[i]);	

			vSum = vZero;									// Reset the sum
			shared = 0;										// Reset the shared
		}
	
		// Free our memory and start over on the next object
		delete [] pTempNormals;
		delete [] pNormals;
	}
}

void CReader3DS::cleanUp() {
	//Close the current File Pointer
	if(mFilePointer) {
		fclose(mFilePointer);
		mFilePointer = NULL;
	}
}

void CReader3DS::displayErrorMessage(const char *errMsg, ...) {
	va_list ap;
	char errText[255] = {0};

	va_start(ap, errMsg);
		vsprintf(errText, errMsg, ap);
	va_end(ap);

	MessageBox(NULL, errText, "ERROR", MB_OK | MB_ICONERROR);
}

