
#include <windows.h>					// Header File For Windows
#include <gl\gl.h>						// Header File For The OpenGL32 Library
#include <gl\glu.h>						// Header File For The GLu32 Library

HDC			mHDC		= NULL;			// Private GDI Device Context
HGLRC		mHRC		= NULL;			// Permanent Rendering Context
HWND		mHWnd	= NULL;				// Holds Our Window Handle
HINSTANCE	mHInstance;					// Holds The Instance Of The Application

bool		mKeys[256];					// Array Used For The Keyboard Routine
bool		mActive = false;			// Window Active Flag
bool		mFULLSCREEN = true;			// Fullscreen Flag Set To TRUE By Default

const GLuint MAX_PARTICLES = 1000;		// Number Of Particles To Create	( NEW )
bool		rainbow = true;				// Rainbow Mode?					( ADD )
bool		spacePressed;				// Spacebar Pressed?				( ADD )
bool		returnPressed;				// Return Pressed?					( ADD )

float		slowDown = 2.0f;			// Slow Down Particles									( ADD )
float		xSpeed;						// Base X Speed (To Allow Keyboard Direction Of Tail)	( ADD )
float		ySpeed;						// Base Y Speed (To Allow Keyboard Direction Of Tail)	( ADD )
float		zoom = -40.0f;				// Used To Zoom Out										( ADD )

GLuint		loop;						// Misc Loop Variable
GLuint		particleColour;				// Current Color Selection
GLuint		delay;						// Rainbow Effect Delay
GLuint		texture[1];					// Storage For Our Particle Texture

typedef struct {						// Create A Structure For Particle

	bool	active;						// Active (Yes/No)
	float	life;						// Particle Life
	float	fade;						// Fade Speed
	
	float	r;							// Red Value
	float	g;							// Green Value
	float	b;							// Blue Value

	float	x;							// X Position
	float	y;							// Y Position
	float	z;							// Z Position

	float	xi;							// X Direction
	float	yi;							// Y Direction
	float	zi;							// Z Direction

	float	xg;							// X Gravity
	float	yg;							// Y Gravity
	float	zg;							// Z Gravity

}particles;								// Particles Structure

particles	particle[MAX_PARTICLES];	// Particle Array (Room For Particle Info)
	
static GLfloat colors[12][3] = {
									{1.0f,0.5f,0.5f},{1.0f,0.75f,0.5f},{1.0f,1.0f,0.5f},{0.75f,1.0f,0.5f},
									{0.5f,1.0f,0.5f},{0.5f,1.0f,0.75f},{0.5f,1.0f,1.0f},{0.5f,0.75f,1.0f},
									{0.5f,0.5f,1.0f},{0.75f,0.5f,1.0f},{1.0f,0.5f,1.0f},{1.0f,0.5f,0.75f}
								};

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
GLvoid	resizeGLScene(GLsizei width, GLsizei height);
int initGL(GLvoid);
int drawGLScene(GLvoid);
GLvoid killGLWindow(GLvoid);
bool createGLWindow(char* title, int width, int height, int bits, bool isFullScreen);

