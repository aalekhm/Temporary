
#include "main.h"

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
//
// Building off of the detail texturing tutorial, we add some awesome volumetric
// fog using the OpenGL extension: glFogCoordfEXT().
// 
// This allows us to do volumetric fog without extra passes, and we are able
// to effect the fog dynamically, as you will see in the application by the +/- keys.
// The effect brings an incredible amount of realism to a scene and is a must
// have in any engine. 
//
// We add a new function to this file:  SetFogCoord(), which takes a depth
// for the fog, and the current y value of the vertex having fog applied to it.
// The volumetric fog is not actually a volume fog, but is a trick that changes
// the color of the polygons to give the appearance of fog.
//
//
// This stores the desired depth that we want to fog
extern float g_FogDepth;

// This is our fog extension function pointer to set a vertice's depth
extern PFNGLFOGCOORDFEXTPROC glFogCoordfEXT;
/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *


// This tells us if we want detail texturing turned on or off
extern bool bDetail;

// This holds the current tile scale value for our detail texture
extern int iDetailScale;

extern int TILING_MODE;

// This loads a .raw file into an array of bytes.  Each value is a height value.
void loadRawHeightMap(LPSTR strName, int nSize, BYTE* pHeightMap) {
	
	FILE* pFile = NULL;

	// Let's open the file in Read/Binary mode.
	pFile = fopen(strName, "rb");

	// Check to see if we found the file and could open it
	if(pFile == NULL) {
		
		// Display our error message and stop the function
		MessageBox(NULL, "Can't find Height Map!", "Error", MB_OK);
		return;
	}

	// Here we load the .raw file into our pHeightMap data array.
	// We are only reading in '1', and the size is the (width * height)
	fread(pHeightMap, 1, nSize, pFile);

	// After we read the data, it's a good idea to check if everything read fine.
	int hasError = ferror(pFile);

	// Check if we received an error.
	if(hasError) {
		MessageBox(NULL, "Can't get Data!", "Error", MB_OK);
	}
	
	// Close the file.
	fclose(pFile);
}

// This returns the height (0 to 255) from a heightmap given an X and Y
int getHeight(BYTE* pHeightMap, int X, int Y) {
	
	// This is used to index into our height map array.
	// When ever we are dealing with arrays, we want to make sure
	// that we don't go outside of them, so we make sure that doesn't
	// happen with a %.  This way x and y will cap out at (MAX_SIZE - 1)

	int x = X % MAP_SIZE;		// Error check our x value
	int y = Y % MAP_SIZE;		// Error check our y value

	if(!pHeightMap)				// Make sure our data is valid
		return 0;

	// Below, we need to treat the single array like a 2D array.
	// We can use the equation: index = (x + (y * arrayWidth) ).
	// This is assuming we are using this assumption array[x][y]
	// otherwise it's the opposite.  Now that we have the correct index,
	// we will return the height in that index.

	return pHeightMap[x + (y*MAP_SIZE)];	// Index into our height array and return the height
}

// This sets the color value for a particular index, depending on the height index
void setVertexColour(BYTE* pHeightMap, int X, int Y) {
	
	if(!pHeightMap)					// Make sure our height data is valid
		return;

	// Here we set the color for a vertex based on the height index.
	// To make it darker, I start with -0.15f.  We also get a ratio
	// of the color from 0 to 1.0 by dividing the height by 256.0f;
	float fColour = -0.15f + (getHeight(pHeightMap, X, Y) / 256.0f);

	// Assign this green shade to the current vertex
	glColor3f(0, fColour, 0);
}

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
// This sets the volumetric fog for the current vertex with the desired depth
void setFogCoord(float depth, float height) {

	// This function takes the depth of the fog, as well as the height
	// of the current vertex.  If the height is greater than the depth, there
	// is no fog applied to it (0), but if it's below the depth, then we
	// calculate the fog value that should be applied to it.  Since the higher
	// the number passed into glFogCoordfEXT() produces more fog, we need to
	// subtract the depth from the height, then negate that value to switch
	// the ratio from 0 to the depth around.  Otherwise it would have more
	// fog on the top of the fog volume than the bottom of it.

	float fogY = 0;

	// Check if the height of this vertex is greater than the depth (needs no fog)
	if(height > depth)
		fogY = 0;
	// Otherwise, calculate the fog depth for the current vertex
	else
		fogY = -(height - depth);

	// Assign the fog coordinate for this vertex using our extension function pointer
	glFogCoordfEXT(fogY);
}
/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

