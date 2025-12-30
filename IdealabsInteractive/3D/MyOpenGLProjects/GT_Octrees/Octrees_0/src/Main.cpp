
#include "main.h"

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

// *Tutorial Note* 
// It's assumed that you have read or at least understand the camera and 
// frustum tutorial. If not, we suggest you review it so you can better understand the 
// more important stuff.  Though we do not use the frustum code yet, it is good to know.
//
// This tutorial demonstrates how to create and draw an octree.  What is an octree?
// An octree is tree of nodes that stores vertex information about the scene/level/world.
// The purpose of it is to partition space.  This allows you to check what the camera
// is looking at, then only draw that part of the screen.  If you a huge level or world, 
// you don't want to pass OpenGL the vertices every frame right?  The 100's of thousands of
// vertices would really bring your game to a crawl of .02 frames per second.  It's true
// that OpenGL does some of the clipping for you, but it isn't as fast as you can do it
// yourself.  That might sound weird because OpenGL is a BEAST and it was programmed by
// really intelligent SGI guru's right???  Well, the problem isn't with their code, it's
// the fact that you are doing a for/while loop for thousands of vertices every frame.
// Loops are tremendously slow and are on of the main causes of slow applications.  To
// avoid this problem, we need to divide space into a bunch of nodes, then test if we
// are looking at each one of those nodes.  If we are, then we will want to draw them,
// otherwise ignore them.  The whole beauty of an octree is, that you can specify how
// many vertices/triangles that you want to store in each node.  Also be sure to use
// vertex arrays for fast rendering.  That way you avoid the huge slow down of calling
// glVertex*() functions thousands of times per frame.  They also avoid loops.  We 
// don't use them in this tutorial.
//
// A node is basically a cube that is responsible for a certain amount of vertices.  
// If that cube collides with the frustum (our camera's view) then we render that cube's 
// vertices.  If that cube has children nodes, then it does not have any vertices stored 
// in it, just children that have vertices (or children that have children that store 
// vertices, etc..). This could recurse down forever, except that we have a max level of 
// recursion/subdivision and a maximum amount of triangles stored in each node.  
// Once a node gets below the maximum amount of triangles it stops subdividing.  Now you
// see why this is exponentionally faster than testing every vertex/triangle to see if
// it's in the frustum.  Testing one cube against the frustum is so much faster than
// testing it's 1000 or so triangles.  
//
// The maximum amount of triangles for a node can depend on the size of your scene.  
// If you make it too low it could become slow because you have thousands of nodes.  
// Doing that much recursion every frame could possibly slow down your frame rate.  
// It's always good to start with something like 1000 and then tweak it to find the best 
// result that works for your scene/level/world.  Since our Terrain example doesn't have
// a huge amount of triangles in it, I chose to set the max at 100 to get more subdivisions.
// If you refer to the *Quick Notes* down at the bottom I will explain more of the detail
// on exactly how the octree works and how it is created.  There is also a HTML file that
// should be combined with this tutorial to give a more visual explanation on how it works.
//
// Note that this tutorial is the first octree tutorial of a 3 part series.  We
// do not do any frustum culling yet because I didn't want to complicate the octree lesson.
// The next octree tutorial will involve frustum culling, while the last one of this series
// will load a .3ds file that has actual world with texture information.  This will demonstrate
// how to have a more real world approach to octree's with texture and normals information.
//
// This tutorial currently loads a text file full of vertices.  I chose this because though
// it is not realistic, it is the best way not to over complicate the tutorial.  You can
// also easily add your own vertices to it to test your own stuff.  The .raw file was
// created by simply loading in a .3ds file with our .3ds loader code, then instead
// of calling glVertex3f() to render the vertex, there was a fprintf() to print that
// vertex out to a file.  After the first frame we quit the program.  Then you have a
// huge text file full of vertices.
//
// The controls are: 
//
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

//////////////////////// FRUSTUM CULLING
// You can tell how many spheres are being rendered out of how 
// many possible in the windows title bar.  You will notice the huge difference
// from 1000 possible spheres when frustum culling is turned off.  It barely moves.
#define		MAX_SPHERES		1000		// This holds the maximum amount of spheres.  (use + and - to change amount)
#define		MAX_DISTANCE	30			// This is the distance that the spheres disappear on either side
#define		SPHERE_SPEED	0.05f		// This is the speed the spheres move
#define		MAX_RADIUS		5			// This is the spheres radius*10 (really it's 0.5)

