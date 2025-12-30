
#include "main.h"
#include <math.h>	// We include math.h so we can use the sqrt() function

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

#include <float.h>	// This is so we can use _isnan() for acos()

//
//
// This file was build from the Ray Plane Collision tutorial.
// We added 5 new functions to this math file:  
//
// This returns the dot product between 2 vectors
// float Dot(CVector3 vVector1, CVector3 vVector2);
//
// This returns the angle between 2 vectors
// double AngleBetweenVectors(CVector3 Vector1, CVector3 Vector2);
//
// This returns an intersection point of a polygon and a line (assuming intersects the plane)
// CVector3 IntersectionPoint(CVector3 vNormal, CVector3 vLine[], double distance);
//
// This returns true if the intersection point is inside of the polygon
// bool InsidePolygon(CVector3 vIntersection, CVector3 Poly[], long verticeCount);
//
// Use this function to test collision between a line and polygon
// bool IntersectedPolygon(CVector3 vPoly[], CVector3 vLine[], int verticeCount);
//
// These will enable to check if we internet not just the plane of a polygon,
// but the actual polygon itself.  Once the line is outside the permiter, it will fail
// on a collision test.
//
//
/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *


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
	
	// Return the distance
	return distance;
}

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

// Since the last tutorial, we added 2 more parameters for the normal and the distance
// from the origin.  This is so we don't have to recalculate it 3 times in our IntersectionPoint() 
// IntersectedPolygon() functions.  We would probably make 2 different functions for
// this so we have the choice of getting the normal and distance back, or not.
// I also changed the vTriangle to "vPoly" because it isn't always a triangle.
// The code doesn't change, it's just more correct (though we only need 3 points anyway).
// For C programmers, the '&' is called a reference and is the same concept as the '*' for addressing.

//This checks to see if a line intersects a plane
bool intersectedPlane(CVector3 vPoly[], CVector3 vLine[], CVector3 &vNormal, float &originDistance) {
	
	// The distances from the 2 points of the line from the plane
	float distance1 = 0.0, distance2 = 0.0;
	
	vNormal = normal(vPoly);

	// Let's find the distance our plane is from the origin.  We can find this value
	// from the normal to the plane (polygon) and any point that lies on that plane (Any vertice)
	originDistance = planeDistance(vNormal, vPoly[0]);
	
	// Get the distance from point1 from the plane using: Ax + By + Cz + D = (The distance from the plane)
	distance1 = (vNormal.x * vLine[0].x) +						// Ax +
				(vNormal.y * vLine[0].y) +						// By +
				(vNormal.z * vLine[0].z) + originDistance;		// Cz + D

	distance2 = (vNormal.x * vLine[1].x) +						// Ax +
				(vNormal.y * vLine[1].y) +						// By +
				(vNormal.z * vLine[1].z) + originDistance;		// Cz + D

	// Now that we have 2 distances from the plane, if we times them together we either
	// get a positive or negative number.  If it's a negative number, that means we collided!
	// This is because the 2 points must be on either side of the plane (IE. -1 * 1 = -1).
	if(distance1*distance2 >= 0)		// Check to see if both point's distances are both negative or both positive
		return false;					// Return false if each point has the same sign.  -1 and 1 would mean each point is on either side of the plane.  -1 -2 or 3 4 wouldn't...

	return true;						// The line intersected the plane, Return TRUE
}

//This computers the dot product of 2 vectors
float dot(CVector3 vVector1, CVector3 vVector2) {
	// The dot product is this equation: V1.V2 = (V1.x * V2.x  +  V1.y * V2.y  +  V1.z * V2.z)
	// In math terms, it looks like this:  V1.V2 = ||V1|| ||V2|| cos(theta)
	// The '.' means DOT.   The || || is magnitude.  So the magnitude of V1 times the magnitude
	// of V2 times the cosine of the angle.  It seems confusing now, but it will become more clear.
	// This function is used for a ton of things, which we will cover in other tutorials.
	// For this tutorial, we use it to compute the angle between 2 vectors.  If the vectors
	// are normalize, the dot product returns the cosine of the angle between the 2 vectors.
	// What does that mean? Well, it doesn't return the actual angle, it returns the value of:
	// cos(angle).	Well, what if we want to get the actual angle?  Then we use the arc cosine.
	// There is more on this in the below function AngleBetweenVectors().  Let's give some
	// applications of using the dot product.  How would you tell if the angle between the
	// 2 vectors is perpendicular (90 degrees)?  Well, if we normalize the vectors we can
	// get rid of the ||V1|| * ||V2|| in front, which just leaves us with:  cos(theta).
	// If a vector is normalize, it's magnitude is 1, so it would be: 1 * 1 * cos(theta) , 
	// which is pointless, so we discard that part of the equation.  So, What is the cosine of 90?
	// If you punch it in your calculator you will find that it's 0.  So that means
	// if the dot product of 2 angles is 0, then they are perpendicular.  What we did in
	// our mind is take the arc cosine of 0, which is 90 (or PI/2 in radians).  More on this below.

	//    (V1.x * V2.x        +        V1.y * V2.y        +        V1.z * V2.z)
	
	return ((vVector1.x * vVector2.x) + (vVector1.y * vVector2.y) + (vVector1.z * vVector2.z));
}

