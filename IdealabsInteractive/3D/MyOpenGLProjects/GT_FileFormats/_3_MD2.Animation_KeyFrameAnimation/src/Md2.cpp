
#include "MD2.h"

CReaderMD2::CReaderMD2() {
	//Here we initialise our structures to 0.
	memset(&m_pMD2Header, 0, sizeof(tMD2Header));

	//set all pointer to NULL;
	m_pSkins		= NULL;
	m_pTexCoords	= NULL;
	m_pFaces		= NULL;
	m_pFrames		= NULL;
}

bool CReaderMD2::importMD2(t3DModel *pGame3DModel, char *fileNameMD2) {
	
	mFilePointer = fopen(fileNameMD2, "rb");
	mFileConsoleLog = fopen("console.log", "wb");

	if(!mFilePointer) {
		sprintf(logText, "Unable to find file %s", fileNameMD2);
		fwrite(logText, 1, strlen(logText), mFileConsoleLog);
		MessageBox(NULL, logText, "ERR", MB_OK);

		return false;
	}

	//Read MD2 HEADER
	fread(&m_pMD2Header, 1, sizeof(tMD2Header), mFilePointer);
//char logtext[255];
//sprintf(logText, "SkinWidth/H %d %d", m_pMD2Header.skinWidth, m_pMD2Header.skinHeight);
//MessageBox(NULL, logText, "ssss", MB_OK);

	//Make sure the version is the crazy no. '8', else its a bad egg
	if(m_pMD2Header.version != 8) {
		sprintf(logText, "Invalid file version %d", m_pMD2Header.version);
		fwrite(logText, 1, strlen(logText), mFileConsoleLog);
		MessageBox(NULL, logText, "ERR", MB_OK);
		
		return false;
	}

	//Read in the Model & Animation data
	readMD2Data();

	// Here we pass in our model structure to it can store the read Quake data
	// in our own model and object structure data
	convertDataStructures(pGame3DModel);

//char logText[255];
//sprintf(logText, "***pObject->numOfFaces %d", pGame3DModel->firstObject->numOfFaces);
//MessageBox(NULL, logText, "ssss", MB_OK);

//pGame3DModel->currentAnim = 1;
//pGame3DModel->currentFrame = pGame3DModel->pAnimations[pGame3DModel->currentAnim].startFrame;

	return true;
}

void CReaderMD2::readMD2Data() {
	m_pSkins		= new tMD2SkinName[m_pMD2Header.numOfSkins];
	m_pTexCoords	= new tMD2TexCoord[m_pMD2Header.numOfTexCoordinates];
	m_pFaces		= new tMD2Face[m_pMD2Header.numOfTriangles];
	m_pFrames		= new tMD2Frame[m_pMD2Header.numOfFrames];
	//OR
	//m_pFrames		= (tMD2Frame*)calloc(m_pMD2Header.numOfFrames, sizeof(tMD2Frame));
	//OR
	//m_pFrames		= (tMD2Frame*)malloc(sizeof(tMD2Frame)*m_pMD2Header.numOfFrames);
	
//sprintf(logText, "m_pFrames = %d", m_pMD2Header.offsetFramesStart);
//MessageBox(NULL, logText, "logText", MB_OK);

	readSkins();
	readTexCoords();
	readFaces();
	readFrames();
}

void CReaderMD2::readSkins() {
	fseek(mFilePointer, m_pMD2Header.offsetSkinsStart, SEEK_SET);
	fread(m_pSkins, sizeof(tMD2SkinName), m_pMD2Header.numOfSkins, mFilePointer);

for(int i = 0; i < m_pMD2Header.numOfSkins; i++) {
	sprintf(logText, "Skins %d %s\r\n", i, m_pSkins[i]);
	fwrite(logText, 1, strlen(logText), mFileConsoleLog);
}
}

void CReaderMD2::readTexCoords() {
	fseek(mFilePointer, m_pMD2Header.offsetTexCoordStart, SEEK_SET);
	fread(m_pTexCoords, sizeof(tMD2TexCoord), m_pMD2Header.numOfTexCoordinates, mFilePointer);
	//OR
	//for(int i = 0;  i < m_pMD2Header.numOfTexCoordinates; i++) {
	//	fread(&m_pTexCoords[i], sizeof(tMD2TexCoord), 1, mFilePointer);
	//}

sprintf(logText, "--------- Texture Coordinates ---------\r\n");
fwrite(logText, 1, strlen(logText), mFileConsoleLog);
for(int i = 0;  i < m_pMD2Header.numOfTexCoordinates; i++) {
	sprintf(logText, "\t %d - %d \t %d\r\n", i, m_pTexCoords[i].u, m_pTexCoords[i].v);
	fwrite(logText, 1, strlen(logText), mFileConsoleLog);
}
}

