#define SCREEN_WIDTH	1024
#define SCREEN_HEIGHT	768

#include <windows.h>					// Header File For Windows
#include <gl\gl.h>						// Header File For The OpenGL32 Library
#include <gl\glu.h>						// Header File For The GLu32 Library
#include <Matrices.h>
#include <timer.h>
#include <Defines.h>
#include <tga.h>
#include <teapot.h>
#include <cameraSimple.h>
#include <GameWorld.h>
#include <Vehicle.h>

HDC			mHDC		= NULL;			// Private GDI Device Context
HGLRC		mHRC		= NULL;			// Permanent Rendering Context
HWND		mHWnd	= NULL;				// Holds Our Window Handle
HINSTANCE	mHInstance;					// Holds The Instance Of The Application

bool		mKeys[256];					// Array Used For The Keyboard Routine
bool		mActive = false;			// Window Active Flag
bool		mFULLSCREEN = true;			// Fullscreen Flag Set To TRUE By Default
int			mMouseX;
int			mMouseY;

//////////////////////////////////////////////////////////
GLuint					_texture[2];				// Storage For Our Font Texture
GLuint					_textureWH[2][2];			// W/H Storage For Our Font Texture
VertexT2F_C4UB_V3F*		m_VB;
int						m_SpriteCount;
int						m_FrameCount;
RectF*					m_ClipRect;
ColorUV*				m_ColorUVTable;

const float		WALK_SPEED = 0.001f;//1 unit/sec = 1/1000 ms
const float		ROTATE_SPEED = 0.1f;//100 Degrees/sec = 100/1000 ms
bool			L_MOUSE_DOWN;
bool			R_MOUSE_DOWN;

////////////////////////////// PERSPECTIVE VIEW CONSTANTS
// constants
const float DEG2RAD = 3.141593f / 180;
const float FOV_Y = 60.0f;              // vertical FOV in degree
const float NEAR_PLANE = 1.0f;
const float FAR_PLANE = 100.0f;
const float CAMERA_ANGLE_X = 45.0f;     // pitch in degree
const float CAMERA_ANGLE_Y = -45.0f;    // heading in degree
const float CAMERA_DISTANCE = 25.0f;    // camera distance
////////////////////////////////////////////////////////////

GameWorld* m_pGameWorld;

Timer*	m_Timer;
double	m_ElapsedTimeMs;
double	m_LastElapsedTimeMs;
double	m_DeltaTimeMs;
double	m_ElapasedGameTimeMs;

Vector3 cameraPosition;
Vector3 cameraAngle;
Vector3 cameraPosition3rdPerson;
Vector3 cameraAngle3rdPerson;
int		cameraDistance3rdPerson;

Vector3 modelPosition;
Vector3 modelAngle;
float bgColor[3];

// 4x4 transform matrices
Matrix4 matrixView;
Matrix4 matrixModel;
Matrix4 matrixModelView;
Matrix4 matrixProjection;

Matrix4 matrixView3rdPerson;
Matrix4 matrixModelView3rdPerson;

Matrix4 matrixViewRearView;
Matrix4 matrixModelViewRearView;

Matrix4 matrixModelView2D;
Matrix4 matrixModel2D;
//////////////////////////////////////////////////////////

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
GLvoid	resizeGLScene(GLsizei width, GLsizei height);
void setViewportSub(int x, int y, int width, int height, float nearPlane, float farPlane);
int initGL(GLvoid);
bool initData(GLvoid);
void initModel();
void initCamera();
void setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ);
void updateModelMatrix();
void updateViewMatrix();
int drawGLScene(GLvoid);
void drawLine(float x1, float y1, float x2, float y2, float r, float g, float b);
GLvoid killGLWindow(GLvoid);
bool createGLWindow(char* title, int width, int height, int bits, bool isFullScreen);
void setMousePosition(int mouseX, int mouseY);
void mouseMove(WPARAM keyState, int x, int y);

void			drawStringFont(const char* cStr, int x, int y, int anchor);
void			drawFont(int x, int y, int charW, int charH, int tX, int tY);
void			drawQuadU(	int TEXTURE_ID, 
							float posX, float posY, float posW, float posH,
							float texX, float texY, float texW, float texH);
void			drawQuadU(	int TEXTURE_ID, 
							std::vector<Vector2> vVertices,
							float texX, float texY, float texW, float texH);
void			setClip(int x, int y, int w, int h );
void			drawVBO();
void			fillRect(u32 COLOR_ID, Rect* rect);
void			setupOrthogonalProjection(double x, double y, double w, double h);

