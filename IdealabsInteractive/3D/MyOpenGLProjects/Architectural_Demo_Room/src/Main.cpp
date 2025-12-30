
#include "main.h"
#include "3DS.h"

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
//
// This tutorial is built from the "PolygonCollision" tutorial.  Instead of a line
// colliding with a polygon, we now can do better collision that checks to see if
// a sphere collides with a polygon.  The code is modular that it can do triangles
// and quads, or any other convex polygon.  This technique is great for camera  
// and world collision.  Since sphere to sphere collision is too simple, as well as
// sphere to plane collision, we will explain both of them in this tutorial.
// What this application does is allow us to move a wire frame sphere around and have
// it run into a triangle.  The sphere will turn green if it is colliding with
// the triangle, while it will be purple when there is no collision.  
// Here are the commands to move the sphere and camera position:
//
// LEFT ARROW - Moves the sphere left along it's current plane
// RIGHT ARROW - Moves the sphere right along it's current plane
// UP ARROW - Moves the sphere up along it's current plane
// DOWN ARROW - Moves the sphere down along it's current plane
// F3 - Moves the sphere towards the front of the triangle's plane
// F4 - Moves the sphere towards the back of the triangle's plane
// 
// F1 - Rotates the camera left
// F2 - Rotates the camera right
// ESC - Quits the program
//
// 
// Sphere and polygon collision is a lot more complicated that you might first assume.
// If you want to jump immediately to the theory, the concepts are explain in the 
// * QUICK NOTES * section near the bottom of this file or 3DMath.cpp.  The next 
// tutorial will show us how to use this code for our camera class, which will 
// allow us to collide and slide into the walls of a world.
//
//

HDC			mHDC		= NULL;			// Private GDI Device Context
HGLRC		mHRC		= NULL;			// Permanent Rendering Context
HWND		mHWnd		= NULL;			// Holds Our Window Handle
HINSTANCE	mHInstance;					// Holds The Instance Of The Application

//////////////// 3DS Specific ////////////////
#define FILE_NAME_3DS	"data/BigBuilding.3ds"

CReader3DS				reader3DS;
vector<t3DModel>		pGame3DModel;
sceneModelData*			pSceneModels;
t3DModel*				tempModel;

CVector3*			modelPositions;
//////////////// 3DS Specific ////////////////
int MAX_MODELS_TO_LOAD = 0;
int MAX_MODELS_IN_SCENE = 0;

bool		mKeys[256];					// Array Used For The Keyboard Routine
bool		mActive = false;			// Window Active Flag
bool		mFULLSCREEN = true;			// Fullscreen Flag Set To TRUE By Default

CVector3 cameraRotation = CVector3(0.0f, 0.0f, 0.0f);
CVector3 cameraPosition = CVector3(0.5f, 0.7f, -2.0f);
const float piOver180 = 0.0174532925f;
const float WALK_SPEED = 0.025f;

static bool L_MOUSE_DOWN = false, R_MOUSE_DOWN = false;

int curTexture = 0, maxTextures;
GLuint* textures = {0};
GLuint fontList;
static short FONT_TEXTURE_ID;
static float FPS;
char debugString[255] = {0};
float camRadius = 0.20f;

int glViewMode = GL_TRIANGLES;
float rotateFaceY = 0.0;
int CURRENT_FRAME_ID = 0;

