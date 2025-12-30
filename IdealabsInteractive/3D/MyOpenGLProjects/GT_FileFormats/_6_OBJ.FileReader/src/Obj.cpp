
#include "OBJ.h"
#include "main.h"

char logText[255];
FILE *mFileConsoleLog;

void log(const char *format, ...) {
	va_list		ap;						// Pointer To List Of Arguments

	va_start(ap, format);					// Parses The String For Variables
	    vsprintf(logText, format, ap);	// And Converts Symbols To Actual Numbers
	va_end(ap);							// Results Are Stored In Text
	
	fwrite(logText, 1, strlen(logText), mFileConsoleLog);
}

CObjLoader::CObjLoader() {
	// Here we initialize all our mesh structures for the character
	
	m_bObjectHasUV = false;
	m_bObjectHasNormals = false;
	m_bJustReadAFace = false;
}

CObjLoader::~CObjLoader() {
	// Here we free all of the meshes in our model
	
}

//This function loads a .obj file into a specified model by a .obj file name
bool CObjLoader::importObj(t3DModel *pGame3DModel, char* strFileName) {

	// Make sure we have a valid model and file name
	if(!pGame3DModel || !strFileName)
		return false;

	// Here we open the desired file for read only and return the file pointer
	
	mFilePointer = fopen(strFileName, "rb");
	mFileConsoleLog = fopen("console.log", "wb");

	// Check to make sure we have a valid file pointer
	if(!mFilePointer) {
		// Create an error message for the attempted file
		log("Unable to find or open the file: %s", strFileName);
		sprintf(logText, "Unable to find or open the file: %s", strFileName);
		MessageBox(NULL, logText, "Error", MB_OK);
		return false;
	}

	// Now that we have a valid file and it's open, let's read in the info!
	readObjFile(pGame3DModel);

	// Now that we have the file read in, let's compute the vertex normals for lighting
	computeNormals(pGame3DModel);

	// Close the .obj file that we opened
	fclose(mFilePointer);
	
	// Return a success!
	return true;
}

//This function is the main loop for reading in the .obj file
void CObjLoader::readObjFile(t3DModel *pModel) {

	char strLine[255] = {0};
	char ch = 0;

	while(!feof(mFilePointer)) {
		
		float x = 0.0f, y = 0.0f, z = 0.0f;

		// Get the beginning character of the current line in the .obj file
		ch = fgetc(mFilePointer);

		switch(ch) {
			// Check if we just read in a 'v' (Could be a vertice/normal/textureCoord)
			case 'v':
				
				if(m_bJustReadAFace) {

					// Save the last object's info into our model structure
					fillInObjectInfo(pModel);
				}
				
				// Decipher this line to see if it's a vertex ("v"), normal ("vn"), or UV coordinate ("vt")
				readVertexInfo();
			break;

			// Check if we just read in a face header ('f')
			case 'f':

				// If we get here we then we need to read in the face information.
				// The face line holds the vertex indices into our vertex array, or if
				// the object has texture coordinates then it holds those too.
				
				readFaceInfo();
			break;
			
			case '\n':
				// If we read in a newline character, we've encountered a blank line in
				// the .obj file.  We don't want to do the default case and skip another
				// line, so we just break and do nothing.
			break;

			default:
				
				// If we get here then we don't care about the line being read, so read past it.
				fgets(strLine, 100, mFilePointer);
			break;
		}
	}

	// Now that we are done reading in the file, we have need to save the last object read.
	fillInObjectInfo(pModel);
}

