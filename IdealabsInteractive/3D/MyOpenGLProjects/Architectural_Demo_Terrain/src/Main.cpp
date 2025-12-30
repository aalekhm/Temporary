
#include "main.h"

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
//
// In this tutorial we build from our past terrain tutorials to bring you a
// neat effect that can be summed up in 1 word, "sweeeet".  Volumetric fog
// is used in many popular games, include Quake.  This technique allows you to
// specify a bounding volume that only will have fog.  In many game levels you
// will notice that large endless "pits of death" use this effect.  We are going
// to use this to apply to our terrain.  It gives our scene a nice morning look.
// Of course the huge bright sun might destroy that morning look, but you can
// just imagine that it just came out :)
// 
// The thought of creating volumetric fog might at first seem scary and complicated,
// but in fact it's as easy at applying a texture map with glTexCoord2*() functions.
// Since we will be using OpenGL's hardware fog and not our own 2 pass/blend technique,
// we will need to load an extension: "glFogCoordfEXT".  This will be done upon
// initialization of our program, right after the multitexture extensions are loaded.
// Some video cards might not support this feature, like the Voodoo* cards.  If you
// find yourself in this predicament, just use vertex coloring to fake it.
//
// It is assumed that you have read through the "Fog" tutorial, since we will be
// using the standard fog function.  If not, you can still easily follow along, as
// the functions are self explanatory.
//
// To apply volumetric fog, the steps are simple.  First you want to initialize the
// fog like normal, then you will set a new property using glFogi() with the new
// defines listed in main.h, then just call glFogCoordfEXT() with a depth value
// before each vertex that should have fog.  If you pass in a 0 it should ignore
// the fog.  The higher the value the more dense the fog.
//
// Currently, the fog doesn't change when the camera moves.  To make it more realistic
// you might want to try and come up with a ratio that will change the GL_FOG_END value
// depending on the y position of the camera.  If the camera is above or in the fog, you
// would want to change the fog's density.
//
//

// This is our fog extension function pointer to set a vertice's depth
PFNGLFOGCOORDFEXTPROC	glFogCoordfEXT = NULL;

// This stores the desired depth that we want to fog
float g_FogDepth = 0;//50.0f;
/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

// Our function pointers for the ARB multitexturing functions
PFNGLMULTITEXCOORD2FARBPROC			glMultiTexCoord2fARB	= NULL;
PFNGLACTIVETEXTUREARBPROC			glActiveTextureARB		= NULL;

// This controls if we have detail texturing on or off
bool bDetail	= true;

// This handles the current scale for the texture matrix for the detail texture
int iDetailScale = 16;
int TILING_MODE = TILE_MODE_SCALING;
bool spacePressed, tPressed;

// This holds the height map data
BYTE gHeightMap[MAP_SIZE*MAP_SIZE];

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

//////////////// 3DS Specific ////////////////
CReader3DS				reader3DS;
vector<t3DModel>		pGame3DModel;
sceneModelData*			pSceneModels;
t3DModel*				tempModel;

CVector3*			modelPositions;
//////////////// 3DS Specific ////////////////
int MAX_MODELS_TO_LOAD = 0;
int MAX_MODELS_IN_SCENE = 0;

GLuint* textures = {0};
int gLoadedTextureCount = 0;
GLuint fontList;
static float FPS;
char debugString[255] = {0};

int glViewMode = GL_TRIANGLES;
float rotateFaceY = 0.0;

bool g_RenderMode = false;

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

	if(mKeys[VK_ADD]) {							// Check if we hit the + key
		g_FogDepth += 1;						// Increase the fog height
		if(g_FogDepth > 200)					// Make sure we don't go past 200
			g_FogDepth = 200;
	}
	else
	if(mKeys[VK_SUBTRACT]) {					// Check if we hit the + key
		g_FogDepth -= 1;						// Increase the fog height
		if(g_FogDepth < 0)					// Make sure we don't go past 200
			g_FogDepth = 0;
	}
	
	if(mKeys[' ']) {
		if(!mKeysPressed[' ']) {
			mKeysPressed[' '] = true;

			// To get a few different ideas of what a detail texture
			// could look like, we want to change it's scale value.
			
			// Times the current scale value by 2 and loop when it get's to 128
			iDetailScale = (iDetailScale * 2) % 128;

			// If the scale value is 0, set it to 1 again
			if(iDetailScale == 0)
				iDetailScale = 1;
			
			//JUMP
			//if(TRAMP <= 0)
			//	TRAMP = GRAVITY*5;
		}
	}
	else {
		mKeysPressed[' '] = false;
	}
	
	if(mKeys['T']) {
		if(!mKeysPressed['T']) {
			mKeysPressed['T'] = true;

			TILING_MODE = TILE_MODE_SCALING - TILING_MODE;
			iDetailScale = 1;
		}
	}
	else {
		mKeysPressed['T'] = false;
	}

	if(mKeys[17]) {
		CROUCH = true;
	}
	else
		CROUCH = false;
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

	gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.001f, 2500.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