CFrustum*	g_Frustum;					// This is our global frustum object

bool		g_bIgnoreFrustum = false;	// We want to check against the frustum initially

// You can change this variable by the +/- keys.
int g_MaxSpheres = MAX_SPHERES / 2;		// Set the spheres on screen to the max/2 allowed.

Sphere g_Spheres[MAX_SPHERES];			// This is our structure that holds the sphere data
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
		
		//bitmapLoader(textureName/*, textures[i]*/);

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

void drawSpheres() {
	glDisable(GL_TEXTURE_2D);

	int spheresRendered = 0;								// This will hold how many spheres are being rendered
	char windowTitleText[255] = {0};						// This will hold the window title info

	// We don't need to calculate this every frame, only when the camera view changes.
	// I just did it every frame anyway.  In this case it isn't a big deal.
	g_Frustum->calculateFrustum();						// Calculate the frustum each frame

	// If you are unfamiliar with Quadrics, see the tutorial on Quadrics at www.GameTutorials.com.
	// They basically allow you to draw circles and cylinders fast and easily.
	GLUquadricObj *pObj = gluNewQuadric();					// Get a Quadric off the stack

	// Loop through all of our allowed spheres and render them to the screen if in the frustum.
	for(int i = 0; i < g_MaxSpheres; i++) {
		
		g_Spheres[i].zPos += SPHERE_SPEED;					// Increase the Z position of the sphere.

		// Below we check if the sphere needs to be draw or not.  If g_bIgnoreFrustum is TRUE,
		// it draws it regardless (which is SLOOOooOoW).  We just pass in the (X, Y, Z)
		// and the radius of the sphere to find out if it is inside of the frustum.
		if(	//g_bIgnoreFrustum
			//||
			//g_Frustum->isSphereInFrustum(g_Spheres[i].xPos, g_Spheres[i].yPos, g_Spheres[i].zPos, g_Spheres[i].radius)
			g_Frustum->isPointInFrustum(g_Spheres[i].xPos, g_Spheres[i].yPos, g_Spheres[i].zPos)
		) {
//MessageBox(NULL, "AsdaS", "AsdA", MB_OK);
			// Set the sphere's color
			glColor3ub(g_Spheres[i].r, g_Spheres[i].g, g_Spheres[i].b);

			// Create a new scope before positiong the sphere so we don't effect the other spheres.
			glPushMatrix();
				
				// Position the sphere on the screen at it's XYZ position.
				glTranslatef(g_Spheres[i].xPos, g_Spheres[i].yPos, g_Spheres[i].zPos);

				// Create a sphere with the desired radius chosen in the beginning.
				gluSphere(pObj, g_Spheres[i].radius, 20, 20);			// Draw the sphere with a radius of 0.5
			glPopMatrix();												// Close the scope of this matrix

			spheresRendered++;
		}

		// Here we check to see if the sphere went out of our range,
		// If so, we need to set it back again with a new random position.
		if(g_Spheres[i].zPos > MAX_DISTANCE) {
			
			// Give the sphere a new random position back in the beginning.
			g_Spheres[i].xPos = (rand() % (MAX_DISTANCE * 10)) * 0.1f;
			g_Spheres[i].yPos = (rand() % (MAX_DISTANCE * 10)) * 0.1f;
			g_Spheres[i].zPos = -MAX_DISTANCE;			// Send it to the back again.
			
			// Give a 50/50 chance for the sphere to be to the left/right or above/below the XY axis.
			// This is because we are centered at the origin
			if(rand() % 2) g_Spheres[i].xPos = -g_Spheres[i].xPos;
			if(rand() % 2) g_Spheres[i].yPos = -g_Spheres[i].yPos;
		}
	}

	// The information tells you how many spheres were rendered and out of how many.
	// Use +/- to increase and decrease the max spheres tested.
	sprintf(debugString, "Spheres rendered = %d / %d", spheresRendered, g_MaxSpheres);

	gluDeleteQuadric(pObj);
}

int drawGLScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	
	camera.moveCamera();
	
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
	sprintf(strBuffer, "MaxTriangles: %d     MaxSubDivisions: %d     TotalEndNodes: %d",
		                g_MaxTriangles,		 g_MaxSubDivisions,		 g_EndNodeCount);
	// Set our window title bar to the subdivision data
	SetWindowText(mHWnd, strBuffer);
	//////////////////////////////////////////////////////////////////////////////////

	//drawDebug();
	
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
// At first an octree (or any space partitioning) can be intimidating.  Once you
// sit down and start coding it, you become to fully understand the wonderful
// benefits and it's simplicity.  If you can create a tree with nodes, you can
// create an octree.  The only complicated part at first might be the splitting
// of polygons that intersect more than one node.  We don't do that in this tutorial
// so it didn't get to complicated and overhauled with code.  It's way to frustrating
// to learn a ton of things at once, instead of the main idea and basic functionality.
// This octree will work fine, except there will possibly be some doubling of vertices
// because we don't split them into separate one.  That is fine, but not idea for a real
// product.  Eventually you want to fix this.  The last octree tutorial will read in
// a fully textured world that will be split correctly.  Also a more modular octree class
// will be created because we are missing UV coordinates, normals, material ID's, and
// more importantly frustum culling.  The next tutorial will tackle the frustum culling.
//
// Let me explain once more briefly the intent of an octree is.  If you have a huge world
// with 10's of thousands of triangles, you don't want to do a for loop and pass those
// all into OpenGL to be rendered.  You only want to pass in the triangles that you can
// see (the camera's view).  To do this, you want to subdivide your world into cubes
// that holds the triangle data for that area/region of the world.  Then, instead of
// checking if every triangle is in your view, you can just check if a cube intersects
// your frustum, meaning it's in the view of your camera.  This is one of the fastest
// way to do this type of space partitioning.  Just one subdivision puts the world
// into 8 cubes which could cut your triangle rendered count down to 1/8 of what it was.
// If just one subdivision could do that, think about 2, 3 or 4 levels of subdivision?
//
// Let's go over the steps to using this octree class.  Once you have your list of vertices
// you can begin.  The first thing you want to do is call this function:
//
// GetSceneDimensions(g_pVertices, g_NumberOfVerts);
//
// This passes in the list of vertices and the vertex count.  This function calculates
// the center point of the world, as well as the width of the cube that is needed to
// create the first node.  Once we have these values stored, we want to create our tree.
// We can call CreateNode() and pass in the starting data for the first node:
//
// CreateNode(g_pVertices, g_NumberOfVerts, g_Octree.GetCenter(), g_Octree.GetWidth());
//
// This takes the vertices, the vertex count, then the starting center and width.
// This will recurse through and create the whole tree, while assigning the vertices
// to the end nodes.  That's it!  You just created the octree.  The last part is drawing
// the octree.  This is simple, you just call:
//
// DrawOctree();
//
// This should be called in your RenderScene() function.  This goes through all the
// nodes until it gets to the end, then it draws the vertices assigned to those end nodes.
// The deconstructor takes care of the clean up by calling DestroyOctree().
//
// The CDebug class has nothing to do with the octree other than the fact that we use
// it to display the nodes visually.  I suggest you keep something like this around so
// you can visualize your subdivided world and get a better idea of how efficient it is.
//
// Be sure to check out the HTML tutorial on the concept of an octree.  It will give
// good visuals on the whole subdivision process so you can get a better grasp of the
// concepts.  That's about it, so enjoy this tutorial and please let us know what you
// do with it.  We are always curious to know where the knowledge goes. :)
//
// Remember, this tutorial is 1 of a 3 part series.  We still have not delved into the
// frustum culling and polygon splitting part of the octree.  Right now your octree does 
// you no good.  This just explains on how to create an octree.  Check out the second 
// tutorial which focuses on the frustum culling.
//
// Ben Humphrey
// Game Programmer
// DigiBen@GameTutorials.com
// www.GameTutorials.com
//
// © 2000-2003 GameTutorials