int WINAPI WinMain(	HINSTANCE	hInstance, 
					HINSTANCE	hPrevInstance,
					LPSTR		lpCmdLine,
					int			nCmdShow
) {
	MSG		msg;
	bool	done = false;

	//if(MessageBox(NULL, "Would you like to run in FULLSCREEN mode?", "Start FullScreen", MB_YESNO) == IDNO)
		mFULLSCREEN = false;

	if(!createGLWindow("openGL Window...Press F1 to toogle between windowed & Fullscreen Mode.", SCREEN_WIDTH, SCREEN_HEIGHT, 16, mFULLSCREEN))
		return 0;

	m_ElapsedTimeMs = m_LastElapsedTimeMs = m_Timer->getElapsedTimeInMilliSec();
	m_ElapasedGameTimeMs = 0;

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
					m_LastElapsedTimeMs = m_ElapsedTimeMs;
					m_ElapsedTimeMs = m_Timer->getElapsedTimeInMilliSec();
					m_DeltaTimeMs = m_ElapsedTimeMs - m_LastElapsedTimeMs;

					drawGLScene();
					SwapBuffers(mHDC);

					m_ElapasedGameTimeMs += m_DeltaTimeMs;
				}
			}

			if(mKeys[VK_F1]) {
				mKeys[VK_F1] = false;
				killGLWindow();
				mFULLSCREEN = !mFULLSCREEN;
				if(!createGLWindow("NeHe's First Polygon Tutorial", SCREEN_WIDTH, SCREEN_HEIGHT, 16, mFULLSCREEN)) {
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

	//resizeGLScene(width, height);
	// set viewport
	setViewportSub(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.1f, 10.0f);

	if(	!initData()) {
		killGLWindow();								// Reset The Display
		MessageBox(NULL,"Error Loading Data.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;								// Return FALSE
	}

	if(	!initGL()) {
		killGLWindow();								// Reset The Display
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;								// Return FALSE
	}

	m_Timer = new Timer();
	
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

bool loadTexture(char* sTexName, GLuint &texid, GLuint* texWH) {
	TGAImg Img;        // Image loader

	// Load our Texture
	if(Img.Load(sTexName) != IMG_OK)
		return false;

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texid); // Set our Tex handle as current

	// Specify filtering and edge actions
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);

	// Create the texture
	if(Img.GetBPP() == 24) {
		glTexImage2D(GL_TEXTURE_2D, 0, 3, Img.GetWidth(), Img.GetHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, Img.GetImg());
	}
	else 
	if(Img.GetBPP() == 32){
		glTexImage2D(GL_TEXTURE_2D, 0, 4, Img.GetWidth(), Img.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, Img.GetImg());
	}
	else
		return false;

	texWH[0] = Img.GetWidth();
	texWH[1] = Img.GetHeight();
	glDisable(GL_TEXTURE_2D);

	return true;
}

bool initData()
{
	if(!loadTexture("data/core.tga", _texture[TEXTURE_CORE], _textureWH[TEXTURE_CORE]))
		return FALSE;

	m_ClipRect = new RectF(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	m_ColorUVTable = new ColorUV[9];
	m_ColorUVTable[COLOR_BLACK].u = 675;	m_ColorUVTable[COLOR_BLACK].v = 55;
	m_ColorUVTable[COLOR_WHITE].u = 675;	m_ColorUVTable[COLOR_WHITE].v = 65;
	m_ColorUVTable[COLOR_RED].u = 675;		m_ColorUVTable[COLOR_RED].v = 149;//98;
	m_ColorUVTable[COLOR_GREEN].u = 675;	m_ColorUVTable[COLOR_GREEN].v = 161;//87;
	m_ColorUVTable[COLOR_BLUE].u = 675;		m_ColorUVTable[COLOR_BLUE].v = 173;//75;
	m_ColorUVTable[COLOR_GREY].u = 57;		m_ColorUVTable[COLOR_GREY].v = 8;
	m_ColorUVTable[COLOR_PINK].u = 675;		m_ColorUVTable[COLOR_PINK].v = 111;
	m_ColorUVTable[COLOR_CYAN].u = 675;		m_ColorUVTable[COLOR_CYAN].v = 123;
	m_ColorUVTable[COLOR_YELLOW].u = 675;	m_ColorUVTable[COLOR_YELLOW].v = 135;

	m_VB = new VertexT2F_C4UB_V3F[SPR_MAX * 4];

	m_pGameWorld = new GameWorld(SCREEN_WIDTH>>1, SCREEN_HEIGHT>>1, SCREEN_WIDTH>>1, SCREEN_HEIGHT>>1);
	Vehicle* vVehicle = NULL;
	vVehicle = new Vehicle(	Vector2((SCREEN_WIDTH>>1) + 100, (SCREEN_HEIGHT>>1) + 100),
							150.0f,
							400.0f,
							1.0f,
							Vehicle::Summing_method::weighted_average
						);
	vVehicle->setSteerBehaviour(Vehicle::STEERING_BEHAVIOUR::SEEK);
	m_pGameWorld->addVehicle(vVehicle);

	vVehicle = new Vehicle(	Vector2((SCREEN_WIDTH>>1) + 100, (SCREEN_HEIGHT>>1) + 150),
							150.0f,
							400.0f,
							1.0f,
							Vehicle::Summing_method::weighted_average
						);
	vVehicle->setSteerBehaviour(Vehicle::STEERING_BEHAVIOUR::ARRIVE);
	m_pGameWorld->addVehicle(vVehicle);

	vVehicle = new Vehicle(	Vector2((SCREEN_WIDTH>>1) + 200, (SCREEN_HEIGHT>>1) + 200),
							150.0f,
							400.0f,
							1.0f,
							Vehicle::Summing_method::weighted_average
						);
	vVehicle->setSteerBehaviour(Vehicle::STEERING_BEHAVIOUR::FLEE);
	m_pGameWorld->addVehicle(vVehicle);

	Vehicle* vVehicleTarget = vVehicle;
	vVehicle = new Vehicle(	Vector2((SCREEN_WIDTH>>1) + 200, (SCREEN_HEIGHT>>1) + 150),
							150.0f,
							400.0f,
							1.0f,
							Vehicle::Summing_method::weighted_average
						);
	vVehicle->setSteerBehaviour(Vehicle::STEERING_BEHAVIOUR::PURSUIT);
	vVehicle->m_pTargetAgent1 = vVehicleTarget;
	m_pGameWorld->addVehicle(vVehicle);

	vVehicle = new Vehicle(	Vector2((SCREEN_WIDTH>>1) + 20, (SCREEN_HEIGHT>>1) + 150),
							150.0f,
							400.0f,
							1.0f,
							Vehicle::Summing_method::weighted_average
							);
	vVehicle->setSteerBehaviour(Vehicle::STEERING_BEHAVIOUR::EVADE);
	vVehicle->m_pTargetAgent1 = vVehicleTarget;
	m_pGameWorld->addVehicle(vVehicle);

	vVehicle = new Vehicle(	Vector2((SCREEN_WIDTH>>1) + 75, (SCREEN_HEIGHT>>1) + 75),
							150.0f,
							400.0f,
							1.0f,
							Vehicle::Summing_method::weighted_average
							);
	vVehicle->setSteerBehaviour(Vehicle::STEERING_BEHAVIOUR::WANDER);
	m_pGameWorld->addVehicle(vVehicle);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// initialize lights
///////////////////////////////////////////////////////////////////////////////
void initLights()
{
	glEnable(GL_LIGHTING);

	// set up light colors (ambient, diffuse, specular)
	GLfloat lightKa[] = {.0f, .0f, .0f, 1.0f};      // ambient light
	GLfloat lightKd[] = {.9f, .9f, .9f, 1.0f};      // diffuse light
	GLfloat lightKs[] = {1, 1, 1, 1};               // specular light
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightKa);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightKd);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightKs);

	// position the light in eye space
	float lightPos[4] = {0, 1, 1, 0};               // directional light
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

	glEnable(GL_LIGHT0);                            // MUST enable each light source after configuration
}

void initCamera()
{
	cameraPosition.x = cameraPosition.y = cameraPosition.z = 0;
	cameraAngle.x = cameraAngle.y = cameraAngle.z = 0;

	matrixView.identity();
	matrixProjection.identity();

	//cameraPosition.x = 0.0f;
	//cameraPosition.y = 3.0f;
	//cameraPosition.z = 0.0f;
	//setCamera(cameraPosition.x, cameraPosition.y, cameraPosition.z, 0.0f, -2.0f, -1.0f);

	cameraPosition3rdPerson.x = 0.0f;
	cameraPosition3rdPerson.y = 0.0f;
	cameraPosition3rdPerson.z = 30.0f;

	cameraAngle3rdPerson.x = -45.0f;
	cameraAngle3rdPerson.y = 45.0f;
	cameraAngle3rdPerson.z = 0.0f;
}

void initModel()
{
	modelPosition.x = modelPosition.y = modelPosition.z = 0;
	modelAngle.x = modelAngle.y = modelAngle.z = 0;
	
	bgColor[0] = bgColor[1] = bgColor[2] = bgColor[3] = 0;

	matrixModel.identity();
	matrixModelView.identity();

	modelPosition.x = modelPosition.y = 0.0f;
	modelPosition.z = -5.0f;
}

int initGL(GLvoid) {
	glShadeModel(GL_SMOOTH);                        // shading mathod: GL_SMOOTH or GL_FLAT
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);          // 4-byte pixel alignment

	// enable/disable features
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	//glEnable(GL_SCISSOR_TEST);

	// track material ambient and diffuse from surface color, call it before glEnable(GL_COLOR_MATERIAL)
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);   // background color
	glClearStencil(0);                              // clear stencil buffer
	glClearDepth(1.0f);                             // 0 is near, 1 is far
	glDepthFunc(GL_LEQUAL);

	initLights();
	initModel();
	initCamera();
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

///////////////////////////////////////////////////////////////////////////////
// set a orthographic frustum with 6 params similar to glOrtho()
// (left, right, bottom, top, near, far)
// Note: this is for row-major notation. OpenGL needs transpose it
///////////////////////////////////////////////////////////////////////////////
void setOrthogonalFrustum(float l, float r, float b, float t, float n, float f)
{
	matrixProjection.identity();
	matrixProjection[0]  =  2 / (r - l);
	matrixProjection[3]  =  -(r + l) / (r - l);
	matrixProjection[5]  =  2 / (t - b);
	matrixProjection[7]  =  -(t + b) / (t - b);
	matrixProjection[10] = -2 / (f - n);
	matrixProjection[11] = -(f + n) / (f - n);
}

///////////////////////////////////////////////////////////////////////////////
// set a perspective frustum with 6 params similar to glFrustum()
// (left, right, bottom, top, near, far)
// Note: this is for row-major notation. OpenGL needs transpose it
///////////////////////////////////////////////////////////////////////////////
void setPerspectiveFrustum(float l, float r, float b, float t, float n, float f)
{
	matrixProjection.identity();
	matrixProjection[0]  =  2 * n / (r - l);
	matrixProjection[2]  =  (r + l) / (r - l);
	matrixProjection[5]  =  2 * n / (t - b);
	matrixProjection[6]  =  (t + b) / (t - b);
	matrixProjection[10] = -(f + n) / (f - n);
	matrixProjection[11] = -(2 * f * n) / (f - n);
	matrixProjection[14] = -1;
	matrixProjection[15] =  0;
}

