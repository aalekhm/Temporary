
#include "main.h"

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
//
// Now that you most likely know and understand how to create a terrain from a
// height map and texture it, we move on to the next step.  We get to apply a
// second texture on top of our current texture to give a fake detail and/or almost
// fony bump mapping to it.  This technique is done by blending and tiling a
// second texture over top of our current terrain texture with multitexturing.  Just
// so the detail texture doesn't over power the terrain texture, we increase the
// gamma on the detail texture to produce a nice blend between the two.
// 
// It is assumed that you have looked over the multitexturing and previous height
// map tutorials before attempting to tackle this one.  If you are just interested
// in the detail texturing, you can skip over to Terrain.cpp and look at all the
// code between the * NEW * blocks.
// 
// If for some reason your video card can't support textures over 256 x 256, you
// can easily just shrink the textures to this tutorial down with any paint program.
//
//
// Our function pointers for the ARB multitexturing functions
PFNGLMULTITEXCOORD2FARBPROC			glMultiTexCoord2fARB	= NULL;
PFNGLACTIVETEXTUREARBPROC			glActiveTextureARB		= NULL;

// This controls if we have detail texturing on or off
bool bDetail	= true;

// This handles the current scale for the texture matrix for the detail texture
int iDetailScale = 16;
int TILING_MODE = TILE_MODE_SCALING;
bool spacePressed, tPressed;

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

// This holds the height map data
BYTE gHeightMap[MAP_SIZE*MAP_SIZE];



HDC			mHDC		= NULL;			// Private GDI Device Context
HGLRC		mHRC		= NULL;			// Permanent Rendering Context
HWND		mHWnd		= NULL;			// Holds Our Window Handle
HINSTANCE	mHInstance;					// Holds The Instance Of The Application

bool		mKeys[256];					// Array Used For The Keyboard Routine
bool		mActive = false;			// Window Active Flag
bool		mFULLSCREEN = true;			// Fullscreen Flag Set To TRUE By Default

CVector3 cameraRotation = CVector3(0.0f, 45.0f, 10.0f);
CVector3 cameraPosition = CVector3(1030, 50, 880);//0.5f, 0.7f, -2.0f);//
const float piOver180 = 0.0174532925f;
const float WALK_SPEED = 5.025f;

static bool L_MOUSE_DOWN = false, R_MOUSE_DOWN = false;

int curTexture = 0, maxTextures;
GLuint* textures = {0};
GLuint fontList;
static float FPS;
char debugString[255] = {0};
float camRadius = 0.20f;

int glViewMode = GL_TRIANGLES;
float rotateFaceY = 0.0;
int CURRENT_FRAME_ID = 0;

bool g_RenderMode = false;

const float GRAVITY = 0.98*2;

#define PLAYER_STAND	0
#define PLAYER_WALK		1
#define PLAYER_JUMP		2
#define PLAYER_CROUCH	3
static int PLAYER_STATE = PLAYER_STAND;
static float jumpAngle = 0;
float TRAMP;
bool CROUCH = false;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
GLvoid	resizeGLScene(GLsizei width, GLsizei height);
int initGL(GLvoid);
int drawGLScene(GLvoid);
GLvoid killGLWindow(GLvoid);
bool createGLWindow(char* title, int width, int height, int bits, bool isFullScreen);

void checkMouseMovement() {
	int screenCenterX = SCREEN_WIDTH>>1;
	int screenCenterY = SCREEN_HEIGHT>>1;

	POINT mousePos;
	GetCursorPos(&mousePos);
	
	//Mouse not moved, hence return;
	if(mousePos.x == screenCenterX && mousePos.y == screenCenterY)
		return;

	SetCursorPos(screenCenterX, screenCenterY);
	
	//Move, Rotate as proportinal to the displacement
	float deltaX = (mousePos.x - screenCenterX)/5.0f;
	float deltaY = (mousePos.y - screenCenterY)/5.0f;
	
	cameraRotation.x -= deltaY;
	cameraRotation.y -= deltaX;
}

