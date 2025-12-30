
#include "MD3.h"

GLuint *texture;

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ NEW
///////////////////////////////////////////////////////////////////////////////////
//
// This version of the tutorial incorporates the animation data stored in the MD3
// character files.  We will be reading in the .cfg file that stores the animation
// data.  The rotations and translations of the models will be done using a matrix.
// There will be no more calls to glTranslatef().  To create the rotation and
// translation matrix, quaternions will be used.  This is because quaternions
// are excellent for interpolating between 2 rotations, as well as not overriding
// another translation causing "gimbal lock".
// 
// So, why do we need to interpolate?  Well, the animations for the character are
// stored in key frames.  Instead of saving each frame of an animation, key frames
// are stored to cut down on memory and disk space.  The files would be huge if every
// frame was saved for every animation, as well as creating a huge memory footprint.  
// Can you imagine having 10+ models in memory with all of that animation data?  
// 
// The animation key frames are stored in 2 ways.  The torso and legs mesh have vertices
// stored for each of the key frames, along with separate rotations and translations
// for the basic bone animation.  Remember, that each .md3 represents a bone, that needs
// to be connected at a joint.  For instance, the torso is connected to the legs, and the
// head is connected to the torso.  So, that makes 3 bones and 2 joints.  If you add the
// weapon, the weapon is connected to the hand joint, which gives us 4 bones and 3 joints.
// Unlike conventional skeletal animation systems, the main animations of the character's
// movement, such as a gesture or swimming animation, are done not with bones, but with 
// vertex key frames, like in the .md2 format. Since the lower, upper, head and weapon models
// are totally different models, which aren't seamlessly connected to each other, then parent
// node needs to end a message (a translation and rotation) down to all it's child nodes to
// tell them where they need to be in order for the animation to look right.  A good example
// of this is when the legs has the DEATH3 animation set,  The legs might kick back into a back
// flip that lands the character on their face, dead.  Well, since the main models are separate,
// if the legs didn't tell the torso where to go, then the model's torso would stay in the same
// place and the body would detach itself from the legs.  The exporter calculates all this stuff
// for you of course.
// 
// But getting back to the interpolation, since we use key frames, if we didn't interpolate
// between them, the animation would look very jumping and unnatural.  It would also go too
// fast.  By interpolating, we create a smooth transition between each key frame.
//
// As seen in the .md2 tutorials, interpolating between vertices is easy if we use the
// linear interpolation function:  p(t) = p0 + t(p1 - p0).  The same goes for translations,
// since it's just 2 3D points.  This is not so for the rotations.  The Quake3 character
// stores the rotations for each key frame in a 3x3 matrix.  This isn't a simple linear
// interpolation that needs to be performed.  If we convert the matrices to a quaternion,  
// then use spherical linear interpolation (SLERP) between the current frame's quaternion 
// and the next key frame's quaternion, we will have a new interpolated quaternion that
// can be converted into a 4x4 matrix to be applied to the current model view matrix in OpenGL.
// After finding the interpolated translation to be applied, we can slip that into the rotation
// matrix before it's applied to the current matrix, which will require only one matrix command.
//
// You'll notice that in the CreateFromMatrix() function in our quaternion class, I allow a
// row and column count to be passed in.  This is just a dirty way to allow a 3x3 or 4x4 matrix
// to be passed in.  Instead of creating a whole new function and copy and pasting the main 
// code, it seemed fitting for a tutorial.  It's obvious that the quaternion class is missing
// a tremendous amount of functions, but I chose to only keep the functions that we would use.
// 
// For those of you who don't know what interpolation us, here is a section abstracted 
// from the MD2 Animation tutorial:
//
// -------------------------------------------------------------------------------------
// Interpolation: Gamedev.net's Game Dictionary say interpolation is "using a ratio 
// to step gradually a variable from one value to another."  In our case, this
// means that we gradually move our vertices from one key frame to another key frame.
// There are many types of interpolation, but we are just going to use linear.
// The equation for linear interpolation is this:
//
//				p(t) = p0 + t(p1 - p0)
//
//				t - The current time with 0 being the start and 1 being the end
//				p(t) - The result of the equation with time t
//				p0 - The starting position
//				p1 - The ending position
//
// Let's throw in an example with numbers to test this equation.  If we have
// a vertex stored at 0 along the X axis and we wanted to move the point to
// 10 with 5 steps, see if you can fill in the equation without a time just yet.
//
// Finished?  You should have come up with:
//
//				p(t) = 0 + t(10 - 0)
//				p(t) = 0 + 10t
//				p(t) = 10t
//
// Now, all we need it a time from 0 to 1 to pass in, which will allow us to find any
// point from 0 to 10, depending on the time.  Since we wanted to find out the distance
// we need to travel each frame if we want to reach the end point in 5 steps, we just
// divide 1 by 5: 1/5 = 0.2
//
// We can then pass this into our equation:
//
//				p(0.2) = 10 * 0.2
//				p(0.2) = 2
//
// What does that tell us?  It tells us we need to move the vertex along the x
// axis each frame by a distance of 2 to reach 10 in 5 steps.  Yah yah, this isn't
// rocket science, but it's important to know that what your mind would have done
// immediately without thinking about it, is linear interpolation.  
//
// Are you starting to see how this applies to our model?  If we only read in key
// frames, then we need to interpolate every vertex between the current and next
// key frame for animation.  To get a perfect idea of what is going on, try
// taking out the interpolation and just render the key frames.  You will notice
// that you can still see what is kinda going on, but it moves at an incredible pace!
// There is not smoothness, just a really fast jumpy animation.
// ------------------------------------------------------------------------------------
//
// Let's jump into the code (hold your breath!)
//
//


// This function converts a quaternion to a rotation matrix
void CQuaternion::createMatrix(float *pMatrix) {
	// Make sure the matrix has allocated memory to store the rotation data
	if(!pMatrix)
		return;

	// This function is a necessity when it comes to doing almost anything
	// with quaternions.  Since we are working with OpenGL, which uses a 4x4
	// homogeneous matrix, we need to have a way to take our quaternion and
	// convert it to a rotation matrix to modify the current model view matrix.
	// We pass in a 4x4 matrix, which is a 1D array of 16 floats.  This is how OpenGL
	// allows us to pass in a matrix to glMultMatrixf(), so we use a single dimensioned array.
	// After about 300 trees murdered and 20 packs of chalk depleted, the
	// mathematicians came up with these equations for a quaternion to matrix converion:
	//
	//     ¦        2     2												 ¦
    //     ¦ 1 - (2y  + 2z )   2xy + 2zw         2xz - 2yw			0	 ¦
    //     ¦															 ¦
    //     ¦                          2     2							 ¦
    // M = ¦ 2xy - 2zw         1 - (2x  + 2z )   2zy + 2xw			0	 ¦
    //     ¦															 ¦
    //     ¦                                            2     2			 ¦
    //     ¦ 2xz + 2yw         2yz - 2xw         1 - (2x  + 2y )	0	 ¦
    //     ¦															 ¦
	//     ¦															 ¦
	//     ¦ 0				   0				 0					1	 |													 ¦
	//     ¦															 ¦
	// 
	// This is of course a 4x4 matrix.  Notice that a rotational matrix can just
	// be a 3x3 matrix, but since OpenGL uses a 4x4 matrix, we need to conform to the man.
	// Remember that the identity matrix of a 4x4 matrix has a diagonal of 1's, where
	// the rest of the indices are 0.  That is where we get the 0's lining the sides, and
	// the 1 at the bottom-right corner.  Since OpenGL matrices are row by column, we fill
	// in our matrix accordingly below.
	
	// Fill in the rows of the 4x4 matrix, according to the quaternion to matrix equations
	// First row
	pMatrix[ 0] = 1.0f - 2.0f * (y*y + z*z);
	pMatrix[ 1] = 2.0f * (x*y + z*w);
	pMatrix[ 2] = 2.0f * (x*z - y*w);
	pMatrix[ 3] = 0.0f;

	// Second row
	pMatrix[ 4] = 2.0f * ( x * y - z * w );  
	pMatrix[ 5] = 1.0f - 2.0f * ( x * x + z * z ); 
	pMatrix[ 6] = 2.0f * (z * y + x * w );  
	pMatrix[ 7] = 0.0f;  

	// Third row
	pMatrix[ 8] = 2.0f * ( x * z + y * w );
	pMatrix[ 9] = 2.0f * ( y * z - x * w );
	pMatrix[10] = 1.0f - 2.0f * ( x * x + y * y );  
	pMatrix[11] = 0.0f;  

	// Fourth row
	pMatrix[12] = 0;  
	pMatrix[13] = 0;  
	pMatrix[14] = 0;  
	pMatrix[15] = 1.0f;

	// Now pMatrix[] is a 4x4 homogeneous matrix that can be applied to an OpenGL Matrix
}