///////////////////////////////////////////////////////////////////////////////
// set a symmetric perspective frustum with 4 params similar to gluPerspective
// (vertical field of view, aspect ratio, near, far)
///////////////////////////////////////////////////////////////////////////////
void setFrustum(float fovY, float aspectRatio, float front, float back)
{
	float tangent = tanf(fovY/2 * DEG2RAD);   // tangent of half fovY
	float height = front * tangent;           // half height of near plane
	float width = height * aspectRatio;       // half width of near plane

	// params: left, right, bottom, top, near, far
	setPerspectiveFrustum(-width, width, -height, height, front, back);
}

///////////////////////////////////////////////////////////////////////////////
// configure projection and viewport of sub window
///////////////////////////////////////////////////////////////////////////////
void setViewportSub(int x, int y, int width, int height, float nearPlane, float farPlane)
{
	// set viewport
	glViewport(x, y, width, height);
	//glScissor(x, y, width, height);
	//setClip(x, y, width, height);

	// set perspective viewing frustum
	setFrustum(FOV_Y, (float)(width)/height, nearPlane, farPlane); // FOV, AspectRatio, NearClip, FarClip

	// copy projection matrix to OpenGL
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(matrixProjection.getTranspose());
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

///////////////////////////////////////////////////////////////////////////////
// set the object position and rotation
///////////////////////////////////////////////////////////////////////////////
void setModelMatrix(float x, float y, float z, float rx, float ry, float rz)
{
	modelPosition[0] = x;
	modelPosition[1] = y;
	modelPosition[2] = z;
	modelAngle[0] = rx;
	modelAngle[1] = ry;
	modelAngle[2] = rz;

	updateModelMatrix();
}

///////////////////////////////////////////////////////////////////////////////
// set the camera position and rotation
///////////////////////////////////////////////////////////////////////////////
void setViewMatrix(float x, float y, float z, float pitch, float heading, float roll)
{
	cameraPosition[0] = x;
	cameraPosition[1] = y;
	cameraPosition[2] = z;
	cameraAngle[0] = pitch;
	cameraAngle[1] = heading;
	cameraAngle[2] = roll;

	updateViewMatrix();
}

///////////////////////////////////////////////////////////////////////////////
// set camera position and lookat direction
///////////////////////////////////////////////////////////////////////////////
void setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ)
{
	float forward[4];
	float up[4];
	float left[4];
	float position[4];
	float invLength;

	// determine forward vector (direction reversed because it is camera)
	forward[0] = -(posX - targetX);    // x
	forward[1] = -(posY - targetY);    // y
	forward[2] = -(posZ - targetZ);    // z
	forward[3] = 0.0f;              // w
	// normalize it without w-component
	invLength = 1.0f / sqrtf(forward[0]*forward[0] + forward[1]*forward[1] + forward[2]*forward[2]);
	forward[0] *= invLength;
	forward[1] *= invLength;
	forward[2] *= invLength;

	// assume up direction is straight up
	up[0] = 0.0f;   // x
	up[1] = 1.0f;   // y
	up[2] = 0.0f;   // z
	up[3] = 0.0f;   // w

	// compute left vector with cross product
	left[0] = up[1]*forward[2] - up[2]*forward[1];  // x
	left[1] = up[2]*forward[0] - up[0]*forward[2];  // y
	left[2] = up[0]*forward[1] - up[1]*forward[0];  // z
	left[3] = 1.0f;                                 // w

	// re-compute orthogonal up vector
	up[0] = forward[1]*left[2] - forward[2]*left[1];    // x
	up[1] = forward[2]*left[0] - forward[0]*left[2];    // y
	up[2] = forward[0]*left[1] - forward[1]*left[0];    // z
	up[3] = 0.0f;                                       // w

	// camera position
	position[0] = -posX;
	position[1] = -posY;
	position[2] = -posZ;
	position[3] = 1.0f;

	// copy axis vectors to matrix
	matrixView.identity();
	matrixView.setColumn(0, left);
	matrixView.setColumn(1, up);
	matrixView.setColumn(2, forward);
	matrixView.setRow(3, position);
}

void translateCamera(Matrix4 &mat, Vector3 camPos)
{
	mat[3] = -camPos.x;
	mat[7] = -camPos.y;
	mat[11] = -camPos.z;
}

void scaleModel(Matrix4 &_matrixModel, float scaleFactor)
{
	_matrixModel[0] *= scaleFactor;
	_matrixModel[1] *= scaleFactor;
	_matrixModel[2] *= scaleFactor;

	_matrixModel[4] *= scaleFactor;
	_matrixModel[5] *= scaleFactor;
	_matrixModel[6] *= scaleFactor;

	_matrixModel[8] *= scaleFactor;
	_matrixModel[9] *= scaleFactor;
	_matrixModel[10] *= scaleFactor;
}

void translateModel(Matrix4 &_matrixModel, Vector3 modelPosition)
{
	_matrixModel[3] = modelPosition.x;
	_matrixModel[7] = modelPosition.y;
	_matrixModel[11] = modelPosition.z;
}

void updateModelMatrix()
{
	// transform objects from object space to world space
	matrixModel.identity();

	matrixModel.rotateZ(modelAngle.z);
	matrixModel.rotateY(modelAngle.y);
	matrixModel.rotateX(modelAngle.x);

	//matrixModel.translate(modelPosition.x, modelPosition.y, modelPosition.z);
	translateModel(matrixModel, modelPosition);

	scaleModel(matrixModel, 1.0f);
	
	matrixModelView = matrixView * matrixModel;
}

///////////////////////////////////////////////////////////////////////////////
// update matrix
///////////////////////////////////////////////////////////////////////////////
void updateViewMatrix()
{
	// transform the camera (viewing matrix) from world space to eye space
	// Notice all values are negated, because we move the whole scene with the
	// inverse of camera transform    matrixView.identity();
	matrixView.identity();
	
	//matrixView.translate(-cameraPosition.x, -cameraPosition.y, -cameraPosition.z);
	translateCamera(matrixView, cameraPosition);
	
	matrixView.rotateZ(-cameraAngle.z);    // roll
	matrixView.rotateY(-cameraAngle.y);    // heading
	matrixView.rotateX(-cameraAngle.x);    // pitch	
	
	/* OR
	const float DEG2RAD = 3.141593f / 180;
	float sx, sy, sz, cx, cy, cz, theta;
	Vector3 left, up, forward;

	// rotation angle about X-axis (pitch)
	theta = cameraAngle.x * DEG2RAD;
	sx = sinf(theta);
	cx = cosf(theta);

	// rotation angle about Y-axis (yaw)
	theta = cameraAngle.y * DEG2RAD;
	sy = sinf(theta);
	cy = cosf(theta);

	// rotation angle about Z-axis (roll)
	theta = cameraAngle.z * DEG2RAD;
	sz = sinf(theta);
	cz = cosf(theta);

	// determine left axis
	left.x = cy*cz;
	left.y = sx*sy*cz + cx*sz;
	left.z = -cx*sy*cz + sx*sz;

	// determine up axis
	up.x = -cy*sz;
	up.y = -sx*sy*sz + cx*cz;
	up.z = cx*sy*sz + sx*cz;

	// determine forward axis
	forward.x = sy;
	forward.y = -sx*cy;
	forward.z = cx*cy;

	matrixView.setColumn(0, left);
	matrixView.setColumn(1, up);
	matrixView.setColumn(2, forward);
	//*/
}

///////////////////////////////////////////////////////////////////////////////
// rotate the camera for subWin2 (3rd person view)
///////////////////////////////////////////////////////////////////////////////
void rotateCamera3rdPerson(int x, int y)
{
	cameraAngle3rdPerson.y += (mMouseX - x);
	cameraAngle3rdPerson.x += (mMouseY - y);
	mMouseX = x;
	mMouseY = y;
}