void checkKeyboardMovement() {
	bool isWalking = false;
	
	float sinTheta = sin( cameraRotation.y * piOver180 );
	float cosTheta = cos( cameraRotation.y * piOver180 );
	float SPEED = (WALK_SPEED + L_MOUSE_DOWN*WALK_SPEED);

	if(mKeys['Q'] || mKeys[VK_LEFT]) {
		cameraRotation.y = ((int)cameraRotation.y + 1) % 360;
	}
	if(mKeys['E'] || mKeys[VK_RIGHT]) {
		cameraRotation.y = ((int)cameraRotation.y - 1) % 360;
	}
	if(mKeys['W'] || L_MOUSE_DOWN) {
		cameraPosition.x -= sinTheta * SPEED;
		cameraPosition.z -= cosTheta * SPEED;
		isWalking = true;
	}
	if(mKeys['S'] || R_MOUSE_DOWN) {
		cameraPosition.x += sinTheta * SPEED;
		cameraPosition.z += cosTheta * SPEED;
		isWalking = true;
	}
	if(mKeys['A']) {
		cameraPosition.x += -cosTheta * SPEED;
		cameraPosition.z +=  sinTheta * SPEED;
		isWalking = true;
	}
	if(mKeys['D']) {
		cameraPosition.x -= -cosTheta * SPEED;
		cameraPosition.z -=  sinTheta * SPEED;
		isWalking = true;
	}
	if(mKeys[VK_UP]) {
		cameraRotation.x = ((int)cameraRotation.x + 1) % 360;
	}
	if(mKeys[VK_DOWN]) {
		cameraRotation.x = ((int)cameraRotation.x - 1) % 360;
	}

	if(isWalking)
		PLAYER_STATE = PLAYER_WALK;
}

void updateCamera(void) {
	checkMouseMovement();
	checkKeyboardMovement();
}

/*PITCH - Rotation around X - axis */
void cameraPitch(float rotation) {
	glRotatef(rotation, 1.0f, 0.0f, 0.0f);
}

/*YAW - Rotation around Y - axis */
void cameraYaw(float rotation) {
	glRotatef(rotation, 0.0f, 1.0f, 0.0f);
}

/*ROLL - Rotation around Z - axis */
void cameraRoll(float rotation) {
	glRotatef(rotation, 0.0f, 0.0f, 1.0f);
}

void cameraMoveTo(CVector3 camPos) {
	glTranslatef(-camPos.x, -camPos.y /*+ CROUCH*0.25f*/, -camPos.z);
}

void moveCamera(void) {
	//Player Camera - Rotation
	cameraPitch(-cameraRotation.x);
	cameraYaw(-cameraRotation.y);
	cameraRoll(-cameraRotation.z);

	//Player Camera - Translation
	cameraMoveTo(cameraPosition);
}

void handleKeyboard(void) {
	if(mKeys[VK_F1]) {
		mKeys[VK_F1] = false;
		killGLWindow();
		mFULLSCREEN = !mFULLSCREEN;
		if(!createGLWindow("openGL window", 640, 480, 16, mFULLSCREEN)) {
			return;
		}
	}
	
	if(mKeys[' ']) {
		if(!spacePressed) {
			spacePressed = true;

			// To get a few different ideas of what a detail texture
			// could look like, we want to change it's scale value.
			
			// Times the current scale value by 2 and loop when it get's to 128
			iDetailScale = (iDetailScale * 2) % 128;

			// If the scale value is 0, set it to 1 again
			if(iDetailScale == 0)
				iDetailScale = 1;


			//JUMP
			//if(TRAMP <= 0)
			//	TRAMP = (GRAVITY)*5;
		}
	}
	else {
		spacePressed = false;
	}
	
	if(mKeys['T']) {
		if(!tPressed) {
			tPressed = true;

			TILING_MODE = TILE_MODE_SCALING - TILING_MODE;
			iDetailScale = 1;
		}
	}
	else {
		tPressed = false;
	}
	
	if(mKeys[17]) {
		CROUCH = true;
	}
	else
		CROUCH = false;
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
					updateCamera();
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

	glViewport(0, 0, width, height);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 2500.0f);

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
	} while( (string[0] == '/') || (string[0] == '\n') );
}

