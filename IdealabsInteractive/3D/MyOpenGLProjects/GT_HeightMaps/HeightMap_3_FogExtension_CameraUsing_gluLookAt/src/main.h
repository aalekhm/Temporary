
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

#include "Terrain.h"					// Include our header file for our Terrain.cpp
#include "CCamera.h"

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

// To allow volumetric fog, we need to add some EXT defines and a typedef
// These are the defines for the flags that are passed into glFogi()
#define GL_FOG_COORDINATE_SOURCE_EXT			0x8450
#define GL_FOG_COORDINATE_EXT					0x8451

// Here we extern our function pointer for volumetric fog positioning
typedef void (APIENTRY * PFNGLFOGCOORDFEXTPROC) (GLfloat coord);

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

// We added a bunch of defines, function prototypes and externs for multitexturing

// These are for our multitexture defines
#define GL_TEXTURE0_ARB					0x84C0
#define GL_TEXTURE1_ARB					0x84C1

#define GL_COMBINE_ARB					0x8570
#define	GL_RGB_SCALE_ARB				0x8573

// Here are the multitexture function prototypes
typedef void (APIENTRY * PFNGLMULTITEXCOORD2FARBPROC) (GLenum target, GLfloat s, GLfloat t);
typedef void (APIENTRY * PFNGLACTIVETEXTUREARBPROC)	 (GLenum target);

// Here we extern our function pointers for multitexturing capabilities
extern PFNGLMULTITEXCOORD2FARBPROC		glMultiTexCoord2fARB;
extern PFNGLACTIVETEXTUREARBPROC		glActiveTextureARB;

const float		GRAVITY = 0.98*2;
const float		piOver180 = 0.0174532925f;
const float		WALK_SPEED = 1.025f;
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

#define TEX_ID_TERRAIN			0
#define TEX_ID_DETAIL			1
#define TEX_ID_SKYBOX_BACK		2
#define TEX_ID_SKYBOX_BACK		3
#define TEX_ID_SKYBOX_BACK		4
#define TEX_ID_SKYBOX_BACK		5
#define TEX_ID_SKYBOX_BACK		6
#define TEX_ID_SKYBOX_BACK		7
#define TEX_ID_WALL 			8
#define TEX_ID_FLOOR 			9
#define TEX_ID_CEILING 			10
#define TEX_ID_ROOMWALL 		11
#define TEX_ID_ROOMFLOOR 		12
#define TEX_ID_ROOMCEILING 		13
#define TEX_ID_FONT				14

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