void CReaderMD2::readFaces() {
	fseek(mFilePointer, m_pMD2Header.offsetTrianglesStart, SEEK_SET);
	fread(m_pFaces, sizeof(tMD2Face), m_pMD2Header.numOfTriangles, mFilePointer);

logText[255];
sprintf(logText, "--------- FACES ---------\r\n");
fwrite(logText, 1, strlen(logText), mFileConsoleLog);

sprintf(logText, "\t--------- VERTEX LIST ---------\r\n");
fwrite(logText, 1, strlen(logText), mFileConsoleLog);
for(int i = 0;  i < m_pMD2Header.numOfTriangles; i++) {
	sprintf(logText, "\t\t VERTEX %d - %d \t %d \t %d\r\n", i, m_pFaces[i].vertexIndices[0], m_pFaces[i].vertexIndices[1], m_pFaces[i].vertexIndices[2]);
	fwrite(logText, 1, strlen(logText), mFileConsoleLog);
}

sprintf(logText, "\t--------- TEXTURE VERTEX LIST ---------\r\n");
fwrite(logText, 1, strlen(logText), mFileConsoleLog);
for(int i = 0;  i < m_pMD2Header.numOfTriangles; i++) {
	sprintf(logText, "\t\t TEXTURE VERTEX %d - %d \t %d \t %d\r\n", i, m_pFaces[i].textureIndices[0], m_pFaces[i].textureIndices[1], m_pFaces[i].textureIndices[2]);
	fwrite(logText, 1, strlen(logText), mFileConsoleLog);
}
}

void CReaderMD2::readFrames() {
	//Create a larger buffer for 1 frame of animation (remember its numOfVertices*(compressedVertex[3]+lightNormalIndex) + 128)
	unsigned char buffer[MD2_MAX_FRAME_SIZE];
	
	fseek(mFilePointer, m_pMD2Header.offsetFramesStart, SEEK_SET);

sprintf(logText, "--------- FRAMES ---------\r\n");
fwrite(logText, 1, strlen(logText), mFileConsoleLog);

	for(int i = 0; i < m_pMD2Header.numOfFrames; i++) {
sprintf(logText, "\t--------- FRAME %d---------\r\n", i);
fwrite(logText, 1, strlen(logText), mFileConsoleLog);

		tMD2AliasFrame *pFrame = (tMD2AliasFrame *)buffer;

		//memset(&m_pFrames, 0, sizeof(tMD2Frame));
		//m_pFrames->pVertices = (tMD2Vertex *)malloc(sizeof(tMD2Vertex)*m_pMD2Header.numOfVertices);
		//(&m_pFrames[i])->pVertices = new tMD2Vertex[m_pMD2Header.numOfVertices];
		m_pFrames[i].pVertices = new tMD2Vertex[m_pMD2Header.numOfVertices];

		fread(pFrame, 1, m_pMD2Header.frameSize, mFilePointer);

		strcpy(m_pFrames[i].frameName, pFrame->frameName);

		tMD2Vertex *pVertices = m_pFrames[i].pVertices;
		for(int j = 0; j < m_pMD2Header.numOfVertices; j++) {
			pVertices[j].vertex[0] = pFrame->aliasVertex[j].compressedVertex[0] * pFrame->scale[0] + pFrame->translate[0];
			pVertices[j].vertex[1] = pFrame->aliasVertex[j].compressedVertex[2] * pFrame->scale[2] + pFrame->translate[2];
			pVertices[j].vertex[2] = -(pFrame->aliasVertex[j].compressedVertex[1] * pFrame->scale[1] + pFrame->translate[1]);

sprintf(logText, "\t\t VERTEX %d - %0.2f \t %0.2f \t %0.2f\r\n", j, pVertices[j].vertex[0], pVertices[j].vertex[1], pVertices[j].vertex[2]);
fwrite(logText, 1, strlen(logText), mFileConsoleLog);

		}
	}
}

