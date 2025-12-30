
#include <windows.h>					// Header File For Windows
#include <mmsystem.h>
#include <gl\gl.h>						// Header File For The OpenGL32 Library
#include <gl\glu.h>						// Header File For The GLu32 Library
#include <stdio.h>
#include <math.h>
#include <conio.h>

#define SCREEN_WIDTH	800
#define SCREEN_HEIGHT	600
#define SCREEN_BPP		32

#define HAS_DEBUG		1

HDC			mHDC		= NULL;			// Private GDI Device Context
HGLRC		mHRC		= NULL;			// Permanent Rendering Context
HWND		mHWnd	= NULL;				// Holds Our Window Handle
HINSTANCE	mHInstance;					// Holds The Instance Of The Application

bool		mKeys[256];					// Array Used For The Keyboard Routine
bool		mActive = false;			// Window Active Flag
bool		mFULLSCREEN = true;			// Fullscreen Flag Set To TRUE By Default

int cameraRotX, cameraRotY, cameraRotZ;
float cameraPosX = 0, cameraPosY, cameraPosZ;
const float piOver180 = 0.0174532925f;
const float MOVE_SPEED = 0.05f;
static bool L_MOUSE_DOWN = false;

bool upPressed;
int curTexture = 0, MAX_TEXTURES;
GLuint* texture;
GLuint fontList;
static short FONT_TEXTURE_ID;
static float FPS;

static float flashLightPos[] = {cameraPosX, cameraPosY, cameraPosZ, 1.0};

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
GLvoid	resizeGLScene(GLsizei width, GLsizei height);
int initGL(GLvoid);
int drawGLScene(GLvoid);
GLvoid killGLWindow(GLvoid);
bool createGLWindow(char* title, int width, int height, int bits, bool isFullScreen);

void checkMouseMovement() {
	if(!L_MOUSE_DOWN)
		return;

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
	
	cameraRotX -= (int)deltaY;
	cameraRotY -= (int)deltaX;
}

