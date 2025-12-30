
#include <stdio.h>						// Header File For Standard Input/Output 
#include <math.h>						// Math Library Header File
#include <windows.h>					// Header File For Windows
#include <gl\gl.h>						// Header File For The OpenGL32 Library
#include <gl\glu.h>						// Header File For The GLu32 Library
#include <gl\glut.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define SCREEN_BPP		16

HDC			mHDC		= NULL;			// Private GDI Device Context
HGLRC		mHRC		= NULL;			// Permanent Rendering Context
HWND		mHWnd	= NULL;				// Holds Our Window Handle
HINSTANCE	mHInstance;					// Holds The Instance Of The Application

bool		mKeys[256];					// Array Used For The Keyboard Routine
bool		mActive = false;			// Window Active Flag
bool		mFULLSCREEN = true;			// Fullscreen Flag Set To TRUE By Default

bool		light;
bool		blend;
bool		filter;

bool		bPressed;
bool		fPressed;
bool		lPressed;

const float piOver180 = 0.0174532925f;
const float MOVE_SPEED = 0.05f;
float		heading;
float		xPos;
float		zPos;
static bool L_MOUSE_DOWN = false;

GLfloat		yRot;						// Y Rotation
GLfloat		zRot;						// Z Rotation

GLfloat		walkBias = 0;
GLfloat		walkBiasAngle = 0;
GLfloat		lookUpDown = 0.0f;
//GLfloat		z = 0.0f;					// Depth Into The Screen

GLuint		texture[3];					// Storage for 3 textures

typedef struct tagVERTEX {
	float x, y, z;
	float u, v;
} VERTEX;

typedef struct tagTRIANGLE {
	VERTEX vertex[3];
} TRIANGLE;

typedef struct tagSECTOR {
	int numTriangles;
	TRIANGLE* triangle;
} SECTOR;


SECTOR			sector1;				// Our Model Goes Here:

void readStr(FILE* f, char* string) {
	do {
		fgets(string, 255, f);
	} while( (string[0] == '/') || (string[0] == '\n') );
}

void setUpWorld() {
	float x, y, z, u, v;
	int numTriangles;
	FILE* fileIn;
	char oneLine[255];
	
	fileIn = fopen("data/world.txt", "rt");
	
	readStr(fileIn, oneLine);
	sscanf(oneLine, "NUMPOLLIES %d\n", &numTriangles);
	
	sector1.triangle = new TRIANGLE[numTriangles];
	sector1.numTriangles = numTriangles;

	for(int i = 0; i < numTriangles; i++) {
		for(int vert = 0; vert < 3; vert++) {
			readStr(fileIn, oneLine);
			sscanf(oneLine, "%f %f %f %f %f", &x, &y, &z, &u, &v);
			sector1.triangle[i].vertex[vert].x = x;
			sector1.triangle[i].vertex[vert].y = y;
			sector1.triangle[i].vertex[vert].z = z;
			sector1.triangle[i].vertex[vert].u = u;
			sector1.triangle[i].vertex[vert].v = v;
		}
	}

	fclose(fileIn);
	return;
}

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
	
	lookUpDown += deltaY;
	heading -= deltaX;
	yRot = heading;
}

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
GLvoid	resizeGLScene(GLsizei width, GLsizei height);
int initGL(GLvoid);
int drawGLScene(GLvoid);
GLvoid killGLWindow(GLvoid);
bool createGLWindow(char* title, int width, int height, int bits, bool isFullScreen);
bool NeHeLoadBitmap(LPTSTR szFileName);					// Creates Texture From A Bitmap File