///////////////////////////////////////////////////////////////////////////////
// zoom the camera for subWin2 (3rd person view)
///////////////////////////////////////////////////////////////////////////////
void zoomCamera3rdPerson(int y)
{
	cameraPosition3rdPerson.z -= (y - mMouseY) * 0.1f;
	mMouseY = y;
}

void updateViewMatrix3rdPerson()
{
	// transform the camera (viewing matrix) from world space to eye space
	// Notice all values are negated, because we move the whole scene with the
	// inverse of camera transform    matrixView.identity();
	matrixView3rdPerson.identity();

	matrixView3rdPerson.rotateZ(-cameraAngle3rdPerson.z);    // roll
	matrixView3rdPerson.rotateY(-cameraAngle3rdPerson.y);    // heading
	matrixView3rdPerson.rotateX(-cameraAngle3rdPerson.x);    // pitch

	//matrixView3rdPerson.translate(-cameraPosition3rdPerson.x, -cameraPosition3rdPerson.y, -cameraPosition3rdPerson.z);
	translateCamera(matrixView3rdPerson, cameraPosition3rdPerson);
}

void updateViewMatrixRearView()
{
	// transform the camera (viewing matrix) from world space to eye space
	// Notice all values are negated, because we move the whole scene with the
	// inverse of camera transform    matrixView.identity();
	matrixViewRearView.identity();

	//matrixViewRearView.translate(cameraPosition.x, cameraPosition.y, cameraPosition.z);
	translateCamera(matrixViewRearView, cameraPosition);

	matrixViewRearView.rotateZ(180 + cameraAngle.z);    // roll
	matrixViewRearView.rotateY(cameraAngle.y);			// heading
	matrixViewRearView.rotateX(180 + cameraAngle.x);    // pitch
}

void moveForward(float distance) {
	Vector3 viewVector = matrixView.getColumn(2);
	viewVector *= distance;
	cameraPosition += viewVector;
}

void strafe(float distance) {
	Vector3 rightVector = matrixView.getColumn(0);
	rightVector *= distance;
	cameraPosition += rightVector;
}

void moveUpward(float distance) {
	Vector3 upVector = matrixView.getColumn(1);
	upVector *= distance;
	cameraPosition += upVector;
}

void handleKeyboard() {
	bool isWalking = false;

	if(mKeys['Q'] || mKeys[VK_LEFT]) {
		cameraAngle.y += ROTATE_SPEED * m_DeltaTimeMs;
		if(cameraAngle.y > 360.0f) cameraAngle.y = 0.0f;
	}
	else
	if(mKeys['E'] || mKeys[VK_RIGHT]) {
		cameraAngle.y -= ROTATE_SPEED * m_DeltaTimeMs;
		if(cameraAngle.y < 0.0f) cameraAngle.y = 360.0f;
	}

	if(mKeys['W']) {
		moveForward(-WALK_SPEED * m_DeltaTimeMs);
		isWalking = true;
	}
	else
	if(mKeys['S']) {
		moveForward(WALK_SPEED * m_DeltaTimeMs);
		isWalking = true;
	}

	if(mKeys['A']) {
		strafe(-WALK_SPEED * m_DeltaTimeMs);
		isWalking = true;
	}
	else
	if(mKeys['D']) {
		strafe(WALK_SPEED * m_DeltaTimeMs);
		isWalking = true;
	}

	if(mKeys[VK_UP]) {
		if(cameraAngle.x < 80) {
			cameraAngle.x += ROTATE_SPEED * m_DeltaTimeMs;
		}
	}
	else
	if(mKeys[VK_DOWN]) {
		if(cameraAngle.x > -80) {
			cameraAngle.x -= ROTATE_SPEED * m_DeltaTimeMs;
		}
	}

	if(mKeys[VK_PRIOR]) {
		moveUpward(WALK_SPEED * m_DeltaTimeMs);
	}
	else
	if(mKeys[VK_NEXT]) {
		moveUpward(-WALK_SPEED * m_DeltaTimeMs);
	}

	//if(isWalking)
	//	PLAYER_STATE = PLAYER_WALK;

	//printf("\tcameraAngle.y = %f\n", cameraAngle.y);
}

void handleMouse() {
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
	
	bool yesRotateX = false;
	if(deltaY <= 0) {
		if(cameraAngle.x < 80) {
			yesRotateX = true;
		}
	}
	else 
	if(deltaY > 0) {
		if(cameraAngle.x > -80) {
			yesRotateX = true;
		}
	}
	
	if(yesRotateX) {
		cameraAngle.x -= deltaY;
	}

	cameraAngle.y -= deltaX;
}

///////////////////////////////////////////////////////////////////////////////
// draw frustum
///////////////////////////////////////////////////////////////////////////////
void drawFrustum(float fovY, float aspectRatio, float nearPlane, float farPlane)
{
	float tangent = tanf(fovY/2 * DEG2RAD);
	float nearHeight = nearPlane * tangent;
	float nearWidth = nearHeight * aspectRatio;
	float farHeight = farPlane * tangent;
	float farWidth = farHeight * aspectRatio;

	// compute 8 vertices of the frustum
	float vertices[8][3];
	// near top right
	vertices[0][0] = nearWidth;     vertices[0][1] = nearHeight;    vertices[0][2] = -nearPlane;
	// near top left
	vertices[1][0] = -nearWidth;    vertices[1][1] = nearHeight;    vertices[1][2] = -nearPlane;
	// near bottom left
	vertices[2][0] = -nearWidth;    vertices[2][1] = -nearHeight;   vertices[2][2] = -nearPlane;
	// near bottom right
	vertices[3][0] = nearWidth;     vertices[3][1] = -nearHeight;   vertices[3][2] = -nearPlane;
	// far top right
	vertices[4][0] = farWidth;      vertices[4][1] = farHeight;     vertices[4][2] = -farPlane;
	// far top left
	vertices[5][0] = -farWidth;     vertices[5][1] = farHeight;     vertices[5][2] = -farPlane;
	// far bottom left
	vertices[6][0] = -farWidth;     vertices[6][1] = -farHeight;    vertices[6][2] = -farPlane;
	// far bottom right
	vertices[7][0] = farWidth;      vertices[7][1] = -farHeight;    vertices[7][2] = -farPlane;

	float colorLine1[4] = { 0.7f, 0.7f, 0.7f, 0.7f };
	float colorLine2[4] = { 0.2f, 0.2f, 0.2f, 0.7f };
	float colorPlane[4] = { 0.5f, 0.5f, 0.5f, 0.5f };

	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// draw the edges around frustum
	glBegin(GL_LINES);
	glColor4fv(colorLine2);
	glVertex3f(0, 0, 0);
	glColor4fv(colorLine1);
	glVertex3fv(vertices[4]);

	glColor4fv(colorLine2);
	glVertex3f(0, 0, 0);
	glColor4fv(colorLine1);
	glVertex3fv(vertices[5]);

	glColor4fv(colorLine2);
	glVertex3f(0, 0, 0);
	glColor4fv(colorLine1);
	glVertex3fv(vertices[6]);

	glColor4fv(colorLine2);
	glVertex3f(0, 0, 0);
	glColor4fv(colorLine1);
	glVertex3fv(vertices[7]);
	glEnd();

	glColor4fv(colorLine1);
	glBegin(GL_LINE_LOOP);
	glVertex3fv(vertices[4]);
	glVertex3fv(vertices[5]);
	glVertex3fv(vertices[6]);
	glVertex3fv(vertices[7]);
	glEnd();

	glColor4fv(colorLine1);
	glBegin(GL_LINE_LOOP);
	glVertex3fv(vertices[0]);
	glVertex3fv(vertices[1]);
	glVertex3fv(vertices[2]);
	glVertex3fv(vertices[3]);
	glEnd();

	// draw near and far plane
	glColor4fv(colorPlane);
	glBegin(GL_QUADS);
	glVertex3fv(vertices[0]);
	glVertex3fv(vertices[1]);
	glVertex3fv(vertices[2]);
	glVertex3fv(vertices[3]);
	glVertex3fv(vertices[4]);
	glVertex3fv(vertices[5]);
	glVertex3fv(vertices[6]);
	glVertex3fv(vertices[7]);
	glEnd();

	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
}