void checkKeyboardMovement() {
	if(mKeys['Q'] || mKeys[VK_LEFT]) {
		cameraRotY = (cameraRotY + 1) % 360;
	}
	if(mKeys['E'] || mKeys[VK_RIGHT]) {
		cameraRotY = (cameraRotY - 1) % 360;
	}
	if(mKeys['W']) {
		cameraPosX -= sin( cameraRotY * piOver180 ) * MOVE_SPEED;
		cameraPosZ -= cos( cameraRotY * piOver180 ) * MOVE_SPEED;
	}
	if(mKeys['S']) {
		cameraPosX += sin( cameraRotY * piOver180 ) * MOVE_SPEED;
		cameraPosZ += cos( cameraRotY * piOver180 ) * MOVE_SPEED;
	}
	if(mKeys['A']) {
		cameraPosX += sin( (cameraRotY-90) * piOver180 ) * MOVE_SPEED;
		cameraPosZ += cos( (cameraRotY-90) * piOver180 ) * MOVE_SPEED;
	}
	if(mKeys['D']) {
		cameraPosX -= sin( (cameraRotY-90) * piOver180 ) * MOVE_SPEED;
		cameraPosZ -= cos( (cameraRotY-90) * piOver180 ) * MOVE_SPEED;
	}
	if(mKeys[VK_UP]) {
		cameraRotX = (cameraRotX + 1) % 360;
	}
	if(mKeys[VK_DOWN]) {
		cameraRotX = (cameraRotX - 1) % 360;
	}
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

void cameraMoveTo(float xTranslate, float yTranslate, float zTranslate) {
	glTranslatef(xTranslate, yTranslate, zTranslate);
}

void moveCamera(void) {
	//Player Camera - Rotation
	cameraPitch(-cameraRotX);
	cameraYaw(-cameraRotY);
	cameraRoll(-cameraRotZ);

	//Player Camera - Translation
	cameraMoveTo(-cameraPosX, -cameraPosY, -cameraPosZ);
}

void handleKeyboard(void) {
	if(mKeys['Z'] && !upPressed) {
		upPressed = true;
		
		curTexture++;
		if(curTexture >= MAX_TEXTURES)
			curTexture = 0;

		//char sCurText[255];
		//sprintf(sCurText, "%d", curTexture);
		//MessageBox(NULL, sCurText, "curTexture", MB_OK);
	}

	if(!mKeys['Z']) {
		upPressed = false;
	}

	if(mKeys[VK_F1]) {
		mKeys[VK_F1] = false;
		killGLWindow();
		mFULLSCREEN = !mFULLSCREEN;
		if(!createGLWindow("openGL window", SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, mFULLSCREEN)) {
			return;
		}
	}
}

bool bitmapLoader(LPCSTR fileName, GLuint &textureID) {
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
sprintf(sCurText, "%d x %d", BMP.bmWidth, BMP.bmHeight);
MessageBox(NULL, sCurText, "W x H", MB_OK);
*/
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

	if(!createGLWindow("openGL Window...Press F1 to toogle between windowed & Fullscreen Mode.", SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, mFULLSCREEN))
		return 0;
	
	DWORD lastTime = timeGetTime();

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
			float timeDiff = static_cast<float>(timeGetTime() - lastTime);
			lastTime = timeGetTime();
			if(timeDiff == 0) timeDiff = 0.1;
			FPS = 1000.0/timeDiff;

			if(mActive) {
				if(mKeys[VK_ESCAPE])
					done = true;
				else {
					drawGLScene();
					SwapBuffers(mHDC);
				}
			}

			updateCamera();
			handleKeyboard();
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

	if(mFULLSCREEN) {
		dwExStyle	= WS_EX_APPWINDOW;
		dwStyle		= WS_POPUP;
		ShowCursor(false);
	}
	else {
		dwExStyle	= WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle		= WS_POPUP;//WS_OVERLAPPEDWINDOW;

		ShowCursor(false);
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

	gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 100.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void buildFont(void) {
	fontList = glGenLists(256);
	glBindTexture(GL_TEXTURE_2D, texture[FONT_TEXTURE_ID]);
	
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
				glVertex2i(16, 0);

				glTexCoord2f(cx + ONE_CHAR_WIDTH, 1 - cy);
				glVertex2i(16, 16);

				glTexCoord2f(cx, 1 - cy);
				glVertex2i(0, 16);
			glEnd();
			
			//Later Translate it to the actual character width rather than a fixed 16 for all...
			glTranslated(16, 0, 0);
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
	glBindTexture(GL_TEXTURE_2D, texture[FONT_TEXTURE_ID]);
	
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
		return 0;
	else
	if(strcmp(WALL_ID, "FLOOR") == 0)
		return 1;
	else
	if(strcmp(WALL_ID, "CEILING") == 0)
		return 2;
	else
	if(strcmp(WALL_ID, "ROOMWALL") == 0)
		return 3;
	else
	if(strcmp(WALL_ID, "ROOMFLOOR") == 0)
		return 4;
	else
	if(strcmp(WALL_ID, "ROOMCEILING") == 0)
		return 5;
	
	return 0;
}

void readMap(char* filePathAndName) {
	char singleLine[255] = "";
	FILE* filePtr = fopen(filePathAndName, "rt");
	
	int maxTextures = 0;
	readStr(filePtr, singleLine);
	sscanf(singleLine, "MAX_TEXTURES %d\n", &MAX_TEXTURES);
	
	texture = new GLuint[MAX_TEXTURES];
	glGenTextures(MAX_TEXTURES, &texture[0]);

	char textureID[255] = "";
	char textureName[255] = "";
	for(int i = 0; i < MAX_TEXTURES; i++) {
		readStr(filePtr, singleLine);
		sscanf(singleLine, "%s %s\n", &textureID, &textureName);
		
		bitmapLoader(textureName, texture[i]);

		if(strcmp(textureID, "FONT") == 0) {
			FONT_TEXTURE_ID = i;
			buildFont();
		}
		//MessageBox(NULL, textureID, textureName, MB_OK);
	}

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
	
	
	///////////////////////////////////
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_TEXTURE_2D);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	readMap("data/map.txt");
///*
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT1);
	
	
	float flashLightColour[] = { 1.5f, 0.2f, 0.2f, 1.0f };
	float flashLightDirection[] = {0.0, 0.0, -1.0};
	glLightfv(GL_LIGHT1, GL_POSITION, flashLightPos);
	glLightfv(GL_LIGHT1, GL_AMBIENT, flashLightColour);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, flashLightColour);
	glLightfv(GL_LIGHT1, GL_SPECULAR, flashLightColour);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, flashLightDirection);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 12.0f);
	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 100.0f);