// This creates a quaternion from a 3x3 or a 4x4 matrix, depending on rowColumnCount
void CQuaternion::createFromMatrix(float *pTheMatrix, int rowColumnCount) {
	
	// Make sure the matrix has valid memory and it's not expected that we allocate it.
	// Also, we do a check to make sure the matrix is a 3x3 or a 4x4 (must be 3 or 4).
	if(!pTheMatrix || ((rowColumnCount != 3) && (rowColumnCount != 4)))
		return;

	// This function is used to take in a 3x3 or 4x4 matrix and convert the matrix
	// to a quaternion.  If rowColumnCount is a 3, then we need to convert the 3x3
	// matrix passed in to a 4x4 matrix, otherwise we just leave the matrix how it is.
	// Since we want to apply a matrix to an OpenGL matrix, we need it to be 4x4.

	// Point the matrix pointer to the matrix passed in, assuming it's a 4x4 matrix
	float *pMatrix = pTheMatrix;
	
	// Create a 4x4 matrix to convert a 3x3 matrix to a 4x4 matrix (If rowColumnCount == 3)
	float m4x4[16] = {0};

	// If the matrix is a 3x3 matrix (which it is for Quake3), then convert it to a 4x4
	if(rowColumnCount == 3) {
		// Set the 9 top left indices of the 4x4 matrix to the 9 indices in the 3x3 matrix.
		// It would be a good idea to actually draw this out so you can visualize it.
		m4x4[0]  = pTheMatrix[0];	m4x4[1]  = pTheMatrix[1];	m4x4[2]  = pTheMatrix[2];
		m4x4[4]  = pTheMatrix[3];	m4x4[5]  = pTheMatrix[4];	m4x4[6]  = pTheMatrix[5];
		m4x4[8]  = pTheMatrix[6];	m4x4[9]  = pTheMatrix[7];	m4x4[10] = pTheMatrix[8];
		
		// Since the bottom and far right indices are zero, set the bottom right corner to 1.
		// This is so that it follows the standard diagonal line of 1's in the identity matrix.
		m4x4[15] = 1;

		// Set the matrix pointer to the first index in the newly converted matrix
		pMatrix = &m4x4[0];
	}

	// The next step, once we made sure we are dealing with a 4x4 matrix, is to check the
	// diagonal of the matrix.  This means that we add up all of the indices that comprise
	// the standard 1's in the identity matrix.  If you draw out the identity matrix of a
	// 4x4 matrix, you will see that they 1's form a diagonal line.  Notice we just assume
	// that the last index (15) is 1 because it is not effected in the 3x3 rotation matrix.

	// Find the diagonal of the matrix by adding up it's diagonal indices.
	// This is also known as the "trace", but I will call the variable diagonal.
	float diagonal = pMatrix[0] + pMatrix[5] + pMatrix[10] + 1;
	float scale = 0.0f;

	// Below we check if the diagonal is greater than zero.  To avoid accidents with
	// floating point numbers, we substitute 0 with 0.00000001.  If the diagonal is
	// great than zero, we can perform an "instant" calculation, otherwise we will need
	// to identify which diagonal element has the greatest value.  Note, that it appears
	// that %99 of the time, the diagonal IS greater than 0 so the rest is rarely used.

	// If the diagonal is greater than zero
	if(diagonal > 0.00000001) {
		// Calculate the scale of the diagonal
		scale = float(sqrt(diagonal ) * 2);

		// Calculate the x, y, x and w of the quaternion through the respective equation
		x = ( pMatrix[9] - pMatrix[6] ) / scale;
		y = ( pMatrix[2] - pMatrix[8] ) / scale;
		z = ( pMatrix[4] - pMatrix[1] ) / scale;
		w = 0.25f * scale;
	}
	else {
		// If the first element of the diagonal is the greatest value
		if ( pMatrix[0] > pMatrix[5] && pMatrix[0] > pMatrix[10] )  
		{	
			// Find the scale according to the first element, and double that value
			scale  = (float)sqrt( 1.0f + pMatrix[0] - pMatrix[5] - pMatrix[10] ) * 2.0f;

			// Calculate the x, y, x and w of the quaternion through the respective equation
			x = 0.25f * scale;
			y = (pMatrix[4] + pMatrix[1] ) / scale;
			z = (pMatrix[2] + pMatrix[8] ) / scale;
			w = (pMatrix[9] - pMatrix[6] ) / scale;	
		} 
		// Else if the second element of the diagonal is the greatest value
		else if ( pMatrix[5] > pMatrix[10] ) 
		{
			// Find the scale according to the second element, and double that value
			scale  = (float)sqrt( 1.0f + pMatrix[5] - pMatrix[0] - pMatrix[10] ) * 2.0f;
			
			// Calculate the x, y, x and w of the quaternion through the respective equation
			x = (pMatrix[4] + pMatrix[1] ) / scale;
			y = 0.25f * scale;
			z = (pMatrix[9] + pMatrix[6] ) / scale;
			w = (pMatrix[2] - pMatrix[8] ) / scale;
		} 
		// Else the third element of the diagonal is the greatest value
		else 
		{	
			// Find the scale according to the third element, and double that value
			scale  = (float)sqrt( 1.0f + pMatrix[10] - pMatrix[0] - pMatrix[5] ) * 2.0f;

			// Calculate the x, y, x and w of the quaternion through the respective equation
			x = (pMatrix[2] + pMatrix[8] ) / scale;
			y = (pMatrix[9] + pMatrix[6] ) / scale;
			z = 0.25f * scale;
			w = (pMatrix[4] - pMatrix[1] ) / scale;
		}
	}
}

// Returns a spherical linear interpolated quaternion between q1 and q2, with respect to t
CQuaternion CQuaternion::Slerp(CQuaternion &q1, CQuaternion &q2, float t)
{
	// Create a local quaternion to store the interpolated quaternion
	CQuaternion qInterpolated;

	// This function is the milk and honey of our quaternion code, the rest of
	// the functions are an appendage to what is done here.  Everyone understands
	// the terms, "matrix to quaternion", "quaternion to matrix", "create quaternion matrix",
	// "quaternion multiplication", etc.. but "SLERP" is the stumbling block, even a little 
	// bit after hearing what it stands for, "Spherical Linear Interpolation".  What that
	// means is that we have 2 quaternions (or rotations) and we want to interpolate between 
	// them.  The reason what it's called "spherical" is that quaternions deal with a sphere.  
	// Linear interpolation just deals with 2 points primarily, where when dealing with angles
	// and rotations, we need to use sin() and cos() for interpolation.  If we wanted to use
	// quaternions for camera rotations, which have much more instant and jerky changes in 
	// rotations, we would use Spherical-Cubic Interpolation.  The equation for SLERP is this:
	//
	// q = (((b.a)^-1)^t)a
	//
	// Go here for an a detailed explanation and proofs of this equation:
	//
	// http://www.magic-software.com/Documentation/quat.pdf
	//
	// Now, Let's code it

	// Here we do a check to make sure the 2 quaternions aren't the same, return q1 if they are
	if(q1.x == q2.x && q1.y == q2.y && q1.z == q2.z && q1.w == q2.w) 
		return q1;

	// Following the (b.a) part of the equation, we do a dot product between q1 and q2.
	// We can do a dot product because the same math applied for a 3D vector as a 4D vector.
	float result = (q1.x * q2.x) + (q1.y * q2.y) + (q1.z * q2.z) + (q1.w * q2.w);

	// If the dot product is less than 0, the angle is greater than 90 degrees
	if(result < 0.0f)
	{
		// Negate the second quaternion and the result of the dot product
		q2 = CQuaternion(-q2.x, -q2.y, -q2.z, -q2.w);
		result = -result;
	}

	// Set the first and second scale for the interpolation
	float scale0 = 1 - t, scale1 = t;

	// Next, we want to actually calculate the spherical interpolation.  Since this
	// calculation is quite computationally expensive, we want to only perform it
	// if the angle between the 2 quaternions is large enough to warrant it.  If the
	// angle is fairly small, we can actually just do a simpler linear interpolation
	// of the 2 quaternions, and skip all the complex math.  We create a "delta" value
	// of 0.1 to say that if the cosine of the angle (result of the dot product) between
	// the 2 quaternions is smaller than 0.1, then we do NOT want to perform the full on 
	// interpolation using.  This is because you won't really notice the difference.

	// Check if the angle between the 2 quaternions was big enough to warrant such calculations
	if(1 - result > 0.1f)
	{
		// Get the angle between the 2 quaternions, and then store the sin() of that angle
		float theta = (float)acos(result);
		float sinTheta = (float)sin(theta);

		// Calculate the scale for q1 and q2, according to the angle and it's sine value
		scale0 = (float)sin( ( 1 - t ) * theta) / sinTheta;
		scale1 = (float)sin( ( t * theta) ) / sinTheta;
	}	

	// Calculate the x, y, z and w values for the quaternion by using a special
	// form of linear interpolation for quaternions.
	qInterpolated.x = (scale0 * q1.x) + (scale1 * q2.x);
	qInterpolated.y = (scale0 * q1.y) + (scale1 * q2.y);
	qInterpolated.z = (scale0 * q1.z) + (scale1 * q2.z);
	qInterpolated.w = (scale0 * q1.w) + (scale1 * q2.w);

	// Return the interpolated quaternion
	return qInterpolated;
}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ NEW

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

bool IsInString(string strString, string strSubString)
{
	// Make sure both of these strings are valid, return false if any are empty
	if(strString.length() <= 0 || strSubString.length() <= 0) return false;

	// grab the starting index where the sub string is in the original string
	unsigned int index = strString.find(strSubString);

	// Make sure the index returned was valid
	if(index >= 0 && index < strString.length())
		return true;

	// The sub string does not exist in strString.
	return false;
}

