
#include "main.h"
#include "3DS.h"

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
// *Tutorial Note* 
// By reading this tutorial we assume you have looked at the previous octree,
// frustum culling and model loading tutorials.  If not, we strongly recommend it.
//
// This is the third tutorial of the 3 part series on octrees.  The first 2 showed
// how to create an octree, along with incorporating frustum culling to it.  With
// this final part, the octree class will now load in a 3D world from a .3DS file.
// The file format doesn't really matter because all of our loader tutorials
// use the t3DModel structure to store the data, which is passed into the octree
// object to partition.  The octree is completely independent from the model loader.
// 
// In the previous tutorials we stored vertices in the end nodes.  This version takes
// a different approach, since we are dealing with a model structure.  Instead of
// storing vertices, we just store face indices.  There are many benefits to this
// method.  The first is that we can easily plug in this data to vertex arrays, which
// increases our efficiency.  The second is that we don't have to recalculate the
// face information when we break up the data into nodes.  In any model loader, you
// are given all the vertices in the model, then for every polygon to render, indices
// into that vertex array.  This makes it so you don't have to store duplicate vertices
// in memory or the 3D file.  Like the first advantage, it allows us to easily render
// the model through vertex arrays.  The last benefit of this method is that we don't
// have to deal with splitting polygons across planes and then perhaps triangulating
// them.  
//
// For optimal performance, we will be using display lists and vertex arrays for rendering
// each node.  You really wouldn't want it any other way.  Due to the fact that our world
// isn't as big as a full level you would find in a game, the speed increase that the
// octree provides won't be as noticeable, especially on high end graphics cards.  Keep
// this in mind when working with your own octree implementation.  In my experience, the
// biggest problem is loops and being fill limited.  Sometimes, your problem is getting
// geometry to the screen, it's the amount of pixels that are being drawn to the screen.
// If you make your view port size smaller and you get a significant increase, then
// your bottle neck is fill limited.  For instance, On a TNT2 card, I get about 10 to
// 15 frames more on this tutorial when I change the resolution from 800 x 600 to 640 x 480.
// That is why I set the resolution smaller to increase frame rate.  
//
// This application allows you to walk around the world using our camera class.  Intially,
// fog is turned on to give it a cool effect.  You can turn this off to get a better view
// of the world.  
//
// The controls are: 
//
// Mouse               - Rotates the camera's view
// Left Mouse Button   - Changes the Render mode from normal to wire frame.
// Right Mouse Button  - Turns lighting On/Off
// Up    Arrow Key | W - Moves the camera along the view vector
// Down  Arrow Key | S - Moves the camera opposite the view vector
// F				   - Turns fog on/off
// Space Bar		   - Turns on/off the yellow debug lines for the visual node subdivisions
// Escape - Quits

FILE* gConsoleLogFilePtr = NULL;

bool gWriteNodeInfoToFile = false;

//////////////////////// 3DS SPECIFIC
// This is our 3DS loader object that will load the g_World data in
CLoad3DS				g3DSReader;

//Stores the various .3ds models in a list
vector<t3DModel>		pGame3DModel;

// This will store our 3ds scene that we will pass into our octree
t3DModel				gWorldModel;

// This stores our current frames per second
char g_strFrameRate[255] = {0};

// This tells us if we want fog on or off - On by default ('F')
bool g_bFog = true;

// This tells us if we want to display the yellow debug lines for our nodes (Space Bar)
bool g_bDisplayNodes = false;
////////////////////////////////////////////////////////// 3DS SPECIFIC

//////////////////////// FRUSTUM CULLING
CFrustum*	g_Frustum;					// This is our global frustum object

bool		g_bIgnoreFrustum = false;	// We want to check against the frustum initially
////////////////////////////////////////////////////////// FRUSTUM CULLING

//////////////////////// OCTREE SPECIFIC

// Here we initialize our single Octree object.  This will hold all of our vertices
COctree g_Octree;

// We created a debug class to show us the nodes visually in yellow cubes.
CDebug g_Debug;

// This will store an array of 3D points that we will read from a file.
CVector3* g_pVertices = NULL;

// This will store the amount of vertices read in from the text file (.raw)
int g_NumberOfVertices = 0;

// The maximum amount of triangles per node.  If a node has equal or less 
// than this, stop subdividing and store the verts in that node
int g_MaxTriangles = 1000;

// The maximum amount of subdivisions allowed (Levels of subdivision)
int g_MaxSubDivisions = 5;

// The current amount of end nodes in our tree (The nodes with vertices stored in them)
int g_EndNodeCount = 0;

// Set the current render mode to triangles initially
bool g_bRenderMode = true;

