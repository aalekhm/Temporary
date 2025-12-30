
#include "main.h"

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
// 
// By reading this tutorial we assume you have looked at the previous octree tutorial,
// as well as the frustum culling tutorial.  If not we strongly recommend it.
//
// This is the second tutorial of the 3 part series on octrees.  The last tutorial
// focused on creating the octree.  This tutorial adds to the previous tutorial
// by adding the frustum culling.   
//

// The controls are: 

// Left Mouse Button   - Changes the Render mode from normal to wire frame.
// Right Mouse Button  - Turns lighting On/Off
// Left  Arrow Key | A - Rotates the camera left
// Right Arrow Key | D - Rotates the camera right
// Up    Arrow Key | W - Moves the camera along the view vector
// Down  Arrow Key | S - Moves the camera opposite the view vector
// F5				   - Increases the max triangle count by 20 for each node
// F6				   - Decreases the max triangle count by 20 for each node
// +				   - Increases the max subdivision count
// -				   - Decreases the max subdivision count
// Escape - Quits
//
//
/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

//////////////////////// FRUSTUM CULLING
CFrustum*	g_Frustum;					// This is our global frustum object

bool		g_bIgnoreFrustum = false;	// We want to check against the frustum initially
////////////////////////////////////////////////////////// FRUSTUM CULLING

//////////////////////// OCTREE SPECIFIC

// This is the file name that is to be loaded
#define TERRAIN_FILE_NAME    "data/Terrain.raw" 

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
int g_MaxTriangles = 100;

// The maximum amount of subdivisions allowed (Levels of subdivision)
int g_MaxSubDivisions = 0;

// The current amount of end nodes in our tree (The nodes with vertices stored in them)
int g_EndNodeCount = 0;

// Set the current render mode to triangles initially
bool g_bRenderMode = true;

// Turn lighting on initially
bool  g_bLighting     = true;							

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
char debugString[255] = {0};

bool g_RenderMode = false;

SCENE scene;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
GLvoid	resizeGLScene(GLsizei width, GLsizei height);
int initGL(GLvoid);
int drawGLScene(GLvoid);
GLvoid killGLWindow(GLvoid);
bool createGLWindow(char* title, int width, int height, int bits, bool isFullScreen);

CCamera camera;

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
///////////////////////////////// LOAD VERTICES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
// This function loads the vertices of our terrain from a text file (Terrain.raw)
void loadVertices() {
	
	// This function reads in the vertices from an ASCII text file (terrain.raw).
	// First, we read in the vertices with a temp CVector3 to get the number of them.
	// Next, we rewind the file pointer and then actually read in the vertices into
	// our allocated CVector3 array g_pVertices[].

	// Create a file pointer and load the model from a file of vertices
	FILE* fp = fopen(TERRAIN_FILE_NAME, "rb");

	// Make sure we opened the file correctly and it was found
	if(!fp) {
		MessageBox(NULL, "Can't Open Terrain File", "Error", MB_OK);
		return ;
	}

	// Create a temporary variable to hold what we read in from the file
	CVector3 vTemp;

	// Read in the number of vertices that are stored in the file
	while(1) {
		
		// Read in a vertice and get the return value. If it's EOF we are done reading
		int result = fscanf(fp, "%f %f %f\n", &vTemp.x, &vTemp.y, &vTemp.z);

		// If we hit End Of File then we are done reading in the vertices, break
		if(result == EOF)
			break;

		// Increase the vertice count
		g_NumberOfVertices++;
	}

	// Allocate the needed memory to hold the vertices
	g_pVertices = new CVector3[g_NumberOfVertices];

	// Go back to the beginning of the file so we can store the vertices now
	rewind(fp);

	// Create a counter for the index of the g_pVertices[] array
	int index = 0;

	// Read in the vertices that are stored in the file
	for(int i = 0; i < g_NumberOfVertices; i++) {

		// Read in the current vertice and at the end add 1 to the index
		fscanf(fp, "%f %f %f\n", &g_pVertices[index].x, &g_pVertices[index].y, &g_pVertices[index].z);

		index++;
	}
	
	// Close our file because we are done
	fclose(fp);
}

///////////////////////////////// RECREATE OCTREE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This function destroys the octree and debug lines then creates a new one
/////
///////////////////////////////// RECREATE OCTREE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
void recreateOctree() {

	// You will not need this function for the octree.  It is just for the tutorial
	// every time we change our subdivision information.

	g_EndNodeCount = 0;						// Reset the end node count

	g_Debug.clear();						// Clear the list of debug lines
	g_Octree.destroyOctree();				// Destroy the octree and start again

	// Get the new scene dimensions since we destroyed the previous octree
	g_Octree.getSceneDimensions(g_pVertices, g_NumberOfVertices);
				
	// Create our new octree with the new subdivision information
	g_Octree.createNode(g_pVertices, g_NumberOfVertices, g_Octree.getCenter(), g_Octree.getWidth());
}

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

