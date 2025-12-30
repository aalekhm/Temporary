
#include <windows.h>					// Header File For Windows
#include <math.h>						// Header File For Windows Math Library		( ADD )
#include <stdio.h>						// Header File For Standard Input/Output	( ADD )
#include <gl\gl.h>						// Header File For The OpenGL32 Library
#include <gl\glu.h>						// Header File For The GLu32 Library

HDC			mHDC		= NULL;			// Private GDI Device Context
HGLRC		mHRC		= NULL;			// Permanent Rendering Context
HWND		mHWnd	= NULL;				// Holds Our Window Handle
HINSTANCE	mHInstance;					// Holds The Instance Of The Application

bool		mKeys[256];					// Array Used For The Keyboard Routine
bool		mActive = false;			// Window Active Flag
bool		mFULLSCREEN = true;			// Fullscreen Flag Set To TRUE By Default

GLuint		base;						// Base Display List For The Font
GLuint		texture[2];					// Storage For Our Font Texture
GLuint		loop;						// Generic Loop Variable

GLfloat		counter1;					// 1st Counter Used To Move Text & For Coloring
GLfloat		counter2;					// 2nd Counter Used To Move Text & For Coloring

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

	hBMP=(HBITMAP)LoadImage(GetModuleHandle(NULL), szFileName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE | LR_LOADTRANSPARENT );

	if (!hBMP)															// Does The Bitmap Exist?
		return FALSE;													// If Not Return False

	GetObject(hBMP, sizeof(BMP), &BMP);									// Get The Object
																		// hBMP:        Handle To Graphics Object
																		// sizeof(BMP): Size Of Buffer For Object Information
																		// &BMP:        Buffer For Object Information

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);								// Pixel Storage Mode (Word Alignment / 4 Bytes)

	// Typical Texture Generation Using Data From The Bitmap
	glBindTexture(GL_TEXTURE_2D, texid);								// Bind To The Texture ID
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// Linear Min Filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// Linear Mag Filter
	glTexImage2D(GL_TEXTURE_2D, 0, 3, BMP.bmWidth, BMP.bmHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, BMP.bmBits);

	DeleteObject(hBMP);													// Delete The Object

	return TRUE;														// Loading Was Successful
}

GLvoid buildFont(GLvoid) {
	float	cx;											// Holds Our X Character Coord
	float	cy;											// Holds Our Y Character Coord
	
	base=glGenLists(256);								// Creating 256 Display Lists

	glBindTexture(GL_TEXTURE_2D, texture[1]);			// Select Our Font Texture
	for (loop=0; loop<256; loop++)						// Loop Through All 256 Lists
	{
		cx=float(loop%16)/16.0f;						// X Position Of Current Character
		cy=float(loop/16)/16.0f;						// Y Position Of Current Character

		glNewList(base+loop,GL_COMPILE);				// Start Building A List
			//NeHe
			//The way we set up our Ortho screen, (0,0) will be at the bottom left of our screen. 
			//(640,480) will be the top right of the screen. 0 is the left side of the screen on 
			//the x axis, 639 is the right side of the screen on the x axis. 0 is the bottom of 
			//the screen on the y axis and 479 is the top of the screen on the y axis. Basically 
			//we've gotten rid of negative coordinates. This is also handy for people that don't 
			//care about perspective and prefer to work with pixels rather than units :) 
			//Aalekh
			//For drawing a texture on a 2D scene,
			//the width & height of the texture are considered to be as '0' to '1' units
			//the texture has a (0,0) @ bottom left and (1, 1) @ top right
			//the 4 texture coordinates are matched with the QUAD coordinates
			//accordingly, remember the QUAD coordinates are in pixels(the amount of width & height you want the image to be).
			//Here since its a font image of 256x256, each character takes up 16x16 pixels of space.
			//Hence a character is 0.0625 x 0.0625 of units.

			glBegin(GL_QUADS);							// Use A Quad For Each Character

				glTexCoord2f(cx,1-cy-0.0625f);			// Texture Coord (Bottom Left)
				glVertex2i(0,0);						// Vertex Coord (Bottom Left)
				glTexCoord2f(cx+0.0625f,1-cy-0.0625f);	// Texture Coord (Bottom Right)
				glVertex2i(32,0);						// Vertex Coord (Bottom Right)
				glTexCoord2f(cx+0.0625f,1-cy);			// Texture Coord (Top Right)
				glVertex2i(32,32);						// Vertex Coord (Top Right)
				glTexCoord2f(cx,1-cy);					// Texture Coord (Top Left)
				glVertex2i(0,32);						// Vertex Coord (Top Left)

			glEnd();									// Done Building Our Quad (Character)
			glTranslated(32,0,0);						// Move To The Right Of The Character
		glEndList();									// Done Building The Display List
	}													// Loop Until All 256 Are Built
}

