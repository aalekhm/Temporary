
#ifndef _MAIN_H_
#define _MAIN_H_

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <fstream>
#include <vector>
#include <gl\gl.h>						// Header File For The OpenGL32 Library
#include <gl\glu.h>						// Header File For The GLu32 Library
#include <gl\glaux.h>
#include <crtdbg.h>
#include <stdarg.h>						// Header File For Variable Argument Routines	( ADD )
#include "3DMath.h"

#include "CCamera.h"

const float		GRAVITY = 0.98;
const float		piOver180 = 0.0174532925f;
const float		WALK_SPEED = 0.25f;
extern bool		mKeys[256];					// Array Used For The Keyboard Routine
extern bool		mKeysPressed[256];			// Array Used For The Keyboard Routine
extern bool		L_MOUSE_DOWN;
extern bool		R_MOUSE_DOWN;

#define PLAYER_STAND	0
#define PLAYER_WALK		1
#define PLAYER_JUMP		2
#define PLAYER_CROUCH	3
static int PLAYER_STATE = PLAYER_STAND;
static float jumpAngle = 0;
static float TRAMP;
static bool CROUCH;

void glPrint(int x, int y, const char* format, ...);

using namespace std;
typedef unsigned char byte;

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define SCREEN_BPP		16

#define MAX_TEXTURES	100

extern GLuint* textures;

#define TEX_ID_FONT				0
#define TEX_ID_FLOOR			1
#define TEX_ID_WALL				2

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

// We added new stuctures for our camera and frustum code.  These should eventually
// go inside of a new .h file like camera.h and frustum.h

// This is our sphere structure for keeping track of the spheres
struct Sphere {
	
	float xPos, yPos, zPos, radius;			// We want to hold a XYZ position and radius
	BYTE r, g, b;							// These will store the color of the sphere
};

// This will allow us to create an object to keep track of our frustum
class CFrustum {
	
	public:
		// Call this every time the camera moves to update the frustum
		void calculateFrustum();

		// This takes a 3D point and returns TRUE if it's inside of the frustum
		bool isPointInFrustum(float x, float y, float z);

		// This takes a 3D point and a radius and returns TRUE if the sphere is inside of the frustum
		bool isSphereInFrustum(float x, float y, float z, float radius);

		// This takes the center and half the length of the cube.
		bool isCubeInFrustum(float x, float y, float z, float size);

	private:
		// This holds the A B C and D values for each side of our frustum.
		float mFrustum[6][4];

};

#endif
/////////////////////////////////////////////////////////////////////////////////
//
// * QUICK NOTES * 
//
// We added some variables and defines to allow multitexturing.
//  
//
// Ben Humphrey (DigiBen)
// Game Programmer
// DigiBen@GameTutorials.com
// Co-Web Host of www.GameTutorials.com
//
//