CMD3Model::CMD3Model() {
	// Here we initialize all our mesh structures for the character
	memset(&m_Head,  0, sizeof(t3DModel));
	memset(&m_Upper, 0, sizeof(t3DModel));
	memset(&m_Lower, 0, sizeof(t3DModel));
	memset(&m_Weapon, 0, sizeof(t3DModel));

	mModelTextures = new GLuint[MAX_TEXTURES];
	glGenTextures(MAX_TEXTURES, &mModelTextures[0]);
}

CMD3Model::~CMD3Model() {
	// Here we free all of the meshes in our model
	destroyModel(&m_Head);
	destroyModel(&m_Upper);
	destroyModel(&m_Lower);
	destroyModel(&m_Weapon);
}

//This loads our Quake3 model from the given path and character name
bool CMD3Model::loadModel(LPSTR strModelPath, LPSTR strModelName) {

	// This function is where all the character loading is taken care of.  We use
	// our CLoadMD3 class to load the 3 mesh and skins for the character. Since we
	// just have 1 name for the model, we add that to _lower.md3, _upper.md3 and _head.md3
	// to load the correct mesh files.

	char	strLowerModelFileName[255] = {0};	// This stores the file name for the lower.md3 model
	char	strUpperModelFileName[255] = {0};	// This stores the file name for the upper.md3 model
	char	strHeadModelFileName[255] = {0};	// This stores the file name for the head.md3 model
	char	strLowerSkinFileName[255] = {0};	// This stores the file name for the lower.md3 skin
	char	strUpperSkinFileName[255] = {0};	// This stores the file name for the upper.md3 skin
	char	strHeadSkinFileName[255] = {0};		// This stores the file name for the head.md3 skin
	CMD3Loader *md3Loader = new CMD3Loader();

	// Make sure valid path and model names were passed in
	if(!strModelPath || !strModelName)
		return false;

	// Store the correct files names for the .md3 and .skin file for each body part.
	// We concatinate this on top of the path name to be loaded from.
	sprintf(strLowerModelFileName, "%s\\%s_lower.md3", strModelPath, strModelName);
	sprintf(strUpperModelFileName, "%s\\%s_upper.md3", strModelPath, strModelName);
	sprintf(strHeadModelFileName, "%s\\%s_head.md3", strModelPath, strModelName);

	// Get the skin file names with their path
	sprintf(strLowerSkinFileName, "%s\\%s_lower.skin", strModelPath, strModelName);
	sprintf(strUpperSkinFileName, "%s\\%s_upper.skin", strModelPath, strModelName);
	sprintf(strHeadSkinFileName, "%s\\%s_head.skin", strModelPath, strModelName);

	// Next we want to load the character meshes.  The CModelMD3 class has member
	// variables for the head, upper and lower body parts.  These are of type t3DModel.
	// Depending on which model we are loading, we pass in those structures to ImportMD3.
	// This returns a true of false to let us know that the file was loaded okay.  The
	// appropriate file name to load is passed in for the last parameter.
	
	// Load the head mesh (*_head.md3) and make sure it loaded properly
	if(!md3Loader->importMD3(&m_Head, strHeadModelFileName)) {
		// Display an error message telling us the file could not be found
		MessageBox(NULL, "Unable to load the HEAD model!", "Error", MB_OK);
		return false;
	}

	// Load the lower mesh (*_lower.md3) and make sure it loaded properly
	if(!md3Loader->importMD3(&m_Lower, strLowerModelFileName)) {
		// Display an error message telling us the file could not be found
		MessageBox(NULL, "Unable to load the LOWER model!", "Error", MB_OK);
		return false;
	}

	// Load the upper mesh (*_upper.md3) and make sure it loaded properly
	if(!md3Loader->importMD3(&m_Upper, strUpperModelFileName)) {
		// Display an error message telling us the file could not be found
		MessageBox(NULL, "Unable to load the UPPER model!", "Error", MB_OK);
		return false;
	}

	// Load the upper skin (*_upper.skin) and make sure it loaded properly
	if(!md3Loader->loadSkin(&m_Upper, strUpperSkinFileName)) {
		// Display an error message telling us the file could not be found
		MessageBox(NULL, "Unable to load the UPPER skin!", "Error", MB_OK);
		return false;
	}

	// Load the lower skin (*_lower.skin) and make sure it loaded properly
	if(!md3Loader->loadSkin(&m_Lower, strLowerSkinFileName)) {
		// Display an error message telling us the file could not be found
		MessageBox(NULL, "Unable to load the LOWER skin!", "Error", MB_OK);
		return false;
	}

	// Load the head skin (*_Head.skin) and make sure it loaded properly
	if(!md3Loader->loadSkin(&m_Head, strHeadSkinFileName)) {
		// Display an error message telling us the file could not be found
		MessageBox(NULL, "Unable to load the HEAD skin!", "Error", MB_OK);
		return false;
	}

	// Once the models and skins were loaded, we need to load then textures.
	// We don't do error checking for this because we call CreateTexture() and 
	// it already does error checking.  Most of the time there is only
	// one or two textures that need to be loaded for each character.  There are
	// different skins though for each character.  For instance, you could have a
	// army looking Lara Croft, or the normal look.  You can have multiple types of
	// looks for each model.  Usually it is just color changes though.

	// Load the lower, upper and head textures.  
	loadModelTextures(&m_Lower, strModelPath);
	loadModelTextures(&m_Upper, strModelPath);
	loadModelTextures(&m_Head,  strModelPath);

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ NEW
	// We added to this function the code that loads the animation config file

	// This stores the file name for the .cfg animation file
	char strConfigFileName[255] = {0};

	// Add the path and file name prefix to the animation.cfg file
	sprintf(strConfigFileName, "%s\\%s_animation.cfg", strModelPath, strModelName);

	// Load the animation config file (*_animation.config) and make sure it loaded properly
	if(!loadAnimations(strConfigFileName)) {
		// Display an error message telling us the file could not be found
		MessageBox(NULL, "Unable to load the Animation Config File!", "Error", MB_OK);
		return false;
	}
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ NEW

	// The character data should all be loaded when we get here (except the weapon).
	// Now comes the linking of the body parts.  This makes it so that the legs (lower.md3)
	// are the parent node, then the torso (upper.md3) is a child node of the legs.  Finally,
	// the head is a child node of the upper body.  What I mean by node, is that you can
	// think of the model having 3 bones and 2 joints.  When you translate the legs you want
	// the whole body to follow because they are inseparable (unless a magic trick goes wrong).
	// The same goes for the head, it should go wherever the body goes.  When we draw the
	// lower body, we then recursively draw all of it's children, which happen to be just the
	// upper body.  Then we draw the upper body's children, which is just the head.  So, to
	// sum this all up, to set each body part's children, we need to link them together.
	// For more information on tags, refer to the Quick Notes and the functions below.

	// Link the lower body to the upper body when the tag "tag_torso" is found in our tag array
	linkModel(&m_Lower, &m_Upper, "tag_torso");

	// Link the upper body to the head when the tag "tag_head" is found in our tag array
	linkModel(&m_Upper, &m_Head, "tag_head");

	// The character was loaded correctly so return true
	return true;
}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ NEW