void handleKeyboard(void) {
	if(mKeys[VK_F1]) {
		mKeys[VK_F1] = false;
		killGLWindow();
		mFULLSCREEN = !mFULLSCREEN;
		if(!createGLWindow("openGL window", 640, 480, 16, mFULLSCREEN)) {
			return;
		}
	}
	
	/////////////////////////////////////////////////////////////
	if(mKeys[VK_ADD] && !mKeysPressed[VK_ADD]) {
		mKeysPressed[VK_ADD] = true;

		g_MaxSubDivisions += 1;					// Increase the current max subdivisions

		// Make sure we don't go over 10 subdivisions, otherwise it will get crazy
		if(g_MaxSubDivisions  > 10) g_MaxSubDivisions = 10;
		
		// Destroy the octree and debug lines and create a new one with the new info
		recreateOctree();
	}
	if(!mKeys[VK_ADD])
		mKeysPressed[VK_ADD] = false;

	/////////////////////////////////////////////////////////////
	if(mKeys[VK_SUBTRACT] && !mKeysPressed[VK_SUBTRACT]) {
		mKeysPressed[VK_SUBTRACT] = true;
		
		g_MaxSubDivisions -= 1;					// Increase the current max subdivisions

		// Make sure we don't go below zero subdivisions
		if(g_MaxSubDivisions < 0) g_MaxSubDivisions = 0;
		
		// Destroy the octree and debug lines and create a new one with the new info
		recreateOctree();
	}
	if(!mKeys[VK_SUBTRACT])
		mKeysPressed[VK_SUBTRACT] = false;

	/////////////////////////////////////////////////////////////
	if(mKeys[VK_F5] && !mKeysPressed[VK_F5]) {
		mKeysPressed[VK_F5] = true;

		g_MaxTriangles += 20;					// Increase the max triangle count by 20

		// Make sure we don't go above 2000
		if(g_MaxTriangles > 2000) g_MaxTriangles  = 2000;
		
		// Destroy the octree and debug lines and create a new one with the new info
		recreateOctree();
	}
	if(!mKeys[VK_F5])
		mKeysPressed[VK_F5] = false;

	/////////////////////////////////////////////////////////////
	if(mKeys[VK_F6] && !mKeysPressed[VK_F6]) {
		mKeysPressed[VK_F6] = true;

		g_MaxTriangles -= 20;					// Decrease the max triangle count by 20

		// Make sure we don't go below 0 (0 would produce as many nodes as triangles)
		if(g_MaxTriangles < 0) g_MaxTriangles  = 0;

		// Destroy the octree and debug lines and create a new one with the new info
		recreateOctree();
	}
	if(!mKeys[VK_F6])
		mKeysPressed[VK_F6] = false;

	/////////////////////////////////////////////////////////////

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

	if(!createGLWindow("openGL Window...Press F1 to toogle between windowed & Fullscreen Mode.", 640, 480, 16, mFULLSCREEN))
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
					camera.updateCamera();
					handleKeyboard();

					drawGLScene();
					SwapBuffers(mHDC);
				}
			}

		}
	}

	// Here we free our vertice data that was loaded from our .raw file
	delete[] g_pVertices;

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
	float nearPlaneDistance = 1.0f;
	float farPlaneDistance = 1000.0f;
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
		
		//bitmapLoader(textureName/*,  textures[i]*/);

		//MessageBox(NULL, textureID, textureName, MB_OK);
	}
	buildFont();
	///////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////// LOADING CAMERA
	camera = CCamera(	CVector3(0.0f, 0.0f, -1.0f),
						CVector3(1.0f, 0.0f,  0.0f),
						CVector3(0.0f, 1.0f,  0.0f)
					);
	float camPosX, camPosY, camPosZ;
	readStr(filePtr, singleLine);
	sscanf(singleLine, "CAMERA_POSITION %f %f %f\n", &camPosX, &camPosY, &camPosZ);
	camera.setPosition(CVector3(camPosX, camPosY, camPosZ));
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
	readMap("data/map.txt");
	
	/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

	loadVertices();

	// Calculate the bounding box of our scene.  This will give us a width of
	// the cube that is needed to surround the whole world.  We want to pass in
	// the vertices and the vertex count into this function to find the farthest point
	// from the center of the world.  That will then be our width.  Depending on the
	// world this doesn't always surround it perfectly. In the next tutorial we will fix that.
	g_Octree.getSceneDimensions(g_pVertices, g_NumberOfVertices);

	// Here we pass in the information to create the root node.  This will then
	// recursively subdivide the root node into the rest of the node.
	g_Octree.createNode(g_pVertices, g_NumberOfVertices, g_Octree.getCenter(), g_Octree.getWidth());

	// Here, we turn on a light and enable lighting.  We don't need to
	// set anything else for lighting because we will just take the defaults.
	// We also want color, so we turn that on too.  We don't load any normals from
	// our .raw file so we will calculate some simple face normals to get a decent
	// perspective of the terrain.

	glEnable(GL_LIGHT0);								// Turn on a light with defaults set
	glEnable(GL_LIGHTING);								// Turn on lighting
	glEnable(GL_COLOR_MATERIAL);						// Allow color

	/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

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

	glPrint(10, SCREEN_HEIGHT - 10, "Use MOUSE RT CLICK - RENDER_MODE");

	glPrint(10, 40, "DEBUG : %s", debugString);

	camera.cameraDebug();

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
	
	camera.moveCamera();
	
