
#include "main.h"

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

// This tutorial demonstrates how to tell if an object is inside of your frustum or not.
// What is a frustum?  A frustum is the camera's view.  If something does NOT appear
// on the screen, then it is not inside of the frustum.  Think of it as a skewed box.
// It is skewed because the farther away, the more you can see.  It has 6 sides.
// There is the right, left, top, bottom, near and far.  Anything rendered outside
// of these planes, is not shown.  You need to know how to test this because it speeds
// up your programs, especially when there are a lot of objects, and/or a large world.
// You don't want to send you whole world down the 3D pipeline, that would kill your
// frame rate, so you want to test to see if the object would even NEED to be drawn
// before you send it to be rendered.  Certainly this is useless if you have very few objects,
// depending on their size of course.
//
// CONTROLS:
//		MOUSE MOVEMENT - Looks around
//      C, SPACE BAR, Mouse LEFT CLICK- Turns culling ON/OFF
//      +/- - Increase and decreases the amount of spheres
//		ESCAPE - quits

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
GLuint fontList;
static float FPS;
char debugString[255] = {0};

int glViewMode = GL_TRIANGLES;

bool g_RenderMode = false;

SCENE scene;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
GLvoid	resizeGLScene(GLsizei width, GLsizei height);
int initGL(GLvoid);
int drawGLScene(GLvoid);
GLvoid killGLWindow(GLvoid);
bool createGLWindow(char* title, int width, int height, int bits, bool isFullScreen);

CCamera camera;

void handleKeyboard(void) {
	if(mKeys[VK_F1]) {
		mKeys[VK_F1] = false;
		killGLWindow();
		mFULLSCREEN = !mFULLSCREEN;
		if(!createGLWindow("openGL window", 640, 480, 16, mFULLSCREEN)) {
			return;
		}
	}
	
	if(L_MOUSE_DOWN) {
		L_MOUSE_DOWN = false;
		g_bIgnoreFrustum = !g_bIgnoreFrustum;
	}
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
	float nearPlaneDistance = 0.5f;
	float farPlaneDistance = 150.0f;
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
	char singleLine[255] = "";
	FILE* filePtr = fopen(filePathAndName, "rt");
	
	int maxTextures = 0;
	readStr(filePtr, singleLine);
	sscanf(singleLine, "MAX_TEXTURES %d\n", &maxTextures);
	
	textures = new GLuint[MAX_TEXTURES];
	glGenTextures(MAX_TEXTURES, &textures[0]);

	char textureID[255] = "";
	char textureName[255] = "";
	for(int i = 0; i < maxTextures; i++) {
		readStr(filePtr, singleLine);
		sscanf(singleLine, "%s %s\n", &textureID, &textureName);
		
		bitmapLoader(textureName, textures[i]);

		//MessageBox(NULL, textureID, textureName, MB_OK);
	}
	buildFont();

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
}

int initGL(GLvoid) {
	readMap("data/map.txt");

	//////// SETTING UP CAMERA
	camera = CCamera(	CVector3(0.0f, 0.0f, -1.0f),
						CVector3(1.0f, 0.0f,  0.0f),
						CVector3(0.0f, 1.0f,  0.0f)
					);
	camera.setPosition(CVector3(0.0f, 1.0f, 0.0f));
	
	/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
	// Here we turn on lighting, but just leave the defaults so at
	// least we have some good shading on the spheres.
	glEnable(GL_LIGHT0);								// Turn on a light
	glEnable(GL_LIGHTING);								// Turn on lighting

	// Once lighting is turned on, we need to enable this to use color.
	glEnable(GL_COLOR_MATERIAL);						// Allow us to use color for the spheres

	srand(GetTickCount());								// Seed the random numbers
	
	// This is where we initialize the sphere's data.  We randomly assign color
	// and place them around the camera.
	for(int i = 0; i < MAX_SPHERES; i++) {
		
		// Assign the radius and position - No farther away than 30 from the camera
		g_Spheres[i].radius		= (rand() % MAX_RADIUS) * 0.1f;
		//g_Spheres[i].xPos		= 0.0f;//(rand() % (MAX_DISTANCE * 10)) * 0.1f;
		//g_Spheres[i].yPos		= 0.0f;//(rand() % (MAX_DISTANCE * 10)) * 0.1f;
		//g_Spheres[i].zPos		= -50.0f;//(rand() % (MAX_DISTANCE * 10)) * 0.1f;

		g_Spheres[i].xPos		= (rand() % (MAX_DISTANCE * 10)) * 0.1f;
		g_Spheres[i].yPos		= (rand() % (MAX_DISTANCE * 10)) * 0.1f;
		g_Spheres[i].zPos		= (rand() % (MAX_DISTANCE * 10)) * 0.1f;

		// We have a 50/50 chance of putting the sphere on the left side and behind the camera.
		// This is because we are centered at the origin
		if(rand() % 2) g_Spheres[i].xPos = -g_Spheres[i].xPos;
		if(rand() % 2) g_Spheres[i].yPos = -g_Spheres[i].yPos;
		if(rand() % 2) g_Spheres[i].zPos = -g_Spheres[i].zPos;

		// Assign the sphere's color randomly
		g_Spheres[i].r = rand() % 256;					// Get a random red color
		g_Spheres[i].g = rand() % 256;					// Get a random green color
		g_Spheres[i].b = rand() % 256;					// Get a random blue color
	}
	/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

	//glShadeModel(GL_SMOOTH);
	//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

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
	
	//camera.checkCameraCollision(&scene);

	camera.moveCamera();
	
	//draw3DGrid();
	//drawGameScene();
	
	drawSpheres();

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
			L_MOUSE_DOWN = true;
		}
		break;
		case WM_LBUTTONUP:		
		{
			L_MOUSE_DOWN = false;
		}
		break;
		case WM_RBUTTONDOWN:
			R_MOUSE_DOWN = true;

			g_RenderMode = !g_RenderMode;	// Change the rendering mode

			// Change the rendering mode to and from lines or triangles
			if(g_RenderMode) {
				// Render the triangles in fill mode		
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	
			}
			else {
				// Render the triangles in wire frame mode
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);	
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
// Nothing was added to this file except the checking for the space bar,
// along with the global variables at the top.
// 
// This concludes the third tutorial in the height map series.
//
//
// Ben Humphrey (DigiBen)
// Game Programmer
// DigiBen@GameTutorials.com
// Co-Web Host of www.GameTutorials.com
// © 2000-2003 GameTutorials
//
     