//This loads the .cfg file that stores all the animation information
bool CMD3Model::loadAnimations(LPSTR strConfigFile) {

	// This function is given a path and name to an animation config file to load.
	// The implementation of this function is arbitrary, so if you have a better way
	// to parse the animation file, that is just as good.  Whatever works.
	// Basically, what is happening here, is that we are grabbing an animation line:
	//
	// "0	31	0	25		// BOTH_DEATH1"
	//
	// Then parsing it's values.  The first number is the starting frame, the next
	// is the frame count for that animation (endFrame would equal startFrame + frameCount),
	// the next is the looping frames (ignored), and finally the frames per second that
	// the animation should run at.  The end of this line is the name of the animation.
	// Once we get that data, we store the information in our tAnimationInfo object, then
	// after we finish parsing the file, the animations are assigned to each model.  
	// Remember, that only the torso and the legs objects have animation.  It is important
	// to note also that the animation prefixed with BOTH_* are assigned to both the legs
	// and the torso animation list, hence the name "BOTH" :)

	// Create an animation object for every valid animation in the Quake3 Character
	tAnimation animations[MAX_ANIMATIONS] = {0};

	// Open the config file
	ifstream fin(strConfigFile);

	// Here we make sure that the file was found and could be opened
	if(fin.fail()) {
		// Return an unsuccessful retrieval
		return false;
	}

	string strWord = "";				// This stores the current word we are reading in
	string strLine = "";				// This stores the current line we read in
	int currentAnim = 0;				// This stores the current animation count
	int torsoOffset = 0;				// The offset between the first torso and leg animation

	// Here we go through every word in the file until a numeric number if found.
	// This is how we know that we are on the animation lines, and past header info.
	// This of course isn't the most solid way, but it works fine.  It wouldn't hurt
	// to put in some more checks to make sure no numbers are in the header info.
	while( fin >> strWord) {
		// If the first character of the word is NOT a number, we haven't hit an animation line
		if(!isdigit(strWord[0])) {
			getline(fin, strLine);
			continue;
		}

		// If we get here, we must be on an animation line, so let's parse the data.
		// We should already have the starting frame stored in strWord, so let's extract it.
		
		// Get the number stored in the strWord string and create some variables for the rest
		int startFrame = atoi(strWord.c_str());
		int numOfFrames = 0, loopingFrames = 0, framesPerSecond = 0;
		
		// Read in the number of frames, the looping frames, then the frames per second
		// for this current animation we are on.
		fin >> numOfFrames >> loopingFrames >> framesPerSecond;

		// Initialize the current animation structure with the data just read in
		animations[currentAnim].startFrame	= startFrame;
		animations[currentAnim].endFrame	= startFrame + numOfFrames;
		animations[currentAnim].loopingFrames = loopingFrames;
		animations[currentAnim].framesPerSecond = framesPerSecond;
		
		// Read past the "//" and read in the animation name (I.E. "BOTH_DEATH1").
		// This might not be how every config file is set up, so make sure.
		fin >> strLine >> strLine;

		// Copy the name of the animation to our animation structure
		strcpy(animations[currentAnim].animationName, strLine.c_str());

		// If the animation is for both the legs and the torso, add it to their animation list
		if(IsInString(strLine, "BOTH")) {
			// Add the animation to each of the upper and lower mesh lists
			m_Upper.pAnimations.push_back(animations[currentAnim]);
			m_Lower.pAnimations.push_back(animations[currentAnim]);
		}
		// If the animation is for the torso, add it to the torso's list
		else 
		if(IsInString(strLine, "TORSO")) {
			m_Upper.pAnimations.push_back(animations[currentAnim]);
		}
		// If the animation is for the legs, add it to the legs's list
		else {
			// Because I found that some config files have the starting frame for the
			// torso and the legs a different number, we need to account for this by finding
			// the starting frame of the first legs animation, then subtracting the starting
			// frame of the first torso animation from it.  For some reason, some exporters
			// might keep counting up, instead of going back down to the next frame after the
			// end frame of the BOTH_DEAD3 anim.  This will make your program crash if so.
			
			// If the torso offset hasn't been set, set it
			if(!torsoOffset)
				torsoOffset = animations[LEGS_WALKCR].startFrame - animations[TORSO_GESTURE].startFrame;

			// Minus the offset from the legs animation start and end frame.
			animations[currentAnim].startFrame -= torsoOffset;
			animations[currentAnim].endFrame -= torsoOffset;

			// Add the animation to the list of leg animations
			m_Lower.pAnimations.push_back(animations[currentAnim]);
		}

		// Increase the current animation count
		currentAnim++;
	}

	// Store the number if animations for each list by the STL vector size() function
	m_Lower.numOfAnimations = m_Lower.pAnimations.size();
	m_Upper.numOfAnimations = m_Upper.pAnimations.size();
	m_Head.numOfAnimations = m_Head.pAnimations.size();
	m_Weapon.numOfAnimations = m_Weapon.pAnimations.size();

	fin.close();

	// Return a success
	return true;
}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ NEW

//This loads the textures for the current model passed in with a directory
void CMD3Model::loadModelTextures(t3DModel *pModel, LPSTR strTexturePath) {

	// This function loads the textures that are assigned to each mesh and it's
	// sub-objects.  For instance, the Lara Croft character has a texture for the body
	// and the face/head, and since she has the head as a sub-object in the lara_upper.md3 model, 
	// the MD3 file needs to contain texture information for each separate object in the mesh.
	// There is another thing to note too...  Some meshes use the same texture map as another 
	// one. We don't want to load 2 of the same texture maps, so we need a way to keep track of
	// which texture is already loaded so that we don't double our texture memory for no reason.
	// This is controlled with a STL vector list of "strings".  Every time we load a texture
	// we add the name of the texture to our list of strings.  Before we load another one,
	// we make sure that the texture map isn't already in our list.  If it is, we assign
	// that texture index to our current models material texture ID.  If it's a new texture,
	// then the new texture is loaded and added to our characters texture array: m_Textures[].

	// Go through all the materials that are assigned to this model
	for(int i = 0; i < pModel->numOfMaterials; i++) {
		
		// Check to see if there is a file name to load in this material
		if(strlen(pModel->pMaterials[i].textureFileName) > 0) {

			// Create a boolean to tell us if we have a new texture to load
			bool bNewTexture = true;

			// Go through all the textures in our string list to see if it's already loaded
			for(unsigned int j = 0; j < vStrTextures.size(); j++) {
				
				// If the texture name is already in our list of texture, don't load it again.
				if((strcmp(pModel->pMaterials[i].textureFileName, vStrTextures[j].c_str()) == 0)) {

					// We don't need to load this texture since it's already loaded
					bNewTexture = false;

					// Assign the texture index to our current material textureID.
					// This ID will them be used as an index into m_Textures[].
					pModel->pMaterials[i].textureID = j;
				}
			}

			// Make sure before going any further that this is a new texture to be loaded
			if(bNewTexture == false) 
				continue;

			char strFullPath[255] = {0};

			// Add the file name and path together so we can load the texture
			sprintf(strFullPath, "%s\\%s", strTexturePath, pModel->pMaterials[i].textureFileName);

			// We pass in a reference to an index into our texture array member variable.
			// The size() function returns the current loaded texture count.  Initially
			// it will be 0 because we haven't added any texture names to our strTextures list.	
			bitmapLoader(strFullPath, mModelTextures[vStrTextures.size()]);

			// Set the texture ID for this material by getting the current loaded texture count
			pModel->pMaterials[i].textureID = vStrTextures.size();

			// Now we increase the loaded texture count by adding the texture name to our
			// list of texture names.  Remember, this is used so we can check if a texture
			// is already loaded before we load 2 of the same textures.  Make sure you
			// understand what an STL vector list is.  We have a tutorial on it if you don't.
			vStrTextures.push_back(pModel->pMaterials[i].textureFileName);
		}
	}
}

void CMD3Model::linkModel(t3DModel *pModel, t3DModel *pLinkedModel, LPSTR strTagName) {
	
	// Make sure we have a valid model, link and tag name, otherwise quit this function
	if(!pModel || !pLinkedModel || !strTagName)
		return;

	// This function is used to link 2 models together at a psuedo joint.  For instance,
	// if we were animating an arm, we would link the top part of the arm to the shoulder,
	// then the forearm to would be linked to the top part of the arm, then the hand to
	// the forearm.  That is what is meant by linking.  That way, when we rotate the
	// arm at the shoulder, the rest of the arm will move with it because they are attached
	// to the same matrix that moves the top of the arm.  You can think of the shoulder
	// as the arm's parent node, and the rest are children that are subject to move to where
	// ever the top part of the arm goes.  That is how bone/skeletal animation works.
	//
	// So, we have an array of tags that have a position, rotation and name.  If we want
	// to link the lower body to the upper body, we would pass in the lower body mesh first,
	// then the upper body mesh, then the tag "tag_torso".  This is a tag that quake set as
	// as a standard name for the joint between the legs and the upper body.  This tag was
	// saved with the lower.md3 model.  We just need to loop through the lower body's tags,
	// and when we find "tag_torso", we link the upper.md3 mesh too that tag index in our
	// pLinks array.  This is an array of pointers to hold the address of the linked model.
	// Quake3 models are set up in a weird way, but usually you would just forget about a
	// separate array for links, you would just have a pointer to a t3DModel in the tag
	// structure, which in retrospect, you wouldn't have a tag array, you would have
	// a bone/joint array.  Stayed tuned for a bone animation tutorial from scratch.  This
	// will show you exactly what I mean if you are confused.
	
	// Go through all of our tags and find which tag contains the strTagName, then link'em
	for(int i = 0; i < pModel->numOfTags; i++) {

		// If this current tag index has the tag name we are looking for
		if(!_stricmp(pModel->pTags[i].tagName, strTagName)) {

			// Link the model's link index to the link (or model/mesh) and return
			pModel->pLinks[i] = pLinkedModel;
			return;
		}
	}
}

//This loads a Quake3 weapon model from the given path and weapon name
bool CMD3Model::loadWeapon(LPSTR strPath, LPSTR strModel) {
	
	char strWeaponModelFileName[255] = {0};		// This stores the file name for the weapon model
	char strWeaponShaderFileName[255] = {0};	// This stores the file name for the weapon shader.
	CMD3Loader *md3Loader = new CMD3Loader();

	// Make sure the path and model were valid, otherwise return false
	if(!strPath || !strModel)
		return false;

	// Concatenate the path and model name together
	sprintf(strWeaponModelFileName, "%s\\%s.md3", strPath, strModel);

	// Next we want to load the weapon mesh.  The CModelMD3 class has member
	// variables for the weapon model and all it's sub-objects.  This is of type t3DModel.
	// We pass in a reference to this model in to ImportMD3 to save the data read.
	// This returns a true of false to let us know that the weapon was loaded okay.  The
	// appropriate file name to load is passed in for the last parameter.

	// Load the weapon mesh (*.md3) and make sure it loaded properly
	if(!md3Loader->importMD3(&m_Weapon, strWeaponModelFileName)) {

		// Display the error message that we couldn't find the weapon MD3 file and return false
		MessageBox(NULL, "Unable to load the WEAPON model!", "Error", MB_OK);
		return false;
	}

	// Unlike the other .MD3 files, a weapon has a .shader file attached with it, not a
	// .skin file.  The shader file has it's own scripting language to describe behaviors
	// of the weapon.  All we care about for this tutorial is it's normal texture maps.
	// There are other texture maps in the shader file that mention the ammo and sphere maps,
	// but we don't care about them for our purposes.  I gutted the shader file to just store
	// the texture maps.  The order these are in the file is very important.  The first
	// texture refers to the first object in the weapon mesh, the second texture refers
	// to the second object in the weapon mesh, and so on.  I didn't want to write a complex
	// .shader loader because there is a TON of things to keep track of.  It's a whole
	// scripting language for goodness sakes! :)  Keep this in mind when downloading new guns.

	// Add the path, file name and .shader extension together to get the file name and path
	sprintf(strWeaponShaderFileName, "%s\\%s.shader", strPath, strModel);

	// Load our textures associated with the gun from the weapon shader file
	if(!md3Loader->loadShader(&m_Weapon, strWeaponShaderFileName)) {

		// Display the error message that we couldn't find the shader file and return false
		MessageBox(NULL, "Unable to load the SHADER file!", "Error", MB_OK);
		return false;
	}

	// We should have the textures needed for each weapon part loaded from the weapon's
	// shader, so let's load them in the given path.
	loadModelTextures(&m_Weapon, strPath);

	// Just like when we loaded the character mesh files, we need to link the weapon to
	// our character.  The upper body mesh (upper.md3) holds a tag for the weapon.
	// This way, where ever the weapon hand moves, the gun will move with it.

	// Link the weapon to the model's hand that has the weapon tag
	linkModel(&m_Upper, &m_Weapon, "tag_weapon");

	// The weapon loaded okay, so let's return true to reflect this
	return true;
}

