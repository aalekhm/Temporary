
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

float spin;

GLfloat		rotTriangle;				// Angle For The Triangle ( NEW )
GLfloat		rotQuad;					// Angle For The Quad ( NEW )

GLfloat diffuseMaterial[4] = { 0.5, 0.5, 0.5, 1.0 };

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
					drawGLScene();
					SwapBuffers(mHDC);
				}
			}
			
			updateCamera();

			if(mKeys[VK_SPACE]) {
				spin = (spin + 0.5);// % 360;
			}

			if(mKeys[VK_F1]) {
				mKeys[VK_F1] = false;
				killGLWindow();
				mFULLSCREEN = !mFULLSCREEN;
				if(!createGLWindow("openGL window", SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, mFULLSCREEN)) {
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

int initGL(GLvoid) {
	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };

	glClearColor (0.0, 0.0, 0.0, 0.0);
	glShadeModel (GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseMaterial);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialf(GL_FRONT, GL_SHININESS, 25.0);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	
	glColorMaterial(GL_FRONT, GL_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

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

void draw3DTriangle(void) {
	glRotatef(rotTriangle, 0.0f, 1.0f, 0.0f);
	
	glBegin(GL_TRIANGLES);								// Drawing Using Triangles
		glColor3f(1.0f, 0.0f, 0.0f);					// Set The Color To Red
		glVertex3f( 0.0f, 1.0f, 0.0f);					// Top		
		glColor3f(0.0f, 1.0f, 0.0f);					// Set The Color To Green
		glVertex3f(-1.0f,-1.0f, 1.0f);					// Bottom Left
		glColor3f(0.0f, 0.0f, 1.0f);					// Set The Color To Blue
		glVertex3f( 1.0f,-1.0f, 1.0f);					// Bottom Right

		glColor3f(1.0f, 0.0f, 0.0f);					// Set The Color To Red
		glVertex3f( 0.0f, 1.0f, 0.0f);					// Top		
		glColor3f(0.0f, 0.0f, 1.0f);					// Set The Color To Green
		glVertex3f(1.0f,-1.0f, 1.0f);					// Bottom Left
		glColor3f(0.0f, 1.0f, 0.0f);					// Set The Color To Blue
		glVertex3f(1.0f,-1.0f,-1.0f);					// Bottom Right
	
		glColor3f(1.0f, 0.0f, 0.0f);					// Set The Color To Red
		glVertex3f( 0.0f, 1.0f, 0.0f);					// Top		
		glColor3f(0.0f, 1.0f, 0.0f);					// Set The Color To Green
		glVertex3f(1.0f,-1.0f,-1.0f);					// Bottom Left
		glColor3f(0.0f, 0.0f, 1.0f);					// Set The Color To Blue
		glVertex3f(-1.0f,-1.0f,-1.0f);					// Bottom Right

		glColor3f(1.0f, 0.0f, 0.0f);					// Set The Color To Red
		glVertex3f( 0.0f, 1.0f, 0.0f);					// Top		
		glColor3f(0.0f, 0.0f, 1.0f);					// Set The Color To Green
		glVertex3f(-1.0f,-1.0f,-1.0f);					// Bottom Left
		glColor3f(0.0f, 1.0f, 0.0f);					// Set The Color To Blue
		glVertex3f(-1.0f,-1.0f,1.0f);					// Bottom Right
	glEnd();											// Finished Drawing The Triangle
	
	rotTriangle += 0.1;
}

void draw3DCube() {
	//glLoadIdentity();
	glTranslatef(1.5f, 0.0f,-6.0f);						// Move Right 3 Units

	glRotatef(rotQuad, 1.0f, 1.0f, 1.0f);
	
	glColor3f(0.5f, 0.5f, 1.0f);
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
	
	rotQuad += 0.5;
}



int drawGLScene() {
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f (1.0, 1.0, 1.0);	
	glLoadIdentity();	
	moveCamera();

	glTranslatef(0.0, 0.0, -5.0);

	glColor4fv(diffuseMaterial);
	glutSolidSphere(1.0, 20, 16);

	return true;
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
		case WM_LBUTTONDOWN:
			diffuseMaterial[0] += 0.1;
			if (diffuseMaterial[0] > 1.0)
			   diffuseMaterial[0] = 0.0;
			//glColor4fv(diffuseMaterial);
			//glutPostRedisplay();
		break;
		case WM_MBUTTONDOWN:
			diffuseMaterial[1] += 0.1;
			if (diffuseMaterial[1] > 1.0)
			   diffuseMaterial[1] = 0.0;
			//glColor4fv(diffuseMaterial);
			//glutPostRedisplay();
		break;
		case WM_RBUTTONDOWN:
			diffuseMaterial[2] += 0.1;
			if (diffuseMaterial[2] > 1.0)
				diffuseMaterial[2] = 0.0;
			//glColor4fv(diffuseMaterial);
			//glutPostRedisplay();
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