int WINAPI WinMain(	HINSTANCE	hInstance, 
					HINSTANCE	hPrevInstance,
					LPSTR		lpCmdLine,
					int			nCmdShow
) {
	MSG		msg;
	bool	done = false;

	if(MessageBox(NULL, "Would you like to run in FULLSCREEN mode?", "Start FullScreen", MB_YESNO) == IDNO)
		mFULLSCREEN = false;

	if(!createGLWindow("openGL Window...Press F1 to toogle between windowed & Fullscreen Mode.", SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, mFULLSCREEN))
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
					
					checkMouseMovement();

					//Lesson #10
					if(mKeys['B'] && !bPressed) {
						bPressed = true;
						blend = !blend;
						if(!blend) {
							glDisable(GL_BLEND);
							glEnable(GL_DEPTH_TEST);
						}
						else {
							glEnable(GL_BLEND);
							glDisable(GL_DEPTH_TEST);
						}
					}
					else
					if(!mKeys['B']) {
						bPressed = false;
					}

					if(mKeys['F'] && !fPressed) {
						fPressed = true;
						++filter;
						if(filter > 2)
							filter = 0;
					}
					else
					if(!mKeys['F']) {
						fPressed = false;
					}

					if(mKeys['L'] && !lPressed) {
						lPressed = true;
						light = !light;

						if(!light) {
							glDisable(GL_LIGHTING);
						}
						else {
							glEnable(GL_LIGHTING);
						}
					}
					else
					if(!mKeys['L']) {
						lPressed = false;
					}

					
					if(mKeys[VK_UP] || mKeys['W']) {
						xPos -= (float)sin(heading * piOver180) * MOVE_SPEED;
						zPos -= (float)cos(heading * piOver180) * MOVE_SPEED;
						
						if(walkBiasAngle > 359.0f) {
							walkBiasAngle = 0.0f;
						}
						else {
							walkBiasAngle += 10;
						}

						walkBias = (float)sin(walkBiasAngle * piOver180) / 20.0f;
					}
					else
					if(mKeys[VK_DOWN] || mKeys['S']) {
						xPos += (float)sin(heading * piOver180) * MOVE_SPEED;
						zPos += (float)cos(heading * piOver180) * MOVE_SPEED;

						if(walkBiasAngle <= 1.0f) {
							walkBiasAngle = 359.0f;
						}
						else {
							walkBiasAngle -= 10;
						}

						walkBias = (float)sin(walkBiasAngle * piOver180) / 20.0f;
					}

					if(mKeys[VK_LEFT] || mKeys['A']) {
						xPos += (float)sin((heading - 90)* piOver180) * MOVE_SPEED;
						zPos += (float)cos((heading - 90) * piOver180) * MOVE_SPEED;

					}
					else
					if(mKeys[VK_RIGHT] || mKeys['D']) {
						xPos -= (float)sin((heading - 90)* piOver180) * MOVE_SPEED;
						zPos -= (float)cos((heading - 90) * piOver180) * MOVE_SPEED;
					}
					
					if(mKeys['Q']) {
						heading += 1.0f;
						yRot = heading;
					}
					else
					if(mKeys['E']) {
						heading -= 1.0f;
						yRot = heading;
					}

					if(mKeys['Z']) {
						zRot += 1.0f;
					}
					else
					if(mKeys['C']) {
						zRot -= 1.0f;
					}

					if(mKeys[VK_PRIOR]) {
						lookUpDown -= 1.0f;
					}
					else
					if(mKeys[VK_NEXT]) {
						lookUpDown += 1.0f;
					}
				}
			}

			if(mKeys[VK_F1]) {
				mKeys[VK_F1] = false;
				killGLWindow();
				mFULLSCREEN = !mFULLSCREEN;
				if(!createGLWindow("NeHe's First Polygon Tutorial", SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, mFULLSCREEN)) {
					return 0;
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

	if(mFULLSCREEN) {
		dwExStyle	= WS_EX_APPWINDOW;
		dwStyle		= WS_POPUP;
		//ShowCursor(false);
	}
	else {
		dwExStyle	= WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle		= WS_POPUP;//WS_OVERLAPPEDWINDOW;
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

int initGL(GLvoid) {
	// Start Of User Initialization
	if (!NeHeLoadBitmap("Data/Mud.bmp"))			// Load The Bitmap
		return FALSE;								// Return False If Loading Failed
	
	glEnable(GL_TEXTURE_2D);						// Enable Texture Mapping ( NEW )
	
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	
	setUpWorld();
	
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

int drawGLScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer
	glLoadIdentity();									// Reset The View
	
	GLfloat x_m, y_m, z_m, u_m, v_m; 
	GLfloat xTranslate = -xPos;
	GLfloat zTranslate = -zPos;
	GLfloat yTranslate = -walkBias - 0.25f;
	GLfloat	sceneRotY = -yRot;//360.0f - yRot;
	GLfloat	sceneRotZ = -zRot;//360.0f - yRot;

	int numTriangles;

	glRotatef(lookUpDown, 1.0f, 0.0f, 0.0f);
	glRotatef(sceneRotY,  0.0f, 1.0f, 0.0f);
	glRotatef(sceneRotZ,  0.0f, 0.0f, 1.0f);

	glTranslatef(xTranslate, yTranslate, zTranslate);
	glBindTexture(GL_TEXTURE_2D, texture[filter]);
	
	numTriangles = sector1.numTriangles;

	// Process Each Triangle
	for(int i = 0; i < numTriangles; i++) {
		glBegin(GL_TRIANGLES);
			glNormal3f(0.0f, 0.0f, 1.0f);
			
			x_m = sector1.triangle[i].vertex[0].x;
			y_m = sector1.triangle[i].vertex[0].y;
			z_m = sector1.triangle[i].vertex[0].z;
			u_m = sector1.triangle[i].vertex[0].u;
			v_m = sector1.triangle[i].vertex[0].v;
			glTexCoord2f(u_m, v_m);
			glVertex3f(x_m, y_m, z_m);

			x_m = sector1.triangle[i].vertex[1].x;
			y_m = sector1.triangle[i].vertex[1].y;
			z_m = sector1.triangle[i].vertex[1].z;
			u_m = sector1.triangle[i].vertex[1].u;
			v_m = sector1.triangle[i].vertex[1].v;
			glTexCoord2f(u_m, v_m);
			glVertex3f(x_m, y_m, z_m);

			x_m = sector1.triangle[i].vertex[2].x;
			y_m = sector1.triangle[i].vertex[2].y;
			z_m = sector1.triangle[i].vertex[2].z;
			u_m = sector1.triangle[i].vertex[2].u;
			v_m = sector1.triangle[i].vertex[2].v;
			glTexCoord2f(u_m, v_m);
			glVertex3f(x_m, y_m, z_m);
		glEnd();
	}
	
	return true;										// Everything Went OK
}

bool NeHeLoadBitmap(LPTSTR szFileName)					// Creates Texture From A Bitmap File
{
	HBITMAP hBMP;														// Handle Of The Bitmap
	BITMAP	BMP;														// Bitmap Structure

	glGenTextures(3, &texture[0]);											// Create The Texture
	hBMP=(HBITMAP)LoadImage(GetModuleHandle(NULL), szFileName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE );

	if (!hBMP)															// Does The Bitmap Exist?
		return FALSE;													// If Not Return False

	GetObject(hBMP, sizeof(BMP), &BMP);									// Get The Object
																		// hBMP:        Handle To Graphics Object
																		// sizeof(BMP): Size Of Buffer For Object Information
																		// &BMP:        Buffer For Object Information

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);								// Pixel Storage Mode (Word Alignment / 4 Bytes)

	// Typical Texture Generation Using Data From The Bitmap
	glBindTexture(GL_TEXTURE_2D, texture[0]);								// Bind To The Texture ID
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	// Linear Min Filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	// Linear Mag Filter
	glTexImage2D(GL_TEXTURE_2D, 0, 3, BMP.bmWidth, BMP.bmHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, BMP.bmBits);

	glBindTexture(GL_TEXTURE_2D, texture[1]);								// Bind To The Texture ID
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// Linear Min Filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// Linear Mag Filter
	glTexImage2D(GL_TEXTURE_2D, 0, 3, BMP.bmWidth, BMP.bmHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, BMP.bmBits);

	glBindTexture(GL_TEXTURE_2D, texture[2]);								// Bind To The Texture ID
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// Linear Mag Filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);	// Linear Min Filter
	
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, BMP.bmWidth, BMP.bmHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, BMP.bmBits); // ( NEW )


	DeleteObject(hBMP);													// Delete The Object

	return TRUE;														// Loading Was Successful
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch(uMsg) {
		case WM_CREATE:
		{
			SetCursorPos(SCREEN_WIDTH>>1, SCREEN_HEIGHT>>1);
			return 0;
		}
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
		case WM_MOUSEMOVE:
		{
		}
		break;
		case WM_MOUSEWHEEL:
		{
			short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			int sign = (zDelta > 0)?-1:1;

			xPos -= sign*sin( heading * piOver180 ) * MOVE_SPEED;
			zPos -= sign*cos( heading * piOver180 ) * MOVE_SPEED;
			
			return 0;
		}
		break;
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