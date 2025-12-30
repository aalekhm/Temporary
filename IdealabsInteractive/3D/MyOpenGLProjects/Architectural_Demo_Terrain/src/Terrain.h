
#ifndef _TERRAIN_H_
#define _TERRAIN_H

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

#define MAP_SIZE		1024	// This is the size of our .raw height map
#define STEP_SIZE		16		// This is width and height of each QUAD
#define HEIGHT_RATIO	1.5f	// This is the ratio that the Y is scaled according to the X and Z

#define TILE_MODE_MANUAL	0
#define TILE_MODE_SCALING	1

// This loads a .raw file of a certain size from the file
void loadRawHeightMap(LPSTR strName, int nSize, BYTE* pHeightMap);

// This returns the height (0 to 255) from a heightmap given an X and Y
int getHeight(BYTE* pHeightMap, int X, int Y);

// This turns heightmap data into primitives and draws them to the screen
void renderHeightMap(BYTE* pHeightMap);

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
#endif