//This frees our Quake3 model and all it's associated data
void CMD3Model::destroyModel(t3DModel *pModel) {
	// To free a model, we need to go through every sub-object and delete
	// their model data.  Since there is only one array of tags and links stored
	// for the model and all of it's objects, we need to only free the model's 
	// tags and links once.

	// Go through all the objects in the model
	for(int i = 0; i < pModel->numOfObjects; i++) {
		// Free the faces, normals, vertices, and texture coordinates.
		if(pModel->pObjects[i].pFaces)			delete [] pModel->pObjects[i].pFaces;
		if(pModel->pObjects[i].pVertices)		delete [] pModel->pObjects[i].pVertices;
		if(pModel->pObjects[i].pTexVertices)	delete [] pModel->pObjects[i].pTexVertices;
		if(pModel->pObjects[i].pNormals)		delete [] pModel->pObjects[i].pNormals;
		
		// Free the tags associated with this model
		if(pModel->pTags)
			delete [] pModel->pTags;

		// Free the links associated with this model (We use free because we used malloc())
		if(pModel->pLinks)
			free(pModel->pLinks);
	}
}

//This recursively draws all the character nodes, starting with the legs
void CMD3Model::drawModel() {

	// This is the function that is called by the client (you) when using the 
	// CModelMD3 class object.  You will notice that we rotate the model by
	// -90 degrees along the x-axis.  This is because most modelers have z up
	// so we need to compensate for this.  Usually I would just switch the
	// z and y values when loading in the vertices, but the rotations that
	// are stored in the tags (joint info) are a matrix, which makes it hard
	// to change those to reflect Y up.  I didn't want to mess with that so
	// this 1 rotate will fix this problem.

	// Rotate the model to compensate for the z up orientation that the model was saved
	glRotatef(-90, 1, 0, 0);

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ NEW
	// Since we have animation now, when we draw the model the animation frames need
	// to be updated.  To do that, we pass in our lower and upper models to UpdateModel().
	// There is no need to pass in the head of weapon, since they don't have any animation.

	// Update the leg and torso animations
	updateModel(&m_Lower);
	updateModel(&m_Upper);

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ NEW
	
	// You might be thinking to draw the model we would just call RenderModel()
	// 4 times for each body part and the gun right?  That sounds logical, but since
	// we are doing a bone/joint animation... and the models need to be linked together,
	// we can't do that.  It totally would ignore the tags.  Instead, we start at the
	// root model, which is the legs.  The legs drawn first, then we go through each of
	// the legs linked tags (just the upper body) and then it with the tag's rotation
	// and translation values.  I ignored the rotation in this tutorial since we aren't
	// doing any animation.  I didn't want to overwhelm you with quaternions just yet :)
	// Normally in skeletal animation, the root body part is the hip area.  Then the legs
	// bones are created as children to the torso.  The upper body is also a child to
	// the torso.  Since the legs are one whole mesh, this works out somewhat the same way.
	// This wouldn't work if the feet and legs weren't connected in the same mesh because
	// the feet rotations and positioning don't directly effect the position and rotation
	// of the upper body, the hips do.  If that makes sense...  That is why the root starts
	// at the hips and moves down the legs, and also branches out to the upper body and
	// out to the arms.

	// Draw the first link, which is the lower body.  This will then recursively go
	// through the models attached to this model and drawn them.
	drawLink(&m_Lower);
}

//This draws the current mesh with an effected matrix stack from the last mesh
void CMD3Model::drawLink(t3DModel *pModel) {
	// This function is our recursive function that handles the bone animation
	// so to speak.  We first draw the model that is passed in (first the legs),
	// then go through all of it's tags and draw them.  Notice that when we
	// draw the model that is linked to the current model a new matrix scope
	// is created with glPushMatrix() and glPopMatrix().  This is because each tag
	// has a rotation and translation operation assigned to it.  For instance, when
	// Lara does her back flip death animation, the legs send a rotation and translation 
	// to the rest of the body to be rotated along with the legs as they flip backwards.  
	// If you didn't do this, Lara's body and head would stay in the same place as the
	// legs did a back flipped and landed on the floor.  Of course, this would look really
	// stupid.  A 270-degree rotation to the rest of the body is done for that animation.
	// Keep in mind, the legs mesh is NEVER translated or rotated.  It only rotates and
	// translates the upper parts of the body.  All the rotation and translation of the
	// legs is done in the canned animation that was created in the modeling program.
	// Keep in mind that I ignore the rotation value for that is given in the tag since
	// it doesn't really matter for a static model.  Also, since the rotation is given
	// in a 3x3 rotation matrix, it would be a bit more code that could make you frustrated.
	
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ NEW
	// Though the changes to this function from the previous tutorial aren't huge, they
	// are pretty powerful.  Since animation is in effect, we need to create a rotational
	// matrix for each key frame, at each joint, to be applied to the child nodes of that 
	// object.  We can also slip in the interpolated translation into that same matrix.
	// The big thing in this function is interpolating between the 2 rotations.  The process
	// involves creating 2 quaternions from the current and next key frame, then using
	// slerp (spherical linear interpolation) to find the interpolated quaternion, then
	// converting that quaternion to a 4x4 matrix, adding the interpolated translation
	// to that matrix, then finally applying it to the current model view matrix in OpenGL.
	// This will then effect the next objects that are somehow explicitly or inexplicitly
	// connected and drawn from that joint.

	// Create some local variables to store all this crazy interpolation data
	CQuaternion qQuat, qNextQuat, qInterpolatedQuat;
	float *pMatrix, *pNextMatrix;
	float finalMatrix[16] = {0};
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ NEW


	// Draw the current model passed in (Initially the legs)
	renderModel(pModel);

	// Now we need to go through all of this models tags and draw them.
	for(int i = 0; i < pModel->numOfTags; i++) {
		// Get the current link from the models array of links (Pointers to models)
		t3DModel *pLinkedModel = pModel->pLinks[i];

		// If this link has a valid address, let's draw it!
		if(pLinkedModel) {
			//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ NEW
			// To find the current translation position for this frame of animation, we times
			// the currentFrame by the number of tags, then add i.  This is similar to how
			// the vertex key frames are interpolated.

			// Let's grab the translation for this new model that will be drawn 
			CVector3 vPosition = pModel->pTags[pModel->currentFrame * pModel->numOfTags + i].vTranslatePosition;
			
			// Grab the next key frame translation position
			CVector3 vNextPosition = pModel->pTags[pModel->nextFrame * pModel->numOfTags + i].vTranslatePosition;

			// By using the equation: p(t) = p0 + t(p1 - p0), with a time t,
			// we create a new translation position that is closer to the next key frame.
			vPosition.x = vPosition.x + pModel->t * (vNextPosition.x - vPosition.x);
			vPosition.y = vPosition.y + pModel->t * (vNextPosition.y - vPosition.y);
			vPosition.z = vPosition.z + pModel->t * (vNextPosition.z - vPosition.z);

			// Now comes the more complex interpolation.  Just like the translation, we
			// want to store the current and next key frame rotation matrix, then interpolate
			// between the 2.

			// Get a pointer to the start of the 3x3 rotation matrix for the current frame
			pMatrix = &pModel->pTags[pModel->currentFrame * pModel->numOfTags + i].rotationMatrix[0][0];

			// Get a pointer to the start of the 3x3 rotation matrix for the next frame
			pNextMatrix = &pModel->pTags[pModel->nextFrame * pModel->numOfTags + i].rotationMatrix[0][0];

			// Now that we have 2 1D arrays that store the matrices, let's interpolate them

			// Convert the current and next key frame 3x3 matrix into a quaternion
			qQuat.createFromMatrix( pMatrix, 3);
			qNextQuat.createFromMatrix( pNextMatrix, 3);

			// Using spherical linear interpolation, we find the interpolated quaternion
			qInterpolatedQuat = qQuat.Slerp(qQuat, qNextQuat, pModel->t);

			// Here we convert the interpolated quaternion into a 4x4 matrix
			qInterpolatedQuat.createMatrix( finalMatrix );

			// To cut out the need for 2 matrix calls, we can just slip the translation
			// into the same matrix that holds the rotation.  That is what index 12-14 holds.
			finalMatrix[12] = vPosition.x;
			finalMatrix[13] = vPosition.y;
			finalMatrix[14] = vPosition.z;
			//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ NEW

			// Start a new matrix scope
			glPushMatrix();
				//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ NEW
				// Finally, apply the rotation and translation matrix to the current matrix
				glMultMatrixf(finalMatrix);
				//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ NEW
				
				// Recursively draw the next model that is linked to the current one.
				// This could either be a body part or a gun that is attached to
				// the hand of the upper body model.
				drawLink(pLinkedModel);

			// End the current matrix scope
			glPopMatrix();
		}
	}
}