//*/



	


	return true;
}

GLvoid killGLWindow() {
	if(mFULLSCREEN) {
		ChangeDisplaySettings(NULL, 0);
		//ShowCursor(true);
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
		
		glBindTexture(GL_TEXTURE_2D, texture[scene.faces[i].WALL_ID]);
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

void drawCube(void) {
/*
	glBegin(GL_QUADS);									// Draw A Quad
		//front
		glColor3f(1.0f,0.0f,0.0f);
		glVertex3f( 1.0f, 1.0f, 1.0f);					// Top Left
		glColor3f(0.0f,1.0f,0.0f);
		glVertex3f(-1.0f, 1.0f, 1.0f);					// Top Right
		glColor3f(0.0f,0.0f,1.0f);
		glVertex3f(-1.0f,-1.0f, 1.0f);					// Bottom Right
		glColor3f(1.0f,0.0f,1.0f);
		glVertex3f( 1.0f,-1.0f, 1.0f);					// Bottom Left
		//back
		glColor3f(1.0f,1.0f,0.0f);
		glVertex3f( 1.0f, 1.0f,-1.0f);					// Top Left
		glColor3f(0.0f,1.0f,1.0f);
		glVertex3f(-1.0f, 1.0f,-1.0f);					// Top Right
		glColor3f(0.5f,0.0f,0.0f);
		glVertex3f(-1.0f,-1.0f,-1.0f);					// Bottom Right
		glColor3f(0.0f,0.5f,0.0f);
		glVertex3f( 1.0f,-1.0f,-1.0f);					// Bottom Left

		//top
		glColor3f(1.0f,1.0f,0.0f);
		glVertex3f( 1.0f, 1.0f,-1.0f);					// Top Left
		glColor3f(0.0f,1.0f,1.0f);
		glVertex3f(-1.0f, 1.0f,-1.0f);					// Top Right
		glColor3f(0.0f,1.0f,0.0f);
		glVertex3f(-1.0f, 1.0f, 1.0f);					// Bottom Right
		glColor3f(1.0f,0.0f,0.0f);
		glVertex3f( 1.0f, 1.0f, 1.0f);					// Bottom Left
		//bottom
		glColor3f(0.0f,0.5f,0.0f);
		glVertex3f( 1.0f,-1.0f,-1.0f);					// Top Left
		glColor3f(0.5f,0.0f,0.0f);
		glVertex3f(-1.0f,-1.0f,-1.0f);					// Top Right
		glColor3f(0.0f,0.0f,1.0f);
		glVertex3f(-1.0f,-1.0f, 1.0f);					// Bottom Right
		glColor3f(1.0f,0.0f,1.0f);
		glVertex3f( 1.0f,-1.0f, 1.0f);					// Bottom Left

		//left face
		glColor3f(0.0f,1.0f,0.0f);
		glVertex3f(-1.0f, 1.0f, 1.0f);					// Top Left
		glColor3f(0.0f,1.0f,1.0f);
		glVertex3f(-1.0f, 1.0f,-1.0f);					// Top Right
		glColor3f(0.5f,0.0f,0.0f);
		glVertex3f(-1.0f,-1.0f,-1.0f);					// Bottom Right
		glColor3f(0.0f,0.0f,1.0f);
		glVertex3f(-1.0f,-1.0f, 1.0f);					// Bottom Left
		//right face
		glColor3f(1.0f,1.0f,0.0f);
		glVertex3f( 1.0f, 1.0f,-1.0f);					// Top Left
		glColor3f(1.0f,0.0f,0.0f);
		glVertex3f( 1.0f, 1.0f, 1.0f);					// Top Right
		glColor3f(1.0f,0.0f,1.0f);
		glVertex3f( 1.0f,-1.0f, 1.0f);					// Bottom Right
		glColor3f(0.0f,0.5f,0.0f);
		glVertex3f( 1.0f,-1.0f,-1.0f);					// Bottom Left

	glEnd();											// Done Drawing The Quad
*/
}

void drawDebug(void) {
	glPrint(10, 10, "FPS : %0.2f", FPS);
}

typedef struct vector_s {
	float	x,
			y,
			z;
} vector_t;

typedef struct vertex_s {
	float	x,
			y,
			z;
} vertex_t;

// misc. gl functions:
void normalize (vector_t *v)
{
    // calculate the length of the vector
    float len = (float)(sqrt((v->x * v->x) + (v->y * v->y) + (v->z * v->z)));

    // avoid division by 0
    if (len == 0.0f)
        len = 1.0f;

    // reduce to unit size
    v->x /= len;
    v->y /= len;
    v->z /= len;
}

// normal(); - finds a normal vector and normalizes it
void normal (vertex_t v[3], vector_t *normal)
{
    vector_t a, b;

    // calculate the vectors A and B
    // note that v[3] is defined with counterclockwise winding in mind
    // a
    a.x = v[0].x - v[1].x;
    a.y = v[0].y - v[1].y;
    a.z = v[0].z - v[1].z;
    // b
    b.x = v[1].x - v[2].x;
    b.y = v[1].y - v[2].y;
    b.z = v[1].z - v[2].z;

    // calculate the cross product and place the resulting vector
    // into the address specified by vector_t *normal
    normal->x = (a.y * b.z) - (a.z * b.y);
    normal->y = (a.z * b.x) - (a.x * b.z);
    normal->z = (a.x * b.y) - (a.y * b.x);

    // normalize
    normalize(normal);
}

int drawGLScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	
	//gluLookAt(0.0, 0.0, 20.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	moveCamera();

	glTranslatef(0.0, -0.5, -1.0f);

	flashLightPos[0] = cameraPosX;
	flashLightPos[1] = cameraPosY;
	flashLightPos[2] = cameraPosZ;

	glLightfv(GL_LIGHT1, GL_POSITION, flashLightPos);
	/*
	//glColor3f(1.0f,0.0f,0.0f);
	GLfloat cubeColor[] = { 0.5f, 0.2f, 0.2f, 1.0f };
	GLfloat cubeSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glEnable(GL_COLOR_MATERIAL);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, cubeColor);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, cubeSpecular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 10.0f);
	
	
	vector_t vecNormal = { 0.0f, 0.0f, 0.0f };
	
	glBegin(GL_QUADS);
		vertex_t V[4];
		V[0].x = -3.0; V[0].y = -3.0; V[0].z = -1.0;
		V[1].x =  3.0; V[1].y = -3.0; V[1].z = -1.0;
		V[2].x =  3.0; V[2].y =  3.0; V[2].z = -1.0;
		V[3].x = -3.0; V[3].y =  3.0; V[3].z = -1.0;

		normal(V, &vecNormal);
		glNormal3f(V[0].x + vecNormal.x, V[0].y + vecNormal.y, V[0].z + vecNormal.z);

		glVertex3f(V[0].x, V[0].y, V[0].z);
		glVertex3f(V[1].x, V[1].y, V[1].z);
		glVertex3f(V[2].x, V[2].y, V[2].z);
		glVertex3f(V[3].x, V[3].y, V[3].z);
	glEnd();
	*/
	///*
	//glDisable(GL_LIGHTING);
	//glColor3f(1.0f,0.0f,0.0f);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	//glEnable(GL_NORMALIZE);
	glBegin(GL_QUADS);									// Draw A Quad
		//front
		//glNormal3f( 0.0f, 0.0f,-1.0f);
		glTexCoord2f(0.0, 0.5); glVertex3f(-0.5f, 0.0f,-1.0f);					// Top Left
		glTexCoord2f(0.5, 0.5); glVertex3f( 0.0f, 0.0f,-1.0f);					// Top Right
		glTexCoord2f(0.5, 1.0); glVertex3f( 0.0f, 0.5f,-1.0f);					// Bottom Right		
		glTexCoord2f(0.0, 1.0); glVertex3f(-0.5f, 0.5f,-1.0f);					// Bottom Left
	
		//glNormal3f( 0.0f, 0.0f,-1.0f);
		glTexCoord2f(0.5, 0.5); glVertex3f( 0.0f, 0.0f,-1.0f);					// Top Left
		glTexCoord2f(1.0, 0.5); glVertex3f( 0.5f, 0.0f,-1.0f);					// Top Right
		glTexCoord2f(1.0, 1.0); glVertex3f( 0.5f, 0.5f,-1.0f);					// Bottom Right
		glTexCoord2f(0.5, 1.0); glVertex3f( 0.0f, 0.5f,-1.0f);					// Bottom Left
		
		//glNormal3f( 0.0f, 0.0f,-1.0f);
		glTexCoord2f(0.0, 0.5); glVertex3f(-0.5f, 0.0f,-1.0f);					// Top Left
		glTexCoord2f(0.0, 0.0); glVertex3f(-0.5f,-0.5f,-1.0f);					// Top Right
		glTexCoord2f(0.5, 0.0); glVertex3f( 0.0f,-0.5f,-1.0f);					// Bottom Right
		glTexCoord2f(0.5, 0.5); glVertex3f( 0.0f, 0.0f,-1.0f);					// Bottom Left
		
		//glNormal3f(0.0, 0.0,-1.0);
		glTexCoord2f(0.5, 0.5); glVertex3f( 0.0f, 0.0f, -1.0f);					// Top Left
		glTexCoord2f(0.5, 0.0); glVertex3f( 0.0f,-0.5f, -1.0f);					// Top Right
		glTexCoord2f(1.0, 0.0); glVertex3f( 0.5f,-0.5f, -1.0f);					// Bottom Right
		glTexCoord2f(1.0, 0.5); glVertex3f( 0.5f, 0.0f, -1.0f);					// Bottom Left
	glEnd();
	/*
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glBegin(GL_QUADS);
		glVertex3f(-0.5, -0.5, -1.0);
		glVertex3f( 0.5, -0.5, -1.0);
		glVertex3f( 0.5,  0.5, -1.0);
		glVertex3f(-0.5,  0.5, -1.0);
	glEnd();
	*/
	//*/
	drawGameScene();
	//glEnable(GL_LIGHTING);
	

	drawCube();
	
	#if HAS_DEBUG == 1
		//drawDebug();
	#endif

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
			SetCursorPos(SCREEN_WIDTH>>1, SCREEN_HEIGHT>>1);
			L_MOUSE_DOWN = true;
		}
		break;
		case WM_LBUTTONUP:		
		{
			L_MOUSE_DOWN = false;

			//MessageBox(NULL, "AsdaS", "asdA", MB_OK);
		}
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}