// This function reads in the vertex information ("v" vertex : "vt" UVCoord)
void CObjLoader::readVertexInfo() {
	CVector3 vNewVector		= {0};
	CVector2 vNewTexCoord	= {0};
	char strLine[255] = {0};
	char ch = 0;

	// Read the next character in the file to see if it's a vertice/normal/UVCoord
	ch = fgetc(mFilePointer);
	
	// If we get a space it must have been a vertex ("v")
	if(ch == ' ') {

		// Here we read in a vertice.  The format is "v x y z"
		fscanf(mFilePointer, "%f %f %f", &vNewVector.x, &vNewVector.y, &vNewVector.z);

		// Read the rest of the line so the file pointer returns to the next line.
		fgets(strLine, 100, mFilePointer);

		// Add a new vertice to our list
		m_pVertices.push_back(vNewVector);

		log("VERTEX = %0.2f\t%0.2f\t%0.2f\r\n", vNewVector.x, vNewVector.y, vNewVector.z);
	}
	else
	// If we get a 't' then it must be a texture coordinate ("vt")
	if(ch == 't') {

		// Here we read in a texture coordinate.  The format is "vt u v"
		fscanf(mFilePointer, "%f %f", &vNewTexCoord.x, &vNewTexCoord.y);

		// Read the rest of the line so the file pointer returns to the next line.
		fgets(strLine, 100, mFilePointer);

		// Add a new vertice to our list
		m_pTextureCoords.push_back(vNewTexCoord);

		// Set the flag that tells us this object has texture coordinates.
		// Now we know that the face information will list the vertice AND UV index.
		// For example, ("f 1 3 2" verses "f 1/1 2/2 3/3")
		m_bObjectHasUV = true;

		log("TEXTURE COORDINATES = %0.2f\t%0.2f\r\n", vNewTexCoord.x, vNewTexCoord.y);
	}
	else
	// If we get a 'n' then it must be a normal ("vn")
	if(ch == 'n') {
		m_bObjectHasNormals = true;
		fgets(strLine, 100, mFilePointer);
	}
	// Ignore the line
	else {

		// We calculate our own normals at the end so we read past them.
		fgets(strLine, 100, mFilePointer);
	}
}

// This is called in ReadObjFile() if we find a line starting with 'f'
void CObjLoader::readFaceInfo() {

	tFace newFace = {0};
	char strLine[255] = {0};

	// This function reads in the face information of the object.
	// A face is a polygon (a triangle in this case) that has information about it.
	// It has the 3D points that make up the polygon and may also have texture coordinates.
	// When reading in an .obj, objects don't have to have UV texture coordinates so we
	// need to read in the face information differently in that case.  If the object does have
	// UV coordinates, then the format will look like this:
	// "f vertexIndex1/coordIndex1 vertexIndex2/coordIndex2 vertexIndex3/coordIndex3"
	// otherwise the format will look like this:"f vertexIndex1 vertexIndex2 vertexIndex3"
	// The index values index into our vertice and texture coordinate arrays.  More explained in RenderScene().
	// *Note* Make sure if you cut this code out for your own use you minus 1 from the indices.
	// This is because arrays are zero based and the .obj indices start at 1.  Look at FillInObjectInfo().

	// Check if this object has texture coordinates before reading in the values
	if(m_bObjectHasUV) {

		// Here we read in the object's vertex and texture coordinate indices.
		// Here is the format: "f vertexIndex1/coordIndex1 vertexIndex2/coordIndex2 vertexIndex3/coordIndex3"
		
		if(!m_bObjectHasNormals)
			fscanf(mFilePointer, "%d/%d %d/%d %d/%d",	&newFace.vertexIndex[0], &newFace.texCoordIndex[0], 
														&newFace.vertexIndex[1], &newFace.texCoordIndex[1], 
														&newFace.vertexIndex[2], &newFace.texCoordIndex[2]
													);
		else {
			int dummyNormal[3];
//MessageBox(NULL, "sssssssssss", "ddd", MB_OK);
			fscanf(mFilePointer, "%d/%d/%d %d/%d/%d %d/%d/%d",	&newFace.vertexIndex[0], &newFace.texCoordIndex[0], &dummyNormal[0],
																&newFace.vertexIndex[1], &newFace.texCoordIndex[1], &dummyNormal[1],
																&newFace.vertexIndex[2], &newFace.texCoordIndex[2], &dummyNormal[2]
															);
		}
	}
	// The object does NOT have texture coordinates
	else {
		// Here we read in just the object's vertex indices.
		// Here is the format: "f vertexIndex1 vertexIndex2 vertexIndex3"
		fscanf(mFilePointer, "%d %d %d", &newFace.vertexIndex[0], &newFace.vertexIndex[1], &newFace.vertexIndex[2]);
	}

	// Read the rest of the line so the file pointer returns to the next line.
	fgets(strLine, 100, mFilePointer);

	// Add the new face to our face list
	m_pFaces.push_back(newFace);

	// We set this flag to TRUE so we know that we just read in some face information.
	// Since face information is the last thing we read in for an object we will check
	// this flag when we read in a vertice.  If it's true, then we just finished reading
	// in an object and we need to save off the information before going to the next object.
	// Since there is no standard header for objects in a .obj file we do it this way.
	m_bJustReadAFace = true;
}