bool g_RenderMode = false;

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
	if(mKeys['Q'] || mKeys[VK_LEFT]) {
		cameraRotation.y = ((int)cameraRotation.y + 1) % 360;
	}
	if(mKeys['E'] || mKeys[VK_RIGHT]) {
		cameraRotation.y = ((int)cameraRotation.y - 1) % 360;
	}
	if(mKeys['W'] || L_MOUSE_DOWN) {
		cameraPosition.x -= sin( cameraRotation.y * piOver180 ) * (WALK_SPEED + L_MOUSE_DOWN*WALK_SPEED);
		cameraPosition.z -= cos( cameraRotation.y * piOver180 ) * (WALK_SPEED + L_MOUSE_DOWN*WALK_SPEED);
		isWalking = true;
	}
	if(mKeys['S'] || R_MOUSE_DOWN) {
		cameraPosition.x += sin( cameraRotation.y * piOver180 ) * (WALK_SPEED+ L_MOUSE_DOWN*WALK_SPEED);
		cameraPosition.z += cos( cameraRotation.y * piOver180 ) * (WALK_SPEED+ L_MOUSE_DOWN*WALK_SPEED);
		isWalking = true;
	}
	if(mKeys['A']) {
		cameraPosition.x += sin( (cameraRotation.y-90) * piOver180 ) * (WALK_SPEED+ L_MOUSE_DOWN*WALK_SPEED);
		cameraPosition.z += cos( (cameraRotation.y-90) * piOver180 ) * (WALK_SPEED+ L_MOUSE_DOWN*WALK_SPEED);
		isWalking = true;
	}
	if(mKeys['D']) {
		cameraPosition.x -= sin( (cameraRotation.y-90) * piOver180 ) * (WALK_SPEED+ L_MOUSE_DOWN*WALK_SPEED);
		cameraPosition.z -= cos( (cameraRotation.y-90) * piOver180 ) * (WALK_SPEED+ L_MOUSE_DOWN*WALK_SPEED);
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
	//*
	//Constant free fall
	float G = -(0.98)*0.025;
	float Y = G;
	if(TRAMP > 0) {
		TRAMP += -(0.98)*0.005;
		Y = TRAMP;
	}
	else
		TRAMP = 0;
	cameraPosition.y += Y;
	//*/
	
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
	else
	if(mKeys[' ']) {
		if(TRAMP <= 0)
			TRAMP = (0.98)*0.035*2;
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

	gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 150.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void buildFont(void) {
	fontList = glGenLists(256);
	glBindTexture(GL_TEXTURE_2D, textures[TEX_ID_FONT-1]);
	
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
	glBindTexture(GL_TEXTURE_2D, textures[TEX_ID_FONT-1]);
	
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

void glPrint(float scaleFactor, int x, int y, int z, const char* format, ...) {
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
	glBindTexture(GL_TEXTURE_2D, textures[TEX_ID_FONT-1]);
	
	/*** Enable BLENDING to remove the black from the FONT borders ***/
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	
	glPushMatrix();
		/*** Set font PRINT position ***/
		glTranslatef(x, y, z);
		glScalef(scaleFactor, scaleFactor, scaleFactor);
		/*** Set FONT start & PRINT the entite TEXT ***/
		glListBase(fontList - 32);
		glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
	glPopMatrix();

	/*** Disable BLENDING ***/
	glDisable(GL_BLEND);
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

	float u;
	float v;

	int TEXTURE_ID;
}FACE;

typedef struct tagScene {
	int maxVertices;
	VERTEX* vertices;

	int maxFaces;
	FACE* faces;
}SCENE;

SCENE scene;

void readModels(t3DModel* pModel, char* objectFileName) {
	//////////////// 3DS Specific ////////////////
	// First we need to actually load the .3DS file.  We just pass in an address to
	// our t3DModel structure and the file name string we want to load ("face.3ds").
	reader3DS.import3DS(pModel, objectFileName);
	
	reader3DS.makeModelList(pModel);

	// Depending on how many textures we found, load each one (Assuming .BMP)
	// If you want to load other files than bitmaps, you will need to adjust CreateTexture().
	// Below, we go through all of the materials and check if they have a texture map to load.
	// Otherwise, the material just holds the color information and we don't need to load a texture.
	/*
	// Go through all the materials
	for(int i = 0; i < pModel->numOfMaterials; i++) {

		// Check to see if there is a file name to load in this material
		if(strlen(pModel->pMaterials[i].materialName) > 0) {

			// Use the name of the texture file to load the bitmap, with a texture ID (i).
			// We pass in our global texture array, the name of the texture, and an ID to reference it.	
			//bitmapLoader(pModel.pMaterials[i].textureFileName, textures[TEX_ID_WALL + i]);
			MessageBox(NULL, pModel->pMaterials[i].textureFileName, "Sdasd", MB_OK);
		}

		//Set the MaterialID for this texture
		pModel->pMaterials[i].materialID = i;
	}
	//*/
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
		
		bitmapLoader(textureName, textures[i]);

		//MessageBox(NULL, textureID, textureName, MB_OK);
	}
	buildFont();
	///////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////// LOADING CAMERA
	/*
	camera = CCamera(	CVector3(0.0f, 0.0f, -1.0f),
						CVector3(1.0f, 0.0f,  0.0f),
						CVector3(0.0f, 1.0f,  0.0f)
					);
	
	*/
	//float camPosX, camPosY, camPosZ;
	readStr(filePtr, singleLine);
	sscanf(singleLine, "CAMERA_POSITION %f %f %f %f %f %f\n", &cameraPosition.x, &cameraPosition.y, &cameraPosition.z, &cameraRotation.x, &cameraRotation.y, &cameraRotation.z);
	
	//camera.setPosition(CVector3(camPosX, camPosY, camPosZ));
	///////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////// LOADING 3DS MODELS
	char MODEL_FILE_NAME[255] = "";
	char TEMP_STR[255] = "";
	
	readStr(filePtr, singleLine);
	sscanf(singleLine, "MODELS_TO_LOAD %d\n", &MAX_MODELS_TO_LOAD);
	
	modelPositions = new CVector3[MAX_MODELS_TO_LOAD];
	for(int i = 0; i < MAX_MODELS_TO_LOAD; i++) {
		t3DModel tempModel = {0};

		readStr(filePtr, singleLine);
		sscanf(singleLine, "%s %d %s %d %s %f\n", &TEMP_STR, &tempModel.MODEL_ID, &TEMP_STR, &tempModel.MODEL_TEXTURE_ID, &MODEL_FILE_NAME, &tempModel.reductionFactor);
		
		readModels(&tempModel, MODEL_FILE_NAME);

		pGame3DModel.push_back(tempModel);
	}
	//////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////
	readStr(filePtr, singleLine);
	sscanf(singleLine, "MAX_MODELS_IN_SCENE %d\n", &MAX_MODELS_IN_SCENE);
	
	pSceneModels = new sceneModelData[MAX_MODELS_IN_SCENE];
	for(int i = 0; i < MAX_MODELS_IN_SCENE; i++) {

		readStr(filePtr, singleLine);
		sscanf(singleLine, "%s %d %f %f %f %f %f %f\n",	&TEMP_STR, 
														&pSceneModels[i].MODEL_ID, 
														&pSceneModels[i].scenePositionX, 
														&pSceneModels[i].scenePositionY, 
														&pSceneModels[i].scenePositionZ,
														&pSceneModels[i].rotX, 
														&pSceneModels[i].rotY, 
														&pSceneModels[i].rotZ
													);
	}
	//////////////////////////////////////////////////////////////////////////////////////////

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
	int TEXTURE_ID = 0;
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
		//MessageBox(NULL, WALL_ID, WALL_ID, MB_OK);
	}
}

int initGL(GLvoid) {
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);						// Allow color
	
	///////////////////////////////////
	glEnable(GL_TEXTURE_2D);
	readMap("data/map.txt");

	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClearDepth(1.0f);
	//glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

/*
float fogColor[4] = {0.0, 0.0, 0.0, 1.0f};				// Let's make the Fog Color black too
	glFogi(GL_FOG_MODE, GL_EXP2);						// Set The Fog Mode
	glFogfv(GL_FOG_COLOR, fogColor);					// Set The Fog Color
	glFogf(GL_FOG_DENSITY, 0.045f);						// Set How Dense Will The Fog Be
	glHint(GL_FOG_HINT, GL_DONT_CARE);					// Set The Fog's calculation accuracy
	glFogf(GL_FOG_START, 0);							// Set The Fog's Start Depth
	glFogf(GL_FOG_END, 50.0f);							// Set The Fog's End Depth

	glEnable(GL_FOG);									// Enable fog (turn it on)
*/	
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

void drawDebug(void) {
	glDisable(GL_LIGHTING);
	glColor3f(1.0, 1.0, 1.0);

	//Enable texturing mode, which might have been disabled after Multi-Texturing
	glEnable(GL_TEXTURE_2D);

	glPrint(10, 25, "DEBUG : %s", debugString);
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
	LEG_POS.y -= 0.3f;					//so go to the Legs, the invisible collision sphere is at the legs

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
		CVector3 vPolygon[4] = {vVertex3, vVertex2, vVertex1, vVertex0};//Give in clockwise direction

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

void drawModels() {
	for(int i = 0; i < MAX_MODELS_IN_SCENE; i++) {
		glPushMatrix();
			glTranslatef(pSceneModels[i].scenePositionX, pSceneModels[i].scenePositionY, pSceneModels[i].scenePositionZ);
			glRotatef(pSceneModels[i].rotX, 1.0, 0.0, 0.0);
			glRotatef(pSceneModels[i].rotY, 0.0, 1.0, 0.0);
			glRotatef(pSceneModels[i].rotZ, 0.0, 0.0, 1.0);
			glCallList(pGame3DModel[pSceneModels[i].MODEL_ID].modelList);
		glPopMatrix();
	}
}

void drawVerticesID() {
	glDisable(GL_LIGHTING);

	for(int i = 0; i < scene.maxVertices; i++) {
		glPrint(0.01, scene.vertices[i].x, scene.vertices[i].y, scene.vertices[i].z, "%d", i+1);
		glPrint(0.005, scene.vertices[i].x , scene.vertices[i].y, scene.vertices[i].z, "%0.2f %0.2f %0.2f", scene.vertices[i].x, scene.vertices[i].y, scene.vertices[i].z);
	}

	glEnable(GL_LIGHTING);
}
/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

int drawGLScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	
	checkCameraCollision(&scene);

	moveCamera();

	glDisable(GL_LIGHTING);
		//draw3DGrid();
		drawGameScene();
	glEnable(GL_LIGHTING);
	
	drawModels();
	
//drawVerticesID();

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
//sprintf(debugString, "dddddd %d", wParam);
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
			//SetCursorPos(SCREEN_WIDTH>>1, SCREEN_HEIGHT>>1);
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

			/*
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
			*/
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
// As you can see, using our sphere-polygon collision function is really simple.
// The hard part is actually creating the code underneath it.  To test the collision
// detection, I set up some keys to move the camera and sphere around.  If the
// sphere is colliding with the triangle it will turn green, otherwise it will 
// be purple.
//
// You might be thinking, "So how does this help me games?".  Well, this code
// is perfect for checking if your character (3rd person view) or camera (1st person view)
// collides with the world.  Eventually, you are going to have to get down to checking
// collision with each polygon, this is a perfect technique.  It's faster than any
// other geometry-polygon collision test that I know of.  Some games, like MDK2
// used a cylinder for their character collision.  This is also another great way.
// An ellipse is another geometric shape that is fast.  Though bounding box checks
// can be slower, these are also still excellent choices, especially if you only
// need to check the front 3 faces of the box.  The next tutorial will show us
// how to create a function for our camera class that checks polygons and collides
// and slides along them.  We will have a little textured world to move around.
// I bet your mouth is drooling already.  
//
// Let's take a look at the explanations given in 3DMath.cpp of the theory/math behind
// sphere to polygon collision:
//
// Basically, here is the overview:  
//
// 1) First you want to check if the sphere collides with the triangle's plane.
//    Remember, that planes are infinite and you could be 500 units from the
//    polygon and it's still going to trigger this first test.  We want to 
//    write a function that classifies the sphere.  Either it's completely 
//    in front of the plane (the side the normal is on), intersecting the
//    plane or completely behind the plane.  Got it so far?  We created a 
//    function called ClassifySphere() that returns BEHIND, FRONT or INTERSECTS.
//    If ClassifySphere() returns INTERSECTS, then we move on to step 2, otherwise
//    we did not collide with the triangle.
// 
// 2) The second step is to get an intersection point right in front of the sphere.
//    This one of the tricky parts.  We know that once we have an intersection point
//    on the plane of the triangle, we just need to use the InsidePolygon() function
//    to see if that point is inside the dimensions of the triangle, just like we
//    did with the Ray to Polygon Collision tutorial.  So, how do we get the point
//    of intersection?  It's not as simple as it might sound.  Since a sphere is
//    infinite, there would be a million points that it collided at.  You can just
//    draw a ray in the direction the sphere was moving because it could have just
//    nicked the bottom of the triangle and your ray would find an intersection
//    point that is outside of the triangle.  Well, it turns out that we need to
//    first try and give it a shot.  We will try the first attempt a different way though.
//    We know that we can find the normal vector of the triangle, which in essence
//    tells us the direction that the triangle is facing.  From ClassifyPoly(),
//	  it also returns the distance the center our sphere is from the plane.  That
//    means we have a distance our sphere center is from the plane, and the normal
//    tells us the direction the plane is in.  If we multiply the normal by the
//    distance from the plane we get an offset.  This offset can then be subtracted
//    from the center of the sphere.  Believe it or not, but we now have a position
//    on the plane in the direction of the plane.  Usually, this intersection points
//    works fine, but if we get around the edges of the polygon, this does not work.
//    What we just did is also called "projecting the center of the sphere onto the plane".
//    Another way to do this is to shoot out a ray from the center of the sphere in
//    the opposite direction of the normal, then we find the intersection of that line
//    and the plane.  My way just takes 3 multiplies and a subtraction.  You choose.
//
// 3) Once we have our psuedo intersection point, we just pass it into InsidePolygon(),
//    along with the triangle vertices and the vertex count.  This will then return
//    true if the intersection point was inside of the triangle, otherwise false.
//    Remember, just because this returns false doesn't mean we stop there!  If
//    we didn't collide yet, we need to skip to step 4.
//
// 4) If we get here, it's assumed that we tried our intersection point and it
//    wasn't in the polygon's perimeter.  No fear!  There is hope!  If we get to step
//    4, that's means that our center point is outside of the polygon's perimeter. Since
//    we are dealing with a sphere, we could still be colliding because of the sphere's radius.
// 	  This last check requires us to find the point on each of the polygon's edges that
//    is closest to the sphere's center.  We have a tutorial on finding this, so make sure
//    you have read it or are comfortable with the concept.  If we are dealing with a
//    triangle, we go through every side and get an edge vector, and calculate the closest
//    point on those lines to our sphere's center.  After getting each closest point, we
//    calculate the distance that point is from our sphere center.  If the distance is
//    less than the radius of the sphere, there was a collision.  This way is pretty fast.  
//    You don't need to calculate all three sides evey time, since the first closest point's 
//    distance could be less than the radius and you return "true".
// 
//
// Quite a bit of theory, but not too bad.  Once again, the next tutorial will 
// demonstrate how to hook this up to our camera class and collide and slide against 
// walls in a world. Check it out at www.GameTutorials.com - You won't find any place like it!
//
// I would like to thank Paul Nettle for the collision detection idea.  If you want a great
// tutorial on collision detection, check out http://www.fluidstudios.com/publications.html.
// I thuroughly enjoyed the tutorial on "Generic Collision Detection for Games Using Ellipsoids".
// This provides a great visual tutorial on this topic as well.  I highly recommend it!
//
// Ben Humphrey (DigiBen)
// Game Programmer
// DigiBen@GameTutorials.com
// Co-Web Host of www.GameTutorials.com
// © 2000-2003 GameTutorials
//