///////////////////////////////////////////////////////////////////////////////
// draw a grid on the xz plane
///////////////////////////////////////////////////////////////////////////////
void drawGrid(float size, float step)
{
	// disable lighting
	glDisable(GL_LIGHTING);

	glBegin(GL_LINES);

	glColor3f(0.3f, 0.3f, 0.3f);
	for(float i=step; i <= size; i+= step)
	{
		glVertex3f(-size, 0,  i);   // lines parallel to X-axis
		glVertex3f( size, 0,  i);
		glVertex3f(-size, 0, -i);   // lines parallel to X-axis
		glVertex3f( size, 0, -i);

		glVertex3f( i, 0, -size);   // lines parallel to Z-axis
		glVertex3f( i, 0,  size);
		glVertex3f(-i, 0, -size);   // lines parallel to Z-axis
		glVertex3f(-i, 0,  size);
	}

	// x-axis
	glColor3f(0.5f, 0, 0);
	glVertex3f(-size, 0, 0);
	glVertex3f( size, 0, 0);

	// z-axis
	glColor3f(0,0,0.5f);
	glVertex3f(0, 0, -size);
	glVertex3f(0, 0,  size);

	glEnd();

	// enable lighting back
	glEnable(GL_LIGHTING);
}

///////////////////////////////////////////////////////////////////////////////
// draw the local axis of an object
///////////////////////////////////////////////////////////////////////////////
void drawAxis(/*Matrix4 &_matrixViewAxis, */float size, bool bInvertZAxis)
{
	glDepthFunc(GL_ALWAYS);     // to avoid visual artifacts with grid lines
	glDisable(GL_LIGHTING);
	glPushMatrix();             //NOTE: There is a bug on Mac misbehaviours of
								//      the light position when you draw GL_LINES
								//      and GL_POINTS. remember the matrix.

	//Vector3 viewVector = -_matrixViewAxis.getColumn(2);
	//Vector3 rightVector = _matrixViewAxis.getColumn(0);
	//Vector3 upVector = _matrixViewAxis.getColumn(1);

	//viewVector *= size;
	//rightVector *= size;
	//upVector *= size;

	// draw axis
	glLineWidth(3);
	glBegin(GL_LINES);
		glColor3f(1, 0, 0);
		glVertex3f(0, 0, 0);
		glVertex3f(size, 0.0f, 0.0f);//glVertex3f(rightVector.x, rightVector.y, rightVector.z);
		
		glColor3f(0, 1, 0);
		glVertex3f(0, 0, 0);
		glVertex3f(0.0f, size, 0.0f);//glVertex3f(upVector.x, upVector.y, upVector.z);

		glColor3f(0, 0, 1);
		glVertex3f(0, 0, 0);
		glVertex3f(0.0f, 0.0f, (!bInvertZAxis)?-size:size);//glVertex3f(viewVector.x, viewVector.y, viewVector.z);
	glEnd();
	glLineWidth(1);

	// draw arrows(actually big square dots)
	glPointSize(5);
	glBegin(GL_POINTS);
		glColor3f(1, 0, 0);
		glVertex3f(size, 0.0f, 0.0f);//glVertex3f(rightVector.x, rightVector.y, rightVector.z);
		glColor3f(0, 1, 0);
		glVertex3f(0.0f, size, 0.0f);//glVertex3f(upVector.x, upVector.y, upVector.z);
		glColor3f(0, 0, 1);
		glVertex3f(0.0f, 0.0f, (!bInvertZAxis)?-size:size);//glVertex3f(viewVector.x, viewVector.y, viewVector.z);
	glEnd();
	glPointSize(1);

	// restore default settings
	glPopMatrix();
	glEnable(GL_LIGHTING);
	glDepthFunc(GL_LEQUAL);
}

void renderWorld()
{
	glPushMatrix();
	{
		// set view matrix ========================================================
		// copy the matrix to OpenGL GL_MODELVIEW matrix
		// Note that OpenGL uses column-major matrix, so transpose the matrix first
		// See updateViewMatrix() how matrixView is constructed. The equivalent
		// OpenGL calls are;
		//    glLoadIdentity();
		//    glRotatef(-cameraAngle[2], 0, 0, 1); // roll
		//    glRotatef(-cameraAngle[1], 0, 1, 0); // heading
		//    glRotatef(-cameraAngle[0], 1, 0, 0); // pitch
		//    glTranslatef(-cameraPosition[0], -cameraPosition[1], -cameraPosition[2]);
		glLoadMatrixf(matrixView.getTranspose());

		// always draw the grid at the origin (before any modeling transform)
		drawGrid(10, 0.5);
		drawAxis(0.5f, false);

		// transform objects ======================================================
		// From now, all transform will be for modeling matrix only.
		// (from object space to world space)
		// See updateModelMatrix() how matrixModel is constructed. The equivalent
		// OpenGL calls are;
		//    glLoadIdentity();
		//    glTranslatef(modelPosition[0], modelPosition[1], modelPosition[2]);
		//    glRotatef(modelAngle[0], 1, 0, 0);
		//    glRotatef(modelAngle[1], 0, 1, 0);
		//    glRotatef(modelAngle[2], 0, 0, 1);
		matrixModelView.identity();
		
		modelAngle.y += 0.05f * m_DeltaTimeMs; if(modelAngle.y >= 360.0f) modelAngle.y = 0;
		//modelAngle.x += 0.05f * m_DeltaTimeMs; if(modelAngle.x >= 360.0f) modelAngle.x = 0;
		updateModelMatrix();

		// compute GL_MODELVIEW matrix by multiplying matrixView and matrixModel
		// before drawing the object:
		// ModelView_M = View_M * Model_M
		// This modelview matrix transforms the objects from object space to eye space.
		// copy modelview matrix to OpenGL after transpose
		glLoadMatrixf(matrixModelView.getTranspose());

		// draw a teapot after ModelView transform
		// v' = Mmv * v
		drawTeapot();

		//for(float i = 0; i <= 180; i++) {
		//	double angle = i*(22/7)/180.0f;
		//	double cosine = cos(angle);
		//	double acosine = acos(cosine)*180/(22/7);
		//	printf("cos(%f) = %f, acos(%f) = %f\n", i, cosine, cosine, acosine);
		//}
	}
	glPopMatrix();
}

void renderStatusBar(int x, int y, int w, int h) {
	fillRect(COLOR_CYAN, new Rect(x, y, w, h));

	fillRect(COLOR_GREY, new Rect(x, y, w, 1));
}

void render2DUpperLeftView() {
	int startX = 0;
	int startY = 0;

	fillRect(COLOR_GREEN, new Rect(startX, startY, 175, 25));
	drawStringFont("First Person View", 10, 5, 0);

	////////////////////////////////////////////
	setupOrthogonalProjection(0, 0, SCREEN_WIDTH>>1, SCREEN_HEIGHT>>1);
	drawVBO();
	////////////////////////////////////////////
}

void renderUpperLeftView()
{
	// set viewport
	setViewportSub(0, SCREEN_HEIGHT>>1, SCREEN_WIDTH>>1, SCREEN_HEIGHT>>1, 0.1f, 10.0f);
	m_SpriteCount = 0;

	// Update View Matrix before handling the Keyboard inputs.
	updateViewMatrix();

	handleKeyboard();
	//handleMouse();
	
	renderWorld();

	render2DUpperLeftView();
}

