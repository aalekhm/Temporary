
#include <windows.h>					// Header File For Windows
#include <gl\gl.h>						// Header File For The OpenGL32 Library
#include <gl\glu.h>						// Header File For The GLu32 Library
#include <stdio.h>
#include <math.h>
#include <conio.h>

#include "3DS.h"

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define SCREEN_BPP		16

#define MAX_TEXTURES	100

HDC			mHDC		= NULL;			// Private GDI Device Context
HGLRC		mHRC		= NULL;			// Permanent Rendering Context
HWND		mHWnd	= NULL;				// Holds Our Window Handle
HINSTANCE	mHInstance;					// Holds The Instance Of The Application

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
#define FILE_NAME_3DS	"Character.3ds"
CLoad3DS  reader3DS;
t3DModel	pGame3DModel;
int glViewMode = GL_TRIANGLES;
float rotateFaceY = 0.0;

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
	
	ShowCursor(false);

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

	gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 100.0f);

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
	reader3DS.Import3DS(&pGame3DModel, FILE_NAME_3DS);

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
		if(pGame3DModel.pObject.size() <= 0)
			return;

		//Get the current Object that we are displaying.
		t3DObject *pObject = &pGame3DModel.pObject[i];

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
					int index = pObject->pFaces[j].vertIndex[whichVertex];

					// Give OpenGL the normal for this vertex.
					//glNormal3f(pObject->pNormals[ index ].x, pObject->pNormals[ index ].y, pObject->pNormals[ index ].z);

					//If the Object has texture associated with it, give it a texture coordinate.
					if(pObject->bHasTexture) {

						//Make sure there are UVM map applied to the Object, or else it wont have tex coordinates
						if(pObject->pTexVerts) {
							glTexCoord2f(pObject->pTexVerts[index].x, pObject->pTexVerts[index].y);
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
							BYTE *pColour = pGame3DModel.pMaterials[pObject->materialID].color;

							//Assign the current Colour to this Object
							glColor3ub(pColour[0], pColour[1], pColour[2]);
						}
					}
					
					//Pass in the current Vertex of the Object (Corner of the current Face).
					glVertex3f(	pObject->pVerts[index].x, 
								pObject->pVerts[index].y,
								pObject->pVerts[index].z
							);
				}
			}
		glEnd();
	}
}

int drawGLScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	moveCamera();

	glTranslatef(0.0, -0.0, -15.0f);

	//drawGameScene();
	draw3DSScene();
	
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