// Turn lighting on initially
bool g_bLighting     = true;

bool g_RenderDebug = false;

// This stores the variable that increases the rotation angle for our terrain (not important)
float g_RotateX = 0;
////////////////////////////////////////////////////////// OCTREE SPECIFIC
/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

// This stores the amount of nodes that are in the frustum
int g_TotalNodesDrawn = 0;

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *


HDC			mHDC		= NULL;			// Private GDI Device Context
HGLRC		mHRC		= NULL;			// Permanent Rendering Context
HWND		mHWnd		= NULL;			// Holds Our Window Handle
HINSTANCE	mHInstance;					// Holds The Instance Of The Application

bool		mKeys[256];					// Array Used For The Keyboard Routine
bool		mKeysPressed[256];					// Array Used For The Keyboard Routine
bool		mActive = false;			// Window Active Flag
bool		mFULLSCREEN = true;			// Fullscreen Flag Set To TRUE By Default
bool		L_MOUSE_DOWN = false;
bool		R_MOUSE_DOWN = false;

GLuint* textures = {0};
int gLoadedTextureCount = 0;
GLuint fontList;
static float FPS;
char debugString[5][255] = {0};

bool g_RenderMode = false;
int glViewMode = GL_TRIANGLES;

SCENE scene;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
GLvoid	resizeGLScene(GLsizei width, GLsizei height);
int initGL(GLvoid);
int drawGLScene(GLvoid);
GLvoid killGLWindow(GLvoid);
bool createGLWindow(char* title, int width, int height, int bits, bool isFullScreen);

CCamera gCamera;

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

void handleKeyboard(void) {
	if(mKeys[VK_F1]) {
		mKeys[VK_F1] = false;
		killGLWindow();
		mFULLSCREEN = !mFULLSCREEN;
		if(!createGLWindow("openGL window", SCREEN_WIDTH, SCREEN_HEIGHT, 16, mFULLSCREEN)) {
			return;
		}
	}

	if(mKeys['R'] && !mKeysPressed['R']) {
		mKeysPressed['R'] = true;

		g_RenderDebug = !g_RenderDebug;
	}
	if(!mKeys['R'])
		mKeysPressed['R'] = false;

	if(mKeys['M'] && !mKeysPressed['M']) {
		mKeysPressed['M'] = true;

		gWriteNodeInfoToFile = true;
	}
	if(!mKeys['M'])
		mKeysPressed['M'] = false;
}

bool bitmapLoader(LPCSTR fileName/*, GLuint textureID*/) {
	HBITMAP hBMP;
	BITMAP BMP;

	hBMP = (HBITMAP)LoadImage(GetModuleHandle(NULL), fileName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);

	if(!hBMP)
		return false;

	GetObject(hBMP, sizeof(BMP), &BMP);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glBindTexture(GL_TEXTURE_2D, textures[gLoadedTextureCount] /*textureID*/);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, BMP.bmWidth, BMP.bmHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, BMP.bmBits);

	gLoadedTextureCount++;
/*
char sCurText[255];
sprintf(sCurText, "%s = %d x %d", fileName, BMP.bmWidth, BMP.bmHeight);
MessageBox(NULL, sCurText, "W x H", MB_OK);
//*/
	DeleteObject(hBMP);

	return true;
}