void render2DUpperRightView() {
	int startX = SCREEN_WIDTH>>1;
	int startY = 0;

	fillRect(COLOR_RED, new Rect(startX, startY, 175, 25));
	drawStringFont("Reverse Camera View", startX + 10, startY + 5, 0);

	////////////////////////////////////////////
	setupOrthogonalProjection(SCREEN_WIDTH>>1, 0, SCREEN_WIDTH>>1, SCREEN_HEIGHT>>1);
	drawVBO();
	////////////////////////////////////////////
}

void renderUpperRightView()
{
	// set viewport
	setViewportSub(SCREEN_WIDTH>>1, SCREEN_HEIGHT>>1, SCREEN_WIDTH>>1, SCREEN_HEIGHT>>1, 0.1f, 10.0f);
	
	// First, transform the camera (viewing matrix) from world space to eye space
	updateViewMatrixRearView();
	
	glPushMatrix();
	{
		glLoadMatrixf(matrixViewRearView.getTranspose());

		// always draw the grid at the origin (before any modeling transform)
		drawGrid(10, 0.5);
		drawAxis(0.5f, false);

		matrixModelViewRearView.identity();

		matrixModelViewRearView = matrixViewRearView * matrixModel;
		glLoadMatrixf(matrixModelViewRearView.getTranspose());

		drawTeapot();
	}
	glPopMatrix();

	render2DUpperRightView();
}

void render2DBottomLeftView() {
	int startX = 0;
	int startY = SCREEN_HEIGHT>>1;

	fillRect(COLOR_BLUE, new Rect(startX, startY, 175, 25));
	drawStringFont("World Camera View", startX + 10, startY + 5, 0);

	////////////////////////////////////////////
	setupOrthogonalProjection(0, SCREEN_HEIGHT>>1, SCREEN_WIDTH>>1, SCREEN_HEIGHT>>1);
	drawVBO();
	////////////////////////////////////////////
}

void render2DBottomRightView()
{
	int startX = SCREEN_WIDTH>>1;
	int startY = SCREEN_HEIGHT>>1;

	fillRect(COLOR_WHITE, new Rect(startX, startY, SCREEN_WIDTH>>1, SCREEN_HEIGHT>>1));
	drawStringFont("2D Camera", startX + 10, startY + 5, 0);
	
	////////////////////////////////////////////
	startY += 10;
	char* txt = new char[255];
	sprintf(txt, "%3.2f\t\t%3.2f\t\t%3.2f\t\t%3.2f", matrixView[0], matrixView[4], matrixView[8]);
	drawStringFont(txt, startX + 20, startY + 20, 0);
	sprintf(txt, "%3.2f\t\t%3.2f\t\t%3.2f\t\t%3.2f", matrixView[1], matrixView[5], matrixView[9]);
	drawStringFont(txt, startX + 20, startY + 35, 0);
	sprintf(txt, "%3.2f\t\t%3.2f\t\t%3.2f\t\t%3.2f", matrixView[2], matrixView[6], matrixView[10]);
	drawStringFont(txt, startX + 20, startY + 50, 0);
	sprintf(txt, "%3.2f\t\t%3.2f\t\t%3.2f\t\t%3.2f", matrixView[3], matrixView[7], matrixView[11]);
	drawStringFont(txt, startX + 20, startY + 65, 0);
	////////////////////////////////////////////

	////////////////////////////////////////////
	setupOrthogonalProjection(SCREEN_WIDTH>>1, SCREEN_HEIGHT>>1, SCREEN_WIDTH>>1, SCREEN_HEIGHT>>1);
	
////////////////////////////////////////////
	m_pGameWorld->setCrosshair(mMouseX, mMouseY);
	m_pGameWorld->update(m_ElapasedGameTimeMs, m_DeltaTimeMs);
	//m_pGameWorld->render();
////////////////////////////////////////////
	
	Vector2 vCrossHairPos = m_pGameWorld->getCrosshair();
	drawQuadU(TEXTURE_CORE, vCrossHairPos.x-9, vCrossHairPos.y-9, 18, 18, 49, 107, 18, 18);

	///*
	for(int i = 0; i < m_pGameWorld->getVehicleCount(); i++) {
		Vehicle* vVehicle = m_pGameWorld->getVehicle(i);
		
		//glPushMatrix();
		//{
		//	matrixModel2D.identity();
		//	matrixModel2D.m[12] = vVehicle->m_vPos.x;
		//	matrixModel2D.m[13] = vVehicle->m_vPos.y;
		//	matrixModel2D.m[14] = 0.0f;
		//	matrixModel2D.m[15] = 0.0f;

		//	matrixModel2D.m[0] = vVehicle->m_vHeading.x;
		//	matrixModel2D.m[1] = vVehicle->m_vHeading.y;
		//	matrixModel2D.m[2] = 0.0f;
		//	matrixModel2D.m[3] = 0.0f;
		//	
		//	matrixModel2D.m[4] = vVehicle->m_vRight.x;
		//	matrixModel2D.m[5] = vVehicle->m_vRight.y;
		//	matrixModel2D.m[6] = 0.0f;
		//	matrixModel2D.m[7] = 0.0f;

		//	matrixModel2D.m[8] = 0.0f;
		//	matrixModel2D.m[9] = 0.0f;
		//	matrixModel2D.m[10] = 0.0f;
		//	matrixModel2D.m[11] = 0.0f;

		//	matrixModelView2D.identity();
		//	matrixModelView2D = matrixProjection * matrixModel2D;
		//	glLoadMatrixf(matrixModelView2D.getTranspose());
		//}
		//glPopMatrix();
		
		std::vector<Vector2> vVehicleVertices = vVehicle->getTransformedVertices();

		if(vVehicle->getSteerBehaviour() == Vehicle::STEERING_BEHAVIOUR::SEEK)
			//drawQuadU(TEXTURE_CORE, vVehicleVertices, 4, 281, 16, 14);
			drawQuadU(TEXTURE_CORE, vVehicleVertices, 22, 251, 16, 13);
		else
		if(vVehicle->getSteerBehaviour() == Vehicle::STEERING_BEHAVIOUR::ARRIVE)
			drawQuadU(TEXTURE_CORE, vVehicleVertices, 22, 266, 16, 13);
		else
		if(vVehicle->getSteerBehaviour() == Vehicle::STEERING_BEHAVIOUR::FLEE)
			//drawQuadU(TEXTURE_CORE, vVehicleVertices, 40, 251, 16, 14);
			drawQuadU(TEXTURE_CORE, vVehicleVertices, 40, 251, 16, 13);
		else
		if(vVehicle->getSteerBehaviour() == Vehicle::STEERING_BEHAVIOUR::PURSUIT)
			drawQuadU(TEXTURE_CORE, vVehicleVertices, 40, 266, 16, 13);
		else
		if(vVehicle->getSteerBehaviour() == Vehicle::STEERING_BEHAVIOUR::EVADE)
			drawQuadU(TEXTURE_CORE, vVehicleVertices, 4, 266, 16, 13);
		else
		if(vVehicle->getSteerBehaviour() == Vehicle::STEERING_BEHAVIOUR::WANDER)
			drawQuadU(TEXTURE_CORE, vVehicleVertices, 4, 251, 16, 13);

		{
			Vector2 vVehiclePosition = vVehicle->getPosition();
			Vector2 vVehicleHeading = vVehicle->getHeading();
			Vector2 vVehicleRight = vVehicle->getRight();

			float x1 = vVehiclePosition.x;
			float y1 = vVehiclePosition.y;

			vVehicleHeading.normalize();
			vVehicleHeading *= 20.0f;
			Vector2 vFwd = vVehiclePosition + vVehicleHeading;
			drawLine(vVehiclePosition.x, vVehiclePosition.y, vFwd.x, vFwd.y, 255.0f, 0.0f, 0.0f);

			vVehicleRight.normalize();
			vVehicleRight *= 20.0f;
			Vector2 vRight = vVehiclePosition + vVehicleRight;
			drawLine(vVehiclePosition.x, vVehiclePosition.y, vRight.x, vRight.y, 255.0f, 0.0f, 0.0f);

			glPointSize(3);
			glBegin(GL_POINTS);
				glColor3f(1, 0, 0);
				glVertex3f(vVehiclePosition.x, vVehiclePosition.y, 0.0f);
			glEnd();

			glPointSize(3);
			glBegin(GL_POINTS);
				glColor3f(0, 1, 0);
				glVertex3f(vVehicleHeading.x, vVehicleHeading.y, 0.0f);
			glEnd();

		}
	}
	//*/

	drawVBO();
	////////////////////////////////////////////
}