typedef struct tagVertex {
	float x, y, z;
}VERTEX;

typedef struct tagFace {
	int vertexID[4];
	//int vertex1;
	//int vertex2;
	//int vertex3;

	float u;
	float v;

	int WALL_ID;
}FACE;

typedef struct tagScene {
	int maxVertices;
	VERTEX* vertices;

	int maxFaces;
	FACE* faces;
}SCENE;

SCENE scene;

int getWallID(char* WALL_ID) {
	if(strcmp(WALL_ID, "WALL") == 0)
		return TEX_ID_WALL;
	else
	if(strcmp(WALL_ID, "FLOOR") == 0)
		return TEX_ID_FLOOR;
	else
	if(strcmp(WALL_ID, "CEILING") == 0)
		return TEX_ID_CEILING;
	else
	if(strcmp(WALL_ID, "ROOMWALL") == 0)
		return TEX_ID_ROOMWALL;
	else
	if(strcmp(WALL_ID, "ROOMFLOOR") == 0)
		return TEX_ID_ROOMFLOOR;
	else
	if(strcmp(WALL_ID, "ROOMCEILING") == 0)
		return TEX_ID_ROOMCEILING;
	
	return 0;
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
	int v0, v1, v2, v3;
	float u, v;
	char WALL_ID[255];
	readStr(filePtr, singleLine);
	sscanf(singleLine, "MAX_FACES %d\n", &maxFaces);
	scene.maxFaces = maxFaces;
	scene.faces = new FACE[maxFaces];
	for(int i = 0; i < maxFaces; i++) {
		readStr(filePtr, singleLine);
		sscanf(singleLine, "%d %d %d %d %f %f %s\n", &v0, &v1, &v2, &v3, &u, &v, &WALL_ID);
		
		scene.faces[i].vertexID[0] = v0;
		scene.faces[i].vertexID[1] = v1;
		scene.faces[i].vertexID[2] = v2;
		scene.faces[i].vertexID[3] = v3;

		scene.faces[i].u = u;
		scene.faces[i].v = v;
		scene.faces[i].WALL_ID = getWallID(WALL_ID);
		//MessageBox(NULL, WALL_ID, WALL_ID, MB_OK);
	}
}