int WINAPI WinMain(	HINSTANCE	hInstance, 
					HINSTANCE	hPrevInstance,
					LPSTR		lpCmdLine,
					int			nCmdShow
) {
	MSG		msg;
	bool	done = false;

	//if(MessageBox(NULL, "Would you like to run in FULLSCREEN mode?", "Start FullScreen", MB_YESNO) == IDNO)
		mFULLSCREEN = false;

	if(!createGLWindow("openGL Window...Press F1 to toogle between windowed & Fullscreen Mode.", SCREEN_WIDTH, SCREEN_HEIGHT, 16, mFULLSCREEN))
		return 0;

	while(!done) {
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if(msg.message == WM_QUIT)
				done = true;
			else  {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else {
			if(mActive) {
				if(mKeys[VK_ESCAPE])
					done = true;
				else {
					gCamera.updateCamera();
					handleKeyboard();

					drawGLScene();
					SwapBuffers(mHDC);
				}
			}

		}
	}

	// Here we free our vertice data that was loaded from our .raw file
	delete[] g_pVertices;
	fclose(gConsoleLogFilePtr);

	killGLWindow();
	return msg.wParam;
}

bool createGLWindow(char* title, int width, int height, int bits, bool isFullScreen) {
	int			iPixelFormat;
	WNDCLASS	wc;
	DWORD		dwExStyle;
	DWORD		dwStyle;
	RECT		windowRect;

	windowRect.left		= (long)0;
	windowRect.right	= (long)width;
	windowRect.top		= (long)0;
	windowRect.bottom	= (long)height;

	mFULLSCREEN = isFullScreen;

	mHInstance			= GetModuleHandle(NULL);
	
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hbrBackground	= NULL;
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);
	wc.hInstance		= mHInstance;
	wc.lpfnWndProc		= (WNDPROC) WndProc;
	wc.lpszClassName	= "OpenGL";
	wc.lpszMenuName		= NULL;
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	
	if(!RegisterClass(&wc)) {
		MessageBox(NULL, "Failed to Register the Window Class.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	if(mFULLSCREEN) {
		DEVMODE	dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize			= sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth	= width;
		dmScreenSettings.dmPelsHeight	= height;
		dmScreenSettings.dmBitsPerPel	= bits;
		dmScreenSettings.dmFields		= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		if(ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
			if(MessageBox(NULL, "The requested FullScreen Mode is not supported\n by your Video Card. Use Windowed Mode instead?", "Error", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
				mFULLSCREEN = false;
			else {
				MessageBox(NULL, "Program will now close.", "ERROR", MB_OK | MB_ICONSTOP);
				return false;
			}
		}
	}
	
	ShowCursor(!false);

	if(mFULLSCREEN) {
		dwExStyle	= WS_EX_APPWINDOW;
		dwStyle		= WS_POPUP;
	}
	else {
		dwExStyle	= WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle		= WS_OVERLAPPEDWINDOW;
	}

	AdjustWindowRectEx(&windowRect, dwStyle, false, dwExStyle);

	if(!(mHWnd = CreateWindowEx(	dwExStyle,
								"OpenGL",
								title,
								dwStyle | 
								WS_CLIPSIBLINGS |
								WS_CLIPCHILDREN,
								0, 0,
								windowRect.right - windowRect.left,
								windowRect.bottom - windowRect.top,
								NULL,
								NULL,
								mHInstance,
								NULL))
	) {
		killGLWindow();								// Reset The Display
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;								// Return FALSE
	}

	if(!(mHDC = GetDC(mHWnd))) {
		killGLWindow();								// Reset The Display
		MessageBox(NULL,"Unable to get Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;								// Return FALSE
	}
	
	static PIXELFORMATDESCRIPTOR pfd = 
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		bits,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		16,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	if(!(iPixelFormat = ChoosePixelFormat(mHDC, &pfd))) {
		killGLWindow();								// Reset The Display
		MessageBox(NULL,"Unable to choose Pixel format.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;								// Return FALSE
	}

	if(!SetPixelFormat(mHDC, iPixelFormat, &pfd)) {
		killGLWindow();								// Reset The Display
		MessageBox(NULL,"Unable to set Pixel format.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;								// Return FALSE
	}

	if(!(mHRC = wglCreateContext(mHDC))) {
		killGLWindow();								// Reset The Display
		MessageBox(NULL,"Unable to create wgl context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;								// Return FALSE
	}

	if(!wglMakeCurrent(mHDC, mHRC)) {
		killGLWindow();								// Reset The Display
		MessageBox(NULL,"Unable to make wgl context","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;								// Return FALSE
	}
								
	ShowWindow(mHWnd, true);
	SetForegroundWindow(mHWnd);
	SetFocus(mHWnd);

	resizeGLScene(width, height);

	if(!initGL()) {
		killGLWindow();								// Reset The Display
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;								// Return FALSE
	}
	
	return true;
}

GLvoid resizeGLScene(GLsizei width, GLsizei height) {
	if(height == 0)
		height = 1;
	
	float fieldOfView = 45.0f;
	float nearPlaneDistance = 0.1f;
	float farPlaneDistance = 50000.0f;
	float whRatio = (GLfloat)width/(GLfloat)height;

	if(!g_Frustum) {
		g_Frustum = new CFrustum();
	}
		
	g_Frustum->setFrustum(fieldOfView, whRatio, nearPlaneDistance, farPlaneDistance);

	glViewport(0, 0, width, height);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(fieldOfView, whRatio, nearPlaneDistance, farPlaneDistance);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void buildFont(void) {
	fontList = glGenLists(256);
	glBindTexture(GL_TEXTURE_2D, textures[TEX_ID_FONT]);
	
	float cx, cy;
	int MAX_CHARS_ON_X = 16;
	float ONE_CHAR_WIDTH = 1/16.0f;

	for(int i = 0; i < 256; i++) {
		cx = (float)(i%16)/16.0f;
		cy = (float)(i/16)/16.0f;

		glNewList(fontList + i, GL_COMPILE);
			glBegin(GL_QUADS);
				glTexCoord2f(cx, 1 - cy - ONE_CHAR_WIDTH);
				glVertex2i(0, 0);

				glTexCoord2f(cx + ONE_CHAR_WIDTH, 1 - cy - ONE_CHAR_WIDTH);
				glVertex2i(8, 0);

				glTexCoord2f(cx + ONE_CHAR_WIDTH, 1 - cy);
				glVertex2i(8, 8);

				glTexCoord2f(cx, 1 - cy);
				glVertex2i(0, 8);
			glEnd();
			
			//Later Translate it to the actual character width rather than a fixed 16 for all...
			glTranslated(8, 0, 0);
		glEndList();
	}
}

void glPrint(int x, int y, unsigned char r, unsigned char g, unsigned char b, const char* format, ...) {
	/***** Extract Text *****/
	char text[256];
	va_list ap;

	if(format == NULL)
		return;

	va_start(ap, format);
		vsprintf(text, format, ap);
	va_end(ap);
	/***********************/
	
	/*** Bind Font Texture ***/
	glBindTexture(GL_TEXTURE_2D, textures[TEX_ID_FONT]);
	
	/*** Disable Depth testing ***/
	glDisable(GL_DEPTH_TEST);
	
	/*** Save PROJECTION MATRIX & set it to ORTHOGONAL Projection ***/
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, SCREEN_WIDTH, 0, SCREEN_HEIGHT, -1, 1);
	
	/*** Save MODELVIEW MATRIX ***/
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
glColor3ub(r, g, b);
	
	/*** Enable BLENDING to remove the black from the FONT borders ***/
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	

	/*** Set font PRINT position ***/
	glTranslated(x, y, 0);
	
	/*** Set FONT start & PRINT the entite TEXT ***/
	glListBase(fontList - 32);
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
	
	/*** Disable BLENDING ***/
	glDisable(GL_BLEND);
glColor3ub(255, 255, 255);
	
	/*** Revert back Original PROJECTION MATRIX ***/
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	/*** Revert back Original MODELVIEW MATRIX ***/
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	/*** Enable DEPTH TEST ***/
	glEnable(GL_DEPTH_TEST);
}

void glPrint(int x, int y, const char* format, ...) {
	/***** Extract Text *****/
	char text[256];
	va_list ap;

	if(format == NULL)
		return;

	va_start(ap, format);
		vsprintf(text, format, ap);
	va_end(ap);
	/***********************/
	
	/*** Bind Font Texture ***/
	glBindTexture(GL_TEXTURE_2D, textures[TEX_ID_FONT]);
	
	/*** Disable Depth testing ***/
	glDisable(GL_DEPTH_TEST);
	
	/*** Save PROJECTION MATRIX & set it to ORTHOGONAL Projection ***/
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, SCREEN_WIDTH, 0, SCREEN_HEIGHT, -1, 1);
	
	/*** Save MODELVIEW MATRIX ***/
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	/*** Enable BLENDING to remove the black from the FONT borders ***/
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	

	/*** Set font PRINT position ***/
	glTranslated(x, y, 0);
	
	/*** Set FONT start & PRINT the entite TEXT ***/
	glListBase(fontList - 32);
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
	
	/*** Disable BLENDING ***/
	glDisable(GL_BLEND);
	
	/*** Revert back Original PROJECTION MATRIX ***/
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	/*** Revert back Original MODELVIEW MATRIX ***/
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	/*** Enable DEPTH TEST ***/
	glEnable(GL_DEPTH_TEST);
}

void readStr(FILE* f, char* string) {
	do {
		fgets(string, 255, f);
	} while(string[0] == '/' || string[0] == '\n');
}

void readModels(t3DModel* pModel, char* objectFileName) {
	//////////////// 3DS Specific ////////////////
	// First we need to actually load the .3DS file.  We just pass in an address to
	// our t3DModel structure and the file name string we want to load ("face.3ds").
	g3DSReader.Import3DS(pModel, objectFileName);

	// Depending on how many textures we found, load each one (Assuming .BMP)
	// If you want to load other files than bitmaps, you will need to adjust CreateTexture().
	// Below, we go through all of the materials and check if they have a texture map to load.
	// Otherwise, the material just holds the color information and we don't need to load a texture.
	///*
	// Go through all the materials
	for(int i = 0; i < gWorldModel.numOfMaterials; i++) {

		// Check to see if there is a file name to load in this material
		if(strlen(gWorldModel.pMaterials[i].strFile) > 0) {

			// Use the name of the texture file to load the bitmap, with a texture ID (i).
			// We pass in our global texture array, the name of the texture, and an ID to reference it.	
			//CreateTexture(g_Texture, g_World.pMaterials[i].strFile, i);
			bitmapLoader(gWorldModel.pMaterials[i].strFile);
		}

		// Set the texture ID for this material
		gWorldModel.pMaterials[i].texureId = i;
	}
	//*/

	//g3DSReader.makeModelList(pModel);
	//////////////// 3DS Specific ////////////////	
}

void readMap(char* filePathAndName) {

	/////////////////// LOADING GAME FONTS
	char singleLine[255] = "";
	FILE* filePtr = fopen(filePathAndName, "rt");
	
	int maxTextures = 0;
	readStr(filePtr, singleLine);
	sscanf(singleLine, "MAX_TEXTURES %d\n", &maxTextures);

	textures = new GLuint[MAX_TEXTURES];
	glGenTextures(MAX_TEXTURES, &textures[0]);

	char textureID[255] = "";
	char textureName[255] = "";
	int TEX_ID = 0;
	for(int i = 0; i < maxTextures; i++) {
		readStr(filePtr, singleLine);
		sscanf(singleLine, "%s %d %s\n", &textureID, &TEX_ID, &textureName);
		
		bitmapLoader(textureName/*,  textures[i]*/);

		//MessageBox(NULL, textureID, textureName, MB_OK);
	}
	buildFont();
	///////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////// LOADING CAMERA
	gCamera = CCamera(	CVector3(0.0f, 0.0f, -1.0f),
						CVector3(1.0f, 0.0f,  0.0f),
						CVector3(0.0f, 1.0f,  0.0f)
					);
	float camPosX, camPosY, camPosZ;
	readStr(filePtr, singleLine);
	sscanf(singleLine, "CAMERA_POSITION %f %f %f\n", &camPosX, &camPosY, &camPosZ);
	gCamera.setPosition(CVector3(camPosX, camPosY, camPosZ));
	///////////////////////////////////////////////////////////////////////////////////////////////
	
	/////////////////// LOADING 3DS MODELS
	char OBJECT_FILE_NAME[255] = "";
	char TEMP_STR[255] = "";
	int MAX_MODELS_TO_LOAD = 0, MODEL_ID = 0;
	readStr(filePtr, singleLine);
	sscanf(singleLine, "MODELS_TO_LOAD %d\n", &MAX_MODELS_TO_LOAD);
	
	readStr(filePtr, singleLine);
	sscanf(singleLine, "%s %d %s\n", &TEMP_STR, &MODEL_ID, &OBJECT_FILE_NAME);
	
	readModels(&gWorldModel, OBJECT_FILE_NAME);

	pGame3DModel.push_back(gWorldModel);
	///////////////////////////////////////////////////////////////////////////////////////////////

	/*
	/////////////////// LOADING SCENE
	int maxVertices = 0;
	float x, y, z;
	readStr(filePtr, singleLine);
	sscanf(singleLine, "MAX_VERTICES %d\n", &maxVertices);
	scene.maxVertices = maxVertices;
	scene.vertices = new VERTEX[maxVertices];
	for(int i = 0; i < maxVertices; i++) {
		readStr(filePtr, singleLine);
		sscanf(singleLine, "%f %f %f\n", &x, &y, &z);

		scene.vertices[i].x = x;
		scene.vertices[i].y = y;
		scene.vertices[i].z = z;
	}

	int maxFaces = 0;
	int v0, v1, v2, v3, TEXTURE_ID;
	float u, v;
	readStr(filePtr, singleLine);
	sscanf(singleLine, "MAX_FACES %d\n", &maxFaces);
	scene.maxFaces = maxFaces;
	scene.faces = new FACE[maxFaces];
	for(int i = 0; i < maxFaces; i++) {
		readStr(filePtr, singleLine);
		sscanf(singleLine, "%d %d %d %d %f %f %d\n", &v0, &v1, &v2, &v3, &u, &v, &TEXTURE_ID);
		
		scene.faces[i].vertexID[0] = v0;
		scene.faces[i].vertexID[1] = v1;
		scene.faces[i].vertexID[2] = v2;
		scene.faces[i].vertexID[3] = v3;

		scene.faces[i].u = u;
		scene.faces[i].v = v;
		scene.faces[i].TEXTURE_ID = TEXTURE_ID;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////
	*/
}

int initGL(GLvoid) {
	gConsoleLogFilePtr = fopen("console.log", "wb");

	readMap("data/map.txt");

	// The first thing that needs to happen before creating our octree is to find
	// the total width of the initial root node.  Now we pass in our t3DModel object
	// to GetSceneDimensions(), instead of vertices and a vertex count, as done
	// in the last octree tutorials.  This will store the initial root node cube width.
	g_Octree.getSceneDimensions(&gWorldModel);

	// Since our model structures stores multiple objects, we can't easily access the
	// total triangle count in the scene with out manually going through and counting the total.
	// This is what we do below.  With the result, we pass this into our createNode() function.
	int totalTriangleCount = g_Octree.getSceneTriangleCount(&gWorldModel);

	// To create the first node we need the world data, the total triangle count in the scene,
	// along with the initial root node center and width.  This function will then recursively
	// subdivide the rest of the world, according to the global restrictions.
	g_Octree.createNode(&gWorldModel, totalTriangleCount, g_Octree.getCenter(), g_Octree.getWidth());
	
	//g_Octree.listNodeInfo(0, 1);

	// The octree should be created by now.  To better increase our efficiency we use display
	// lists for every end node.  This way, we just have to call a display list ID to draw
	// a node, verses the slow loops we normal had.  Vertex arrays are also used to optimize
	// our rendering of the octree.

	// Below we get the display list base ID and store it in the root node.  This should return 1
	// since we don't use display lists anywhere before this.  Notice that we use our global
	// variable that stores our end node count to pass in the total amount of list ID's needed.
	// If you are unfamiliar with displays, basically what you do is section off a certain
	// amount of ID's, and then you are returns a base pointer to the start of those ID's.
	// You can use the ID's from the base pointer to the base pointer ID + the number of
	// ID's that were saved off for that base pointer.  Each of the ID's correspond to a
	// bunch of OpenGL commands.  That means that each end node has it's own ID that
	// corresponds to a bunch of OpenGL commands.  So, for instance, if pass in a bunch
	// of vertices to OpenGL, we can assign this action to a display list.  That way we
	// just call a display list ID to perform that action.  Think of it as a function.
	// You just need to call a function to do a bunch of tasks, which eliminates extra
	// code, and also is saved on the video card for faster processing.  
	g_Octree.setDisplayListID( glGenLists(g_EndNodeCount));

	// Now we go through every single end node and create a display list for them all.
	// That way, when we draw the end node, we just use it's display list ID to render
	// the node, instead of looping through all the objects and manually give the verts to opengl.
	// The parameters for this function is the node (starting with the root node), 
	// the world data and current display list base ID.  The base ID is stored in the root
	// node's ID, so we just pass that in.  The reason we do this is because, if you create
	// other display lists before you create the octree, you don't want to assume the octree
	// ID's go from 1 to the end node count.  By passing in the base ID, we then will add
	// this ID to other nodes.  Right now, when they are created they are assigned the
	// end node count at the time upon creating them.  This will make more sense when looking
	// at the octree code.
	g_Octree.createDisplayList(&g_Octree, &gWorldModel, g_Octree.getDisplayListID());

	// Here, we turn on a light and enable lighting.  We don't need to
	// set anything else for lighting because we will just take the defaults.
	// We also want color, so we turn that on too.  We don't load any normals from
	// our .raw file so we will calculate some simple face normals to get a decent
	// perspective of the terrain.

	glEnable(GL_LIGHT0);								// Turn on a light with defaults set
	glEnable(GL_LIGHTING);								// Turn on lighting
	glEnable(GL_COLOR_MATERIAL);						// Allow color
	
	glCullFace(GL_BACK);								// Set culling to backface culling
	glEnable(GL_CULL_FACE);								// Enable backface culling
	glEnable(GL_DEPTH_TEST);							// Enable Depth Testing
	glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping

	return true;
}

GLvoid killGLWindow() {
	if(mFULLSCREEN) {
		ChangeDisplaySettings(NULL, 0);
		ShowCursor(true);
	}

	if(mHRC) {
		if(!wglMakeCurrent(NULL, NULL))
			MessageBox(NULL, "Release of DC and RC failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);

		if(!wglDeleteContext(mHRC))
			MessageBox(NULL, "Release of DC and RC failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);

		mHRC = NULL;
	}

	if(mHDC && !ReleaseDC(mHWnd, mHDC)) {
		MessageBox(NULL, "Release of Device Context failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		mHDC = NULL;
	}

	if(mHWnd && !DestroyWindow(mHWnd)) {
		MessageBox(NULL, "Release of mHWnd failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		mHWnd = NULL;
	}

	if(!UnregisterClass("OpenGL", mHInstance)) {
		MessageBox(NULL, "UnRegisterClass failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		mHInstance = NULL;
	}
}

void drawDebug(void) {
	glDisable(GL_LIGHTING);
	glColor3f(1.0, 1.0, 1.0);

	//Enable texturing mode, which might have been disabled after Multi-Texturing
	glEnable(GL_TEXTURE_2D);
	
	glPrint(10, SCREEN_HEIGHT - 10, 0, 255, 255, "MOUSE [ LEFT ] - RENDER_MODE");
	glPrint(10, SCREEN_HEIGHT - 20, 0, 255, 255, "MOUSE [ RIGHT ] - ON/OFF LIGHTING");
	glPrint(10, SCREEN_HEIGHT - 30, 0, 255, 255, "KEYBOARD [ R ] - ON/OFF OCTREE DEBUG");
	

	glPrint(10, 50, 0, 255, 255, "DEBUG[0]: %s", debugString[0]);
	glPrint(10, 40, 0, 255, 255, "DEBUG[1]: %s", debugString[1]);
	glPrint(10, 30, 0, 255, 255, "DEBUG[2]: %s", debugString[2]);
	glPrint(10, 20, 0, 255, 255, "DEBUG[3]: %s", debugString[3]);
	glPrint(10, 10, 0, 255, 255, "DEBUG[4]: %s", debugString[4]);

	gCamera.cameraDebug();
	
	if(g_bLighting)
		glEnable(GL_LIGHTING);
}

void drawGameScene(void) {

	for(int i = 0; i < scene.maxFaces; i++) {
		int vertex0 = scene.faces[i].vertexID[0] - 1;
		int vertex1 = scene.faces[i].vertexID[1] - 1;
		int vertex2 = scene.faces[i].vertexID[2] - 1;
		int vertex3 = scene.faces[i].vertexID[3] - 1;

		float u = scene.faces[i].u;
		float v = scene.faces[i].v;
		
		glBindTexture(GL_TEXTURE_2D, textures[scene.faces[i].TEXTURE_ID]);
		glBegin(GL_QUADS);
			glTexCoord2f(0, 0);
			glVertex3f(	scene.vertices[vertex0].x,
						scene.vertices[vertex0].y,
						scene.vertices[vertex0].z
					);
			
			glTexCoord2f(u, 0);
			glVertex3f(	scene.vertices[vertex1].x,
						scene.vertices[vertex1].y,
						scene.vertices[vertex1].z
					);
			
			glTexCoord2f(u, v);
			glVertex3f(	scene.vertices[vertex2].x,
						scene.vertices[vertex2].y,
						scene.vertices[vertex2].z
					);
			
			glTexCoord2f(0, v);
			glVertex3f(	scene.vertices[vertex3].x,
						scene.vertices[vertex3].y,
						scene.vertices[vertex3].z
					);
		glEnd();
	}
}

void draw3DGrid() {
	glDisable(GL_TEXTURE_2D);

	// This function was added to give a better feeling of moving around.
	// A black background just doesn't give it to ya :)  We just draw 100
	// green lines vertical and horizontal along the X and Z axis.

	// Turn the lines GREEN
	glColor3f(0.0, 1.0, 0.0);

	// Draw a 1x1 grid along the X and Z axis'
	for(float i = -50; i < 50; i++) {
		
		// Start drawing some lines
		glBegin(GL_LINES);
			
			// Do the horizontal lines (along the X)
			glVertex3f(-50.0f, 0.0f, i);
			glVertex3f( 50.0f, 0.0f, i);

			// Do the vertical lines (along the Z)
			glVertex3f(i, 0.0f, -50);
			glVertex3f(i, 0.0f,  50);
		glEnd();
	}

	glEnable(GL_TEXTURE_2D);
}

int drawGLScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	
	gCamera.moveCamera();
	
/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

	// Each frame we calculate the new frustum.  In reality you only need to
	// calculate the frustum when we move the camera.
	g_Frustum->calculateFrustum();

	// Initialize the total node count that is being draw per frame
	g_TotalNodesDrawn = 0;

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
	
g_Octree.listNodeInfo(0, 1);

	// Here we draw the octree, starting with the root node and recursing down each node.
	// When we get to each of the end nodes we will draw the vertices assigned to them.
	g_Octree.drawOctree(&g_Octree, &gWorldModel);

	// Render the cube'd nodes to visualize the octree (in wire frame mode)
	if(g_RenderDebug)
		g_Debug.renderDebugLines();


	
	//////////////////////////////////////////////////////////////////////////////////
	// To view our octree information I set the window's title bar to the some basic
	// information such as the max triangles per node, the max subdivisions, 
	// total end nodes and the total drawn end nodes that are currently in the frustum.
	// Display in window mode the current subdivision information
	sprintf(debugString[0], "MaxTri: %d, MaxSubDiv: %d", g_MaxTriangles, g_MaxSubDivisions);
	sprintf(debugString[1], "TotEndNodes: %d", g_EndNodeCount);
	sprintf(debugString[2], "TotNodesDrawn: %d", g_TotalNodesDrawn);
	// Set our window title bar to the subdivision data
	//SetWindowText(mHWnd, strBuffer);
	//////////////////////////////////////////////////////////////////////////////////

	drawDebug();
	
	return true;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch(uMsg) {
		case WM_CREATE:
		{
			SetCursorPos(SCREEN_WIDTH>>1, SCREEN_HEIGHT>>1);
			return 0;
		}
		break;
		case WM_ACTIVATE:
		{
			if(!HIWORD(wParam))
				mActive = true;
			else
				mActive = false;

			return 0;
		}
		case WM_SYSCOMMAND:
		{
			switch(wParam) {
				case SC_SCREENSAVE:
				case SC_MONITORPOWER:
					return 0;
			}
		}
		break;
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}
		case WM_KEYDOWN:
		{
			mKeys[wParam] = true;
			return 0;
		}
		case WM_KEYUP:
		{
			mKeys[wParam] = false;
			return 0;
		}
		case WM_SIZE:
		{
			resizeGLScene(LOWORD(lParam), HIWORD(lParam));
			return 0;
		}
		case WM_LBUTTONDOWN:
		{
			//L_MOUSE_DOWN = true;

			// Change the rendering mode to lines or triangles depending on it's current status
			g_bRenderMode = !g_bRenderMode;

			// Change the rendering mode to and from lines or triangles
			if(g_bRenderMode) {
				// Render the triangles in fill mode		
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	
			}
			else {
				// Render the triangles in wire frame mode
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);	
			}

		}
		break;
		case WM_LBUTTONUP:		
		{
			//L_MOUSE_DOWN = false;
		}
		break;
		case WM_RBUTTONDOWN:
			//R_MOUSE_DOWN = true;

			g_bLighting = !g_bLighting;						// Turn lighting ON/OFF

			if(g_bLighting) {								// If lighting is ON
				glEnable(GL_LIGHTING);						// Enable OpenGL lighting
			} else {
				glDisable(GL_LIGHTING);						// Disable OpenGL lighting
			}

		break;
		case WM_RBUTTONUP:
			//R_MOUSE_DOWN = false;

		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////
//
// * QUICK NOTES * 
//
// If this tutorial was too much of a jump from the previous tutorials, I apologize.
// I tried to think of people who were advanced and wanted all the optimizations, and
// at the same time try not to lose anyone.  Fog was added just for a cool effect.
// This can actually be used to help speed up the octree.  If you make the fog really
// dense, you can then bring in the near plane for our frustum.  This will make it so
// you can't see so far, which means you won't have to display as many end nodes.
// 
// Let's go over the functions that need to be called from the client to make the
// octree work:
// 
//  First we need to get the cube's bounding box of the world:
//
//  // Gets the scene dimensions for our entire model
// 	g_Octree.getSceneDimensions(&g_World);
//
//  Then we need to find our how many polygons make up the world:
//
//  // Returns the whole scene's polygon count
//	g_Octree.getSceneTriangleCount(&g_World);
//
//  Once we find the polygon count, we can pass that into:
//
//  // This starts with the root node and recursively creates the octree
//	g_Octree.createNode(&g_World, TotalTriangleCount, g_Octree.getCenter(), g_Octree.getWidth());
//
//  Since we are using display lists, we want to find the display list base ID:
//
//  // This creates and sets the base list ID for all the end nodes
//	g_Octree.setDisplayListID( glGenLists(g_EndNodeCount) );
//
//  To finally create the display list, we take the base list ID and pass it into:
//
//  //  This recursively creates a display list for every end node
//	g_Octree.createDisplayList(&g_Octree, &g_World, g_Octree.getDisplayListID());
//
//  Once the octree is finished, we can finally call our drawOctree() function in RenderScene():
//
//  // Draw the octree with the root world being passed in, along with the root node.
//  g_Octree.drawOctree(&g_Octree, &g_World);
// 
// I hope this tutorial helps give you ideas for your own octree implementation.  There is
// no one way to do it.  This is just what I came up with.  I will strongly suggest that you
// stick with using vertex arrays if you haven't before.  The impact is huge without them, 
// especially when you get up to rendering thousands of polygons.
//
// I would like to thank Elmar Moelzer <moelzere@aon.at> for the Park.3ds art.  You can visit
// his company's page for more cool art and game oriented stuff.  He is pretty amazing!
// It is located at www.mediastudio-graz.com.
// 
// Good luck!
// 
//
// Ben Humphrey (DigiBen)
// Game Programmer
// DigiBen@GameTutorials.com
// www.GameTutorials.com
//
// © 2000-2003 GameTutorials
