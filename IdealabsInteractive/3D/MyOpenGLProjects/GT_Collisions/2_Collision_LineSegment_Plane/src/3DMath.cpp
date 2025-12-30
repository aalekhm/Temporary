
#include "main.h"
#include <math.h>	// We include math.h so we can use the sqrt() function

// Because in our future tutorials we will be reusing all of this code quite often,
// we will put it in a separate .cpp file.  Also this is nice because then we can
// just copy 3DMath.cpp and 3DMath.h into a new project without having to cut and paste code.
// Eventually, once this gets big enough you will probably want to make a library out of it.

// *FINDING THE NORMAL OF A POLYGON*

// In order to find the normal of a polygon, we need to take the cross product of 2
// vectors from that polygon.  What that means, is that we need to get the directions
// of 2 of the sides of our triangle.  After all, that is all a vector is, just a direction
// and a length.  The length of our vectors in this case isn't really important.  We are
// just going to use the direction.  Then, with those 2 vectors we can find a vector coming
// straight out of the polygon, perpendicular (90 degrees) to the polygon.  To give you some
// examples of a cross product of 2 vectors, let's assume that we have a piece of paper
// lying flat on a table.  Where would the cross product of that paper be pointing?
// Straight up!  But, the paper isn't a polygon really?  Well, all you are really getting
// is the normal to a plane.  A plane is infinite, it goes on forever.  If you drew a triangle
// in the middle of the piece of paper, the normal of that triangle would be the same as the paper,
// so it doesn't matter does it?  That's why you are really getting a normal to the polygon's plane.
// If you stuck the piece of paper straight up vertically, where would the normal be?  It would 
// follow the paper perfectly as a 90 degree angle, so it would now we perfectly horizontal.  Now, 
// depending on the order of your vertices, it could either being pointing out on one side of the paper, 
// or on the other side.  You just have to decide which order you want to use, then ALWAYS stick
// with that.  Usually, you want to do something called back face culling which doesn't
// draw the back of a polygon.  That way you aren't drawing twice as much as you could be.
// Think about it, it you have a huge 3D model, do you really need to draw the inside of
// the model?  Of course not, that would be wasteful.  You always want to have your objects
// enclosed so they don't show back of polygons.  Like in a 3D maze, you would just have
// polygon thin walls, you would have thick walls which comprised of a cube like shape.
// That way, when you get into BSP trees, you can do something called Solid Node BSP tree's.


/////////////////////////////////////// CROSS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This returns a perpendicular vector from 2 given vectors by taking the cross product.
/////
/////////////////////////////////////// CROSS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
CVector3 cross(CVector3 vVector1, CVector3 vVector2) {
	
	CVector3 vNormal;		// The vector to hold the cross product

	// Once again, if we are given 2 vectors (directions of 2 sides of a polygon)
	// then we have a plane define.  The cross product finds a vector that is perpendicular
	// to that plane, which means it's point straight out of the plane at a 90 degree angle.
	// The equation for the cross product is simple, but difficult at first to memorize:
	
	// The X value for the vector is:  (V1.y * V2.z) - (V1.z * V2.y)													// Get the X value
	vNormal.x = ((vVector1.y * vVector2.z) - (vVector1.z * vVector2.y));

	// The Y value for the vector is:  (V1.z * V2.x) - (V1.x * V2.z)
	vNormal.y = ((vVector1.z * vVector2.x) - (vVector1.x * vVector2.z));
	
	// The Z value for the vector is:  (V1.x * V2.y) - (V1.y * V2.x)
	vNormal.z = ((vVector1.x * vVector2.y) - (vVector1.y * vVector2.x));

	// *IMPORTANT* This is not communitive. You can not change the order or this or
	// else it won't work correctly.  It has to be exactly like that.  Just remember,
	// If you are trying to find the X, you don't use the X value of the 2 vectors, and
	// it's the same for the Y and Z.  You notice you use the other 2, but never that axis.
	// If you look at the camera rotation tutorial, you will notice it's the same for rotations.

	// Now we have a vector pointing straight out of the plane of the polygon.  This is the
	// direction the polygon is facing.  You use this information for numerous things like
	// lighting, camera viewing, back face culling, collision and tons of other stuff.
	
	return vNormal;		// Return the cross product (Direction the polygon is facing - Normal)
}