void renderBottomRightView()
{
	setViewportSub(SCREEN_WIDTH>>1, 0, SCREEN_WIDTH>>1, SCREEN_HEIGHT>>1, 0.1f, 10.0f);

	m_SpriteCount = 0;

	render2DBottomRightView();
}

void renderBottomLeftView()
{
	// set viewport
	setViewportSub(0, 0, SCREEN_WIDTH>>1, SCREEN_HEIGHT>>1, 0.1f, 100.0f);
	m_SpriteCount = 0;

	// First, transform the camera (viewing matrix) from world space to eye space
	updateViewMatrix3rdPerson();

	glPushMatrix();
	{
		glLoadMatrixf(matrixView3rdPerson.getTranspose());

		// draw grid wrt the "3rd Person View"
		drawGrid(10, 1);

		{
			Matrix4 _matrixCameraModel;
			_matrixCameraModel.identity();

			// We treat our "FPS Camera" as any other world object.
			// But, to take our "FPS Camera" to the "3rd Person View", 
			// We have to invert the "FPS Camera View Matrix" & reassign the cameraPosition.
			_matrixCameraModel = matrixView.invert();
			translateModel(_matrixCameraModel, cameraPosition);

			matrixModelView3rdPerson.identity();
			matrixModelView3rdPerson =  matrixView3rdPerson * _matrixCameraModel;
			glLoadMatrixf(matrixModelView3rdPerson.getTranspose());
			
			drawAxis(1.0f, false);
			drawCamera();
			drawFrustum(FOV_Y, 1, 1, 10);
		}

		// transform objects ======================================================
		// From now, all transform will be for modeling matrix only.
		// (from object space to world space)
		// See updateModelMatrix() how matrixModel is constructed. The equivalent
		// OpenGL calls are;
		//    glLoadIdentity();
		//    glTranslatef(modelPosition[0], modelPosition[1], modelPosition[2]);
		//    glRotatef(modelAngle[0], 1, 0, 0);
		//    glRotatef(modelAngle[1], 0, 1, 0);
		//    glRotatef(modelAngle[2], 0, 0, 1);
		matrixModelView3rdPerson.identity();
		matrixModelView3rdPerson = matrixView3rdPerson * matrixModel;

		// compute GL_MODELVIEW matrix by multiplying matrixView and matrixModel
		// before drawing the object:
		// ModelView_M = View_M * Model_M
		// This modelview matrix transforms the objects from object space to eye space.
		// copy modelview matrix to OpenGL after transpose
		glLoadMatrixf(matrixModelView3rdPerson.getTranspose());

		// draw a teapot after ModelView transform
		// v' = Mmv * v
		drawAxis(5.0f, false);
		drawTeapot();
	}
	glPopMatrix();

	render2DBottomLeftView();
}