void CMD3Model::renderModel(t3DModel *pModel) {
	// This function actually does the rendering to OpenGL.  If you have checked out
	// our other file loading tutorials, it looks pretty much the same as those.  I
	// left out the normals though.  You can go to any other loading and copy the code
	// from those. Usually the Quake models creating the lighting effect in their textures
	// anyway.  

	// Make sure we have valid objects just in case. (size() is in the STL vector class)
	if(pModel->pObjects.size() <= 0)
		return;
//char logText[255];
//sprintf(logText, "pModel->numOfObjects == %d", pModel->numOfObjects);
//fwrite(logText, 1, strlen(logText), mLogFile);
//MessageBox(NULL, logText, "logText", MB_OK);
	// Go through all of the objects stored in this model
	for(int i = 0; i < pModel->numOfObjects; i++) {
		// Get the current object that we are displaying
		t3DObject *currentObject = &pModel->pObjects[i];
		
		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ NEW
		// Now that we have animation for our model, we need to interpolate between
		// the vertex key frames.  The .md3 file format stores all of the vertex 
		// key frames in a 1D array.  This means that in order to go to the next key frame,
		// we need to follow this equation:  currentFrame * numberOfVertices
		// That will give us the index of the beginning of that key frame.  We just
		// add that index to the initial face index, when indexing into the vertex array.

		// Find the current starting index for the current key frame we are on
		int currentIndex = pModel->currentFrame * currentObject->numOfVertices;

		// Since we are interpolating, we also need the index for the next key frame
		int nextIndex = pModel->nextFrame * currentObject->numOfVertices; 
		//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ NEW


		// If the object has a texture assigned to it, let's bind it to the model.
		// This isn't really necessary since all models have textures, but I left this
		// in here to keep to the same standard as the rest of the model loaders.
		if(currentObject->bHasTexture) {
			
			//Turn Texture Mapping On
			glEnable(GL_TEXTURE_2D);

			// Grab the texture index from the materialID index into our material list
			int textureID = pModel->pMaterials[currentObject->materialID].textureID;

			// Bind the texture index that we got from the material textureID
			glBindTexture(GL_TEXTURE_2D, mModelTextures[textureID]);
		}
		else {
			
			//Turn Texture Mapping Off
			glDisable(GL_TEXTURE_2D);
		}

		// Start drawing our model triangles
		glBegin(GL_TRIANGLES);
			// Go through all of the faces (polygons) of the object and draw them
			for(int j = 0; j < currentObject->numOfFaces; j++) {
				
				// Go through each vertex of the triangle and draw it.
				for(int whichVertex = 0; whichVertex < 3; whichVertex++) {

					// Get the index for the current point in the face list
					int index = currentObject->pFaces[j].vertexIndex[whichVertex];

					// Make sure there is texture coordinates for this (%99.9 likelyhood)
					if(currentObject->pTexVertices) {
						
						// Assign the texture coordinate to this vertex
						glTexCoord2f(	currentObject->pTexVertices[index].x,
										currentObject->pTexVertices[index].y
									);
					}
					
					//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ NEW
					// Like in the MD2 Animation tutorial, we use linear interpolation
					// between the current and next point to find the point in between,
					// depending on the model's "t" (0.0 to 1.0).

					// Store the current and next frame's vertex by adding the current
					// and next index to the initial index given from the face data.
					CVector3 vPoint1 = currentObject->pVertices[ currentIndex + index ];
					CVector3 vPoint2 = currentObject->pVertices[ nextIndex + index];

					// By using the equation: p(t) = p0 + t(p1 - p0), with a time t,
					// we create a new vertex that is closer to the next key frame.
					glVertex3f(vPoint1.x + pModel->t * (vPoint2.x - vPoint1.x),
							   vPoint1.y + pModel->t * (vPoint2.y - vPoint1.y),
							   vPoint1.z + pModel->t * (vPoint2.z - vPoint1.z));
					//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ NEW
				}
			}

		// Stop drawing polygons
		glEnd();
	}
}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ NEW
// This returns a specific model from the character (kLower, kUpper, kHead, kWeapon)
t3DModel *CMD3Model::getModel(int whichPart) {
	
	// Return the legs model if desired
	if(whichPart == kLower)
		return &m_Lower;

	// Return the torso model if desired
	if(whichPart == kUpper) 
		return &m_Upper;

	// Return the head model if desired
	if(whichPart == kHead) 
		return &m_Head;

	// Return the weapon model
	return &m_Weapon;
}

// This sets the current frame of animation, depending on it's fps and t
void CMD3Model::updateModel(t3DModel *pModel) {
	// Initialize a start and end frame, for models with no animation
	int startFrame = 0;
	int endFrame = 1;

	// This function is used to keep track of the current and next frames of animation
	// for each model, depending on the current animation.  Some models down have animations,
	// so there won't be any change.

	// Here we grab the current animation that we are on from our model's animation list
	tAnimation *pAnim = &pModel->pAnimations[pModel->currentAnim];

	// If there is any animations for this model
	if(pModel->numOfAnimations) {
		// Set the starting and end frame from for the current animation
		startFrame = pAnim->startFrame;
		endFrame = pAnim->endFrame;
	}

	// This gives us the next frame we are going to.  We mod the current frame plus
	// 1 by the current animations end frame to make sure the next frame is valid.
	pModel->nextFrame = (pModel->currentFrame + 1) % endFrame;

	// If the next frame is zero, that means that we need to start the animation over.
	// To do this, we set nextFrame to the starting frame of this animation.
	if(pModel->nextFrame == 0)
		pModel->nextFrame = startFrame;

	// Next, we want to get the current time that we are interpolating by.  Remember,
	// if t = 0 then we are at the beginning of the animation, where if t = 1 we are at the end.
	// Anything from 0 to 1 can be thought of as a percentage from 0 to 100 percent complete.
	setCurrentTime(pModel);
}

//This sets time t for the interpolation between the current and next key frame
void CMD3Model::setCurrentTime(t3DModel *pModel) {

	float elapsedTime = 0.0f;

	// This function is very similar to finding the frames per second.
	// Instead of checking when we reach a second, we check if we reach
	// 1 second / our animation speed. (1000 ms / animationSpeed).
	// That's how we know when we need to switch to the next key frame.
	// In the process, we get the t value for how far we are at to going to the
	// next animation key frame.  We use time to do the interpolation, that way
	// it runs the same speed on any persons computer, regardless of their specs.
	// It might look choppier on a junky computer, but the key frames still be
	// changing the same time as the other persons, it will just be not as smooth
	// of a transition between each frame.  The more frames per second we get, the
	// smoother the animation will be.  Since we are working with multiple models 
	// we don't want to create static variables, so the t and elapsedTime data are 
	// stored in the model's structure.
	
	// Return if there is no animations in this model
	if(!pModel->pAnimations.size())
		return;

	// Get the current time in milliseconds
	float time = (float)GetTickCount();

	// Find the time that has elapsed since the last time that was stored
	elapsedTime = time - pModel->lastTime;

	// Store the animation speed for this animation in a local variable
	int animationSpeed = pModel->pAnimations[pModel->currentAnim].framesPerSecond;

	// To find the current t we divide the elapsed time by the ratio of:
	//
	// (1_second / the_animation_frames_per_second)
	//
	// Since we are dealing with milliseconds, we need to use 1000
	// milliseconds instead of 1 because we are using GetTickCount(), which is in 
	// milliseconds. 1 second == 1000 milliseconds.  The t value is a value between 
	// 0 to 1.  It is used to tell us how far we are from the current key frame to 
	// the next key frame.
	float t = elapsedTime / (1000.0f / animationSpeed);

	// If our elapsed time goes over the desired time segment, start over and go 
	// to the next key frame.
	if (elapsedTime >= (1000.0f / animationSpeed) )
	{
		// Set our current frame to the next key frame (which could be the start of the anim)
		pModel->currentFrame = pModel->nextFrame;

		// Set our last time for the model to the current time
		pModel->lastTime = time;
	}

	// Set the t for the model to be used in interpolation
	pModel->t = t;
}

// This sets the current animation that the upper body will be performing
void CMD3Model::setTorsoAnimation(LPSTR strAnimation) {
	
	// Go through all of the animations in this model
	for(int i = 0; i < m_Upper.numOfAnimations; i++) {
		
		// If the animation name passed in is the same as the current animation's name
		if( !_stricmp(m_Upper.pAnimations[i].animationName, strAnimation) ) {
			
			// Set the legs animation to the current animation we just found and return
			m_Upper.currentAnim = i;
			m_Upper.currentFrame = m_Upper.pAnimations[m_Upper.currentAnim].startFrame;
			return;
		}
	}
}