//This returns a vector between 2 points
CVector3 makeVector(CVector3 vPoint1, CVector3 vPoint2) {
	
	CVector3 vVector = {0};		// Initialize our variable to zero
	
	// In order to get a vector from 2 points (a direction) we need to
	// subtract the second point from the first point.
	vVector.x = vPoint1.x - vPoint2.x;	// Get the X value of our new vector
	vVector.y = vPoint1.y - vPoint2.y;	// Get the Y value of our new vector
	vVector.z = vPoint1.z - vPoint2.z;	// Get the Z value of our new vector
	
	// Now that we have our new vector between the 2 points, we will return it.
	return vVector;		// Return our new vector
}

//This returns the magnitude of a normal (or any other vector)
float magnitude(CVector3 vVector) {
	
	// This will give us the magnitude or "Norm" as some say of, our normal.
	// The magnitude has to do with the length of the vector.  We use this
	// information to normalize a vector, which gives it a length of 1.
	// Here is the equation:  magnitude = sqrt(V.x^2 + V.y^2 + V.z^2)   Where V is the vector

	return (float)sqrt( (vVector.x * vVector.x) + (vVector.y * vVector.y) + (vVector.z * vVector.z) );
}

//This returns a normalize vector (A vector exactly of length 1)
CVector3 normalize(CVector3 vVector) {

	// Get the magnitude of our normal
	float mag = magnitude(vVector);

	// Now that we have the magnitude, we can divide our normal by that magnitude.
	// That will make our normal a total length of 1.  This makes it easier to work with too.

	vVector.x = vVector.x / mag;		// Divide the X value of our vector by it's magnitude
	vVector.y = vVector.y / mag;		// Divide the Y value of our vector by it's magnitude
	vVector.z = vVector.z / mag;		// Divide the Z value of our vector by it's magnitude

	// Finally, return our normalized vector.
	return vVector;							// Return the new normal of length 1.
}

//This returns the normal of a polygon (The direction the polygon is facing)
CVector3 normal(CVector3 vTriangle[]) {

	// You might want to error check to make sure it's valid
	// Get 2 vectors from the polygon

	CVector3 vVector1 = makeVector(vTriangle[2], vTriangle[0]);
	CVector3 vVector2 = makeVector(vTriangle[1], vTriangle[0]);

	// We Chose to get the vectors surrounding the first point of the polygon.
	// We could have chosen to get 2 other sides of the triangle, but we chose these 2.
	// Now that we have the 2 side vectors, we will take their cross product.
	// (*NOTE*) It is important that pass in the vector of the bottom side of the triangle
	// first, and then pass in the vector of the left side second.  If we switch them,
	// it will turn the normal the opposite way.  Try it, switch them like this: Cross(vVector2, vVector1);
	// Like I said before, it's important to ALWAYS work in the same direction.  In our case,
	// we chose that we always work counter-clockwise.

	CVector3 vNormal = cross(vVector1, vVector2);

	// Now that we have the direction of the normal, we want to do one last thing.
	// Right now, it's an unknown length, it is probably pretty long in length.
	// We want to do something which gives the normal a length of 1.  This is called
	// normalizing.  To do this we divide the normal by it's magnitude.  Well how do we
	// find it's magnitude? We use this equation: magnitude = sqrt(x^2 + y^2 + z^2)
	
	vNormal = normalize(vNormal);		// Use our function we created to normalize the normal (Makes it a length of one)

	// Now return the normalized normal
	// (*NOTE*) If you want to understand what normalizing our normal does, comment out
	// the above line.  Then you can see how long the normal is before we normalize it.
	// I strongly recommend this.  And remember, it doesn't matter how long the normal is,
	// (of course besides (0, 0, 0)), if we normalize it, it will always be of length 1 afterwards.

	return vNormal;						// Return our normal at our desired length
}

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

//This returns the distance between a plane and Origin
float planeDistance(CVector3 vNormal, CVector3 vPoint) {

	float distance = 0.0f;				// This variable holds the distance from the plane to the point

	// Use the plane equation to find the distance (Ax + By + Cz + D = 0)  We want to find D.
	// For more information about the plane equation, read about it in the function below (IntersectedPlane())
	// Basically, A B C is the X Y Z value of our normal and the x y z is our x y z of our point.
	// D is the distance from the origin.  So, we need to move this equation around to find D.
	// We come up with D = -(Ax + By + Cz)
	
	// Basically, the negated dot product of the normal of the plane and the point. (More about the dot product in another tutorial)
	//D = -N DOT P, where N - normal, P - a Point on the plane.
	//D = -(A, B, C) DOT (x, y, z)
	//D = -(Ax + By + Cz)
	//so, 
	distance = -((vNormal.x*vPoint.x) + (vNormal.y*vPoint.y) + (vNormal.z*vPoint.z));
char logText[255] = {0};
sprintf(logText, "distance = %f", distance);
//MessageBox(NULL, logText, "logText", MB_OK);
	// Return the distance
	return distance;
}