GLvoid buildFont_(GLvoid) {//READ...
	base = glGenLists(1);

	glBindTexture(GL_TEXTURE_2D, texture[1]);

	glNewList(base, GL_COMPILE);
		glBegin(GL_QUADS);
			//NeHe
			//The way we set up our Ortho screen, (0,0) will be at the bottom left of our screen. 
			//(640,480) will be the top right of the screen. 0 is the left side of the screen on 
			//the x axis, 639 is the right side of the screen on the x axis. 0 is the bottom of 
			//the screen on the y axis and 479 is the top of the screen on the y axis. Basically 
			//we've gotten rid of negative coordinates. This is also handy for people that don't 
			//care about perspective and prefer to work with pixels rather than units :) 
			//Aalekh
			//For drawing a texture on a 2D scene,
			//the width & height of the texture are considered to be as '0' to '1' units
			//the texture has a (0,0) @ bottom left and (1, 1) @ top right
			//the 4 texture coordinates are matched with the QUAD coordinates
			//accordingly, remember the QUAD coordinates are in pixels(the amount of width & height you want the image to be).
			//Here since its a font image of 256x256, each character takes up 16x16 pixels of space.
			//Hence a character is 0.0625 x 0.0625 of units.
			
			glTexCoord2d(0.0, 1.0-0.0625f);
			glVertex2i(0, 0);

			glTexCoord2d(0.0625f, 1.0-0.0625f);
			glVertex2i(16, 0);

			glTexCoord2d(0.0625f, 1.0f);
			glVertex2i(16, 16);
	
			glTexCoord2d(0.0f, 1.0f);
			glVertex2i(0, 16);
		glEnd();
	glEndList();
}

GLvoid killFont(GLvoid) {								// Delete The Font From Memory
	glDeleteLists(base, 256);							// Delete All 256 Display Lists
}

GLvoid glPrint(GLint x, GLint y, char* string, int set) {	// Where The Printing Happens
	if(set > 1) {											// Is set Greater Than One?
		set = 1;											// If So, Make Set Equal One
	}

	glBindTexture(GL_TEXTURE_2D, texture[1]);				// Select Our Font Texture

	glDisable(GL_DEPTH_TEST);								// Disables Depth Testing
	
	glMatrixMode(GL_PROJECTION);							// Select The Projection Matrix
	glPushMatrix();											// Store The Projection Matrix
	
	glLoadIdentity();										// Reset The Projection Matrix
	glOrtho(0, 640, 0, 480, -1, 1);							// Set Up An Ortho Screen

	glMatrixMode(GL_MODELVIEW);								// Select The Modelview Matrix
	glPushMatrix();											// Store The Modelview Matrix
	glLoadIdentity();										// Reset The Modelview Matrix
	
	glTranslated(x, y, 0);									// Position The Text (0,0 - Bottom Left)
	
	glListBase(base - 32 + (128*set));						// Choose The Font Set (0 or 1)
	glCallLists(strlen(string), GL_UNSIGNED_BYTE, string);	// Write The Text To The Screen

	glMatrixMode(GL_PROJECTION);							// Select The Projection Matrix
	glPopMatrix();											// Restore The Old Projection Matrix
	
	glMatrixMode(GL_MODELVIEW);								// Select The Modelview Matrix
	glPopMatrix();											// Restore The Old Modelview Matrix

	glEnable(GL_DEPTH_TEST);								// Enables Depth Testing
}