//This function converts the .md2 structures to our own model and object structures
void CReaderMD2::convertDataStructures(t3DModel *pModel) {
	int i = 0, j = 0;
sprintf(logText, "--------- AFTER CONVERSION ---------\r\n");
fwrite(logText, 1, strlen(logText), mFileConsoleLog);

	// Like the previous function, not a lot was changed her from the last tutorial.
	// Since we are dealing with multiple key frames, we need to pretty much do the 
	// same things for each frame.  This entails adding a for loop to go through
	// every frame.  We use index 'i' into the m_pFrames array to set each key frame.
	// This is one tricky thing in the middle of this for loop though, since we only
	// need to save the face and texture information once, this is only calculated
	// once and stored in the first frame of animation.  We can then reference the 
	// polygon information for every other frame from the first frame.  That way
	// we don't copy the same information for every key frame, which would make a huge
	// memory footprint.

	//Initialize the Model structure
	memset(pModel, 0, sizeof(t3DModel));

	//Set the number of Frames to the numOfObjects in our Model.
	pModel->numOfObjects = m_pMD2Header.numOfFrames;

	//Create our Animation List and store it our Model
	parseAnimations(pModel);

	//Go through every Key Frame and store its Vertices info in out pObject list.
	for(int i = 0; i < pModel->numOfObjects; i++) {
sprintf(logText, "\t--------- OBJECTTTTTTTTTTTTTTTTTT %d ---------\r\n", i);
fwrite(logText, 1, strlen(logText), mFileConsoleLog);
		
		//Create a local Object to store the first Frane of the Animation's data
		t3DObject currentObject = {0};
		
		//Assign maxVertices, maxFaces and maxTextureCordinates to our Object
		currentObject.numOfVertices			= m_pMD2Header.numOfVertices;
		currentObject.numOfFaces			= m_pMD2Header.numOfTriangles;
		currentObject.numOfTextureVertex	= m_pMD2Header.numOfTexCoordinates;

		//Allocate memory for the Vertices.
		currentObject.pVertices		= new CVector3[currentObject.numOfVertices];
			
sprintf(logText, "\t\t--------- VERTEXXXXX ---------\r\n");
fwrite(logText, 1, strlen(logText), mFileConsoleLog);
		// Go through all of the vertices and assign them over to our structure
		for(int j = 0; j < currentObject.numOfVertices; j++) {
			currentObject.pVertices[j].x = (&m_pFrames[i])->pVertices[j].vertex[0];
			currentObject.pVertices[j].y = (&m_pFrames[i])->pVertices[j].vertex[1];
			currentObject.pVertices[j].z = (&m_pFrames[i])->pVertices[j].vertex[2];

sprintf(logText, "\t\t\t VERTEX %d - %0.2f \t %0.2f \t %0.2f\r\n", j, currentObject.pVertices[j].x, currentObject.pVertices[j].y, currentObject.pVertices[j].z);
fwrite(logText, 1, strlen(logText), mFileConsoleLog);

		}		
		
		// We can now free the old vertices stored in this frame of animation
		// since we have them stored in the pObject list of our model
		delete m_pFrames[i].pVertices;

		// Now comes the tricky part, since we only need to store the UV coordinates
		// and face indices once, we only do this on the first frame.  If the
		// current frame is past the first frame, we just add the current frame
		// as it is to our list, then continue to the next frame.  If we are
		// on the first frame, we still need to store the UV and face information.
		// This will only go in the first frame, and will be referenced from the
		// remaining frames when we animate the model.

		// Check if we are past the first key frame/Object
		if(i > 0) {
			// Here we add the current object (or frame) to our list object list
			pModel->pObjects.push_back(currentObject);
			continue;	// Go on to the next key frame
		}
		
		// We will only get here ONCE because we just need this information
		// calculated for the first key frame.
			
		// Allocate memory for our UV Texture Coordinates and Face information
		currentObject.pTexVertices	= new CVector2[currentObject.numOfTextureVertex];
		currentObject.pFaces		= new tFace[currentObject.numOfFaces];

sprintf(logText, "\t\t--------- TEXXXXX VERTEXXXXX ---------\r\n");
fwrite(logText, 1, strlen(logText), mFileConsoleLog);
		// Go through all of the uv coordinates and assign them over to our structure.
		// The UV coordinates are not normal uv coordinates, they have a pixel ratio of
		// 0 to 256.  We want it to be a 0 to 1 ratio, so we divide the u value by the
		// skin width and the v value by the skin height.  This gives us our 0 to 1 ratio.
		// For some reason also, the v coodinate is flipped upside down.  We just subtract
		// the v coordinate from 1 to remedy this problem.
		for(int j = 0; j < currentObject.numOfTextureVertex; j++) {
			currentObject.pTexVertices[j].x = m_pTexCoords[j].u / float(m_pMD2Header.skinWidth);
			currentObject.pTexVertices[j].y = 1 - m_pTexCoords[j].v / float(m_pMD2Header.skinHeight);
sprintf(logText, "\t\t\t TEX VERTICES FACE %d TEXCO = %0.2f %0.2f\r\n", j, currentObject.pTexVertices[j].x, currentObject.pTexVertices[j].y);
fwrite(logText, 1, strlen(logText), mFileConsoleLog);
		}

sprintf(logText, "\t\t--------- MMMMMMMMMMMMMMMMMMMM ---------\r\n");
fwrite(logText, 1, strlen(logText), mFileConsoleLog);
		// Go through all of the face data and assign it over to OUR structure
		for(int j = 0; j < currentObject.numOfFaces; j++) {

			//Assign the Vertex indices to our Face data
			currentObject.pFaces[j].vertexIndex[0] = m_pFaces[j].vertexIndices[0];
			currentObject.pFaces[j].vertexIndex[1] = m_pFaces[j].vertexIndices[1];
			currentObject.pFaces[j].vertexIndex[2] = m_pFaces[j].vertexIndices[2];

			//Assign the Texture Coordinate indices to our Face data
			currentObject.pFaces[j].texCoordIndex[0] = m_pFaces[j].textureIndices[0];
			currentObject.pFaces[j].texCoordIndex[1] = m_pFaces[j].textureIndices[1];
			currentObject.pFaces[j].texCoordIndex[2] = m_pFaces[j].textureIndices[2];
sprintf(logText, "\t\t\tFACE %d VI = %d %d %d %d %d %d\r\n", j, m_pFaces[j].vertexIndices[0], m_pFaces[j].vertexIndices[1], m_pFaces[j].vertexIndices[2], currentObject.pFaces[j].texCoordIndex[0], currentObject.pFaces[j].texCoordIndex[1], currentObject.pFaces[j].texCoordIndex[2]);
fwrite(logText, 1, strlen(logText), mFileConsoleLog);

		}

		// Here we add the currentObject (or frame) to our list Object list
		pModel->pObjects.push_back(currentObject);
	}
}