/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

	// Each frame we calculate the new frustum.  In reality you only need to
	// calculate the frustum when we move the camera.
	g_Frustum->calculateFrustum();

	// Initialize the total node count that is being draw per frame
	g_TotalNodesDrawn = 0;

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

	// Here we draw the octree, starting with the root node and recursing down each node.
	// When we get to each of the end nodes we will draw the vertices assigned to them.
	g_Octree.drawOctree(&g_Octree);

	// Render the cube'd nodes to visualize the octree (in wire frame mode)
	g_Debug.renderDebugLines();
	
	//////////////////////////////////////////////////////////////////////////////////
	char strBuffer[255] = {0};							// Create a character buffer
	// To view our octree information I set the window's title bar to the some basic
	// information such as the max triangles per node, the max subdivisions, 
	// total end nodes and the total drawn end nodes that are currently in the frustum.
	// Display in window mode the current subdivision information
	sprintf(strBuffer, "MaxTriangles: %d     MaxSubDivisions: %d     TotalEndNodes: %d       TotalNodesDraw: %d",
		                g_MaxTriangles,		 g_MaxSubDivisions,		 g_EndNodeCount,		 g_TotalNodesDrawn);
	// Set our window title bar to the subdivision data
	SetWindowText(mHWnd, strBuffer);
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
			L_MOUSE_DOWN = false;
		}
		break;
		case WM_RBUTTONDOWN:
			R_MOUSE_DOWN = true;

			g_bLighting = !g_bLighting;						// Turn lighting ON/OFF

			if(g_bLighting) {								// If lighting is ON
				glEnable(GL_LIGHTING);						// Enable OpenGL lighting
			} else {
				glDisable(GL_LIGHTING);						// Disable OpenGL lighting
			}

		break;
		case WM_RBUTTONUP:
			R_MOUSE_DOWN = false;
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////
//
// * QUICK NOTES * 
//
// This tutorial was an add on to the last octree tutorial which just showed us
// how to create an octree.  There wasn't a lot added, but the power that is provides
// is incredible.  We now have a functional octree that only draws the vertices in
// our view.  There are 2 things we added to this tutorial since the last:
// 
// 1) We added the frustum code (frustum.cpp and frustum.h).  This is all explained
// in our frustum tutorial on our site if you don't understand the code.  Once we
// create our global CFrustum object: g_Frustum, we just need to call this every frame:
// 
// g_Frustum.CalculateFrustum();
// 
// We only really need to call it when the camera moves but you can do those checks 
// yourself.  Then, once the frustum planes are calculated we can use this function to
// check if the end nodes are in our frustum:
//
// g_Frustum.CubeInFrustum(center.x, center.y, center.z, cubeWidth / 2);
// 
// That's it for the frustum!  Simple, yet wonderfully usefull.  We left in the sphere
// and point code in frustum.cpp from the frustum tutorial just so if you wanted to
// include the frustum.cpp file in your application/game you didn't have to copy and
// paste them back in.  Just ingore those for this tutorial though.
// 
// 2) Finally we added a counter (g_TotalNodesDrawn) to tell us how many of the end 
// nodes are being drawn.  This let's us know how well the octree is working.  You 
// can view this information in the window's title bar, along with the other subdivision 
// information.
// 
// Also notice that we took our the rotating.  This is so all the cubes stay axis aligned.
//
// Hopefully by breaking the octree tutorial up into multiple parts it isn't so hard
// to digest.  The next tutorial will focus on reading in a real world with texture
// information, normals, etc...  Then you can actually use this code in a project.
// I also included the HTML octree tutorial with this tutorial as well just in case
// some people didn't want to download the first octree tutorial but wanted to get right
// to the good stuff.  This HTML file will explain in more detail the octree theory, etc...
// 
// Good luck!
// 
//
// Ben Humphrey
// Game Programmer
// DigiBen@GameTutorials.com
// www.GameTutorials.com
//
// © 2000-2003 GameTutorials
