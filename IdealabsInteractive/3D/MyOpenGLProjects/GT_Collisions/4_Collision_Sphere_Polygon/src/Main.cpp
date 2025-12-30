
#include "main.h"

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


/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *  		

// This	is the array of 3 vertices that will hold our triangle coordinates		
CVector3 vTriangle[3];

// This is the center of our sphere.  We are able to move it with the arrow keys.
CVector3 vPosition;

// This is the current rotate of our camera (F1 and F2 keys)
float rotateY = 0;

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

HDC			mHDC		= NULL;			// Private GDI Device Context
HGLRC		mHRC		= NULL;			// Permanent Rendering Context
HWND		mHWnd		= NULL;			// Holds Our Window Handle
HINSTANCE	mHInstance;					// Holds The Instance Of The Application

bool		mKeys[256];					// Array Used For The Keyboard Routine
bool		mActive = false;			// Window Active Flag
bool		mFULLSCREEN = true;			// Fullscreen Flag Set To TRUE By Default

int cameraRotX, cameraRotY, cameraRotZ;
float cameraPosX, cameraPosY, cameraPosZ;
const float piOver180 = 0.0174532925f;
const float MOVE_SPEED = 0.05f;
static bool L_MOUSE_DOWN = false;

GLuint *textures;

int glViewMode = GL_TRIANGLES;
float rotateFaceY = 0.0;
int CURRENT_FRAME_ID = 0;

bool g_RenderMode = false;

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
	else
	if(mKeys['I']) {
		vPosition.y += 0.01f;
	}
	else
	if(mKeys['K']) {
		vPosition.y -= 0.01f;
	}
	else
	if(mKeys['J']) {
		vPosition.x -= 0.01f;
	}
	else
	if(mKeys['L']) {
		vPosition.x += 0.01f;
	}
	else
	if(mKeys[VK_NEXT]) {
		vPosition.z += 0.01f;
	}
	else
	if(mKeys[VK_PRIOR]) {
		vPosition.z -= 0.01f;
	}

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

int initGL(GLvoid) {
	textures = new GLuint[MAX_TEXTURES];
	glGenTextures(MAX_TEXTURES, &textures[0]);

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
	
	// Here is where we define our triangle.  Remember it is important which
	// direction you define the vertices.  This is important because it allows
	// us to know which side the normal will be on when we calculate the triangle's
	// normal.  Though clockwise is more natural to me to use, more people use
	// counter-clockwise.  Also, if you enable back face culling in OpenGL, the
	// default is counter-clockwise (GL_CCW).  We start with the bottom-left
	// vertices, then the bottom-right and finally, the top vertex.
	vTriangle[0] = CVector3(-1,  0,   0);
	vTriangle[1] = CVector3( 1,  0,   0);
	vTriangle[2] = CVector3( 0,  1,   0);

	// We need an initial position for the sphere's center, so we place it
	// right in the middle of the triangle, initially colliding with it.
	vPosition = CVector3(0, 0.5f, 0);

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

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

	//glEnable(GL_CULL_FACE);								// Turn culling on
	//glCullFace(GL_FRONT);								// Quake2 uses front face culling apparently

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

	glTranslatef(0.0, -0.0, -5.0f);
	glRotatef(rotateY, 0, 1, 0);

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

	// We need a radius for our sphere, so let's create it here with 0.1
	float radius = 0.1f;

	glBegin(GL_TRIANGLES);
		
		glColor3ub(255, 0, 0);
		glVertex3f(	vTriangle[0].x, vTriangle[0].y, vTriangle[0].z);

		glColor3ub(0, 255, 0);
		glVertex3f(	vTriangle[1].x, vTriangle[1].y, vTriangle[1].z);

		glColor3ub(0, 0, 255);
		glVertex3f(	vTriangle[2].x, vTriangle[2].y, vTriangle[2].z);
	glEnd();

	// Instead of calculating the sphere ourselves, we are going to use quadrics.
	// Check out the tutorial on quadrics if this is confusing for you.

	// Allocate a quadric object to use as a sphere
	GLUquadricObj *pObj = gluNewQuadric();			// Get a Quadric off the stack

	// To make it easier to see, we want the sphere to be in wire frame
	gluQuadricDrawStyle(pObj, GLU_LINE);			// Draw the sphere normally

	// Move the sphere to it's center position
	glTranslatef(vPosition.x, vPosition.y, vPosition.z);
	
	// Now we get to the wonderful function that does it all for us.  All we
	// need to do is pass in the array of vertices for the triangle, the center
	// of the sphere and it's radius.  This will return a true or false, depending
	// on if we collided or not.  True = The Sphere Collided
	bool bCollided = spherePolygonCollision(vTriangle, vPosition, 3, radius);

	// If we collided we want the sphere to be green, otherwise it should be purple
	if(bCollided)
		glColor3ub(255, 0, 0);
	else
		glColor3ub(0, 255, 0);

	// Draw the quadric as a sphere with the radius of .1 and a 15 by 15 detail.
	// To increase the detail of the sphere, just increase the 2 last parameters.
	gluSphere(pObj, radius, 15, 15);
	
	// Free the Quadric
	gluDeleteQuadric(pObj);

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

//char logText[255] = {0};
//sprintf(logText, "vNormal = %f %f %f", vNormal.x, vNormal.y, vNormal.z);
//MessageBox(NULL, logText, "logText", MB_OK);


	//rotateFaceY = (float)(((UINT)rotateFaceY + 1) % 360);

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

			//glViewMode = (glViewMode == GL_TRIANGLES)?GL_LINES:GL_TRIANGLES;
		}
		break;
		case WM_RBUTTONDOWN:
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
