
#include <windows.h>					// Header File For Windows
#include <math.h>						// Header File For Windows Math Library		( ADD )
#include <stdio.h>						// Header File For Standard Input/Output	( ADD )
#include <stdarg.h>						// Header File For Variable Argument Routines	( ADD )
#include <gl\gl.h>						// Header File For The OpenGL32 Library
#include <gl\glu.h>						// Header File For The GLu32 Library

HDC			mHDC		= NULL;			// Private GDI Device Context
HGLRC		mHRC		= NULL;			// Permanent Rendering Context
HWND		mHWnd	= NULL;				// Holds Our Window Handle
HINSTANCE	mHInstance;					// Holds The Instance Of The Application

bool		mKeys[256];					// Array Used For The Keyboard Routine
bool		mActive = false;			// Window Active Flag
bool		mFULLSCREEN = true;			// Fullscreen Flag Set To TRUE By Default

GLuint		texture[1];					// One Texture Map ( NEW )
GLuint		base;						// Base Display List For The Font Set	( ADD )
GLuint		rot;						// Used To Rotate The Text		( ADD )

GLYPHMETRICSFLOAT 	gmf[256];			// Storage For Information About Our Font

bool		QUIT_NOW = false;
int			prevMouseX;
int			prevMouseY;
float		textLength;

GLvoid buildFont(GLvoid) {				// Build Our Bitmap Font
	

	HFONT hFont;						// Windows Font ID
	base = glGenLists(256);				// Storage For 256 Characters

	hFont = CreateFont( -12,			// Height Of Font
						0,
						0,
						0,
						FW_BOLD,
						false,
						false,
						false,
						ANSI_CHARSET,//SYMBOL_CHARSET,
						OUT_TT_PRECIS,
						CLIP_DEFAULT_PRECIS,
						ANTIALIASED_QUALITY,
						FF_DONTCARE|DEFAULT_PITCH,	// Family And Pitch
						"Comic Sans MS"
					);
	
	SelectObject(mHDC, hFont);

	wglUseFontOutlines( mHDC,					// Select The Current DC
						0,						// Starting Character
						255,					// Number Of Display Lists To Build
						base,					// Starting Display Lists
						0.1f,					// Deviation From The True Outlines
						0.2f,					// Font Thickness In The Z Direction
						WGL_FONT_POLYGONS,		// Use Polygons, Not Lines
						gmf						// Address Of Buffer To Recieve Data
					);
}

GLvoid killFont(GLvoid) {						// Delete The Font
	glDeleteLists(base, 256);					// Delete All 256 Characters
}

GLvoid glPrint(const char* fmt, ...) {			// Custom GL "Print" Routine
	float length = 0;							// Used To Find The Length Of The Text
	char text[256];								// Holds Our String
	va_list ap;									// Pointer To List Of Arguments
	
	if(fmt == NULL)								// If There's No Text
		return;									// Do Nothing
		
	va_start(ap, fmt);							// Parses The String For Variables
		vsprintf(text, fmt, ap);				// And Converts Symbols To Actual Numbers
	va_end(ap);									// Results Are Stored In Text

	for(unsigned int i = 0; i < strlen(text); i++) {	// Loop To Find Text Length
		length += gmf[text[i]].gmfCellIncX;				// Increase Length By Each Characters Width
	}

	glTranslatef(-length/2, 0.0f, 0.0f);		// Center Our Text On The Screen

	glPushAttrib(GL_LIST_BIT);					// Pushes The Display List Bits
	glListBase(base);							// Sets The Base Character to 0

	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
	glPopAttrib();										// Pops The Display List Bits

	
	//MessageBox(NULL, "build font OK", (LPSTR)text, MB_OK);
}

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
GLvoid	resizeGLScene(GLsizei width, GLsizei height);
int initGL(GLvoid);
int drawGLScene(GLvoid);
GLvoid killGLWindow(GLvoid);
bool createGLWindow(char* title, int width, int height, int bits, bool isFullScreen);

bool NeHeLoadBitmap(LPTSTR szFileName, GLuint &texid)					// Creates Texture From A Bitmap File
{
	HBITMAP hBMP;														// Handle Of The Bitmap
	BITMAP	BMP;														// Bitmap Structure

	glGenTextures(1, &texid);											// Create The Texture
	hBMP=(HBITMAP)LoadImage(GetModuleHandle(NULL), szFileName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE );

	if (!hBMP)															// Does The Bitmap Exist?
		return FALSE;													// If Not Return False

	GetObject(hBMP, sizeof(BMP), &BMP);									// Get The Object
																		// hBMP:        Handle To Graphics Object
																		// sizeof(BMP): Size Of Buffer For Object Information
																		// &BMP:        Buffer For Object Information

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);								// Pixel Storage Mode (Word Alignment / 4 Bytes)

	// Typical Texture Generation Using Data From The Bitmap
	glBindTexture(GL_TEXTURE_2D, texid);								// Bind To The Texture ID

	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, BMP.bmWidth, BMP.bmHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, BMP.bmBits); // ( NEW )
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);	// Linear Min Filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// Linear Mag Filter
	
	// Texturing Contour Anchored To The Object
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	// Texturing Contour Anchored To The Object
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glEnable(GL_TEXTURE_GEN_S);			// Auto Texture Generation
	glEnable(GL_TEXTURE_GEN_T);			// Auto Texture Generation

	DeleteObject(hBMP);													// Delete The Object

	return TRUE;														// Loading Was Successful
}