void render2D()
{
	setViewportSub(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f);
	m_SpriteCount = 0;

	fillRect(COLOR_YELLOW, new Rect(SCREEN_WIDTH>>1, 0, 2, SCREEN_HEIGHT));
	fillRect(COLOR_YELLOW, new Rect(0, SCREEN_HEIGHT>>1, SCREEN_WIDTH, 2));

	////////////////////////////////////////////
	setupOrthogonalProjection(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	drawVBO();
	////////////////////////////////////////////
}

int drawGLScene() {

	// clear buffer
	glClearColor(0.1f, 0.1f, 0.1f, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);	// Clear The Screen And The Depth Buffer

	setClip(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	renderUpperLeftView();
	renderBottomLeftView();
	renderUpperRightView();
	renderBottomRightView();

	render2D();

	return true;
}

void setMousePosition(int mouseX, int mouseY) {
	mMouseX = mouseX;
	mMouseY = mouseY;
}

void mouseMove(WPARAM keyState, int x, int y) {
	if(L_MOUSE_DOWN) {
		rotateCamera3rdPerson(x, y);
	}
	else
	if(R_MOUSE_DOWN) {
		zoomCamera3rdPerson(y);
	}
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
			//resizeGLScene(LOWORD(lParam), HIWORD(lParam));

			// set viewport
			setViewportSub(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.1f, 10.0f);

			return 0;
		}
		case WM_LBUTTONDOWN:
		{
			setMousePosition(LOWORD(lParam), HIWORD(lParam));
			L_MOUSE_DOWN = true;
		}
		break;
		case WM_LBUTTONUP:
		{
			setMousePosition(LOWORD(lParam), HIWORD(lParam));
			L_MOUSE_DOWN = false;
		}
		break;
		case WM_RBUTTONDOWN:
		{
			setMousePosition(LOWORD(lParam), HIWORD(lParam));
			R_MOUSE_DOWN = true;
		}
		break;
		case WM_RBUTTONUP:
		{
			setMousePosition(LOWORD(lParam), HIWORD(lParam));
			R_MOUSE_DOWN = false;
		}
		break;
		case WM_MOUSEMOVE:
		{
			//setMousePosition(LOWORD(lParam), HIWORD(lParam));
			mouseMove(wParam, LOWORD(lParam), HIWORD(lParam));
		}
		break;

	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void fillRect(u32 COLOR_ID, Rect* rect) {
	drawQuadU(TEXTURE_CORE, rect->X, rect->Y, rect->Width, rect->Height, m_ColorUVTable[COLOR_ID].u, m_ColorUVTable[COLOR_ID].v, 1, 1);
}


void drawFont(int xX, int yY, int charW, int charH, int tX, int tY) {
	drawQuadU(TEXTURE_CORE, xX, yY, charW, charH, FONT_TEXTURE_START_X+tX, tY, charW, charH);
}

void drawQuadU(	int TEXTURE_ID, 
				float posX, float posY, float posW, float posH,
				float texX, float texY, float texW, float texH
) {
	GLuint TEX_WIDTH = _textureWH[TEXTURE_ID][0];
	GLfloat tX = texX/TEX_WIDTH, tY = texY/TEX_WIDTH, tW = texW/TEX_WIDTH, tH = texH/TEX_WIDTH;

	RectF* drawRect = new RectF(posX, posY, posW, posH);
	RectF* posIntersectRect = new RectF(0, 0, posW, posH);
	RectF::Intersect(*posIntersectRect, *m_ClipRect, *drawRect);

	if(posIntersectRect->Width > 0 && posIntersectRect->Height > 0) 
	{
		////////// If IntersectRect width is less than texture width
		if(posIntersectRect->Width < texW) {
			texW = posIntersectRect->Width;
			tW = texW/TEX_WIDTH;

			////////// If posX < clipRectX, shift texture X equal to the diff in X
			if(posIntersectRect->X > posX) {
				texX += (posIntersectRect->X - posX);
				tX = texX/TEX_WIDTH;
			}
		}

		////////// If IntersectRect height is less than texture height
		if(posIntersectRect->Height < texH) {
			texH = posIntersectRect->Height;
			tH = texH/TEX_WIDTH;

			////////// If posY < clipRectY, shift texture Y equal to the diff in Y
			if(posIntersectRect->Y > posY) {
				texY += (posIntersectRect->Y - posY);
				tY = texY/TEX_WIDTH;
			}
		}

		//using VertexT2F_C4UB_V3F
		{	
			m_VB[m_SpriteCount+0].tu = tX;	m_VB[m_SpriteCount+0].tv = tY;
			m_VB[m_SpriteCount+0].color = 0xffffffff;
			m_VB[m_SpriteCount+0].vx = posIntersectRect->X;	m_VB[m_SpriteCount+0].vy = posIntersectRect->Y;	m_VB[m_SpriteCount+0].vz = 0.0f;
		}
		{	
			m_VB[m_SpriteCount+1].tu = tX;		m_VB[m_SpriteCount+1].tv = tY+tH;	
			m_VB[m_SpriteCount+1].color = 0xffffffff;
			m_VB[m_SpriteCount+1].vx = posIntersectRect->X;	m_VB[m_SpriteCount+1].vy = posIntersectRect->Y+posIntersectRect->Height; m_VB[m_SpriteCount+1].vz = 0.0f;
		}
		{	
			m_VB[m_SpriteCount+2].tu = tX+tW;	m_VB[m_SpriteCount+2].tv = tY+tH;	
			m_VB[m_SpriteCount+2].color = 0xffffffff;
			m_VB[m_SpriteCount+2].vx = posIntersectRect->X+posIntersectRect->Width;	m_VB[m_SpriteCount+2].vy = posIntersectRect->Y+posIntersectRect->Height; m_VB[m_SpriteCount+2].vz = 0.0f;
		}
		{	
			m_VB[m_SpriteCount+3].tu = tX+tW;	m_VB[m_SpriteCount+3].tv = tY;		
			m_VB[m_SpriteCount+3].color = 0xffffffff;
			m_VB[m_SpriteCount+3].vx = posIntersectRect->X+posIntersectRect->Width;	m_VB[m_SpriteCount+3].vy = posIntersectRect->Y; m_VB[m_SpriteCount+3].vz = 0.0f;
		}

		m_SpriteCount += 4;
	}

	delete drawRect;
	delete posIntersectRect;
}

void drawQuadU(	int TEXTURE_ID, 
				std::vector<Vector2> vVertices,
				float texX, float texY, float texW, float texH
) {
   GLuint TEX_WIDTH = _textureWH[TEXTURE_ID][0];
   GLfloat tX = texX/TEX_WIDTH, tY = texY/TEX_WIDTH, tW = texW/TEX_WIDTH, tH = texH/TEX_WIDTH;

   {
	   //using VertexT2F_C4UB_V3F
	   {	
		   m_VB[m_SpriteCount+0].tu = tX;	m_VB[m_SpriteCount+0].tv = tY;
		   m_VB[m_SpriteCount+0].color = 0xffffffff;
		   m_VB[m_SpriteCount+0].vx = vVertices[0].x;
		   m_VB[m_SpriteCount+0].vy = vVertices[0].y;
		   m_VB[m_SpriteCount+0].vz = 0.0f;
	   }
	   {	
		   m_VB[m_SpriteCount+1].tu = tX;		m_VB[m_SpriteCount+1].tv = tY+tH;	
		   m_VB[m_SpriteCount+1].color = 0xffffffff;
		   m_VB[m_SpriteCount+1].vx = vVertices[1].x;
		   m_VB[m_SpriteCount+1].vy = vVertices[1].y;
		   m_VB[m_SpriteCount+1].vz = 0.0f;
	   }
	   {	
		   m_VB[m_SpriteCount+2].tu = tX+tW;	m_VB[m_SpriteCount+2].tv = tY+tH;	
		   m_VB[m_SpriteCount+2].color = 0xffffffff;
		   m_VB[m_SpriteCount+2].vx = vVertices[2].x;
		   m_VB[m_SpriteCount+2].vy = vVertices[2].y;
		   m_VB[m_SpriteCount+2].vz = 0.0f;
	   }
	   {	
		   m_VB[m_SpriteCount+3].tu = tX+tW;	m_VB[m_SpriteCount+3].tv = tY;		
		   m_VB[m_SpriteCount+3].color = 0xffffffff;
		   m_VB[m_SpriteCount+3].vx = vVertices[3].x;
		   m_VB[m_SpriteCount+3].vy = vVertices[3].y;
		   m_VB[m_SpriteCount+3].vz = 0.0f;
	   }

	   m_SpriteCount += 4;
   }
}

void drawStringFont(const char* cStr, int x, int y, int anchor) {
	int strWidth = CHARACTER_WIDTH * (strlen(cStr) + 1);//getStringWidthTillPos(cStr, strlen(cStr));

	int TOTAL_ROWS	= 7;
	//int CHAR_HEIGHT = 13;//WWidgetRenderer::iFont->GetHeight()/TOTAL_ROWS;

	int COL = 0, ROW = 0, X = 0, Y = 0;

	int HLEFT = 0;
	int HCENTER = 1;
	int HRIGHT = 2;

	int xX = x;
	int yY = y;

	if(anchor == HCENTER)
		xX = xX - (strWidth/2);
	else
	if(anchor == HRIGHT)
		xX = xX - strWidth;

	for(int i = 0 ; i < strlen(cStr); i++) {
		char c = cStr[i];//charAt((char*)cStr, i);

		if(c == '\r') {
			continue;
		}

		if(c > ' ') {

			//int CHAR_WIDTH = 7;//getCharWidth(c);
			COL = (c-33)%13;
			ROW = (c-33)/13;

			X = COL*CHARACTER_WIDTH;
			Y = ROW*CHARACTER_HEIGHT;

			drawFont(xX, yY, CHARACTER_WIDTH, CHARACTER_HEIGHT, X, Y);

			xX += (CHARACTER_WIDTH + 1);
		}
		else
		if(c == ' ' || c == '\t') {
			xX += CHARACTER_WIDTH + 1;
		}
		else
		if(c == '\n') {
			xX = x;
			yY += CHARACTER_HEIGHT + 3;
		}
	}
}

void glSelectTexture(int id) {
	glBindTexture(GL_TEXTURE_2D, _texture[id]); // Select Our Font Texture
}

void drawVBO() {
	glEnable(GL_TEXTURE_2D);
	glSelectTexture(TEXTURE_CORE);
	
	//glInterleavedArrays( GL_T2F_V3F, 0, m_VB);
	//glInterleavedArrays( GL_T2F_V3F, 0, m_VB);
	glInterleavedArrays( GL_T2F_C4UB_V3F, 0, m_VB);
	//glInterleavedArrays( GL_T2F_C3F_V3F, 0, m_VB);
	glDrawArrays( GL_QUADS, 0, m_SpriteCount);

	// Disable vertex arrays (Note: glInterleavedArrays implicitly calls
	// glEnableClientState for vertex, texture coord and color arrays)
	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );

	glDisable(GL_TEXTURE_2D);
}

void setClip(int x, int y, int w, int h ) {
	if(m_ClipRect == NULL)
		m_ClipRect = new RectF(x, y, w, h);

	m_ClipRect->X = x;
	m_ClipRect->Y = y;
	m_ClipRect->Width = w;
	m_ClipRect->Height = h;
}

void setupOrthogonalProjection(double x, double y, double w, double h) {
	glDisable(GL_LIGHTING);

	setOrthogonalFrustum(x, x+w, y+h, y, 0.0, 1.0);
	// Setup and orthogonal, pixel-to-pixel projection matrix
	glMatrixMode(GL_PROJECTION);
	// copy projection matrix to OpenGL
	glLoadMatrixf(matrixProjection.getTranspose());

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void drawLine(float x1, float y1, float x2, float y2, float r, float g, float b) {
	glBegin(GL_LINES);
		glColor3f(r, g, b);
		glVertex2f(x1, y1);
		glColor3f(r, g, b);
		glVertex2f(x2, y2);
	glEnd();
}
/*
- clean the screen
- 3D:
	- enable lighting, z-test, etc
	- set active matrix mode to projection
	- load identity and establish a perspective projection
	- set active matrix mode back to modelview
	- draw everything 3D
- 2D:
	- disable lighting, z-test, etc
	- set active matrix mode to projection
	- load identity and establish an orthogonal projection
	- set active matrix mode back to modelview
	- draw everything 2D
-	swap buffers
*/