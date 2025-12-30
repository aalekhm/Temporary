
#ifndef _3DMATH_H_
#define _3DMATH_H_

// This file will be added upon heavily in the future tutorials, especially for collision.
// Math is a intricate part of game programming, especially in 3D game programming.
// If you are not comfortable with the concepts explain, make it a huge goal to practice
// and strive to understand them.  You can not be an effective 3D programmer without
// these basics.  Don't get too proud if you do, there is still much more after we cover
// all the basics :)  The Dot Product and Plain Equation are our next topics.

// This is used to hold 3D points and vectors. This class is also used in our camera
// tutorials.  Please refer back to those for more information.  Basically, we just
// want a class that holds 3 floats for our x, y, and z values.  This will be added upon later. 

struct CVector2 {
	public:
		float x, y;
};

struct CVector3 {
	public:
		float x, y, z;
};

//	This returns a perpendicular vector from 2 given vectors by taking the cross product.
CVector3 cross(CVector3 vVector1, CVector3 vVector2);

//	This returns a vector between 2 points
CVector3 makeVector(CVector3 vPoint1, CVector3 vPoint2);

//	This returns the magnitude of a normal (or any other vector)
float magnitude(CVector3 vVector);

//	This returns a normalize vector (A vector exactly of length 1)
CVector3 normalize(CVector3 vVector);

//	This returns the normal of a polygon (The direction the polygon is facing)
CVector3 normal(CVector3 vTriangle[]);

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
// This returns the distance the plane is from the origin (0, 0, 0)
// It takes the normal to the plane, along with ANY point that lies on the plane (any corner)
float planeDistance(CVector3 vNormal, CVector3 vPoint);

// This takes a triangle (plane) and line and returns true if they intersected
bool intersectedPlane(CVector3 vTriangle[], CVector3 vLine[]);
/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

#endif