bool NeheLoadTexture(LPSTR szFileName, GLuint &texId) {			// Creates Texture From A Bitmap File
	HBITMAP	hBMP;													// Handle Of The Bitmap
	BITMAP	BMP;													// Bitmap Structure

	hBMP = (HBITMAP)LoadImage(	GetModuleHandle(NULL),				
								szFileName, 
								IMAGE_BITMAP, 
								0, 
								0, 
								LR_CREATEDIBSECTION | LR_LOADFROMFILE
							);

	if(!hBMP)														// Does The Bitmap Exist?
		return false;												// If Not Return False

	GetObject(hBMP, sizeof(BMP), &BMP);								// Get The Object

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);							// Pixel Storage Mode (Word Alignment / 4 Bytes)
	
	// Typical Texture Generation Using Data From The Bitmap
	glBindTexture(GL_TEXTURE_2D, texId);								// Bind To The Texture ID
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// Linear Min Filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// Linear Mag Filter
	
	glTexImage2D(GL_TEXTURE_2D, 0, 3, BMP.bmWidth, BMP.bmHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, BMP.bmBits);

	DeleteObject(hBMP);													// Delete The Object

	return true;														// Loading Was Successful
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

					if (mKeys[VK_ADD] && slowDown > 1.0f) 
						slowDown -= 0.01f;		// Speed Up Particles

					if (mKeys[VK_SUBTRACT] && slowDown < 4.0f)
						slowDown += 0.01f;	// Slow Down Particles

					if (mKeys[VK_PRIOR]) 
						zoom += 0.1f;		// Zoom In
					
					if (mKeys[VK_NEXT]) 
						zoom -= 0.1f;		// Zoom Out

					if (mKeys[VK_RETURN] && !returnPressed)		// Return Key Pressed
					{
						returnPressed = true;			// Set Flag Telling Us It's Pressed
						rainbow = !rainbow;		// Toggle Rainbow Mode On / Off
					}

					if (!mKeys[VK_RETURN]) 
						returnPressed = false;		// If Return Is Released Clear Flag
					
					if ((mKeys[' '] && !spacePressed) || (rainbow && (delay > 25)))	// Space Or Rainbow Mode
					{
						if (mKeys[' ']) 
							rainbow=false;	// If Spacebar Is Pressed Disable Rainbow Mode
						spacePressed = true;			// Set Flag Telling Us Space Is Pressed
						
						delay=0;			// Reset The Rainbow Color Cycling Delay
						particleColour++;				// Change The Particle Color
						if (particleColour > 11) 
							particleColour = 0;		// If Color Is To High Reset It
					}
					
					if (!mKeys[' '])
						spacePressed = false;		// If Spacebar Is Released Clear Flag
					
					// If Up Arrow And Y Speed Is Less Than 200 Increase Upward Speed
					if (mKeys[VK_UP] && ySpeed < 200) 
						ySpeed+=1.0f;
					
					// If Down Arrow And Y Speed Is Greater Than -200 Increase Downward Speed
					if (mKeys[VK_DOWN] && ySpeed > -200) 
						ySpeed -= 1.0f;
					
					// If Right Arrow And X Speed Is Less Than 200 Increase Speed To The Right
					if (mKeys[VK_RIGHT] && xSpeed < 200) 
						xSpeed += 1.0f;
					
					// If Left Arrow And X Speed Is Greater Than -200 Increase Speed To The Left
					if (mKeys[VK_LEFT] && xSpeed > -200) 
						xSpeed -= 1.0f;
					
					delay++;			// Increase Rainbow Mode Color Cycling Delay Counter
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

	gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 200.0f);	//( MODIFIED )

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