// This sets the current texture coordinate of the terrain, based on the X and Z
void setTextureCoordinates(float x, float z) {

	// We want to calculate the texture coordinate for the current vertex.
	// To do this, we just take the current x and y and divide them by the
	// MAP_SIZE of our terrain.  This is of course assuming the height map
	// has the same width and height.  Notice that we negate the v value (IE, (u, v)).
	// This is because we need to flip the texture upside down so it lines up correctly.
	// This simple math we use to find the texture coordinate can be explained easily.
	// We know that our terrain is made up of a grid, and that if we want to stretch
	// the entire texture over top of that grid, we just divide the current section
	// of the grid by the total section of the grid, which gives us a ratio from 0
	// to 1.  This works great because the (u, v) coordinates for a texture map go
	// from (0, 0) in the top left corner to (1, 1) in the bottom right corner.

	// Find the (u, v) coordinate for the current vertex
	float u = (float) x  / (float) MAP_SIZE;
	float v = -(float) z / (float) MAP_SIZE;
	
	// Instead of using the normal API for assigning the current texture
	// coordinate, we want to use the glMultiTexCoord2fARB() function.
	// This will allow us to choose the (u, v) coordinate for every texture
	// map, since we are using multitexturing.  Due to the fact that we
	// are going to be using the texture matrix to set the tiling of our
	// detail texture, we just assign the same (u, v) coordinate for both
	// textures.  

	// Give OpenGL the current terrain texture coordinate for our height map
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, u, v);
	
	// Give OpenGL the current detail texture coordinate for our height map
	if(TILING_MODE == TILE_MODE_SCALING) {

		// Use the 'texture' array to scale/tile as per requirement
		glMultiTexCoord2fARB(GL_TEXTURE1_ARB, u, v);
	}
	else 
	if(TILING_MODE == TILE_MODE_MANUAL) {

		// OR
		// Manually tile the 'detail' texture as per requirement

		u = (float) x  / (float) MAP_SIZE * iDetailScale;
		v = -(float) z / (float) MAP_SIZE * iDetailScale;

		glMultiTexCoord2fARB(GL_TEXTURE1_ARB, u, v);
	}
}

// This turns heightmap data into primitives and draws them to the screen
// This renders the height map as QUADS
void renderHeightMap(BYTE* pHeightMap) {

	int X = 0, Y = 0;					// Create some variables to walk the array with.
	int x, y, z;						// Create some variables for readability
	bool bSwitchSides = false;

	if(!pHeightMap)						// Make sure our height data is valid
		return;
	
	// Activate the first texture ID and bind the tree background to it
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[TEX_ID_TERRAIN]);

	// If we want detail texturing on, let's render the second texture
	if(bDetail) {

		// Activate the second texture ID and bind the fog texture to it
		glActiveTextureARB(GL_TEXTURE1_ARB);
		glEnable(GL_TEXTURE_2D);

		// Here we turn on the COMBINE properties and increase our RGB
		// gamma for the detail texture.  2 seems to work just right.
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 2);

		// Bind the detail texture
		glBindTexture(GL_TEXTURE_2D, textures[TEX_ID_DETAIL]);
		

		//////////////////// NEEDED ONLY FOR 'TILE_MODE_SCALING' ////////////////////
		// Now we want to enter the texture matrix.  This will allow us
		// to change the tiling of the detail texture.
		glMatrixMode(GL_TEXTURE);

			// Reset the current matrix and apply our chosen scale value
			glLoadIdentity();
			glScalef(	(float)(TILING_MODE == TILE_MODE_SCALING)?iDetailScale:1, 
						(float)(TILING_MODE == TILE_MODE_SCALING)?iDetailScale:1, 
						1
					);

		// Leave the texture matrix and set us back in the model view matrix
		glMatrixMode(GL_MODELVIEW);
		////////////////////////////////////////////////////////////////////////////

	}

	// We want to render triangle strips
	glBegin(GL_TRIANGLE_STRIP);

		// Go through all of the rows of the height map
		for(X = 0; X < MAP_SIZE; X += STEP_SIZE) {

			// Chechk if we need to render the opposite way for this column
			if(bSwitchSides) {
				/*				X = 0
				Y = 0			.
								|\
								| \
								|  \
								|___\X + STEP_SIZE 
								.
								|\
								| \
								|  \
								|___\X + STEP_SIZE
								.
								.
								.
								.
								.							/\
								.						   /  \
								|\							||
								| \							||
								|  \						||
								|___\X + STEP_SIZE
				Y = MAP_SIZE
				*/

				// Render a column of the terrain, for this current X.
				// We start at MAP_SIZE and render down to 0.
				for(Y = MAP_SIZE; Y >= 0; Y -= STEP_SIZE) {

					// Get the (X, Y, Z) value for the top left vertex		
					x = X;
					y = getHeight(pHeightMap, X, Y );	
					z = Y;
/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
					// Set the fog coordinate for this vertex, depending on it's height
					// and the current depth of the fog.
					setFogCoord(g_FogDepth, (float)y);
/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
					// Set the current texture coordinate and render the vertex
					setTextureCoordinates((float) x, (float) z);
					glVertex3i(x, y, z);			// Send this vertex to OpenGL to be rendered (integer points are faster)

					// Get the (X, Y, Z) value for the top right vertex		
					x = X + STEP_SIZE; 
					y = getHeight(pHeightMap, X + STEP_SIZE, Y ); 
					z = Y;
/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
					// Set the fog coordinate for this vertex, depending on it's height
					// and the current depth of the fog.
					setFogCoord(g_FogDepth, (float)y);
/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
					// Set the current texture coordinate and render the vertex
					setTextureCoordinates((float) x, (float) z);
					glVertex3i(x, y, z);			// Send this vertex to OpenGL to be rendered
				}
			}
			else {
				/*			X = 0	  X + STEP_SIZE 
				Y = 0			 _____
								 \    |						||
								  \   |						||
								   \  |					    ||
								    \ |					   \  /
								     \|                     \/
								      .X + STEP_SIZE 
								 _____
								 \    |
								  \   |
								   \  |
								    \ | 
								     \|
								      .X + STEP_SIZE 
									  .
									  .
									  .
									  .
									  .
				       			 _____
								 \    |
								  \   |
								   \  |
								    \ | 
								     \|
				Y = MAP_SIZE		  .X + STEP_SIZE 

				*/
				// Render a column of the terrain, for this current X.
				// We start at 0 and render down up to MAP_SIZE.
				for ( Y = 0; Y <= MAP_SIZE; Y += STEP_SIZE ) {

					// Get the (X, Y, Z) value for the bottom right vertex		
					x = X + STEP_SIZE; 
					y = getHeight(pHeightMap, X + STEP_SIZE, Y ); 
					z = Y;
/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
					// Set the fog coordinate for this vertex, depending on it's height
					// and the current depth of the fog.
					setFogCoord(g_FogDepth, (float)y);
/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
					// Set the current texture coordinate and render the vertex
					setTextureCoordinates( (float)x, (float)z );
					glVertex3i(x, y, z);



					// Get the (X, Y, Z) value for the bottom left vertex		
					x = X;							
					y = getHeight(pHeightMap, X, Y );	
					z = Y;
/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
					// Set the fog coordinate for this vertex, depending on it's height
					// and the current depth of the fog.
					setFogCoord(g_FogDepth, (float)y);
/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
					// Set the current texture coordinate and render the vertex
					setTextureCoordinates( (float)x, (float)z );
					glVertex3i(x, y, z);		
				}
			}

			// Switch the direction the column renders to allow the fluid tri strips
			bSwitchSides = !bSwitchSides;
		}
	glEnd();

	// Now let's clean up our multitexturing so it doesn't effect anything else

	// Turn the second multitexture pass off
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glDisable(GL_TEXTURE_2D);

	// Turn the first multitexture pass off
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glDisable(GL_TEXTURE_2D);
}