int WINAPI WinMain(	HINSTANCE	hInstance, 
					HINSTANCE	hPrevInstance,
					LPSTR		lpCmdLine,
					int			nCmdShow
) {
	MSG		msg;
	bool	done = false;

	//if(MessageBox(NULL, "Would you like to run in FULLSCREEN mode?", "Start FullScreen", MB_YESNO) == IDNO)
	//	mFULLSCREEN = false;

	//Get Current Display Resolution...
	DEVMODE deviceMode;
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &deviceMode);

	if(!createGLWindow("openGL Window...Press F1 to toogle between windowed & Fullscreen Mode.", deviceMode.dmPelsWidth, deviceMode.dmPelsHeight, deviceMode.dmBitsPerPel, mFULLSCREEN))
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
			
			/*
			if(mKeys[VK_F1]) {
				mKeys[VK_F1] = false;
				killGLWindow();
				mFULLSCREEN = !mFULLSCREEN;
				if(!createGLWindow("openGL window", 640, 480, 16, mFULLSCREEN)) {
					return 0;
				}
			}
			*/

			if(QUIT_NOW)
				done = true;
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
	/*
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
	*/
	if(mFULLSCREEN) {
		dwExStyle	= WS_EX_APPWINDOW;
		dwStyle		= WS_POPUP;
		ShowCursor(false);
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

int initGL(GLvoid) {									// All Setup For OpenGL Goes Here
	if(!NeHeLoadBitmap("data/Lights.bmp", texture[0])) {
		return 0;
	}
	
	buildFont();										//Build The Font							( ADD )

	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

	glEnable(GL_LIGHT0);								// Enable Default Light (Quick And Dirty)	( NEW )
	glEnable(GL_LIGHTING);								// Enable Lighting							( NEW )
	glEnable(GL_COLOR_MATERIAL);						// Enable Coloring Of Material				( NEW )
	
	glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping
	glBindTexture(GL_TEXTURE_2D, texture[0]);			// Select The Texture

	return true;
}

GLvoid killGLWindow() {
	if(mFULLSCREEN) {
		//ChangeDisplaySettings(NULL, 0);
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

		killFont();
	}
}

int drawGLScene_() {										// Here's Where We Do All The Drawing
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer
	glLoadIdentity();									// Reset The View
	
	// Position The Text
	glTranslatef(1.1f*float(cos(rot/64.0f)),0.8f*float(sin(rot/60.0f)),-8.0f);

	glRotatef(rot,1.0f,0.0f,0.0f);				// Rotate On The X Axis
	glRotatef(rot*1.2f,0.0f,1.0f,0.0f);			// Rotate On The Y Axis
	glRotatef(rot*1.4f,0.0f,0.0f,1.0f);			// Rotate On The Z Axis
	
	glTranslatef(-0.35f,-0.35f,0.1f);			// Center On X, Y, Z Axis
	
	glPrint("Idealabs Interactive");								// Draw A Skull And Crossbones Symbol
	rot += 1.0f;								// Increase The Rotation Variable

	return true;
}

int drawGLScene() {										// Here's Where We Do All The Drawing
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer
	glLoadIdentity();									// Reset The View
	
	// Position The Text
	glTranslatef(1.1f*float(cos(rot/64.0f)),0.8f*float(sin(rot/60.0f)),-8.0f);

	//glRotatef(rot,1.0f,0.0f,0.0f);				// Rotate On The X Axis
	glRotatef(rot*0.8f,0.0f,1.0f,0.0f);			// Rotate On The Y Axis
	glRotatef(rot*0.8f,0.0f,0.0f,1.0f);			// Rotate On The Z Axis
	
	glTranslatef(-0.35f,-0.35f,0.1f);			// Center On X, Y, Z Axis
	
	glPrint("Idealabs Interactive");								// Draw A Skull And Crossbones Symbol
	rot += 1.0f;								// Increase The Rotation Variable

	return true;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch(uMsg) {
		case WM_CREATE:
		{
			POINT curPos;
			if(GetCursorPos(&curPos)) {
				prevMouseX = curPos.x;
				prevMouseY = curPos.y;
			}
		}
		return 0;
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
			QUIT_NOW = true;
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
		case WM_MOUSEMOVE://WM_LBUTTONDOWN:
		{
			///*
				int xPos = LOWORD(lParam);
				int yPos = HIWORD(lParam);

				int dX = xPos - prevMouseX;
				if(dX < 0)
					dX = -dX;
				int dY = yPos - prevMouseY;
				if(dY < 0)
					dY = -dY;

				if(dX > 20 || dY > 20) {
					QUIT_NOW = true;
				}

				prevMouseX = xPos;
				prevMouseY = yPos;
			//*/
			return 0;
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


bool WINAPI ScreenSaverConfigureDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    return FALSE;
}

// needed for SCRNSAVE.LIB
bool WINAPI RegisterDialogClasses(HANDLE hInst)
{
  return TRUE;
}