int initGL(GLvoid) {
	if(!NeheLoadTexture("data/Particle.bmp", texture[0]))
		return false;

	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClearDepth(1.0f);
	
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping
	glBindTexture(GL_TEXTURE_2D, texture[0]);			// Select Our Texture
	
	for(loop = 0; loop < MAX_PARTICLES; loop++) {					// Initialize All The Textures
		particle[loop].active = true;								// Make All The Particles Active
		particle[loop].life = 1.0f;									// Give All The Particles Full Life

		particle[loop].fade = float(rand()%100/1000.0f + 0.03f);	// Random Fade Speed
		
		particle[loop].r = colors[ loop * (12/MAX_PARTICLES) ][0];		// Select Red Rainbow Color
		particle[loop].g = colors[ loop * (12/MAX_PARTICLES) ][1];		// Select Green Rainbow Color
		particle[loop].b = colors[ loop * (12/MAX_PARTICLES) ][2];		// Select Blue Rainbow Color

		particle[loop].xi = float( rand()%50 - 26.0f ) * 10.0f;			// Random Speed On X Axis
		particle[loop].yi = float( rand()%50 - 25.0f ) * 10.0f;			// Random Speed On Y Axis
		particle[loop].zi = float( rand()%50 - 25.0f ) * 10.0f;			// Random Speed On Z Axis

		particle[loop].xg = 0.0f;									// Set Horizontal Pull To Zero
		particle[loop].yg = -0.8f;									// Set Vertical Pull Downward
		particle[loop].zg = 0.0f;									// Set Pull On Z Axis To Zero
	}

	return true;												// Initialization Went OK
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

int drawGLScene_() {													// Where We Do All The Drawing
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);				// Clear Screen And Depth Buffer
	glLoadIdentity();												// Reset The ModelView Matrix
	
	for(loop = 0; loop < MAX_PARTICLES; loop++) {					// Loop Through All The Particles
		if(particle[loop].active) {									// If The Particle Is Active
			
			//x, y and z are temporary variables that we'll use to hold the particles x, y and z position.
			float x = particle[loop].x;								// Grab Our Particle X Position
			float y = particle[loop].y;								// Grab Our Particle Y Position
			float z = particle[loop].z + zoom;						// Particle Z Pos + Zoom

			// Draw The Particle Using Our RGB Values, Fade The Particle Based On It's Life
			glColor4f(particle[loop].r, particle[loop].g, particle[loop].b, particle[loop].life);
			
			//All that we have to do now is draw our particle.
			glBegin(GL_TRIANGLE_STRIP);										// Build Quad From A Triangle Strip
				glTexCoord2d(1, 1);		glVertex3f(x + 0.5f, y + 0.5f, z);	// Top Right
				glTexCoord2d(0, 1);		glVertex3f(x - 0.5f, y + 0.5f, z);	// Top Left
				glTexCoord2d(1, 0);		glVertex3f(x + 0.5f, y - 0.5f, z);	// Bottom Right
				glTexCoord2d(0, 0);		glVertex3f(x - 0.5f, y - 0.5f, z);	// Bottom Left
			glEnd();														// Done Building Triangle Strip
			
			//Now we can move the particle.
			particle[loop].x += particle[loop].xi/(slowDown*1000);			// Move On The X Axis By X Speed
			particle[loop].y += particle[loop].yi/(slowDown*1000);			// Move On The Y Axis By Y Speed
			particle[loop].z += particle[loop].zi/(slowDown*1000);			// Move On The Z Axis By Z Speed
			
			//The resistance is applied to the y and z moving speed the same way it's applied to the x moving speed. 
			particle[loop].xi += particle[loop].xg;							// Take Pull On X Axis Into Account
			particle[loop].yi += particle[loop].yg;							// Take Pull On Y Axis Into Account
			particle[loop].zi += particle[loop].zg;							// Take Pull On Z Axis Into Account
			
			//Takes some life away from the particle.
			particle[loop].life -= particle[loop].fade;						// Reduce Particles Life By 'Fade'

			//If the particle is dead (burnt out), we'll rejuvenate it. 
			//We do this by giving it full life and a new fade speed. 
			if(particle[loop].life < 0.0f) {								// If Particle Is Burned Out
				particle[loop].life = 1.0f;									// Give It New Life
				particle[loop].fade = float(rand()%100/1000.0f) + 0.03f;	// Random Fade Value

				particle[loop].x = 0;										// Center On X Axis
				particle[loop].y = 0;										// Center On Y Axis
				particle[loop].z = 0;										// Center On Z Axis
	
				particle[loop].xi = xSpeed + float(rand()%60 - 32.0f);		// X Axis Speed And Direction
				particle[loop].yi = ySpeed + float(rand()%60 - 32.0f);		// Y Axis Speed And Direction
				particle[loop].zi = float(rand()%60 - 32.0f);				// Z Axis Speed And Direction

				particle[loop].r = colors[particleColour][0];				// Select Red From Color Table
				particle[loop].g = colors[particleColour][1];				// Select Green From Color Table
				particle[loop].b = colors[particleColour][2];				// Select Blue From Color Table
			}

			// If Number Pad 8 And Y Gravity Is Less Than 1.5 Increase Pull Upwards
			if(mKeys[VK_NUMPAD8] && particle[loop].yg < 1.5f)
				particle[loop].yg += 0.01f;

			// If Number Pad 2 And Y Gravity Is Greater Than -1.5 Increase Pull Downwards
			if(mKeys[VK_NUMPAD2] && particle[loop].yg > -1.5f)
				particle[loop].yg -= 0.01f;

			// If Number Pad 6 And X Gravity Is Less Than 1.5 Increase Pull Right
			if(mKeys[VK_NUMPAD6] && particle[loop].xg < 1.5f)
				particle[loop].xg += 0.01f;
			
			// If Number Pad 4 And X Gravity Is Greater Than -1.5 Increase Pull Left
			if (mKeys[VK_NUMPAD4] && particle[loop].xg > -1.5f) 
				particle[loop].xg -= 0.01f;

			if(mKeys[VK_TAB]) {										// Tab Key Causes A Burst

				particle[loop].x = 0.0f;							// Center On X Axis
				particle[loop].y = 0.0f;							// Center On Y Axis
				particle[loop].z = 0.0f;							// Center On Z Axis

				particle[loop].xi = float((rand()%50)-26.0f)*10.0f;	// Random Speed On X Axis
				particle[loop].yi = float((rand()%50)-26.0f)*10.0f;	// Random Speed On Y Axis
				particle[loop].zi = float((rand()%50)-26.0f)*10.0f;	// Random Speed On Z Axis
			}
		}
	}


	return true;
}