/////////////////////////////////////////////////////////////////////////////////
//
// * QUICK NOTES * 
//
// Though being one of the simplest ways to display terrain from a height map,
// it's a good start.  The next step we would take would be to make a quad tree
// that only displayed the vertices that we could see from our view frustum.
// This would allow us to display large terrains, but not have to push so many polygons.
// This is a form of space partitioning.  There isn't as many good places to learn
// about space partitioning as I would like, but the best place is www.GameInstitute.com.
// They have an excellent terrain rendering course that deals with all of these problems,
// including line of sight, terrain rendering techniques, adaptable meshes, geomorphing, etc..
// 
// Let's go over the steps that we accomplished during this tutorial
//
// 1) First, we need to read the height map from the .raw file.  This is simple because
//    there is no header to a .raw file, it is just the image bits.  This file format
//    isn't what you generally want to use because you have to either know what the
//    size and type are, or guess, but I thought it fitting for this tutorial.
// 
// 2) After we read our height map data, we then needed to display it.  This was
//    also a simple function because we are just making QUADS with a set size.
//    I chose to do 16 by 16 quads, but you can change this to what ever you want.
//    With our height map array, we treated it as a 2D array and did 2 for loops
//    to draw each quad for each row and column.  Instead of doing lighting, I
//    just gave each vertex a green intensity. depending on it's height.  This makes
//    the terrain look like there is lighting applied.  This also makes it easier to
//    see the definition in the terrain until lighting and texture maps are applied.
//    
// 
// That's pretty much it.  As for doing texturing, this would be a good way to do
// texture tiling.  You can go into a paint program and create a blank
// file the size of your height map.  This will allow you to set up your textures
// to see how you want it to look.  Then, make a 2D array that stores an index
// into your texture array, then just bind that texture to the current quad that matches. 
// it's X and Y position.  Maybe we will write a tutorial on this. 
// 
// The other option is to let OpenGL find the texture coordinates for you, or you can
// find them yourself by stretching a texture over the whole thing (which isn't always desirable),
// or wrap it.  Remember, (0, 0) is the top left corner and (1,1) is the bottom right corner.
// You will have to find the ratio you want to wrap it with.  Here is an example:
// 
// glTexCoord2f(   (float)x / (float)MAP_SIZE,	
//				 - (float)z / (float)MAP_SIZE	);
//
// Make sure you cast integers to floats when necessary otherwise it will truncate it.
// The z is actually the Y value of the height map, but the z value in world coordinates.
// Check out the next height map tutorial to see texturing a height map in action,
// along with rendering the terrain using triangle strips.
//
// Good luck!
//
//
// Ben Humphrey (DigiBen)
// Game Programmer
// DigiBen@GameTutorials.com
// Co-Web Host of www.GameTutorials.com
//
//