//This sets the current animation that the lower body will be performing
void CMD3Model::setLegsAnimation(LPSTR strAnimation) {
	// Go through all of the animations in this model
	for(int i = 0; i < m_Lower.numOfAnimations; i++) {
		
		// If the animation name passed in is the same as the current animation's name
		if( !_stricmp(m_Lower.pAnimations[i].animationName, strAnimation) ) {
			
			// Set the legs animation to the current animation we just found and return
			m_Lower.currentAnim = i;
			m_Lower.currentFrame = m_Lower.pAnimations[m_Lower.currentAnim].startFrame;
			return;
		}
	}
}
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ NEW


/////////////////////////////////////////////////////CMD3Loader///////////////////////////////////////////////////////////
CMD3Loader::CMD3Loader() {
	// Here we initialize our structures to 0
	memset(&mMD3Header, 0, sizeof(tMD3Header));

	// Set the pointers to null
	m_pSkins = NULL;
	m_pTexCoords = NULL;
	m_pFaces = NULL;
	m_pBones = NULL;
}

//This is called by the client to open the .Md3 file, read it, then clean up
bool CMD3Loader::importMD3(t3DModel *pModel, LPSTR strFileName) {

	// This function handles the entire model loading for the .md3 models.
	// What happens is we load the file header, make sure it's a valid MD3 model,
	// then load the rest of the data, then call our CleanUp() function.

	// Open the MD3 file in binary
	mFilePointer = fopen(strFileName, "rb");
	
	// Make sure we have a valid file pointer (we found the file)
	if(!mFilePointer) {
		// Display an error message and don't load anything if no file was found
		sprintf(logText, "Unable to find the file: %s!", strFileName);
		MessageBox(NULL, logText, "Error", MB_OK);

		return false;
	}

	// Now that we know the file was found and it's all cool, let's read in
	// the header of the file.  If it has the correct 4 character ID and version number,
	// we can continue to load the rest of the data, otherwise we need to print an error.

	// Read the header data and store it in our m_Header member variable
	fread(&mMD3Header, 1, sizeof(tMD3Header), mFilePointer);

	// Get the 4 character ID
	char *ID = mMD3Header.fileID;
	// The ID MUST equal "IDP3" and the version MUST be 15, or else it isn't a valid
	// .MD3 file.  This is just the numbers ID Software chose.

	// Make sure the ID == IDP3 and the version is this crazy number '15' or else it's a bad egg
	if(	ID[0] != 'I' 
		||
		ID[1] != 'D' 
		||
		ID[2] != 'P' 
		||
		ID[3] != '3' 
		||
		mMD3Header.version != 15
	) {
		// Display an error message for bad file format, then stop loading
		sprintf(logText, "Invalid file format (Version not 15): %s!", strFileName);
		MessageBox(NULL, logText, "Error", MB_OK);

		return false;	
	}

	// Read in the model and animation data
	readMD3Data(pModel);

	// Clean up after everything
	cleanUp();

	// Return a success
	return true;
}

//This loads the texture information for the model from the *.skin file
bool CMD3Loader::loadSkin(t3DModel *pModel, LPSTR skinFileName) {
	// Make sure valid data was passed in
	if(!pModel || !skinFileName) return false;

	// This function is used to load a .skin file for the .md3 model associated
	// with it.  The .skin file stores the textures that need to go with each
	// object and subject in the .md3 files.  For instance, in our Lara Croft model,
	// her upper body model links to 2 texture; one for her body and the other for
	// her face/head.  The .skin file for the lara_upper.md3 model has 2 textures:
	//
	// u_torso,models/players/laracroft/default.bmp
	// u_head,models/players/laracroft/default_h.bmp
	//
	// Notice the first word, then a comma.  This word is the name of the object
	// in the .md3 file.  Remember, each .md3 file can have many sub-objects.
	// The next bit of text is the Quake3 path into the .pk3 file where the 
	// texture for that model is stored  Since we don't use the Quake3 path
	// because we aren't making Quake, I just grab the texture name at the
	// end of the string and disregard the rest.  of course, later this is
	// concatenated to the original MODEL_PATH that we passed into load our character.
	// So, for the torso object it's clear that default.bmp is assigned to it, where
	// as the head model with the pony tail, is assigned to default_h.bmp.  Simple enough.
	// What this function does is go through all the lines of the .skin file, and then
	// goes through all of the sub-objects in the .md3 file to see if their name is
	// in that line as a sub string.  We use our cool IsInString() function for that.
	// If it IS in that line, then we know that we need to grab it's texture file at
	// the end of the line.  I just parse backwards until I find the last '/' character,
	// then copy all the characters from that index + 1 on (I.E. "default.bmp").
	// Remember, it's important to note that I changed the texture files from .tga
	// files to .bmp files because that is what all of our tutorials use.  That way
	// you don't have to sift through tons of image loading code.  You can write or
	// get your own if you really want to use the .tga format.

	// Open the skin file
	ifstream fin(skinFileName);

	// Make sure the file was opened
	if(fin.fail()) {
		// Display the error message and return false
		MessageBox(NULL, "Unable to load skin!", "Error", MB_OK);
		return false;	
	}

	// These 2 variables are for reading in each line from the file, then storing
	// the index of where the bitmap name starts after the last '/' character.
	string strLine = "";
	int textureNameStart = 0;

	// Go through every line in the .skin file
	while(getline(fin, strLine)) {
		// Loop through all of our objects to test if their name is in this line
		for(int i = 0; i < pModel->numOfObjects; i++) {
			// Check if the name of this object appears in this line from the skin file
			// Check if the name of this object appears in this line from the skin file
			if( IsInString(strLine, pModel->pObjects[i].objectName) ) {
				// To abstract the texture name, we loop through the string, starting
				// at the end of it until we find a '/' character, then save that index + 1.
				for(int j = strLine.length() - 1; j > 0; j--)
				{
					// If this character is a '/', save the index + 1
					if(strLine[j] == '/')
					{
						// Save the index + 1 (the start of the texture name) and break
						textureNameStart = j + 1;
						break;
					}
				}

				// Create a local material info structure
				tMaterial material;

				// Copy the name of the file into our texture file name variable.
				// Notice that with string we can pass in the address of an index
				// and it will only pass in the characters from that point on. Cool huh?
				// So now the strFile name should hold something like ("bitmap_name.bmp")
				strcpy(material.textureFileName, &strLine[textureNameStart]);
				
				// The tile or scale for the UV's is 1 to 1 
				material.uTile = material.vTile = 1;

				// Store the material ID for this object and set the texture boolean to true
				pModel->pObjects[i].materialID = pModel->numOfMaterials;
				pModel->pObjects[i].bHasTexture = true;

				// Here we increase the number of materials for the model
				pModel->numOfMaterials++;

				// Add the local material info structure to our model's material list
				pModel->pMaterials.push_back(material);
			}
		}
	}

	// Close the file and return a success
	fin.close();
	return true;
}

//This loads the basic shader texture info associated with the weapon model
bool CMD3Loader::loadShader(t3DModel *pModel, LPSTR shaderFileName) {
	// Make sure valid data was passed in
	if(!pModel || !shaderFileName)
		return false;

	// This function is used to load the .shader file that is associated with
	// the weapon model.  Instead of having a .skin file, weapons use a .shader file
	// because it has it's own scripting language to describe the behavior of the
	// weapon.  There is also many other factors like environment map and sphere map
	// textures, among other things.  Since I am not trying to replicate Quake, I
	// just care about the weapon's texture.  I went through each of the blocks
	// in the shader file and deleted everything except the texture name (of course
	// I changed the .tga files to .bmp for our purposes).  All this file now includes
	// is a texture name on each line.  No parsing needs to be done.  It is important
	// to keep in mind that the order of which these texture are stored in the file
	// is in the same order each sub-object is loaded in the .md3 file.  For instance,
	// the first texture name on the first line of the shader is the texture for
	// the main gun object that is loaded, the second texture is for the second sub-object
	// loaded, and so on. I just want to make sure that you understand that I hacked
	// up the .shader file so I didn't have to parse through a whole language.  This is
	// not a normal .shader file that we are loading.  I only kept the relevant parts.

	// Open the shader file
	ifstream fin(shaderFileName);

	// Make sure the file was opened
	if(fin.fail()) {
		
		// Display the error message and return false
		MessageBox(NULL, "Unable to load shader!", "Error", MB_OK);
		return false;
	}

	// These variables are used to read in a line at a time from the file, and also
	// to store the current line being read so that we can use that as an index for the 
	// textures, in relation to the index of the sub-object loaded in from the weapon model.
	string strLine = "";
	int currentIndex = 0;

	// Go through and read in every line of text from the file
	while(getline(fin, strLine)) {
		
		// Create a local material info structure
		tMaterial material;
			
		// Copy the name of the file into our texture file name variable
		strcpy(material.textureFileName, strLine.c_str());

		// The tile or scale for the UV's is 1 to 1 
		material.uTile = material.vTile = 1;

		// Store the material ID for this object and set the texture boolean to true
		pModel->pObjects[currentIndex].materialID = pModel->numOfMaterials;
		pModel->pObjects[currentIndex].bHasTexture = true;

		// Here we increase the number of materials for the model
		pModel->numOfMaterials++;
		
		// Save the Material in a List
		// Add the local material info structure to our model's material list
		pModel->pMaterials.push_back(material);

		// Here we increase the material index for the next texture (if any)
		currentIndex++;
	}

	// Close the file and return a success
	fin.close();
	return true;
}

