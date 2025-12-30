
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

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
#include "Terrain.h"					// Include our header file for our Terrain.cpp
/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

using namespace std;
typedef unsigned char byte;

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define SCREEN_BPP		16

#define MAX_TEXTURES	100

extern GLuint* textures;

#define TEX_ID_SKYBOX_BACK		0
#define TEX_ID_SKYBOX_BACK		1
#define TEX_ID_SKYBOX_BACK		2
#define TEX_ID_SKYBOX_BACK		3
#define TEX_ID_SKYBOX_BACK		4
#define TEX_ID_SKYBOX_BACK		5
#define TEX_ID_TERRAIN			6
#define TEX_ID_WALL 			7
#define TEX_ID_FLOOR 			8
#define TEX_ID_CEILING 			9
#define TEX_ID_ROOMWALL 		10
#define TEX_ID_ROOMFLOOR 		11
#define TEX_ID_ROOMCEILING 		12
#define TEX_ID_FONT				13

#endif