///////////////////////////////// CREATE SKY BOX \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This creates a sky box centered around X Y Z with a width, height and length
/////
///////////////////////////////// CREATE SKY BOX \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void createSkyBox(float x, float y, float z, float width, float height, float length)
{
	// Turn on texture mapping if it's not already
	glEnable(GL_TEXTURE_2D);

	// Bind the BACK texture of the sky map to the BACK side of the cube
	glBindTexture(GL_TEXTURE_2D, textures[TEX_ID_SKYBOX_BACK]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// This centers the sky box around (x, y, z)
	x = x - width  / 2;
	y = y - height / 2;
	z = z - length / 2;

	// Start drawing the side as a QUAD
	glBegin(GL_QUADS);		
		
		// Assign the texture coordinates and vertices for the BACK Side
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y,			z);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x + width, y + height, z); 
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x,			y + height, z);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x,			y,			z);
		
	glEnd();

	// Bind the FRONT texture of the sky map to the FRONT side of the box
	glBindTexture(GL_TEXTURE_2D, textures[TEX_ID_SKYBOX_FRONT]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	// Start drawing the side as a QUAD
	glBegin(GL_QUADS);	
	
		// Assign the texture coordinates and vertices for the FRONT Side
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x,			y,			z + length);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x,			y + height, z + length);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y + height, z + length); 
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y,			z + length);
	glEnd();

	// Bind the BOTTOM texture of the sky map to the BOTTOM side of the box
	glBindTexture(GL_TEXTURE_2D, textures[TEX_ID_SKYBOX_BOTTOM]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Start drawing the side as a QUAD
	glBegin(GL_QUADS);		
	
		// Assign the texture coordinates and vertices for the BOTTOM Side
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x,			y,			z);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x,			y,			z + length);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y,			z + length); 
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y,			z);
	glEnd();

	// Bind the TOP texture of the sky map to the TOP side of the box
	glBindTexture(GL_TEXTURE_2D, textures[TEX_ID_SKYBOX_TOP]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Start drawing the side as a QUAD
	glBegin(GL_QUADS);		
		
		// Assign the texture coordinates and vertices for the TOP Side
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y + height, z);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y + height, z + length); 
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x,			y + height,	z + length);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x,			y + height,	z);
		
	glEnd();

	// Bind the LEFT texture of the sky map to the LEFT side of the box
	glBindTexture(GL_TEXTURE_2D, textures[TEX_ID_SKYBOX_LEFT]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Start drawing the side as a QUAD
	glBegin(GL_QUADS);		
		
		// Assign the texture coordinates and vertices for the LEFT Side
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x,			y + height,	z);	
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x,			y + height,	z + length); 
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x,			y,			z + length);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x,			y,			z);		
		
	glEnd();

	// Bind the RIGHT texture of the sky map to the RIGHT side of the box
	glBindTexture(GL_TEXTURE_2D, textures[TEX_ID_SKYBOX_RIGHT]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Start drawing the side as a QUAD
	glBegin(GL_QUADS);		

		// Assign the texture coordinates and vertices for the RIGHT Side
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y,			z);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y,			z + length);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x + width, y + height,	z + length); 
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y + height,	z);
	glEnd();
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
	
	// Depending on how many textures we found, load each one (Assuming .BMP)
	// If you want to load other files than bitmaps, you will need to adjust CreateTexture().
	// Below, we go through all of the materials and check if they have a texture map to load.
	// Otherwise, the material just holds the color information and we don't need to load a texture.
	///*
	// Go through all the materials
	int count = 0;
	for(int i = 0; i < pModel->numOfMaterials; i++) {
		int matID = -1;

		// Check to see if there is a file name to load in this material
		if(strlen(pModel->pMaterials[i].materialName) > 0) {
			

			// Use the name of the texture file to load the bitmap, with a texture ID (i).
			// We pass in our global texture array, the name of the texture, and an ID to reference it.
			if(strcmp(pModel->pMaterials[i].textureFileName, "") != 0) {
				matID = gLoadedTextureCount;

				bitmapLoader(pModel->pMaterials[i].textureFileName/*, textures[TEX_ID_BLDG_0_0 + count]*/);
				//MessageBox(NULL, pModel->pMaterials[i].textureFileName, "Sdasd", MB_OK);
			}
		}

		//Set the MaterialID for this texture
		pModel->pMaterials[i].materialID = matID;
	}
	//*/

	reader3DS.makeModelList(pModel);
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
		
		bitmapLoader(textureName/*, textures[i]*/);

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

	/////////////////// LOADING 3DS MODELS
	char OBJECT_FILE_NAME[255] = "";
	char TEMP_STR[255] = "";
	
	readStr(filePtr, singleLine);
	sscanf(singleLine, "MODELS_TO_LOAD %d\n", &MAX_MODELS_TO_LOAD);
	
	modelPositions = new CVector3[MAX_MODELS_TO_LOAD];
	for(int i = 0; i < MAX_MODELS_TO_LOAD; i++) {
		t3DModel tempModel = {0};

		readStr(filePtr, singleLine);
		sscanf(singleLine, "%s %d %s %d %s %f\n", &TEMP_STR, &tempModel.MODEL_ID, &TEMP_STR, &tempModel.MODEL_TEXTURE_ID, &OBJECT_FILE_NAME, &tempModel.reductionFactor);
		
		readModels(&tempModel, OBJECT_FILE_NAME);

		pGame3DModel.push_back(tempModel);
	}
	///////////////////////////////////////////////////////////////////////////////////////////////
	
	/////////////////// LOADING MODEL SCENE POSITIONS
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
	///////////////////////////////////////////////////////////////////////////////////////////////

	/*
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
	*/
}