GLvoid glPrint_(GLint x, GLint y, char* string, int set) {	// Where The Printing Happens
	glDisable(GL_DEPTH_TEST);								// Disables Depth Testing
	
	glMatrixMode(GL_PROJECTION);							// Select The Projection Matrix
	glPushMatrix();											// Store The Projection Matrix
	
	glLoadIdentity();										// Reset The Projection Matrix
	glOrtho(0, 640, 0, 480, -1, 1);							// Set Up An Ortho Screen

	glMatrixMode(GL_MODELVIEW);								// Select The Modelview Matrix
	glPushMatrix();											// Store The Modelview Matrix
	glLoadIdentity();										// Reset The Modelview Matrix
	
	glTranslated(x, y, 0);									// Position The Text (0,0 - Bottom Left)
	
	//glListBase(base - 32 + (128*set));						// Choose The Font Set (0 or 1)
	//glCallLists(strlen(string), GL_UNSIGNED_BYTE, string);	// Write The Text To The Screen

	glCallList(base);

	glMatrixMode(GL_PROJECTION);							// Select The Projection Matrix
	glPopMatrix();											// Restore The Old Projection Matrix
	
	glMatrixMode(GL_MODELVIEW);								// Select The Modelview Matrix
	glPopMatrix();											// Restore The Old Modelview Matrix

	glEnable(GL_DEPTH_TEST);								// Enables Depth Testing
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

			if(mKeys[VK_F1]) {
				mKeys[VK_F1] = false;
				killGLWindow();
				mFULLSCREEN = !mFULLSCREEN;
				if(!createGLWindow("openGL window", 640, 480, 16, mFULLSCREEN)) {
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

int initGL(GLvoid) {
	// Start Of User Initialization
	glGenTextures(2, &texture[0]);										// Create The Texture

	if (!NeHeLoadBitmap("Data/Bumps.bmp", texture[0]))			// Load The Bitmap
		return FALSE;											// Return False If Loading Failed
	if (!NeHeLoadBitmap("Data/Font.bmp", texture[1]))			// Load The Bitmap
		return FALSE;											// Return False If Loading Failed
	
	glEnable(GL_TEXTURE_2D);									// Enable Texture Mapping ( NEW )

	buildFont();												// Build The Font

	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

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

	killFont();								// Destroy The Font
}

int drawGLScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTranslatef(0.0f, 0.0f, -5.0f);
	glRotatef(45.0f, 0.0f, 0.0f, 1.0f);

	glRotatef(counter1*30.0f, 1.0f, 1.0f, 0.0f);

	glDisable(GL_BLEND);							// Disable Blending Before We Draw In 3D
	glColor3f(1.0f,1.0f,1.0f);						// Bright White
	glBegin(GL_QUADS);							// Draw Our First Texture Mapped Quad
		glTexCoord2d(0.0f,0.0f);					// First Texture Coord
		glVertex2f(-1.0f,-1.0f);					// First Vertex
		glTexCoord2d(0.0f,1.0f);					// Second Texture Coord
		glVertex2f(-1.0f, 1.0f);					// Second Vertex
		glTexCoord2d(1.0f,1.0f);					// Third Texture Coord
		glVertex2f( 1.0f, 1.0f);					// Third Vertex
		glTexCoord2d(1.0f,0.0f);					// Fourth Texture Coord
		glVertex2f( 1.0f,-1.0f);					// Fourth Vertex
	glEnd();								// Done Drawing The First Quad

	glRotatef(90.0f,1.0f,1.0f,0.0f);			// Rotate On The X & Y Axis By 90 Degrees (Left To Right)
	glBegin(GL_QUADS);							// Draw Our First Texture Mapped Quad
		glTexCoord2d(0.0f,0.0f);					// First Texture Coord
		glVertex2f(-1.0f,-1.0f);					// First Vertex
		glTexCoord2d(0.0f,1.0f);					// Second Texture Coord
		glVertex2f(-1.0f, 1.0f);					// Second Vertex
		glTexCoord2d(1.0f,1.0f);					// Third Texture Coord
		glVertex2f( 1.0f, 1.0f);					// Third Vertex
		glTexCoord2d(1.0f,0.0f);					// Fourth Texture Coord
		glVertex2f( 1.0f,-1.0f);					// Fourth Vertex
	glEnd();								// Done Drawing Our Second Quad

	
	glEnable(GL_BLEND);							// Enable Blending
	
	glLoadIdentity();							// Reset The View
	
	// Pulsing Colors Based On Text Position
	glColor3f(1.0f*float(cos(counter1)),1.0f*float(sin(counter2)),1.0f-0.5f*float(cos(counter1+counter2)));
	
	glPrint(int((280+250*cos(counter1))),int(235+200*sin(counter2)),"1Aa",0);	// Print GL Text To The Screen

	glColor3f(1.0f*float(sin(counter2)),1.0f-0.5f*float(cos(counter1+counter2)),1.0f*float(cos(counter1)));
	glPrint(int((280+230*cos(counter2))),int(235+200*sin(counter1)),"1Aa",1);	// Print GL Text To The Screen
	
	glColor3f(0.0f,0.0f,1.0f);						// Set Color To Blue
	glPrint(int(240+200*cos((counter2+counter1)/5)),2,"Giuseppe D'Agata",0);	// Draw Text To The Screen

	glColor3f(1.0f,1.0f,1.0f);						// Set Color To White
	glPrint(int(242+200*cos((counter2+counter1)/5)),2,"Giuseppe D'Agata",0);	// Draw Offset Text To The Screen
	
	counter1 += 0.01f;								// Increase The First Counter
	counter2 += 0.0081f;							// Increase The Second Counter
	
	/*
	//Aalekh
	glPrint(400.0f, 100.0f, "asdasd", 0);
	*/

	return true;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch(uMsg) {
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
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}