//Finds the angle between 2 vectors
double angleBetweenVectors(CVector3 vVector1, CVector3 vVector2) {
	// Remember, above we said that the Dot Product of returns the cosine of the angle
	// between 2 vectors?  Well, that is assuming they are unit vectors (normalize vectors).
	// So, if we don't have a unit vector, then instead of just saying  arcCos(DotProduct(A, B))
	// We need to divide the dot product by the magnitude of the 2 vectors multiplied by each other.
	// Here is the equation:   arc cosine of (V . W / || V || * || W || )
	// the || V || means the magnitude of V.  This then cancels out the magnitudes dot product magnitudes.
	// But basically, if you have normalize vectors already, you can forget about the magnitude part.

	// Get the dot product of the vectors
	float dotProduct = dot(vVector1, vVector2);

	// Get the product of both of the vectors magnitudes
	float vectorsMagnitude = magnitude(vVector1) * magnitude(vVector2);

	// Get the arc cosine of the (dotProduct / vectorsMagnitude) which is the angle in RADIANS.
	// (IE.   PI/2 radians = 90 degrees      PI radians = 180 degrees    2*PI radians = 360 degrees)
	// To convert radians to degress use this equation:   radians * (PI / 180)
	// TO convert degrees to radians use this equation:   degrees * (180 / PI)
	double angle = acos(dotProduct / vectorsMagnitude);

	// Here we make sure that the angle is not a -1.#IND0000000 number, which means indefinate.
	// acos() thinks it's funny when it returns -1.#IND0000000.  If we don't do this check,
	// our collision results will sometimes say we are colliding when we aren't.  I found this
	// out the hard way after MANY hours and already wrong written tutorials :)  Usually
	// this value is found when the dot product and the maginitude are the same value.
	// We want to return 0 when this happens.
	if(_isnan(angle))
		return 0;
	
	// Return the angle in radians
	return angle;
}

bool isPointInsidePolygon(CVector3 vIntersectionPoint, CVector3 vPoly[], long verticeCount) {
	
	const double MATCH_FACTOR = 0.9999;		// Used to cover up the error in floating point
	double angle = 0.0;						// Initialize the angle
	CVector3 vA, vB;						// Create temp vectors

	// Just because we intersected the plane, doesn't mean we were anywhere near the polygon.
	// This functions checks our intersection point to make sure it is inside of the polygon.
	// This is another tough function to grasp at first, but let me try and explain.
	// It's a brilliant method really, what it does is create triangles within the polygon
	// from the intersection point.  It then adds up the inner angle of each of those triangles.
	// If the angles together add up to 360 degrees (or 2 * PI in radians) then we are inside!
	// If the angle is under that value, we must be outside of polygon.  To further
	// understand why this works, take a pencil and draw a perfect triangle.  Draw a dot in
	// the middle of the triangle.  Now, from that dot, draw a line to each of the vertices.
	// Now, we have 3 triangles within that triangle right?  Now, we know that if we add up
	// all of the angles in a triangle we get 180° right?  Well, that is kinda what we are doing,
	// but the inverse of that.  Say your triangle is an equilateral triangle, so add up the angles
	// and you will get 180° degree angles.  60 + 60 + 60 is 360°.

	for(int i = 0; i < verticeCount; i++) {

		vA = makeVector(vPoly[i], vIntersectionPoint);

		vB = makeVector(vPoly[(i + 1)%verticeCount], vIntersectionPoint);

		angle += angleBetweenVectors(vA, vB);
	}

	// Now that we have the total angles added up, we need to check if they add up to 360 degrees.
	// Since we are using the dot product, we are working in radians, so we check if the angles
	// equals 2*PI.  We defined PI in 3DMath.h.  You will notice that we use a MATCH_FACTOR
	// in conjunction with our desired degree.  This is because of the inaccuracy when working
	// with floating point numbers.  It usually won't always be perfectly 2 * PI, so we need
	// to use a little twiddling.  I use .9999, but you can change this to fit your own desired accuracy.
	if(angle >= (MATCH_FACTOR * (2.0 * PI)))		// If the angle is greater than 2 PI, (360 degrees)
		return true;								// The point is inside of the polygon

	return false;									// If you get here, it obviously wasn't inside the polygon, so Return FALSE
}