int drawGLScene()										// Here's Where We Do All The Drawing
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear Screen And Depth Buffer
	glLoadIdentity();										// Reset The ModelView Matrix

	for (loop=0;loop<MAX_PARTICLES;loop++)					// Loop Through All The Particles
	{
		if (particle[loop].active)							// If The Particle Is Active
		{
			float x=particle[loop].x;						// Grab Our Particle X Position
			float y=particle[loop].y;						// Grab Our Particle Y Position
			float z=particle[loop].z+zoom;					// Particle Z Pos + Zoom

			// Draw The Particle Using Our RGB Values, Fade The Particle Based On It's Life
			glColor4f(particle[loop].r,particle[loop].g,particle[loop].b,particle[loop].life);

			glBegin(GL_TRIANGLE_STRIP);						// Build Quad From A Triangle Strip
			    glTexCoord2d(1,1); glVertex3f(x+0.5f,y+0.5f,z); // Top Right
				glTexCoord2d(0,1); glVertex3f(x-0.5f,y+0.5f,z); // Top Left
				glTexCoord2d(1,0); glVertex3f(x+0.5f,y-0.5f,z); // Bottom Right
				glTexCoord2d(0,0); glVertex3f(x-0.5f,y-0.5f,z); // Bottom Left
			glEnd();										// Done Building Triangle Strip

			particle[loop].x+=particle[loop].xi/(slowDown*1000);// Move On The X Axis By X Speed
			particle[loop].y+=particle[loop].yi/(slowDown*1000);// Move On The Y Axis By Y Speed
			particle[loop].z+=particle[loop].zi/(slowDown*1000);// Move On The Z Axis By Z Speed

			particle[loop].xi+=particle[loop].xg;			// Take Pull On X Axis Into Account
			particle[loop].yi+=particle[loop].yg;			// Take Pull On Y Axis Into Account
			particle[loop].zi+=particle[loop].zg;			// Take Pull On Z Axis Into Account
			particle[loop].life-=particle[loop].fade;		// Reduce Particles Life By 'Fade'

			if (particle[loop].life<0.0f)					// If Particle Is Burned Out
			{
				particle[loop].life=1.0f;					// Give It New Life
				particle[loop].fade=float(rand()%100)/1000.0f+0.003f;	// Random Fade Value
				particle[loop].x=0.0f;						// Center On X Axis
				particle[loop].y=0.0f;						// Center On Y Axis
				particle[loop].z=0.0f;						// Center On Z Axis
				particle[loop].xi=xSpeed+float((rand()%60)-32.0f);	// X Axis Speed And Direction
				particle[loop].yi=ySpeed+float((rand()%60)-30.0f);	// Y Axis Speed And Direction
				particle[loop].zi=float((rand()%60)-30.0f);	// Z Axis Speed And Direction
				particle[loop].r=colors[particleColour][0];			// Select Red From Color Table
				particle[loop].g=colors[particleColour][1];			// Select Green From Color Table
				particle[loop].b=colors[particleColour][2];			// Select Blue From Color Table
			}

			// If Number Pad 8 And Y Gravity Is Less Than 1.5 Increase Pull Upwards
			if (mKeys[VK_NUMPAD8] && (particle[loop].yg<1.5f)) particle[loop].yg+=0.01f;

			// If Number Pad 2 And Y Gravity Is Greater Than -1.5 Increase Pull Downwards
			if (mKeys[VK_NUMPAD2] && (particle[loop].yg>-1.5f)) particle[loop].yg-=0.01f;

			// If Number Pad 6 And X Gravity Is Less Than 1.5 Increase Pull Right
			if (mKeys[VK_NUMPAD6] && (particle[loop].xg<1.5f)) particle[loop].xg+=0.01f;

			// If Number Pad 4 And X Gravity Is Greater Than -1.5 Increase Pull Left
			if (mKeys[VK_NUMPAD4] && (particle[loop].xg>-1.5f)) particle[loop].xg-=0.01f;

			if (mKeys[VK_TAB])										// Tab Key Causes A Burst
			{
				particle[loop].x=0.0f;								// Center On X Axis
				particle[loop].y=0.0f;								// Center On Y Axis
				particle[loop].z=0.0f;								// Center On Z Axis
				particle[loop].xi=float((rand()%50)-26.0f)*10.0f;	// Random Speed On X Axis
				particle[loop].yi=float((rand()%50)-25.0f)*10.0f;	// Random Speed On Y Axis
				particle[loop].zi=float((rand()%50)-25.0f)*10.0f;	// Random Speed On Z Axis
			}
		}
    }
	return TRUE;											// Everything Went OK
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