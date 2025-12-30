
#include <windows.h>					// Header File For Windows
#include <gl\gl.h>						// Header File For The OpenGL32 Library
#include <gl\glu.h>						// Header File For The GLu32 Library
#include <gl\glut.h>
#include <math.h>

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

int cameraRotX, cameraRotY, cameraRotZ;
float cameraPosX, cameraPosY, cameraPosZ;
const float piOver180 = 0.0174532925f;
const float MOVE_SPEED = 0.1f;
const float M_PI = 3.1428571428571428571428571428571;

GLuint	fontList;
GLuint texture[1];

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
GLvoid	resizeGLScene(GLsizei width, GLsizei height);
int initGL(GLvoid);
int drawGLScene(GLvoid);
GLvoid killGLWindow(GLvoid);
bool createGLWindow(char* title, int width, int height, int bits, bool isFullScreen);

bool NeHeLoadBitmap(LPCSTR fileName, GLuint &texID) {
	HBITMAP hBmp;
	BITMAP  bmp;

	hBmp = (HBITMAP)LoadImage(GetModuleHandle(NULL), fileName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);

	if(!hBmp)
		return false;
	
	GetObject(hBmp, sizeof(bmp), &bmp);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	glBindTexture(GL_TEXTURE_2D, texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, bmp.bmWidth, bmp.bmHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, bmp.bmBits);

	DeleteObject(hBmp);

	return true;
}

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
					updateCamera();
					
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

	gluPerspective(60.0f, (GLfloat)width/(GLfloat)height, 0.1f, 100.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

int charWidths[] = { 55, 26, 21, 28, 29, 47, 51, 13, 24, 24, 23, 28, 20, 26, 16, 40, 37, 24, 46, 40, 46, 40, 32, 36, 40, 38, 17, 17, 29, 26, 29, 35, 40, 56, 45, 33, 41, 51, 43, 45, 50, 26, 26, 60, 47, 60, 53, 42, 44, 59, 55, 37, 42, 43, 52, 60, 60, 60, 60, 27, 38, 27, 16, 45, 14, 41, 39, 31, 40, 31, 30, 39, 46, 20, 26, 50, 24, 56, 45, 31, 38, 37, 36, 32, 33, 42, 39, 52, 52, 42, 40, 26, 19, 26, 18};
float percentSize = 1.0f;

void createMyFont_() {
	float colX, colY;

	fontList = glGenLists(96);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	
	float width = (float)(1.0/16.0);
	float height = (float)(1.0/10.0);

	for(int i = 0; i < 96; i++) {
		colX = (float)(i%16)/16;
		colY = (float)(i/16)/10;
		
		glNewList(fontList+i, GL_COMPILE);
			glBegin(GL_QUADS);
				///*
				glTexCoord2f(colX, 1 - height - colY);
				glVertex2i(0, 0);
				//glVertex3f(-1.0f, -1.0f,  1.0f);	// Bottom Left Of The Texture and Quad

				glTexCoord2f(width+colX, 1 - height - colY);
				glVertex2i(64*percentSize, 0);
				//glVertex3f( 1.0f, -1.0f,  1.0f);	// Bottom Right Of The Texture and Quad

				glTexCoord2f(width+colX, 1.0 - colY);
				glVertex2i(64*percentSize, 64*percentSize);
				//glVertex3f( 1.0f,  1.0f,  1.0f);	// Top Right Of The Texture and Quad

				glTexCoord2f(colX, 1.0 - colY);
				glVertex2i(0, 64*percentSize);
				//glVertex3f(-1.0f,  1.0f,  1.0f);	// Top Left Of The Texture and Quad
				//*/
			glEnd();
			glTranslated(charWidths[i]*percentSize, 0, 0);
		glEndList();
	}
}

GLvoid glDrawString(char* string, GLint x, GLint y, int fontType) {	// Where The Printing Happens
	if(fontType > 1) {											// Is set Greater Than One?
		fontType = 1;											// If So, Make Set Equal One
	}

	glBindTexture(GL_TEXTURE_2D, texture[0]);				// Select Our Font Texture
	glDisable(GL_DEPTH_TEST);								// Disables Depth Testing
	
	
	glMatrixMode(GL_PROJECTION);							// Select The Projection Matrix
	glPushMatrix();											// Store The Projection Matrix	
	glLoadIdentity();										// Reset The Projection Matrix

	glOrtho(0, SCREEN_WIDTH, 0, SCREEN_HEIGHT, -1, 1);							// Set Up An Ortho Screen

	glMatrixMode(GL_MODELVIEW);								// Select The Modelview Matrix
	glPushMatrix();											// Store The Modelview Matrix
	glLoadIdentity();										// Reset The Modelview Matrix
	
	
	glTranslated(x, y, 0);									// Position The Text (0,0 - Bottom Left)
	
	glListBase(fontList - 32);// + (128*fontType));			// Choose The Font Set (0 or 1)
	glCallLists(strlen(string), GL_UNSIGNED_BYTE, string);	// Write The Text To The Screen

	glMatrixMode(GL_PROJECTION);							// Select The Projection Matrix
	glPopMatrix();											// Restore The Old Projection Matrix
	
	glMatrixMode(GL_MODELVIEW);								// Select The Modelview Matrix
	glPopMatrix();											// Restore The Old Modelview Matrix

	glEnable(GL_DEPTH_TEST);								// Enables Depth Testing
}

int initGL(GLvoid) {
	// Start Of User Initialization
	glGenTextures(1, &texture[0]);										// Create The Texture

	if (!NeHeLoadBitmap("Data/Font.bmp", texture[0]))			// Load The Bitmap
		return FALSE;											// Return False If Loading Failed
	
	glEnable(GL_TEXTURE_2D);									// Enable Texture Mapping ( NEW )

	createMyFont_();												// Build The Font

	glShadeModel(GL_SMOOTH);
	glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	moveCamera();
	
	glTranslatef(0.0, 0.0, -5.0);
	
	///*
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);
		glVertex2f(-0.5, -0.5);

		glTexCoord2f(1.0, 0.0);
		glVertex2f(0.5, -0.5);

		glTexCoord2f(1.0, 1.0);
		glVertex2f(0.5, 0.5);

		glTexCoord2f(0.0, 1.0);
		glVertex2f(-0.5, 0.5);
	glEnd();
	//*/
	
	glLoadIdentity();							// Reset The View
	glDrawString("Idealabs Interactive!!!", 0, 0, 1);
	glDrawString("ABCDEFGHIJKL", 0, 70, 1);

	return true;
}

