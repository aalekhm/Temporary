
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

using namespace std;
typedef unsigned char byte;

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define SCREEN_BPP		16

extern bool		mKeys[256];					// Array Used For The Keyboard Routine
extern bool		mKeysPressed[256];			// Array Used For The Keyboard Routine
extern bool		L_MOUSE_DOWN;
extern bool		R_MOUSE_DOWN;

extern int		glViewMode;
extern GLuint*	textures;

#define MAX_TEXTURES	100

extern int		MAX_MODELS_TO_LOAD;
extern int		MAX_MODELS_IN_SCENE;

#define TEX_ID_FONT			1
#define TEX_ID_WALL 		2
#define TEX_ID_FLOOR 		3
#define TEX_ID_CEILING 		4
#define TEX_ID_ROOMWALL 	5
#define TEX_ID_ROOMFLOOR 	6
#define TEX_ID_ROOMCEILING 	7
#define TEX_ID_SOFA			8
#define TEX_ID_SIDEBOARD	9

#define MODEL_ID_ARMCHAIR	0
#define MODEL_ID_SOFA		1
#define MODEL_ID_SIDEBOARD	2
//#define MODEL_ID_FIREPLACE	3

#endif