//This returns the intersection point of the line that intersects the plane
CVector3 intersectionPoint(CVector3 vNormal, CVector3 vLine[], double distance) {
	
	CVector3 vPoint = {0}, vLineDir = {0};
	double numerator = 0.0, denominator = 0.0, dist = 0.0;

	// Here comes the confusing part.  We need to find the 3D point that is actually
	// on the plane.  Here are some steps to do that:
	
	// 1)  First we need to get the vector of our line, Then normalize it so it's a length of 1
	vLineDir = makeVector(vLine[1], vLine[0]);		// Get the Vector of the line
	vLineDir = normalize(vLineDir);					// Normalize the lines vector

	// 2) Use the plane equation (distance = Ax + By + Cz + D) to find the distance from one of our points to the plane.
	//    Here I just chose a arbitrary point as the point to find that distance.  You notice we negate that
	//    distance.  We negate the distance because we want to eventually go BACKWARDS from our point to the plane.
	//    By doing this is will basically bring us back to the plane to find our intersection point.
	numerator = - (	vNormal.x * vLine[0].x + 
					vNormal.y * vLine[0].y + 
					vNormal.z * vLine[0].z + distance);

	// 3) If we take the dot product between our line vector and the normal of the polygon,
	//    this will give us the cosine of the angle between the 2 (since they are both normalized i.e length 1).
	//    We will then divide our Numerator by this value to find the offset towards the plane from our arbitrary point.
	denominator = dot(vNormal, vLineDir);			// Get the dot product of the line's vector and the normal of the plane

	// Since we are using division, we need to make sure we don't get a divide by zero error
	// If we do get a 0, that means that there are INFINITE points because the the line is
	// on the plane (the normal is perpendicular to the line - (Normal.Vector = 0)).  
	// In this case, we should just return any point on the line.

	if(denominator == 0.0)							// Check so we don't divide by zero
		return vLine[0];							// Return an arbitrary point on the line

	// We divide the (distance from the point to the plane) by (the dot product)
	// to get the distance (dist) that we need to move from our arbitrary point.  We need
	// to then times this distance (dist) by our line's vector (direction).  When you times
	// a scalar (single number) by a vector you move along that vector.  That is what we are
	// doing.  We are moving from our arbitrary point we chose from the line BACK to the plane
	// along the lines vector.  It seems logical to just get the numerator, which is the distance
	// from the point to the line, and then just move back that much along the line's vector.
	// Well, the distance from the plane means the SHORTEST distance.  What about in the case that
	// the line is almost parallel with the polygon, but doesn't actually intersect it until half
	// way down the line's length.  The distance from the plane is short, but the distance from
	// the actual intersection point is pretty long.  If we divide the distance by the dot product
	// of our line vector and the normal of the plane, we get the correct length.  Cool huh?

	dist = numerator / denominator;					// Divide to get the multiplying (percentage) factor

	// Now, like we said above, we times the dist by the vector, then add our arbitrary point.
	// This essentially moves the point along the vector to a certain distance.  This now gives
	// us the intersection point.  Yay!

	vPoint.x = (float)(vLine[0].x + (vLineDir.x * dist));
	vPoint.y = (float)(vLine[0].y + (vLineDir.y * dist));
	vPoint.z = (float)(vLine[0].z + (vLineDir.z * dist));

	return vPoint;
	/*
												  (A)vLine[0]
												.  |
											.   (*)|
								  H		.		   |
									.			   |  dY ==> numerator(Shortest distance of point vLine[0] form the Plane)
								.				   |
							.					   |
						.	 					   |
	_______________(P)___________________________(0)__________________________________PLANE
				.				dX	
			.
		.
	(B)vLine[1]	
		
	The Line (A - B) intersects the Plane at point P. 
	
	since
			cos(*) = dY / H;
		ie, adjacentSide / Hypotenuse = dY / H;
		ie, normalized(planeNormal) / normalized(lineDirection) = dY / H;
		ie, denominator = dY / H;
			
	Therefore,
		ie, H = dY / cos(THETA);
		ie, H = numerator / denominator
		ie, H = (Shortest distance of the point from the Plane) / (angle between the LINE vector and the PLANE NORMAL);

	hence, 
		P(x) = A(x) + vLineDir(x)*H
		P(y) = A(y) + vLineDir(y)*H
		P(z) = A(z) + vLineDir(z)*H

		here,	vLineDir can be thought of as the SLOPE of a line as in 2D space.
	*/
}

//This checks if a line is intersecting a polygon
bool intersectedPolygon(CVector3 vPoly[], CVector3 vLine[], int verticeCount) {
	
	CVector3 vNormal = {0};
	float originDistance = 0;

	// First we check to see if our line intersected the plane.  If this isn't true
	// there is no need to go on, so return false immediately.
	// We pass in address of vNormal and originDistance so we only calculate it once
	if(!intersectedPlane(vPoly, vLine, vNormal,	originDistance))
		return false;

	// Now that we have our normal and distance passed back from IntersectedPlane(), 
	// we can use it to calculate the intersection point.  The intersection point
	// is the point that actually is ON the plane.  It is between the line.  We need
	// this point test next, if we are inside the polygon.  To get the I-Point, we
	// give our function the normal of the plan, the points of the line, and the originDistance.
	CVector3 vIntersectionPoint = intersectionPoint(vNormal, vLine, originDistance);

	// Now that we have the intersection point, we need to test if it's inside the polygon.
	// To do this, we pass in :
	// (our intersection point, the polygon, and the number of vertices our polygon has)
	if(isPointInsidePolygon(vIntersectionPoint, vPoly, verticeCount))
		return true;				// We collided!	  Return success

	// If we get here, we must have NOT collided
	return false;					// There was no collision, so return false
	
	

}
/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