void handlerKeyboard() {
	if(mKeys[VK_F1]) {
		mKeys[VK_F1] = false;
		killGLWindow();
		mFULLSCREEN = !mFULLSCREEN;
		if(!createGLWindow("openGL window", SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, mFULLSCREEN)) {
			return;
		}
	}
}

void keyPressed(int keyCode) {
	mKeys[keyCode] = true;

	handlerKeyboard();
}

void keyReleased(int keyCode) {
	mKeys[keyCode] = false;

	handlerKeyboard();
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch(uMsg) {
		case WM_CREATE:
		{
			SetCursorPos(SCREEN_WIDTH>>1, SCREEN_HEIGHT>>1);
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
			keyPressed(wParam);
			return 0;
		}
		case WM_KEYUP:
		{
			keyReleased(wParam);
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
		case WM_LBUTTONDOWN:

		break;
		case WM_MBUTTONDOWN:

		break;
		case WM_RBUTTONDOWN:

		break;
		case WM_MOUSEWHEEL:
			short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			int sign = (zDelta > 0)?-1:1;

			cameraPosX -= sign*sin( cameraRotY * piOver180 ) * MOVE_SPEED*10;
			cameraPosZ -= sign*cos( cameraRotY * piOver180 ) * MOVE_SPEED*10;
			
			return 0;
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}