
#include "main.h"

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

// This file handles all of the terrain functions.  
//
// * NOTE * 
//
// The terrain might look small on the screen, and considering how fast we
// move about it, but it's actually quite huge and could be a whole world 
// to walk around.  So don't be fooled :)
//
//
//

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

	// Give OpenGL the current texture coordinate for our height map
	glTexCoord2f(	(float) x  / (float) MAP_SIZE,
					-(float) z / (float) MAP_SIZE
				);
}

// This turns heightmap data into primitives and draws them to the screen
// This renders the height map as QUADS
void renderHeightMap(BYTE* pHeightMap) {

	int X = 0, Y = 0;					// Create some variables to walk the array with.
	int x, y, z;						// Create some variables for readability
	bool bSwitchSides = false;

	if(!pHeightMap)						// Make sure our height data is valid
		return;
	
	// The difference from the way we render the terrain from
	// the original way we started with, is that we don't use GL_QUADS
	// anymore, we now use a GL_TRIANGLE_STIP.  This means that we don't
	// need to pass in the same vertex more than once.  Each 2 vertices
	// are connected to the next 2.  Since we want to do this in one strip,
	// we are going to need to reverse the order every other column.  It's
	// like moving the lawn.  Go to the end and turn around and come back
	// the way you came.  If you don't do it this way, you will get polygons
	// stretching across the whole terrain.  We could just do a new glBegin()
	// and glEnd() for every column, but I believe this way is faster.  
	// Not that that really matters though, because rendering a terrain
	// with glVertex*() calls in incredibly slow.  We will most likely want
	// to eventually switch this to vertex arrays.

	// Bind the terrain texture to our terrain
	glBindTexture(GL_TEXTURE_2D, textures[TEX_ID_TERRAIN]);

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
					// Set the color value of the current vertice
					setTextureCoordinates((float) x, (float) z);
					glVertex3i(x, y, z);			// Send this vertex to OpenGL to be rendered (integer points are faster)

					// Get the (X, Y, Z) value for the top right vertex		
					x = X + STEP_SIZE; 
					y = getHeight(pHeightMap, X + STEP_SIZE, Y ); 
					z = Y;
					// Set the color value of the current vertice
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
					// Set the current texture coordinate and render the vertex
					setTextureCoordinates( (float)x, (float)z );
					glVertex3i(x, y, z);

					// Get the (X, Y, Z) value for the bottom left vertex		
					x = X;							
					y = getHeight(pHeightMap, X, Y );	
					z = Y;
					// Set the current texture coordinate and render the vertex
					setTextureCoordinates( (float)x, (float)z );
					glVertex3i(x, y, z);		
				}
			}

			// Switch the direction the column renders to allow the fluid tri strips
			bSwitchSides = !bSwitchSides;
		}
	glEnd();

	// Reset the color
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

}

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
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