int initGL(GLvoid) {
	//glEnable(GL_LIGHTING);
	//glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);						// Allow color
	
	///////////////////////////////////
	glEnable(GL_TEXTURE_2D);
	readMap("data/map.txt");

	/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
	// Here we initialize our multitexturing functions
	glActiveTextureARB			= (PFNGLACTIVETEXTUREARBPROC)	wglGetProcAddress("glActiveTextureARB");
	glMultiTexCoord2fARB		= (PFNGLMULTITEXCOORD2FARBPROC)	wglGetProcAddress("glMultiTexCoord2fARB");

	// We should have our multitexturing functions defined and ready to go now, but let's make sure
	// that the current version of OpenGL is installed on the machine.  If the extension functions
	// could not be found, our function pointers will be NULL.
	if(!glActiveTextureARB || !glMultiTexCoord2fARB) {
		
		// Print a error message and quit.
		MessageBox(NULL, "Your current setups does not support multitexturing", "Error", MB_OK);
		PostQuitMessage(0);
	}
	/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
	loadRawHeightMap("data/Terrain.raw", MAP_SIZE * MAP_SIZE, gHeightMap);

	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClearDepth(1.0f);
	//glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	//glCullFace(GL_BACK);								// Quake2 uses front face culling apparently
	//glEnable(GL_CULL_FACE);								// Turn culling on

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

void drawGameScene(void) {
	//glColor3f(0.0, 0.8, 0.0);

	for(int i = 0; i < scene.maxFaces; i++) {
		int vertex0 = scene.faces[i].vertexID[0] - 1;
		int vertex1 = scene.faces[i].vertexID[1] - 1;
		int vertex2 = scene.faces[i].vertexID[2] - 1;
		int vertex3 = scene.faces[i].vertexID[3] - 1;

		float u = scene.faces[i].u;
		float v = scene.faces[i].v;
		
		glBindTexture(GL_TEXTURE_2D, textures[scene.faces[i].WALL_ID]);
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

void drawDebug(void) {
	glPrint(10, SCREEN_HEIGHT - 10, "Use Mouse Right Click to toggle RENDER_MODE");
	glPrint(10, SCREEN_HEIGHT - 20, "Use 'T' to toggle TILING mode");
	glPrint(10, SCREEN_HEIGHT - 30, "Use 'SPACE' increase TILING DETAIL");

	glPrint(10, 20, "iDetail = %d, TILING = %s", iDetailScale, (TILING_MODE==0)?"MANUAL":"SCALING");
	glPrint(10, 10, "CAM : %0.2f %0.2f %0.2f", cameraPosition.x, cameraPosition.y, cameraPosition.z);
}

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
//This checks all the polygons in our list and offsets the camera if collided
void checkCameraCollision(SCENE* scene) {

	// This function is pretty much a direct rip off of SpherePolygonCollision()
	// We needed to tweak it a bit though, to handle the collision detection once 
	// it was found, along with checking every triangle in the list if we collided.  
	// pVertices is the world data. If we have space partitioning, we would pass in 
	// the vertices that were closest to the camera. What happens in this function 
	// is that we go through every triangle in the list and check if the camera's 
	// sphere collided with it.  If it did, we don't stop there.  We can have 
	// multiple collisions so it's important to check them all.  One a collision 
	// is found, we calculate the offset to move the sphere off of the collided plane.
	
	CVector3 LEG_POS = cameraPosition;	//Camera is our eyes
	LEG_POS.y -= 0.15f;					//so go to the Legs, the invisible collision sphere is at the legs

	// Go through all the triangles
	for(int i = 0; i < scene->maxFaces; i++) {
		int vertex0 = scene->faces[i].vertexID[0] - 1;
		int vertex1 = scene->faces[i].vertexID[1] - 1;
		int vertex2 = scene->faces[i].vertexID[2] - 1;
		int vertex3 = scene->faces[i].vertexID[3] - 1;
		
		CVector3 vVertex0 = CVector3(scene->vertices[vertex0].x, scene->vertices[vertex0].y, scene->vertices[vertex0].z);
		CVector3 vVertex1 = CVector3(scene->vertices[vertex1].x, scene->vertices[vertex1].y, scene->vertices[vertex1].z);
		CVector3 vVertex2 = CVector3(scene->vertices[vertex2].x, scene->vertices[vertex2].y, scene->vertices[vertex2].z);
		CVector3 vVertex3 = CVector3(scene->vertices[vertex3].x, scene->vertices[vertex3].y, scene->vertices[vertex3].z);

		// Store of the current triangle we testing
		CVector3 vPolygon[4] = {vVertex3, vVertex2, vVertex1, vVertex0};

		// 1) STEP ONE - Finding the sphere's classification
	
		// We want the normal to the current polygon being checked
		CVector3 vNormal = normal(vPolygon);

		// This will store the distance our sphere is from the plane
		float distance = 0.0f;

		// This is where we determine if the sphere is in FRONT, BEHIND, or INTERSECTS the plane
		int classification = classifySphere(LEG_POS, vNormal, vPolygon[0], camRadius, distance);
//char logText[255] = {0};
//sprintf(debugString, "class = %d %0.2f %0.2f %0.2f ", classification, vNormal.x, vNormal.y, vNormal.z);
//MessageBox(NULL, logText, "logText", MB_OK);

		// If the sphere intersects the polygon's plane, then we need to check further
		if(classification == INTERSECTS) {
			
			// 2) STEP TWO - Finding the psuedo intersection point on the plane

			// Now we want to project the sphere's center onto the triangle's plane
			CVector3 vOffset = vNormal * distance;

			// Once we have the offset to the plane, we just subtract it from the center
			// of the sphere.  "vIntersection" is now a point that lies on the plane of the triangle.
			CVector3 vIntersection = LEG_POS - vOffset;

			// 3) STEP THREE - Check if the intersection point is inside the triangles perimeter

			// We first check if our intersection point is inside the triangle, if not,
			// the algorithm goes to step 4 where we check the sphere again the polygon's edges.

			// We do one thing different in the parameters for EdgeSphereCollision though.
			// Since we have a bulky sphere for our camera, it makes it so that we have to 
			// go an extra distance to pass around a corner. This is because the edges of 
			// the polygons are colliding with our peripheral view (the sides of the sphere).  
			// So it looks likes we should be able to go forward, but we are stuck and considered 
			// to be colliding.  To fix this, we just pass in the radius / 2.  Remember, this
			// is only for the check of the polygon's edges.  It just makes it look a bit more
			// realistic when colliding around corners.  Ideally, if we were using bounding box 
			// collision, cylinder or ellipses, this wouldn't really be a problem.

			if(	insidePolygon(vIntersection, vPolygon, 4) 
				||
				edgeSphereCollision(LEG_POS, vPolygon, 4, camRadius / 2)
			) {

				// If we get here, we have collided!  To handle the collision detection
				// all it takes is to find how far we need to push the sphere back.
				// GetCollisionOffset() returns us that offset according to the normal,
				// radius, and current distance the center of the sphere is from the plane.
				vOffset = getCollisionOffset(vNormal, camRadius, distance);
//sprintf(debugString, "vNormal = %0.2f %0.2f %0.2f ", vNormal.x, vNormal.y, vNormal.z);
				// Now that we have the offset, we want to ADD it to the position and
				// view vector in our camera.  This pushes us back off of the plane.  We
				// don't see this happening because we check collision before we render
				// the scene.
				cameraPosition = cameraPosition + vOffset;

			}
		}
	}
}

void checkCameraCollisionWithTerrain() {
	// Check if the camera is below the height of the terrain at x and z,
	// but we add 10 to make it so the camera isn't on the floor.
	if(cameraPosition.y < getHeight(gHeightMap, (int)cameraPosition.x, (int)cameraPosition.z) + 10) {
		
		// Set the new position of the camera so it's above the terrain + 10
		CVector3 vNewCameraPos = cameraPosition;
		vNewCameraPos.y = getHeight(gHeightMap, (int)cameraPosition.x, (int)cameraPosition.z) + 10;
		
		cameraPosition = vNewCameraPos;
	}
}

int drawGLScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	
	//Constant free fall
	float G = -GRAVITY;
	float Y = G;
	if(TRAMP > 0) {
		TRAMP += -GRAVITY;
		Y = TRAMP;
	}
	else
		TRAMP = 0;
	cameraPosition.y += Y;
	
	checkCameraCollisionWithTerrain();
	//checkCameraCollision(&scene);

	moveCamera();

	drawGameScene();
	
	/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
	renderHeightMap(gHeightMap);
	/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
	
	//Enable texturing mode, which might have been disabled after Multi-Texturing
	glEnable(GL_TEXTURE_2D);
	//sprintf(debugString, "iDetail = %d, TILING = %s", iDetailScale, (TILING_MODE==0)?"MANUAL":"SCALING");
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
     