int initGL(GLvoid) {
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);						// Allow color
	
	///////////////////////////////////
	glEnable(GL_TEXTURE_2D);
	readMap("data/map.txt");

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

	// If we have loaded the mulitextures correctly, then we can now test to see
	// if the video card supports hardware accelerated fog.  We do the same things
	// for every extension.  First we tell wglGetProcAddress() which extension
	// we want, which then returns a function pointer.  Afterwards, the function
	// pointer is checked to make sure the current video card drivers or setup
	// support it.

	// Find the correct function pointer that houses the fog coordinate function
	glFogCoordfEXT = (PFNGLFOGCOORDFEXTPROC) wglGetProcAddress("glFogCoordfEXT");
	// Before trying to use this function pointer, we need to make sure it was
	// given a valid address.  If not, then we need to quit because something is wrong.
	if(!glFogCoordfEXT) {
		// Print an error message and quit.
		MessageBox(NULL, "Your current setup does not support volumetric fog", "Error", MB_OK);
		PostQuitMessage(0);
	}

	// It is assumed that by getting here, we should be able to do volumetric fog
	// with this video card.  Now comes the setup and initialization.  Just like
	// when we create normal fog, we need to turn on GL_FOG, give a fog color,
	// as well as give the start and end distance for the thickness of the fog.
	// The new information that will need to be given will be to glFogi().
	// The new flags we defined tell OpenGL that we want per vertex fog.
	// Notice that we don't use GL_FOG_DENSITY.  It doesn't seem to have any effect.

	// Pick a tan color for our fog with a full alpha
	float fogColour[4] = {0.8f, 0.8f, 0.8f, 1.0f};

	glEnable(GL_FOG);						// Turn on fog
	glFogi(GL_FOG_MODE, GL_LINEAR);			// Set the fog mode to LINEAR (Important)
	glFogfv(GL_FOG_COLOR, fogColour);		// Give OpenGL our fog color
	glFogf(GL_FOG_START, 0.0);				// Set the start position for the depth at 0
	glFogf(GL_FOG_END, 50.0);				// Set the end position for the detph at 50

	// Now we tell OpenGL that we are using our fog extension for per vertex
	// fog calculations.  For each vertex that needs fog applied to it we must
	// use the glFogCoordfEXT() function with a depth value passed in.
	// These flags are defined in main.h and are not apart of the normal opengl headers.
	glFogi(GL_FOG_COORDINATE_SOURCE_EXT, GL_FOG_COORDINATE_EXT);
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