//This function is called after an object is read in to fill in the model structure
void CObjLoader::fillInObjectInfo(t3DModel *pModel) {
	
	t3DObject newObject = {0};
	int textureOffset = 0, vertexOffset = 0;
	int i = 0;

	// If we get here then we just finished reading in an object
	// and need to increase the object count.
	pModel->numOfObjects++;

	// Add a new object to the list of objects in our model
	pModel->pObjects.push_back(newObject);

	// Get a pointer to the current object so our code is easier to read
	t3DObject *pObject = &(pModel->pObjects[pModel->numOfObjects - 1]);

	// Now that we have our list's full of information, we can get the size
	// of these lists by calling size() from our vectors.  That is one of the
	// wonderful things that the Standard Template Library offers us.  Now you
	// never need to write a link list or constantly call malloc()/new.

	// Here we get the number of faces, vertices and texture coordinates
	pObject->numOfFaces				= m_pFaces.size();
	pObject->numOfVertices			= m_pVertices.size();
	pObject->numOfTextureVertices	= m_pTextureCoords.size();

	// If we read in any faces for this object (required)
	if(pObject->numOfFaces) {

		// Allocate enough memory to store all the faces in our object
		pObject->pFaces = new tFace[pObject->numOfFaces];
	}

	// If we read in any vertices for this object (required)
	if(pObject->numOfVertices) {

		// Allocate enough memory to store all the vertices in our object
		pObject->pVertices = new CVector3[pObject->numOfVertices];
	}

	// If we read in any texture coordinates for this object (optional)
	if(pObject->numOfTextureVertices) {
		
		pObject->pTexVertices	= new CVector2[pObject->numOfTextureVertices];
		pObject->bHasTexture = true;
	}

	// Go through all of the faces in the object
	for(int i = 0; i < pObject->numOfFaces; i++) {

		// Copy the current face from the temporary list to our Model list
		pObject->pFaces[i] = m_pFaces[i];

		// Because of the problem with .obj files not being very object friendly,
		// if a new object is found in the file, the face and texture indices start
		// from the last index that was used in the last object.  That means that if
		// the last one was 8, it would then go to 9 for the next object.  We need to
		// bring that back down to 1, so we just create an offset that we subtract from
		// the vertex and UV indices.

		// Check if this is the first face of the object
		if(i == 0) {

			// If the first index is NOT 1, then we must be past the first object
			if(pObject->pFaces[0].vertexIndex[0] != 1) {
				
				// To create the offset we take the current starting point and then minus 1.
				// Lets say the last object ended at 8.  Well we would then have 9 here.
				// We want to then subtract 8 from the 9 to get back to 1.
				vertexOffset = pObject->pFaces[0].vertexIndex[0] -1;

				// The same goes for texture coordinates, if we have them do the same
				if(pObject->numOfTextureVertices > 0) {
					
					// Take the current index and minus 1 from it
					textureOffset = pObject->pFaces[0].texCoordIndex[0] - 1;
				}
			}
		}

		// Because the face indices start at 1, we need to minus 1 from them due
		// to arrays being zero based.  This is VERY important!
		for(int j = 0; j < 3; j++) {

			// For each index, minus 1 to conform with zero based arrays.
			// We also need to add the vertex and texture offsets to subtract
			// the total amount necessary for this to work.  The first object
			// will have a offset of 0 for both since it starts at 1.
			pObject->pFaces[i].vertexIndex[j]	-= 1 + vertexOffset;
			pObject->pFaces[i].texCoordIndex[j] -= 1 + textureOffset;
		}
	}

	// Go through all the vertices in the object
	for(i = 0; i < pObject->numOfVertices; i++) {
		
		// Copy the current vertice from the temporary list to our Model list
		pObject->pVertices[i] = m_pVertices[i];
	}

	// Go through all of the texture coordinates in the object (if any)
	for(i = 0; i < pObject->numOfTextureVertices; i++) {
		
		// Copy the current UV coordinate from the temporary list to our Model list
		pObject->pTexVertices[i] = m_pTextureCoords[i];
	}

	// Since .OBJ files don't have materials, we set the material ID to -1.
	// We need to manually give it a material using AddMaterial().
	pObject->materialID = -1;

	// Now that we have all the information from out list's, we need to clear them
	// so we can be ready for the next object that we read in.
	m_pVertices.clear();
	m_pFaces.clear();
	m_pTextureCoords.clear();

	// Reset these booleans to be prepared for the next object
	m_bObjectHasUV   = false;
	m_bJustReadAFace = false;
}

//This function assigns a material to a specific object in our array of objects
void CObjLoader::setObjectMaterial(t3DModel *pModel, int whichObject, int materialID) {
	// Make sure we have a valid model or else quit the function
	if(!pModel) return;

	// Make sure we don't index over the array of objects
	if(whichObject >= pModel->numOfObjects) return;

	// Here we set the material ID assigned to this object
	pModel->pObjects[whichObject].materialID = materialID;
}