//This function fills in the animation list for each animation by name and frame
void CReaderMD2::parseAnimations(t3DModel *pModel) {
	tAnimation animation = {0};
	string strLastName = "";

	//t3DModel *model = pModel;
	//vector<tAnimation> ANIM;

	// This function felt like a hack when I wrote it.  You aren't really given
	// any good information about the animations, other than the fact that each
	// key frame has a name assigned to it with a frame number for that animation.
	// For instance, the first animation is the "stand" animation.  The first frame
	// would have the name of: "stand01" or perhaps "stand1".  The 40th frame is
	// usually the last frame for the standing animation, so it would look like:
	// "stand40".  After this frame, the next animation is the running animation.
	// The next frame is labeled something like "run01".  You now know that the
	// standing animation is from frame 1 to frame 40 of the total frames.  The
	// start of the run animation is 41.  We will know how long the run animation
	// goes when we run into another animation name besides "run..".  That is how
	// I went about finding out the animation information.  I just grab each frame
	// name and check if it's the same animation name as the last name we found.
	// If it is, I just ignore it and continue to the next frame.  Once I find that
	// it's not, I then have the last frame saved off from the index 'i', and then
	// I create a new animation to add to the list, then start from the beginning.
	// It wasn't until later that I found on www.planetquake.com that there is a
	// standard frame count for each animation and they are all the same.  I figure
	// this way makes it modular so you don't have to stick to those standards anyway.

	// Go through all of the frames of animation and parse each animation
	for(int i = 0; i < pModel->numOfObjects; i++) {

		//Assign the name of this frame of Animation to a dtring object
		string strName = (&m_pFrames[i])->frameName;
		int frameNum = 0;
		
		// Go through and extract the frame numbers and erase them from the name
		for(unsigned int j = 0; j < strName.length(); j++) {
			
			if( isdigit(strName[j]) && j >= strName.length() - 2) {

				// Use a C function to convert the character to a integer.
				// Notice we use the address to pass in the current character and on
				frameNum = atoi(&strName[j]);

				//Erase the frameNum from the name so we extract the Animation Name
				strName.erase(j, strName.length() - j);

				break;
			}
		}

		//Check if the Animation name is not the same as the last Frame
		//or if we are on the last frame of Animation for this Model
		if(strName != strLastName || i == pModel->numOfObjects - 1) {
//MessageBox(NULL, strName.c_str(), "asdasd", MB_OK);
			// If this animation frame is NOT the first frame
			if(strLastName != "") {

				// Copy the last animation name into our new animation's name
				strcpy(animation.animName, strLastName.c_str());

				// Set the last frame of this animation to i
				animation.endFrame = i;

				// Add the animation to our list and reset the animation object for next time
				
				pModel->pAnimations.push_back(animation);

				memset(&animation, 0, sizeof(tAnimation));

				// Increase the number of animations for this model
				pModel->numOfAnimations++;
sprintf(logText, "ANIMATION NAME %s = %d %d\r\n", animation.animName, animation.startFrame, animation.endFrame);
fwrite(logText, 1, strlen(logText), mFileConsoleLog);

			}

			// Set the starting frame number to the current frame number we just found,
			// minus 1 (since 0 is the first frame) and add 'i'.
			animation.startFrame = frameNum - 1 + i;
		}

		// Store the current animation name in the strLastName string to check it latter
		strLastName = strName;
	}
}