//This function reads in all of the model's data, except the animation frames
void CMD3Loader::readMD3Data(t3DModel *pModel) {
	int i = 0;

	// This member function is the BEEF of our whole file.  This is where the
	// main data is loaded.  The frustrating part is that once the data is loaded,
	// you need to do a billion little things just to get the model loaded to the screen
	// in a correct manner.

	// Here we allocate memory for the bone information and read the bones in.
	m_pBones = new tMD3Bone[mMD3Header.numOfFrames];
	fread(m_pBones, sizeof(tMD3Bone), mMD3Header.numOfFrames, mFilePointer);

	// Since we don't care about the bone positions, we just free it immediately.
	// It might be cool to display them so you could get a visual of them with the model.

	// Free the unused bones
	delete [] m_pBones;

	// Next, after the bones are read in, we need to read in the tags.  Below we allocate
	// memory for the tags and then read them in.  For every frame of animation there is
	// an array of tags.
	pModel->pTags = new tMD3Tag[mMD3Header.numOfFrames * mMD3Header.numOfTags];
	fread(pModel->pTags, sizeof(tMD3Tag), mMD3Header.numOfFrames * mMD3Header.numOfTags, mFilePointer);

	// Assign the number of tags to our model
	pModel->numOfTags = mMD3Header.numOfTags;

	// Now we want to initialize our links.  Links are not read in from the .MD3 file, so
	// we need to create them all ourselves.  We use a double array so that we can have an
	// array of pointers.  We don't want to store any information, just pointers to t3DModels.
	// Aa - Links is an pointer to a MODEL
	pModel->pLinks = (t3DModel**) malloc( sizeof(t3DModel) * mMD3Header.numOfTags);
	
	// Initilialize our link pointers to NULL
	for(i = 0; i < mMD3Header.numOfTags; i++)
		pModel->pLinks[i] = NULL;

	// Now comes the loading of the mesh data.  We want to use ftell() to get the current
	// position in the file.  This is then used to seek to the starting position of each of
	// the mesh data arrays.

	// Get the current offset into the file
	long meshOffset = ftell(mFilePointer);

	// Create a local meshHeader that stores the info about the mesh
	tMD3MeshInfo meshHeader;

	// Go through all of the sub-objects in this mesh
	for(i = 0; i < mMD3Header.numOfMeshes; i++) {

		// Seek to the start of this mesh and read in it's header
		fseek(mFilePointer, meshOffset, SEEK_SET);
		fread(&meshHeader, sizeof(tMD3MeshInfo), 1, mFilePointer);

		// Here we allocate all of our memory from the header's information
		m_pSkins		= new tMD3Skin[meshHeader.numOfSkins];
		m_pTexCoords	= new tMD3TexCoord[meshHeader.numOfVertices];
		m_pVertices		= new tMD3Triangle[meshHeader.numOfVertices * meshHeader.numOfMeshFrames];
		m_pFaces		= new tMD3Face[meshHeader.numOfTriangles];

		// Read in the skin information
		fread(m_pSkins, sizeof(tMD3Skin), meshHeader.numOfSkins, mFilePointer);

		// Seek to the start of the triangle/face data, then read it in
		fseek(mFilePointer, meshOffset + meshHeader.offsetTriangleStart, SEEK_SET);
		fread(m_pFaces, sizeof(tMD3Face), meshHeader.numOfTriangles, mFilePointer);

		// Seek to the start of the UV coordinate data, then read it in
		fseek(mFilePointer, meshOffset + meshHeader.offsetUVStart, SEEK_SET);
		fread(m_pTexCoords, sizeof(tMD3TexCoord), meshHeader.numOfVertices, mFilePointer);

		// Seek to the start of the vertex/face index information, then read it in.
		fseek(mFilePointer, meshOffset + meshHeader.offsetVertexStart, SEEK_SET);
		fread(m_pVertices, sizeof(tMD3Triangle), meshHeader.numOfVertices * meshHeader.numOfMeshFrames, mFilePointer);

		// Now that we have the data loaded into the Quake3 structures, let's convert them to
		// our data types like t3DModel and t3DObject.  That way the rest of our model loading
		// code will be mostly the same as the other model loading tutorials.
		convertToDataStructures(pModel, meshHeader);

		// Free all the memory for this mesh since we just converted it to our structures
		delete [] m_pSkins;
		delete [] m_pFaces;
		delete [] m_pTexCoords;
		delete [] m_pVertices;

		// Increase the offset into the file
		meshOffset += meshHeader.meshSize;
	}
}

void CMD3Loader::convertToDataStructures(t3DModel *pModel, tMD3MeshInfo meshHeader) {
	int i = 0;

	// This is function takes care of converting all of the Quake3 structures to our
	// structures that we have been using in all of our mode loading tutorials.  You
	// do not need this function if you are going to be using the Quake3 structures.
	// I just wanted to make it modular with the rest of the tutorials so you (me really) 
	// can make a engine out of them with an abstract base class.  Of course, each model
	// has some different data variables inside of the, depending on each format, but that
	// is perfect for some cool inheritance.  Just like in the .MD2 tutorials, we only
	// need to load in the texture coordinates and face information for one frame
	// of the animation (eventually in the next tutorial).  Where, the vertex information
	// needs to be loaded for every new frame, since it's vertex key frame animation 
	// used in .MD3 models.  Half-life models do NOT do this I believe.  It's just
	// pure bone/skeletal animation.  That will be a cool tutorial if the time ever comes.

	// Increase the number of objects (sub-objects) in our model since we are loading a new one
	pModel->numOfObjects++;

	// Create a empty object structure to store the object's info before we add it to our list
	t3DObject currentObject = {0};

	// Copy the name of the object to our object structure
	strcpy(currentObject.objectName, meshHeader.meshName);

	// Assign the vertex, texture coord and face count to our new structure
	currentObject.numOfVertices			= meshHeader.numOfVertices;
	currentObject.numOfTextureVertex	= meshHeader.numOfVertices;
	currentObject.numOfFaces			= meshHeader.numOfTriangles;

	// Allocate memory for the vertices, texture coordinates and face data.
	// Notice that we multiply the number of vertices to be allocated by the
	// number of frames in the mesh.  This is because each frame of animation has a 
	// totally new set of vertices.  This will be used in the next animation tutorial.
	currentObject.pVertices				= new CVector3[currentObject.numOfVertices * meshHeader.numOfMeshFrames];
	currentObject.pTexVertices			= new CVector2[currentObject.numOfVertices];
	currentObject.pFaces				= new tFace[currentObject.numOfFaces];

	// Go through all of the vertices and assign them over to our structure
	for(i = 0; i < currentObject.numOfVertices * meshHeader.numOfMeshFrames; i++) {

		// For some reason, the ratio 64 is what we need to divide the vertices by,
		// otherwise the model is gargantuanly huge!  If you use another ratio, it
		// screws up the model's body part position.  I found this out by just
		// testing different numbers, and I came up with 65.  I looked at someone
		// else's code and noticed they had 64, so I changed it to that.  I have never
		// read any documentation on the model format that justifies this number, but
		// I can't get it to work without it.  Who knows....  Maybe it's different for
		// 3D Studio Max files verses other software?  You be the judge.  I just work here.. :)
		currentObject.pVertices[i].x = m_pVertices[i].vertex[0] / 64.0f;
		currentObject.pVertices[i].y = m_pVertices[i].vertex[1] / 64.0f;
		currentObject.pVertices[i].z = m_pVertices[i].vertex[2] / 64.0f;
	}

	// Go through all of the uv coords and assign them over to our structure
	for(i = 0; i < currentObject.numOfVertices; i++) {

		// Since I changed the images to bitmaps, we need to negate the V ( or y) value.
		// This is because I believe that TARGA (.tga) files, which were originally used
		// with this model, have the pixels flipped horizontally.  If you use other image
		// files and your texture mapping is crazy looking, try deleting this negative.
		currentObject.pTexVertices[i].x = m_pTexCoords[i].texCoords[0];
		currentObject.pTexVertices[i].y = -m_pTexCoords[i].texCoords[1];
	}

	// Go through all of the face data and assign it over to OUR structure
	for(i = 0; i < currentObject.numOfFaces; i++) {

		// Assign the vertex indices to our face data
		currentObject.pFaces[i].vertexIndex[0] = m_pFaces[i].vertexIndexes[0];
		currentObject.pFaces[i].vertexIndex[1] = m_pFaces[i].vertexIndexes[1];
		currentObject.pFaces[i].vertexIndex[2] = m_pFaces[i].vertexIndexes[2];

		// Assign the texture coord indices to our face data (same as the vertex indices)
		currentObject.pFaces[i].texCoordIndex[0] = m_pFaces[i].vertexIndexes[0];
		currentObject.pFaces[i].texCoordIndex[1] = m_pFaces[i].vertexIndexes[1];
		currentObject.pFaces[i].texCoordIndex[2] = m_pFaces[i].vertexIndexes[2];
	}

	// Here we add the current object to our list object list
	pModel->pObjects.push_back(currentObject);
}

void CMD3Loader::cleanUp() {

	// Since we free all of the member variable arrays in the same function as
	// we allocate them, we don't need to do any other clean up other than
	// closing the file pointer, which could probably also be done in the same
	// function.  I left it here so you can add more of your cleanup if you add
	// to this class. 

	// Close the current file pointer
	fclose(mFilePointer);
	mFilePointer = NULL;
	//fclose(mFileConsoleLog);
	//mFileConsoleLog = NULL;
}