void renderViewPoint() {

	glDisable(GL_TEXTURE_2D);
	GLUquadricObj *pObj = gluNewQuadric();			// Get a Quadric off the stack
	gluQuadricDrawStyle(pObj, GLU_LINE);			// Draw the sphere normally
	glColor3ub(255, 255, 0);
	//gluSphere(pObj, 0.002, 15, 15);
	CVector3 fwdPoint = camera.cameraPosition + (camera.viewVector*0.5);
	sprintf(debugString, "%0.2f, %0.2f, %0.2f", fwdPoint.x, fwdPoint.y, fwdPoint.z);

	glBegin(GL_LINES);
		glVertex3f(	fwdPoint.x,
					fwdPoint.y,
					fwdPoint.z
				);
		
		glVertex3f(	fwdPoint.x + camera.viewVector.x*0.125,
					fwdPoint.y + camera.viewVector.y*0.125,
					fwdPoint.z + camera.viewVector.z*0.125
				);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(	fwdPoint.x,
					fwdPoint.y,
					fwdPoint.z
				);
		
		glVertex3f(	fwdPoint.x + camera.upVector.x*0.125,
					fwdPoint.y + camera.upVector.y*0.125,
					fwdPoint.z + camera.upVector.z*0.125
				);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(	fwdPoint.x,
					fwdPoint.y,
					fwdPoint.z
				);
		
		glVertex3f(	fwdPoint.x + camera.rightVector.x*0.125,
					fwdPoint.y + camera.rightVector.y*0.125,
					fwdPoint.z + camera.rightVector.z*0.125
				);
	glEnd();
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

	//Enable texturing mode, which might have been disabled after Multi-Texturing
	glEnable(GL_TEXTURE_2D);

	glColor3f(1.0, 1.0, 1.0);

	glPrint(10, SCREEN_HEIGHT - 10, "Use MOUSE RT CLICK - RENDER_MODE");
	glPrint(10, SCREEN_HEIGHT - 20, "Use 'T' - TILING MODE");
	glPrint(10, SCREEN_HEIGHT - 30, "Use 'SPACE' - TILING DETAIL");
	glPrint(10, SCREEN_HEIGHT - 40, "Use '+' / '-' to INCR/DECR FOG");
sprintf(debugString, "L_MOUSE_DOWN = %d", L_MOUSE_DOWN);
	glPrint(10, 20, "DEBUG : %s", debugString);
	glPrint(10, 10, "iDetail = %d, TILING = %s", iDetailScale, (TILING_MODE==0)?"MANUAL":"SCALING");

	camera.cameraDebug();
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
	
	CVector3 LEG_POS = camera.cameraPosition;	//Camera is our eyes
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
		int classification = classifySphere(LEG_POS, vNormal, vPolygon[0], camera.camRadius, distance);
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
				edgeSphereCollision(LEG_POS, vPolygon, 4, camera.camRadius / 2)
			) {

				// If we get here, we have collided!  To handle the collision detection
				// all it takes is to find how far we need to push the sphere back.
				// GetCollisionOffset() returns us that offset according to the normal,
				// radius, and current distance the center of the sphere is from the plane.
				vOffset = getCollisionOffset(vNormal, camera.camRadius, distance);
//sprintf(debugString, "vNormal = %0.2f %0.2f %0.2f ", vNormal.x, vNormal.y, vNormal.z);
				// Now that we have the offset, we want to ADD it to the position and
				// view vector in our camera.  This pushes us back off of the plane.  We
				// don't see this happening because we check collision before we render
				// the scene.
				camera.cameraPosition = camera.cameraPosition + vOffset;

			}
		}
	}
}

void checkCameraCollisionWithTerrain() {
	// Check if the camera is below the height of the terrain at x and z,
	// but we add 10 to make it so the camera isn't on the floor.
	if(camera.cameraPosition.y < getHeight(gHeightMap, (int)camera.cameraPosition.x, (int)camera.cameraPosition.z) + 10) {
		
		// Set the new position of the camera so it's above the terrain + 10
		CVector3 vNewCameraPos = camera.cameraPosition;
		vNewCameraPos.y = getHeight(gHeightMap, (int)camera.cameraPosition.x, (int)camera.cameraPosition.z) + 10;
		
		camera.cameraPosition = vNewCameraPos;
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

int drawGLScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	
	camera.moveCamera();

	if(camera.CAMERA_HAS_COLLISIONS) {
			checkCameraCollisionWithTerrain();
			//checkCameraCollision(&scene);
	}

	//drawGameScene();
	
	/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
	glDisable(GL_LIGHTING);
	renderHeightMap(gHeightMap);
	sprintf(debugString, "iDetail = %d, TILING = %s", iDetailScale, (TILING_MODE==0)?"MANUAL":"SCALING");
	sprintf(debugString, "g_FogDepth = %0.2f", g_FogDepth);

	// Create the sky box and center it around the terrain
	createSkyBox(500, 0, 500, 2000, 2000, 2000);
	glEnable(GL_LIGHTING);
	/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

	//renderViewPoint();
	drawModels();
	

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
     