//This checks to see if a line intersects a plane
bool intersectedPlane(CVector3 vTriangle[], CVector3 vLine[]) {
	
	// The distances from the 2 points of the line from the plane
	float distance1 = 0.0, distance2 = 0.0;
	
	// We need to get the normal of our plane to go any further
	CVector3 vNormal = normal(vTriangle);

	// Now that we have the normal, we need to calculate the distance our triangle is from the Origin.
	// Since we would have the same triangle, but -10 down the z axis, we need to know
	// how far our plane is to the origin.  The origin is (0, 0, 0), so we need to find
	// the shortest distance our plane is from (0, 0, 0).  This way we can test the collision.
	// The direction the plane is facing is important (We know this by the normal), but it's
	// also important WHERE that plane is in our 3D space.  I hope this makes sense.

	// We created a function to calculate the distance for us.  All we need is the normal
	// of the plane, and then ANY point located on that plane.  Well, we have 3 points.  Each
	// point of the triangle is on the plane, so we just pass in one of our points.  It doesn't
	// matter which one, so we will just pass in the first one.  We get a single value back.
	// That is the distance.  Just like our normalized normal is of length 1, our distance
	// is a single value too.  It's like if you were to measure something with a ruler,
	// you don't measure it according to the X Y and Z of our world, you just want ONE number.

	float distanceFromOrigin = planeDistance(vNormal, vTriangle[0]);

	// Now the next step is simple, but hard to understand at first.  What we need to
	// do is get the distance of EACH point from our plane.  Above we got the distance of the
	// plane to the point (0, 0, 0) which happens to be the origin, now we need to get a distance
	// for each point.  If the distance is a negative number, then the point is BEHIND the plane.
	// If the distance is positive, then the point is in front of the plane.  Basically, if the
	// line collides with the plane, there should be a negative and positive distance.  make sense?
	// If the line pierces the plane, it will have a negative distance and a positive distance,
	// meaning that a point will be on one side of the plane, and one point on the other.  But we
	// will do the check after this, first we need to get the distance of each point to the plane.

	// Now, we need to use something called the plane equation to get the distance from each point.
	// Here is the plane Equation:  (Ax + By + Cz + D = The distance from the plane)
	// If "The distance from the plane" is 0, that means that the point is ON the plane, which all the polygon points should be.
	// A, B and C is the Normal's X Y and Z values.  x y and z is the Point's x y and z values.
	// "the Point" meaning one of the points of our line.  D is the distance that the plane
	// is from the origin.  We just calculated that and stored it in "distanceFromOrigin".
	// Let's fill in the equation with our data:

	// Get the distance from point1 from the plane using: Ax + By + Cz + D = (The distance from the plane)
	distance1 = (vNormal.x * vLine[0].x) +						//Ax +
				(vNormal.y * vLine[0].y) +						//By +
				(vNormal.z * vLine[0].z) + distanceFromOrigin;	//Cx + D

	// We just got the first distance from the first point to the plane, now let's get the second.
	
	// Get the distance from point2 from the plane using Ax + By + Cz + D = (The distance from the plane)
	distance2 = (vNormal.x * vLine[1].x) +						//Ax +
				(vNormal.y * vLine[1].y) +						//By +
				(vNormal.z * vLine[1].z) + distanceFromOrigin;	//Cz + D

char logText[255] = {0};
sprintf(logText, "distance1, distance2 = %f, %f", distance1, distance2);
//MessageBox(NULL, logText, "logText", MB_OK);

	// Ok, we should have 2 distances from the plane, from each point of our line.
	// Remember what I said about an intersection?  If one is negative and one is positive,
	// that means that they are both on either side of the plane.  So, all we need to do
	// is multiply the 2 distances together, and if the result is less than 0, we intersection.
	// This works because, any number times a negative number is always negative, IE (-1 * 1 = -1)
	// If they are both positive or negative values then it will be above zero.
	
	// Check to see if both point's distances are both negative or both positive
	if(distance1 * distance2 >= 0)
		return false;					// Return false if each point has the same sign.  -1 and 1 would mean each point is on either side of the plane.  -1 -2 or 3 4 wouldn't...

	
	// The line intersected the plane, Return TRUE
	return true;
}

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

