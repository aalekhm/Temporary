
#include <windows.h>					// Header File For Windows
#include <gl\gl.h>						// Header File For The OpenGL32 Library
#include <gl\glu.h>						// Header File For The GLu32 Library
#include <stdio.h>
#include <math.h>
#include <conio.h>

#include "main.h"
#include "3DS.h"
#include "MD2.h"

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define SCREEN_BPP		16

#define MAX_TEXTURES	100

HDC			mHDC		= NULL;			// Private GDI Device Context
HGLRC		mHRC		= NULL;			// Permanent Rendering Context
HWND		mHWnd	= NULL;				// Holds Our Window Handle
HINSTANCE	mHInstance;					// Holds The Instance Of The Application
char strWindowTitle[255] = {0};

bool		mKeys[256];					// Array Used For The Keyboard Routine
bool		mActive = false;			// Window Active Flag
bool		mFULLSCREEN = true;			// Fullscreen Flag Set To TRUE By Default

int cameraRotX, cameraRotY, cameraRotZ;
float cameraPosX, cameraPosY, cameraPosZ;
const float piOver180 = 0.0174532925f;
const float MOVE_SPEED = 0.05f;
static bool L_MOUSE_DOWN = false;

bool upPressed;
int loadedTextureCount = 0;
GLuint* texture;

//3DS related
#define FILE_NAME_3DS		"data/face.3ds"
#define FILE_NAME_MD2		"data/tris.md2"
#define TEXTURE_NAME_3DS	"data/face.bmp"
#define TEXTURE_NAME_MD2	"data/hobgoblin.bmp"