//This function adds a material to our model manually since .obj has no such info
void CObjLoader::addMaterial(t3DModel *pModel, char *strMaterialName, char *strMaterialFileName, int r, int g, int b) {
	tMaterial newMaterial = {0};

	// Set the RGB value for this material [0 - RED		1 - GREEN	2 - BLUE]
	newMaterial.colour[0] = r; newMaterial.colour[1] = g; newMaterial.colour[2] = b;

	// If we have a file name passed in, let's copy it to our material structure
	if(strMaterialFileName) {
		strcpy(newMaterial.textureFileName, strMaterialFileName);
	}

	// If we have a material name passed in, let's copy it to our material structure
	if(strMaterialName) {
		strcpy(newMaterial.materialName, strMaterialName);
	}

	// Now we add this material to model's list.  Once again we use the incredibly
	// helpfull STL vector functions for allocating dynamic memory.
	pModel->pMaterials.push_back(newMaterial);

	// Increase the material count
	pModel->numOfMaterials++;
}

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
CVector3 AddVector(CVector3 vVector1, CVector3 vVector2)
{
	CVector3 vResult;							// The variable to hold the resultant vector
	
	vResult.x = vVector2.x + vVector1.x;		// Add Vector1 and Vector2 x's
	vResult.y = vVector2.y + vVector1.y;		// Add Vector1 and Vector2 y's
	vResult.z = vVector2.z + vVector1.z;		// Add Vector1 and Vector2 z's

	return vResult;								// Return the resultant vector
}

// This divides a vector by a single number (scalar) and returns the result
CVector3 DivideVectorByScaler(CVector3 vVector1, float Scaler)
{
	CVector3 vResult;							// The variable to hold the resultant vector
	
	vResult.x = vVector1.x / Scaler;			// Divide Vector1's x value by the scaler
	vResult.y = vVector1.y / Scaler;			// Divide Vector1's y value by the scaler
	vResult.z = vVector1.z / Scaler;			// Divide Vector1's z value by the scaler

	return vResult;								// Return the resultant vector
}

// This returns the cross product between 2 vectors
CVector3 Cross(CVector3 vVector1, CVector3 vVector2)
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
CVector3 Normalize(CVector3 vNormal)
{
	double Magnitude;							// This holds the magnitude			

	Magnitude = Mag(vNormal);					// Get the magnitude

	vNormal.x /= (float)Magnitude;				// Divide the vector's X by the magnitude
	vNormal.y /= (float)Magnitude;				// Divide the vector's Y by the magnitude
	vNormal.z /= (float)Magnitude;				// Divide the vector's Z by the magnitude

	return vNormal;								// Return the normal
}

///////////////////////////////// COMPUTER NORMALS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This function computes the normals and vertex normals of the objects
/////
///////////////////////////////// COMPUTER NORMALS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void CObjLoader::computeNormals(t3DModel *pModel) {

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

			vNormal  = Cross(vVector1, vVector2);		// Return the cross product of the 2 vectors (normalize vector, but not a unit vector)
			pTempNormals[i] = vNormal;					// Save the un-normalized normal for the vertex normals
			vNormal  = Normalize(vNormal);				// Normalize the cross product to give us the polygons normal

			pNormals[i] = vNormal;						// Assign the normal to the list of normals
		}

		//////////////// Now Get The Vertex Normals /////////////////

		CVector3 vSum = {0.0, 0.0, 0.0};
		CVector3 vZero = vSum;
		int shared=0;

		for (int i = 0; i < pObject->numOfVertices; i++)			// Go through all of the vertices
		{
			for (int j = 0; j < pObject->numOfFaces; j++)	// Go through all of the triangles
			{												// Check if the vertex is shared by another face
				if (pObject->pFaces[j].vertexIndex[0] == i || 
					pObject->pFaces[j].vertexIndex[1] == i || 
					pObject->pFaces[j].vertexIndex[2] == i)
				{
					vSum = AddVector(vSum, pTempNormals[j]);// Add the un-normalized normal of the shared face
					shared++;								// Increase the number of shared triangles
				}
			}      
			
			// Get the normal by dividing the sum by the shared.  We negate the shared so it has the normals pointing out.
			pObject->pNormals[i] = DivideVectorByScaler(vSum, float(-shared));

			// Normalize the normal for the final vertex normal
			pObject->pNormals[i] = Normalize(pObject->pNormals[i]);	

			vSum = vZero;									// Reset the sum
			shared = 0;										// Reset the shared
		}
	
		// Free our memory and start over on the next object
		delete [] pTempNormals;
		delete [] pNormals;
	}
}