CReader3DS  reader3DS;
CReaderMD2  readerMD2;
t3DModel	pGame3DModel;
int glViewMode = GL_TRIANGLES;
float rotateFaceY = 0.0;
int CURRENT_FRAME_ID = 0;

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
	
	cameraRotX -= deltaY;
	cameraRotY -= deltaX;
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
	if(mKeys[VK_F1]) {
		mKeys[VK_F1] = false;
		killGLWindow();
		mFULLSCREEN = !mFULLSCREEN;
		if(!createGLWindow("openGL window", 640, 480, 16, mFULLSCREEN)) {
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

void readStr(FILE* f, char* string) {
	do {
		fgets(string, 255, f);
	} while( (string[0] == '/') || (string[0] == '\n') );
}

typedef struct tagVertex {
	float x, y, z;
	float u, v;
}VERTEX;

typedef struct tagFace {
	int vertex0;
	int vertex1;
	int vertex2;
	int vertex3;

	int u;
	int v;

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
	sscanf(singleLine, "MAX_TEXTURES %d\n", &maxTextures);
	
	char textureID[255] = "";
	char textureName[255] = "";
	for(int i = 0; i < maxTextures; i++, loadedTextureCount++) {
		readStr(filePtr, singleLine);
		sscanf(singleLine, "%s %s\n", &textureID, &textureName);
		
		bitmapLoader(textureName, texture[i]);
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
		
		scene.faces[i].vertex0 = v0;
		scene.faces[i].vertex1 = v1;
		scene.faces[i].vertex2 = v2;
		scene.faces[i].vertex3 = v3;

		scene.faces[i].u = u;
		scene.faces[i].v = v;
		scene.faces[i].WALL_ID = getWallID(WALL_ID);
		//MessageBox(NULL, WALL_ID, WALL_ID, MB_OK);
	}
}

int initGL(GLvoid) {
	texture = new GLuint[MAX_TEXTURES];
	glGenTextures(MAX_TEXTURES, &texture[0]);

	// First we need to actually load the .3DS file.  We just pass in an address to
	// our t3DModel structure and the file name string we want to load ("face.3ds").
	//reader3DS.import3DS(&pGame3DModel, FILE_NAME_3DS);

	readerMD2.importMD2(&pGame3DModel, FILE_NAME_MD2);
	bitmapLoader(TEXTURE_NAME_MD2, texture[0]);

	// Depending on how many textures we found, load each one (Assuming .BMP)
	// If you want to load other files than bitmaps, you will need to adjust CreateTexture().
	// Below, we go through all of the materials and check if they have a texture map to load.
	// Otherwise, the material just holds the color information and we don't need to load a texture.
	
	/*
	// Go through all the materials
	for(int i = 0; i < pGame3DModel.numOfMaterials; i++) {

		// Check to see if there is a file name to load in this material
		if(strlen(pGame3DModel.pMaterials[i].materialName) > 0) {

			// Use the name of the texture file to load the bitmap, with a texture ID (i).
			// We pass in our global texture array, the name of the texture, and an ID to reference it.	
			bitmapLoader(pGame3DModel.pMaterials[i].textureFileName, texture[i]);
		}

		//Set the MaterialID for this texture
		pGame3DModel.pMaterials[i].materialID = i;
	}
	*/

	//readMap("data/map.txt");
	
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);						// Allow color
	
	///////////////////////////////////
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glEnable(GL_CULL_FACE);							// Turn culling on
	glCullFace(GL_FRONT);							// Quake2 uses front face culling apparently

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

void drawGameScene() {
	for(int i = 0; i < scene.maxFaces; i++) {
		int vertex0 = scene.faces[i].vertex0 - 1;
		int vertex1 = scene.faces[i].vertex1 - 1;
		int vertex2 = scene.faces[i].vertex2 - 1;
		int vertex3 = scene.faces[i].vertex3 - 1;
		int u = scene.faces[i].u;
		int v = scene.faces[i].v;

		
		glBindTexture(GL_TEXTURE_2D, texture[scene.faces[i].WALL_ID]);
		glBegin(GL_QUADS);
			glTexCoord2i(0, 0);
			glVertex3f(	scene.vertices[vertex0].x,
						scene.vertices[vertex0].y,
						scene.vertices[vertex0].z
					);
			
			glTexCoord2i(u, 0);
			glVertex3f(	scene.vertices[vertex1].x,
						scene.vertices[vertex1].y,
						scene.vertices[vertex1].z
					);
			
			glTexCoord2i(u, v);
			glVertex3f(	scene.vertices[vertex2].x,
						scene.vertices[vertex2].y,
						scene.vertices[vertex2].z
					);
			
			glTexCoord2i(0, v);
			glVertex3f(	scene.vertices[vertex3].x,
						scene.vertices[vertex3].y,
						scene.vertices[vertex3].z
					);
		glEnd();
	}
}

void draw3DSScene() {
	glRotatef(rotateFaceY, 0.0, 1.0, 0.0);
	rotateFaceY += 0.5;

	// I am going to attempt to explain what is going on below up here as not to clutter the 
	// code below.  We have a model that has a certain amount of objects and textures.  We want 
	// to go through each object in the model, bind it's texture map to it, then render it.
	// To render the current object, we go through all of it's faces (Polygons).  
	// What is a face you ask?  A face is just (in this case) a triangle of the object.
	// For instance, a cube has 12 faces because each side has 2 triangles.
	// You might be thinking.  Well, if there are 12 faces in a cube, that makes
	// 36 vertices that we needed to read in for that object.  Not really true.  Because
	// a lot of the vertices are the same, since they share sides, they only need to save
	// 8 vertices, and ignore the duplicates.  Then, you have an array of all the
	// unique vertices in that object.  No 2 vertices will be the same.  This cuts down
	// on memory.  Then, another array is saved, which is the index numbers for each face,
	// which index in to that array of vertices.  That might sound silly, but it is better
	// than saving tons of duplicate vertices.  The same thing happens for UV coordinates.
	// You don't save duplicate UV coordinates, you just save the unique ones, then an array
	// that index's into them.  This might be confusing, but most 3D files use this format.
	// This loop below will stay the same for most file formats that you load, so all you need
	// to change is the loading code.  You don't need to change this loop (Except for animation).

	// Since we know how many models our MODEL/SCENE has, go through each of them.
	for(int i = 0; i < pGame3DModel.numOfObjects; i++) {

		//Make sure we have valid Objects just in case.
		if(pGame3DModel.pObjects.size() <= 0)
			return;

		//Get the current Object that we are displaying.
		t3DObject *pObject = &pGame3DModel.pObjects[i];

		//Check to see if the Object has a texture map, if so bind the texture to it.
		if(pObject->bHasTexture) {

			//Turn On Texture Mapping & Turn Off Colour...
			glEnable(GL_TEXTURE_2D);

			//Reset the colour to normal again,
			//IMP, need to set it to WHITE, 
			//else if it turns BLACK(you wont see anything on the screen, if ur background is set to BLACK too)
			//watch out for 'ub' & 'b', sometimes one forgets to set it by mistake n the colour u get is BLACK
			glColor3ub(255, 255, 255);

			//Bind the texture map to the Object by its materialID
			glBindTexture(GL_TEXTURE_2D, texture[pObject->materialID]);
		}
		else {
			//Turn Off Texture Mapping & Turn On Colour...
			glDisable(GL_TEXTURE_2D);

			//Reset the colour to normal again,
			//IMP, need to set it to WHITE, 
			//else if it turns black(you wont see anything on the screen, if ur background is set to black too)
			//watch out for 'ub' & 'b', sometimes one forgets to set it by mistake n the colour u get is BLACK
			glColor3ub(255, 255, 255);
		}

		//This determines if we are in WireFrame mode or Normal mode
		//Begin drawing with our selected mode (triangles or lines)
		glBegin(glViewMode);

			//Go through all of the faces (polygons) of the Object and draw them
			for(int j = 0; j < pObject->numOfFaces; j++) {
				
				//Go throught each corner(Vertex) of the Triangle and draw it.
				for(int whichVertex = 0; whichVertex < 3; whichVertex++) {
					
					//Get the index for each point of the Face
					int index = pObject->pFaces[j].vertexIndex[whichVertex];

					// Give OpenGL the normal for this vertex.
					//glNormal3f(pObject->pNormals[ index ].x, pObject->pNormals[ index ].y, pObject->pNormals[ index ].z);

					//If the Object has texture associated with it, give it a texture coordinate.
					if(pObject->bHasTexture) {

						//Make sure there are UVM map applied to the Object, or else it wont have tex coordinates
						if(pObject->pTexVertices) {
							glTexCoord2f(pObject->pTexVertices[index].x, pObject->pTexVertices[index].y);
						}
					}
					else {
						// Make sure there is a valid material/color assigned to this object.
						// You should always at least assign a material color to an object, 
						// but just in case we want to check the size of the material list.
						// if the size is at least one, and the material ID != -1,
						// then we have a valid material.
							
						if(pGame3DModel.pMaterials.size() && pObject->materialID >= 0) {
							
							//Get and set the colour that the Object is, since it must have a texture.
							BYTE *pColour = pGame3DModel.pMaterials[pObject->materialID].colour;

							//Assign the current Colour to this Object
							glColor3ub(pColour[0], pColour[1], pColour[2]);
						}
					}
					
					//Pass in the current Vertex of the Object (Corner of the current Face).
					glVertex3f(	pObject->pVertices[index].x, 
								pObject->pVertices[index].y,
								pObject->pVertices[index].z
							);
				}
			}
		glEnd();
	}
}

//This returns time t for the interpolation between the current and next key frame
float returnCurrentTime(t3DModel *pModel, int nextFrame) {
	static float elapsedTime   = 0.0f;
	static float lastTime	  = 0.0f;

	// This function is very similar to finding the frames per second.
	// Instead of checking when we reach a second, we check if we reach
	// 1 second / our animation speed. (1000 ms / kAnimationSpeed).
	// That's how we know when we need to switch to the next key frame.
	// In the process, we get the t value for how we are at to going to the
	// next animation key frame.  We use time to do the interpolation, that way
	// it runs the same speed on any persons computer, regardless of their specs.
	// It might look chopier on a junky computer, but the key frames still be
	// changing the same time as the other persons, it will just be not as smooth
	// of a transition between each frame.  The more frames per second we get, the
	// smoother the animation will be.

	// Get the current time in milliseconds
	float time = (float)GetTickCount();

	// Find the time that has elapsed since the last time that was stored
	elapsedTime = time - lastTime;

	// To find the current t we divide the elapsed time by the ratio of 1 second / our anim speed.
	// Since we aren't using 1 second as our t = 1, we need to divide the speed by 1000
	// milliseconds to get our new ratio, which is a 5th of a second.
	float t = elapsedTime / (1000.0f / kAnimationSpeed);
	
	// If our elapsed time goes over a 5th of a second, we start over and go to the next key frame
	if (elapsedTime >= (1000.0f / kAnimationSpeed) )
	{
		// Set our current frame to the next key frame (which could be the start of the anim)
		pModel->currentFrame = nextFrame;

		// Set our last time to the current time just like we would when getting our FPS.
		lastTime = time;
	}

	// Return the time t so we can plug this into our interpolation.
	return t;
}


void animateMD2(t3DModel* pModel) {
	//Make sure we have valid Objects, just in case. (size() is in the vector class)
	if(pModel->pObjects.size() <= 0)
		return;

	//Here we grab the current Animation that we are on from our Model's Animation List
	tAnimation *pAnim = &pModel->pAnimations[pModel->currentAnim];

	//This gives the current Frame we are on. We mod the current frame plus
	// 1 by the current animations end frame to make sure the next frame is valid.
	// If the next frame is past our end frame, then we go back to zero.  We check this next.
	int nextFrame = (pModel->currentFrame + 1) % pAnim->endFrame;

	// If the next frame is zero, that means that we need to start the animation over.
	// To do this, we set nextFrame to the starting frame of this animation.
	if(nextFrame == 0)
		nextFrame = pAnim->startFrame;

	//Get the Current Key Frame Object we are on
	t3DObject *pCurFrameObject = &pModel->pObjects[pModel->currentFrame];

	//Get the Next Key Frame Object we are interpolating to
	t3DObject* pNextFrameObject = &pModel->pObjects[nextFrame];

	// Get the first key frame so we have an address to the texture and face information
	// since we are saving the texture and vertices index information only once...
	t3DObject *pFirstFrameObject = &pModel->pObjects[0];


	// Next, we want to get the current time that we are interpolating by.  Remember,
	// if t = 0 then we are at the beginning of the animation, where if t = 1 we are at the end.
	// Anyhing from 0 to 1 can be thought of as a percentage from 0 to 100 percent complete.
	float t = returnCurrentTime(pModel, nextFrame);
	
	//Check to see if the Object has a texture map, if so bind the texture to it.
	if(/*pObject->bHasTexture*/ pFirstFrameObject->pTexVertices) {

		//Turn On Texture Mapping & Turn Off Colour...
		glEnable(GL_TEXTURE_2D);

		//Reset the colour to normal again,
		//IMP, need to set it to WHITE, 
		//else if it turns BLACK(you wont see anything on the screen, if ur background is set to BLACK too)
		//watch out for 'ub' & 'b', sometimes one forgets to set it by mistake n the colour u get is BLACK
		glColor3ub(255, 255, 255);

		//Bind the texture map to the Object by its materialID
		glBindTexture(GL_TEXTURE_2D, texture[0]);//pObject->materialID]);
	}
	else {
		//Turn Off Texture Mapping & Turn On Colour...
		glDisable(GL_TEXTURE_2D);

		//Reset the colour to normal again,
		//IMP, need to set it to WHITE, 
		//else if it turns black(you wont see anything on the screen, if ur background is set to black too)
		//watch out for 'ub' & 'b', sometimes one forgets to set it by mistake n the colour u get is BLACK
		glColor3ub(255, 255, 255);
	}

	// Start rendering lines or triangles, depending on our current rendering mode (Lft Mouse Btn)
	glBegin(glViewMode);

		// Go through all of the faces (polygons) of the current frame and draw them
		for(int j = 0; j < pCurFrameObject->numOfFaces; j++) {

			// Go through each corner of the triangle and draw it.
			for(int whichVertex = 0; whichVertex < 3; whichVertex++) {

				// Get the index for each point of the face
				int vertexIndex = pFirstFrameObject->pFaces[j].vertexIndex[whichVertex];

				// Make sure there was a UVW map applied to the object.  Notice that
				// we use the first frame to check if we have texture coordinates because
				// none of the other frames hold this information, just the first by design.
				if(pFirstFrameObject->pTexVertices) {

					// Get the index for each texture coordinate for this face
					int texIndex = pFirstFrameObject->pFaces[j].texCoordIndex[whichVertex];

					// Set the Texture Coordinates for this vertex
					glTexCoord2f(	pFirstFrameObject->pTexVertices[texIndex].x,
									pFirstFrameObject->pTexVertices[texIndex].y
								);
				}

				// Now we get to the interpolation part! (*Bites his nails*)
				// Below, we first store the vertex we are working on for the current
				// frame and the frame we are interpolating too.  Next, we use the
				// linear interpolation equation to smoothly transition from one
				// key frame to the next.
				
				// Store the current and next frame's vertex
				CVector3 vPoint1 = pCurFrameObject->pVertices[vertexIndex];
				CVector3 vPoint2 = pNextFrameObject->pVertices[vertexIndex];

				// By using the equation: p(t) = p0 + t(p1 - p0), with a time t
				// passed in, we create a new vertex that is closer to the next key frame.
				glVertex3f(	vPoint1.x + t * (vPoint2.x - vPoint1.x),	// Find the interpolated X
							vPoint1.y + t * (vPoint2.y - vPoint1.y),	// Find the interpolated Y
							vPoint1.z + t * (vPoint2.z - vPoint1.z)		// Find the interpolated Z
						);

//sprintf(readerMD2.logText, "tttt %f === %0.2f \t %0.2f \t %0.2f\r\n", t, m, n, o);
//fwrite(readerMD2.logText, 1, strlen(readerMD2.logText), readerMD2.mFileConsoleLog);
			}
		}

	// Stop rendering the triangles
	glEnd();

}

void drawMD2Object() {
	//glRotatef(rotateFaceY, 0.0, 1.0, 0.0);
	//rotateFaceY += 0.5;
char logText[255];

	// I am going to attempt to explain what is going on below up here as not to clutter the 
	// code below.  We have a model that has a certain amount of objects and textures.  We want 
	// to go through each object in the model, bind it's texture map to it, then render it.
	// To render the current object, we go through all of it's faces (Polygons).  
	// What is a face you ask?  A face is just (in this case) a triangle of the object.
	// For instance, a cube has 12 faces because each side has 2 triangles.
	// You might be thinking.  Well, if there are 12 faces in a cube, that makes
	// 36 vertices that we needed to read in for that object.  Not really true.  Because
	// a lot of the vertices are the same, since they share sides, they only need to save
	// 8 vertices, and ignore the duplicates.  Then, you have an array of all the
	// unique vertices in that object.  No 2 vertices will be the same.  This cuts down
	// on memory.  Then, another array is saved, which is the index numbers for each face,
	// which index in to that array of vertices.  That might sound silly, but it is better
	// than saving tons of duplicate vertices.  The same thing happens for UV coordinates.
	// You don't save duplicate UV coordinates, you just save the unique ones, then an array
	// that index's into them.  This might be confusing, but most 3D files use this format.
	// This loop below will stay the same for most file formats that you load, so all you need
	// to change is the loading code.  You don't need to change this loop (Except for animation).

	// Since we know how many models our MODEL/SCENE has, go through each of them.
	//for(int i = 0; i < 1/*pGame3DModel.numOfObjects*/; i++) {

		//Make sure we have valid Objects just in case.
		//if(pGame3DModel.pObjects.size() <= 0)
		//	return;

		//Get the current Object that we are displaying.
		t3DObject *pObject = &pGame3DModel.pObjects[CURRENT_FRAME_ID];

		//Check to see if the Object has a texture map, if so bind the texture to it.
		//pObject->bHasTexture = true;
		if(/*pObject->bHasTexture*/ pObject->pTexVertices) {

			//Turn On Texture Mapping & Turn Off Colour...
			glEnable(GL_TEXTURE_2D);

			//Reset the colour to normal again,
			//IMP, need to set it to WHITE, 
			//else if it turns BLACK(you wont see anything on the screen, if ur background is set to BLACK too)
			//watch out for 'ub' & 'b', sometimes one forgets to set it by mistake n the colour u get is BLACK
			glColor3ub(255, 255, 255);

			//Bind the texture map to the Object by its materialID
			glBindTexture(GL_TEXTURE_2D, texture[0]);//pObject->materialID]);
		}
		else {
			//Turn Off Texture Mapping & Turn On Colour...
			glDisable(GL_TEXTURE_2D);

			//Reset the colour to normal again,
			//IMP, need to set it to WHITE, 
			//else if it turns black(you wont see anything on the screen, if ur background is set to black too)
			//watch out for 'ub' & 'b', sometimes one forgets to set it by mistake n the colour u get is BLACK
			glColor3ub(255, 255, 255);
		}

		//This determines if we are in WireFrame mode or Normal mode
		//Begin drawing with our selected mode (triangles or lines)
		glBegin(glViewMode);

			//Go through all of the faces (polygons) of the Object and draw them
			for(int j = 0; j < pObject->numOfFaces; j++) {
//sprintf(logText, "FACES %d %d", pObject->numOfFaces, j);
//MessageBox(NULL, logText, "ssss", MB_OK);
				
				//Go throught each corner(Vertex) of the Triangle and draw it.
				for(int whichVertex = 0; whichVertex < 3; whichVertex++) {
//sprintf(logText, "VERTEX %d", whichVertex);
//MessageBox(NULL, logText, "ssss", MB_OK);
					
					//Get the index for each point of the Face
					int vertexIndex = pObject->pFaces[j].vertexIndex[whichVertex];
//sprintf(logText, "index %d", index);
//MessageBox(NULL, logText, "ssss", MB_OK);

					// Give OpenGL the normal for this vertex.
					//glNormal3f(pObject->pNormals[ index ].x, pObject->pNormals[ index ].y, pObject->pNormals[ index ].z);

					//If the Object has texture associated with it, give it a texture coordinate.
					if(/*pObject->bHasTexture*/ pObject->pTexVertices) {
						int texIndex = pObject->pFaces[j].texCoordIndex[whichVertex];
						//Make sure there are UVM map applied to the Object, or else it wont have tex coordinates
						if(pObject->pTexVertices) {
							glTexCoord2f(pObject->pTexVertices[texIndex].x, pObject->pTexVertices[texIndex].y);
						}
					}
					else {
						// Make sure there is a valid material/color assigned to this object.
						// You should always at least assign a material color to an object, 
						// but just in case we want to check the size of the material list.
						// if the size is at least one, and the material ID != -1,
						// then we have a valid material.
							
						if(pGame3DModel.pMaterials.size() && pObject->materialID >= 0) {
							
							//Get and set the colour that the Object is, since it must have a texture.
							BYTE *pColour = pGame3DModel.pMaterials[pObject->materialID].colour;

							//Assign the current Colour to this Object
							glColor3ub(pColour[0], pColour[1], pColour[2]);
						}
					}
					
					//Pass in the current Vertex of the Object (Corner of the current Face).
					glVertex3f(	pObject->pVertices[vertexIndex].x, 
								pObject->pVertices[vertexIndex].y,
								pObject->pVertices[vertexIndex].z
							);
				}
			}
		glEnd();
	//}
}

int drawGLScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	moveCamera();

	glTranslatef(0.0, -0.0, -85.0f);

	//drawGameScene();
	//draw3DSScene();
	//drawMD2Object();
	animateMD2(&pGame3DModel);
	
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

			glViewMode = (glViewMode == GL_TRIANGLES)?GL_LINES:GL_TRIANGLES;

			//MessageBox(NULL, "AsdaS", "asdA", MB_OK);
		}
		break;
		case WM_RBUTTONUP:
			// To cycle through the animations, we just increase the model's current animation
			// by 1.  You'll notice that we also mod this result by the total number of
			// animations in our model, to make sure we go back to the beginning once we reach
			// the end of our animation list.  

			// Increase the current animation and mod it by the max animations
			pGame3DModel.currentAnim = (pGame3DModel.currentAnim + 1) % pGame3DModel.numOfAnimations;
			
			// Set the current frame to be the starting frame of the new animation
			pGame3DModel.currentFrame = pGame3DModel.pAnimations[pGame3DModel.currentAnim].startFrame;

			// Display the current animation in our window
			sprintf(strWindowTitle, "www.GameTutorials.com - Md2 Animation: %s", 
					pGame3DModel.pAnimations[pGame3DModel.currentAnim].animName);
			
			SetWindowText(mHWnd, strWindowTitle);

			//pGame3DModel.currentFrame = (pGame3DModel.currentFrame + 1) % pGame3DModel.pAnimations[pGame3DModel.currentAnim].endFrame;
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}