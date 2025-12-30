//***********************************************************************//
//																		 //
//		- "Talk to me like I'm a 3 year old!" Programming Lessons -		 //
//                                                                       //
//		$Author:		DigiBen			DigiBen@GameTutorials.com		 //
//																		 //
//		$Program:		SpherePolygonCollision							 //
//																		 //
//		$Description:	This demonstrates sphere and polygon collision.	 //
//																		 //
//		$Date:			1/23/02											 //
//																		 //
//***********************************************************************//

#include "main.h"
#include <math.h>	// We include math.h so we can use the sqrt() function
#include <float.h>	// This is so we can use _isnan() for acos()


/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
//
//
// This file was build from the Ray Plane Collision tutorial.
// We added 4 new functions to this math file:  
//
// This returns the absolute value of the number passed in
// float absolute(float num);
//
// This tells us if the sphere is in FRONT, BEHIND, or INTERSECTS the plane passed in
// int classifySphere(CVector3 &vCenter, 
//				      CVector3 &vNormal, CVector3 &vPoint, float radius, float &distance);
//
// This returns true if the sphere intersects the edge line segments of the polygon
// bool edgeSphereCollision(CVector3 &vPosition, 
//							CVector3 vPolygon[], int vertexCount, float radius);
//
// This returns true if our sphere collides with the polygon passed in (Client function)
// bool spherePolygonCollision(CVector3 vPolygon[], CVector3 &vCenter, float radius);
//
//
// These functions will enable us to check if we intersect a polygon's plane, and then
// more importantly, the polygon itself.  Look down near the bottom of this file to 
// see these functions and their explanations.  It's pretty intimidating to look at all
// this code and think that every function is used and needed to perform sphere collision
// isn't it?  It is a really bad idea to just copy and paste these functions into your
// own code and never understand what they all do and why.  It is imperative that you
// know what EACH one of these functions does.  If you want to do 3D graphics, you will
// have pretty poor games if you don't have decent collision detection and know how
// to adapt it to your needs.  
//
//


/////////////////////////////////////// ABSOLUTE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This returns the absolute value of the number passed in
/////
/////////////////////////////////////// ABSOLUTE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

float absolute(float num)
{
	// If num is less than zero, we want to return the absolute value of num.
	// This is simple, either we times num by -1 or subtract it from 0.
	if(num < 0)
		return (0 - num);

	// Return the original number because it was already positive
	return num;
}

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *


/////////////////////////////////////// CROSS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This returns a perpendicular vector from 2 given vectors by taking the cross product.
/////
/////////////////////////////////////// CROSS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
												
CVector3 cross(CVector3 vVector1, CVector3 vVector2)
{
	CVector3 vNormal;									// The vector to hold the cross product

	// The X value for the vector is:  (V1.y * V2.z) - (V1.z * V2.y)													// Get the X value
	vNormal.x = ((vVector1.y * vVector2.z) - (vVector1.z * vVector2.y));
														
	// The Y value for the vector is:  (V1.z * V2.x) - (V1.x * V2.z)
	vNormal.y = ((vVector1.z * vVector2.x) - (vVector1.x * vVector2.z));
														
	// The Z value for the vector is:  (V1.x * V2.y) - (V1.y * V2.x)
	vNormal.z = ((vVector1.x * vVector2.y) - (vVector1.y * vVector2.x));

	return vNormal;										// Return the cross product (Direction the polygon is facing - Normal)
}


/////////////////////////////////////// MAGNITUDE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This returns the magnitude of a normal (or any other vector)
/////
/////////////////////////////////////// MAGNITUDE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

float getMagnitude(CVector3 vNormal)
{
	// This will give us the magnitude or "Norm" as some say, of our normal.
	// Here is the equation:  magnitude = sqrt(V.x^2 + V.y^2 + V.z^2)  Where V is the vector

	return (float)sqrt( (vNormal.x * vNormal.x) + (vNormal.y * vNormal.y) + (vNormal.z * vNormal.z) );
}


/////////////////////////////////////// NORMALIZE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This returns a normalize vector (A vector exactly of length 1)
/////
/////////////////////////////////////// NORMALIZE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

CVector3 normalize(CVector3 vNormal)
{
	float magnitude = getMagnitude(vNormal);				// Get the magnitude of our normal

	// Now that we have the magnitude, we can divide our normal by that magnitude.
	// That will make our normal a total length of 1.  This makes it easier to work with too.

	vNormal.x /= magnitude;								// Divide the X value of our normal by it's magnitude
	vNormal.y /= magnitude;								// Divide the Y value of our normal by it's magnitude
	vNormal.z /= magnitude;								// Divide the Z value of our normal by it's magnitude

	// Finally, return our normalized normal.

	return vNormal;										// Return the new normal of length 1.
}


/////////////////////////////////////// NORMAL \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This returns the normal of a polygon (The direction the polygon is facing)
/////
/////////////////////////////////////// NORMAL \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

CVector3 normal(CVector3 vPolygon[])					
{														// Get 2 vectors from the polygon (2 sides), Remember the order!
	CVector3 vVector1 = vPolygon[2] - vPolygon[0];
	CVector3 vVector2 = vPolygon[1] - vPolygon[0];

	CVector3 vNormal = cross(vVector1, vVector2);		// Take the cross product of our 2 vectors to get a perpendicular vector

	// Now we have a normal, but it's at a strange length, so let's make it length 1.

	vNormal = normalize(vNormal);						// Use our function we created to normalize the normal (Makes it a length of one)

	return vNormal;										// Return our normal at our desired length
}

// This calculates a vector between 2 points and returns the result
CVector3 getVector(CVector3 vPoint1, CVector3 vPoint2) {
	CVector3 vVector;							// The variable to hold the resultant vector

	vVector.x = vPoint1.x - vPoint2.x;			// Subtract point1 and point2 x's
	vVector.y = vPoint1.y - vPoint2.y;			// Subtract point1 and point2 y's
	vVector.z = vPoint1.z - vPoint2.z;			// Subtract point1 and point2 z's

	return vVector;								// Return the resultant vector
}

// This adds 2 vectors together and returns the result
CVector3 addVector(CVector3 vVector1, CVector3 vVector2)
{
	CVector3 vResult;							// The variable to hold the resultant vector
	
	vResult.x = vVector2.x + vVector1.x;		// Add Vector1 and Vector2 x's
	vResult.y = vVector2.y + vVector1.y;		// Add Vector1 and Vector2 y's
	vResult.z = vVector2.z + vVector1.z;		// Add Vector1 and Vector2 z's

	return vResult;								// Return the resultant vector
}

// This divides a vector by a single number (scalar) and returns the result
CVector3 divideVectorByScaler(CVector3 vVector1, float Scaler)
{
	CVector3 vResult;							// The variable to hold the resultant vector
	
	vResult.x = vVector1.x / Scaler;			// Divide Vector1's x value by the scaler
	vResult.y = vVector1.y / Scaler;			// Divide Vector1's y value by the scaler
	vResult.z = vVector1.z / Scaler;			// Divide Vector1's z value by the scaler

	return vResult;								// Return the resultant vector
}

/////////////////////////////////// PLANE DISTANCE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This returns the distance between a plane and the origin
/////
/////////////////////////////////// PLANE DISTANCE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
									
float planeDistance(CVector3 Normal, CVector3 Point)
{	
	float distance = 0;									// This variable holds the distance from the plane tot he origin

	// Use the plane equation to find the distance (Ax + By + Cz + D = 0)  We want to find D.
	// So, we come up with D = -(Ax + By + Cz)
														// Basically, the negated dot product of the normal of the plane and the point. (More about the dot product in another tutorial)
	distance = - ((Normal.x * Point.x) + (Normal.y * Point.y) + (Normal.z * Point.z));

	return distance;									// Return the distance
}


/////////////////////////////////// INTERSECTED PLANE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This checks to see if a line intersects a plane
/////
/////////////////////////////////// INTERSECTED PLANE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
											
bool intersectedPlane(CVector3 vPoly[], CVector3 vLine[], CVector3 &vNormal, float &originDistance)
{
	float distance1=0, distance2=0;						// The distances from the 2 points of the line from the plane
			
	vNormal = normal(vPoly);							// We need to get the normal of our plane to go any further

	// Let's find the distance our plane is from the origin.  We can find this value
	// from the normal to the plane (polygon) and any point that lies on that plane (Any vertex)
	originDistance = planeDistance(vNormal, vPoly[0]);

	// Get the distance from point1 from the plane using: Ax + By + Cz + D = (The distance from the plane)

	distance1 = ((vNormal.x * vLine[0].x)  +					// Ax +
		         (vNormal.y * vLine[0].y)  +					// Bx +
				 (vNormal.z * vLine[0].z)) + originDistance;	// Cz + D
	
	// Get the distance from point2 from the plane using Ax + By + Cz + D = (The distance from the plane)
	
	distance2 = ((vNormal.x * vLine[1].x)  +					// Ax +
		         (vNormal.y * vLine[1].y)  +					// Bx +
				 (vNormal.z * vLine[1].z)) + originDistance;	// Cz + D

	// Now that we have 2 distances from the plane, if we times them together we either
	// get a positive or negative number.  If it's a negative number, that means we collided!
	// This is because the 2 points must be on either side of the plane (IE. -1 * 1 = -1).

	if(distance1 * distance2 >= 0)			// Check to see if both point's distances are both negative or both positive
	   return false;						// Return false if each point has the same sign.  -1 and 1 would mean each point is on either side of the plane.  -1 -2 or 3 4 wouldn't...
					
	return true;							// The line intersected the plane, Return TRUE
}


/////////////////////////////////// DOT \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This computers the dot product of 2 vectors
/////
/////////////////////////////////// DOT \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

float dot(CVector3 vVector1, CVector3 vVector2) 
{
	// The dot product is this equation: V1.V2 = (V1.x * V2.x  +  V1.y * V2.y  +  V1.z * V2.z)
	// In math terms, it looks like this:  V1.V2 = ||V1|| ||V2|| cos(theta)
	
			 //    (V1.x * V2.x        +        V1.y * V2.y        +        V1.z * V2.z)
	return ( (vVector1.x * vVector2.x) + (vVector1.y * vVector2.y) + (vVector1.z * vVector2.z) );
}


/////////////////////////////////// ANGLE BETWEEN VECTORS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This checks to see if a point is inside the ranges of a polygon
/////
/////////////////////////////////// ANGLE BETWEEN VECTORS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

double angleBetweenVectors(CVector3 Vector1, CVector3 Vector2)
{							
	// Get the dot product of the vectors
	double dotProduct = dot(Vector1, Vector2);				

	// Get the product of both of the vectors magnitudes
	double vectorsMagnitude = getMagnitude(Vector1) * getMagnitude(Vector2) ;

	// Get the angle in radians between the 2 vectors
	double angle = acos( dotProduct / vectorsMagnitude );

	// Here we make sure that the angle is not a -1.#IND0000000 number, which means indefinate
	if(_isnan(angle))
		return 0;
	
	// Return the angle in radians
	return( angle );
}


/////////////////////////////////// INTERSECTION POINT \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This returns the intersection point of the line that intersects the plane
/////
/////////////////////////////////// INTERSECTION POINT \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
											
CVector3 intersectionPoint(CVector3 vNormal, CVector3 vLine[], double distance)
{
	CVector3 vPoint, vLineDir;					// Variables to hold the point and the line's direction
	double Numerator = 0.0, Denominator = 0.0, dist = 0.0;

	// 1)  First we need to get the vector of our line, Then normalize it so it's a length of 1
	vLineDir = vLine[1] - vLine[0];		// Get the Vector of the line
	vLineDir = normalize(vLineDir);				// Normalize the lines vector


	// 2) Use the plane equation (distance = Ax + By + Cz + D) to find the 
	// distance from one of our points to the plane.
	Numerator = - (vNormal.x * vLine[0].x +		// Use the plane equation with the normal and the line
				   vNormal.y * vLine[0].y +
				   vNormal.z * vLine[0].z + distance);

	// 3) If we take the dot product between our line vector and the normal of the polygon,
	Denominator = dot(vNormal, vLineDir);		// Get the dot product of the line's vector and the normal of the plane
				  
	// Since we are using division, we need to make sure we don't get a divide by zero error
	// If we do get a 0, that means that there are INFINATE points because the the line is
	// on the plane (the normal is perpendicular to the line - (Normal.Vector = 0)).  
	// In this case, we should just return any point on the line.

	if( Denominator == 0.0)						// Check so we don't divide by zero
		return vLine[0];						// Return an arbitrary point on the line

	dist = Numerator / Denominator;				// Divide to get the multiplying (percentage) factor
	
	// Now, like we said above, we times the dist by the vector, then add our arbitrary point.
	vPoint.x = (float)(vLine[0].x + (vLineDir.x * dist));
	vPoint.y = (float)(vLine[0].y + (vLineDir.y * dist));
	vPoint.z = (float)(vLine[0].z + (vLineDir.z * dist));

	return vPoint;								// Return the intersection point
}


/////////////////////////////////// INSIDE POLYGON \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This checks to see if a point is inside the ranges of a polygon
/////
/////////////////////////////////// INSIDE POLYGON \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

bool insidePolygon(CVector3 vIntersection, CVector3 Poly[], long verticeCount)
{
	const double MATCH_FACTOR = 0.99;		// Used to cover up the error in floating point
	double Angle = 0.0;						// Initialize the angle
	CVector3 vA, vB;						// Create temp vectors
	
	for (int i = 0; i < verticeCount; i++)		// Go in a circle to each vertex and get the angle between
	{	
		vA = Poly[i] - vIntersection;			// Subtract the intersection point from the current vertex
												// Subtract the point from the next vertex
		vB = Poly[(i + 1) % verticeCount] - vIntersection;
												
		Angle += angleBetweenVectors(vA, vB);	// Find the angle between the 2 vectors and add them all up as we go along
	}
											
	if(Angle >= (MATCH_FACTOR * (2.0 * PI)) )	// If the angle is greater than 2 PI, (360 degrees)
		return true;							// The point is inside of the polygon
		
	return false;								// If you get here, it obviously wasn't inside the polygon, so Return FALSE
}


/////////////////////////////////// INTERSECTED POLYGON \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This checks if a line is intersecting a polygon
/////
/////////////////////////////////// INTERSECTED POLYGON \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

bool intersectedPolygon(CVector3 vPoly[], CVector3 vLine[], int verticeCount)
{
	CVector3 vNormal;
	float originDistance = 0;

	// First, make sure our line intersects the plane
									 // Reference   // Reference
	if(!intersectedPlane(vPoly, vLine,   vNormal,   originDistance))
		return false;

	// Now that we have our normal and distance passed back from intersectedPlane(), 
	// we can use it to calculate the intersection point.  
	CVector3 vIntersection = intersectionPoint(vNormal, vLine, originDistance);

	// Now that we have the intersection point, we need to test if it's inside the polygon.
	if(insidePolygon(vIntersection, vPoly, verticeCount))
		return true;							// We collided!	  Return success

	return false;								// There was no collision, so return false
}


/////////////////////////////////// DISTANCE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This returns the distance between 2 3D points
/////
/////////////////////////////////// DISTANCE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

float getDistance(CVector3 vPoint1, CVector3 vPoint2)
{
	// This is the classic formula used in beginning algebra to return the
	// distance between 2 points.  Since it's 3D, we just add the z dimension:
	// 
	// Distance = sqrt(  (P2.x - P1.x)^2 + (P2.y - P1.y)^2 + (P2.z - P1.z)^2 )
	//
	double distance = sqrt( (vPoint2.x - vPoint1.x) * (vPoint2.x - vPoint1.x) +
						    (vPoint2.y - vPoint1.y) * (vPoint2.y - vPoint1.y) +
						    (vPoint2.z - vPoint1.z) * (vPoint2.z - vPoint1.z) );

	// Return the distance between the 2 points
	return (float)distance;
}


////////////////////////////// CLOSEST POINT ON LINE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This returns the point on the line vA_vB that is closest to the point vPoint
/////
////////////////////////////// CLOSEST POINT ON LINE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

CVector3 closestPointOnLine(CVector3 vA, CVector3 vB, CVector3 vPoint)
{
	// Create the vector from end point vA to our point vPoint.
	CVector3 vVector1 = vPoint - vA;

	// Create a normalized direction vector from end point vA to end point vB
    CVector3 vVector2 = normalize(vB - vA);

	// Use the distance formula to find the distance of the line segment (or magnitude)
    float d = getDistance(vA, vB);

	// Using the dot product, we project the vVector1 onto the vector vVector2.
	// This essentially gives us the distance from our projected vector from vA.
    float t = dot(vVector2, vVector1);

	// If our projected distance from vA, "t", is less than or equal to 0, it must
	// be closest to the end point vA.  We want to return this end point.
    if (t <= 0) 
		return vA;

	// If our projected distance from vA, "t", is greater than or equal to the magnitude
	// or distance of the line segment, it must be closest to the end point vB.  So, return vB.
    if (t >= d) 
		return vB;
 
	// Here we create a vector that is of length t and in the direction of vVector2
    CVector3 vVector3 = vVector2 * t;

	// To find the closest point on the line segment, we just add vVector3 to the original
	// end point vA.  
    CVector3 vClosestPoint = vA + vVector3;

	// Return the closest point on the line segment
	return vClosestPoint;
}


/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

////////////////////////////// SPHERE POLYGON COLLISION \\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This returns true if our sphere collides with the polygon passed in
/////
////////////////////////////// SPHERE POLYGON COLLISION \\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

bool spherePolygonCollision(CVector3 vPolygon[], 
							CVector3 &vSphereCenter, int vertexCount, float sphereRadius)
{
	// This function is the only function we need to call for testing if a sphere
	// collides with a polygon.  The rest are just helper functions called within here.
	// The theory is actually quite difficult to understand, especially if you are
	// a beginner to collision detection and are following the tutorials in order, but
	// I will try to be as gentle and descriptive as possible.  Let go!
	// Basically, here is the overview:  
	//
	// 1) First you want to check if the sphere collides with the polygon's plane.
	//    Remember, that planes are infinite and you could be 500 units from the
	//    polygon and it's still going to trigger this first test.  We want to 
	//    write a function that classifies the sphere.  Either it's completely 
	//    in front of the plane (the side the normal is on), intersecting the
	//    plane or completely behind the plane.  Got it so far?  We created a 
	//    function called ClassifySphere() that returns BEHIND, FRONT or INTERSECTS.
	//    If ClassifySphere() returns INTERSECTS, then we move on to step 2, otherwise
	//    we did not collide with the polygon.
	// 
	// 2) The second step is to get an intersection point right in front of the sphere.
	//    This one of the tricky parts.  We know that once we have an intersection point
	//    on the plane of the polygon, we just need to use the InsidePolygon() function
	//    to see if that point is inside the dimensions of the polygon, just like we
	//    did with the Ray to Polygon Collision tutorial.  So, how do we get the point
	//    of intersection?  It's not as simple as it might sound.  Since a sphere infinite
	//    points, there would be a million points that it collided at.  You can't just
	//    draw a ray in the direction the sphere was moving because it could have just
	//    nicked the bottom of the polygon and your ray would find an intersection
	//    point that is outside of the polygon.  Well, it turns out that we need to
	//    first try and give it a shot.  We will try the first attempt a different way though.
	//    We know that we can find the normal vector of the polygon, which in essence
	//    tells us the direction that the polygon is facing.  From ClassifyPoly(),
	//	  it also returns the distance the center our sphere is from the plane.  That
	//    means we have a distance our sphere center is from the plane, and the normal
	//    tells us the direction the plane is in.  If we multiply the normal by the
	//    distance from the plane we get an offset.  This offset can then be subtracted
	//    from the center of the sphere.  Believe it or not, but we now have a position
	//    on the plane in the direction of the plane.  Usually, this intersection points
	//    works fine, but if we get around the edges of the polygon, this does not work.
	//    What we just did is also called "projecting the center of the sphere onto the plane".
	//    Another way to do this is to shoot out a ray from the center of the sphere in
	//    the opposite direction of the normal, then we find the intersection of that line
	//    and the plane.  My way just takes 3 multiplies and a subtraction.  You choose.
	//
	// 3) Once we have our psuedo intersection point, we just pass it into InsidePolygon(),
	//    along with the polygon vertices and the vertex count.  This will then return
	//    true if the intersection point was inside of the polygon, otherwise false.
	//    Remember, just because this returns false doesn't mean we stop there!  If
	//    we didn't collide yet, we need to skip to step 4.
	//
	// 4) If we get here, it's assumed that we tried our intersection point and it
	//    wasn't in the polygon's perimeter.  No fear!  There is hope!  If we get to step
	//    4, that's means that our center point is outside of the polygon's perimeter. Since
	//    we are dealing with a sphere, we could still be colliding because of the sphere's radius.
	//	  This last check requires us to find the point on each of the polygon's edges that
	//    is closest to the sphere's center.  We have a tutorial on finding this, so make sure
	//    you have read it or are comfortable with the concept.  If we are dealing with a
	//    triangle, we go through every side and get an edge vector, and calculate the closest
	//    point on those lines to our sphere's center.  After getting each closest point, we
	//    calculate the distance that point is from our sphere center.  If the distance is
	//    less than the radius of the sphere, there was a collision.  This way is pretty fast.  
	//    You don't need to calculate all three sides evey time, since the first closest point's 
	//    distance could be less than the radius and you return "true".
	//
	// That's the overview, *phew!*.  I bet you are reading this just wanting to cry because
	// that seems like so much math and theory to digest, so the code must be atrocious!
	// Well, you are partially right :)  It's not that bad actually, quite straight forward.
	// I will label the steps in the code so you can go back and forth to the overview and code.
	// I might mention that by having our CVector3 class operator overloaded it cuts down the 
	// code tremendously.  If you are confused with this concept of C++, just create functions
	// to add, subtract and multiply vectors or scalars together.
	//

	// 1) STEP ONE - Finding the sphere's classification
	
	// Let's use our Normal() function to return us the normal to this polygon
	CVector3 vNormal = normal(vPolygon);

	// This will store the distance our sphere is from the plane
	float distance = 0.0f;

	// This is where we determine if the sphere is in FRONT, BEHIND, or INTERSECTS the plane
	// of the polygon.  We pass is our sphere center, the polygon's normal, a point on
	// the plane (vertex), the sphere's radius and an empty float to fill the distance with.
	int CLASSIFICATION = classifySphere(	vSphereCenter,
											vNormal,
											vPolygon[0],
											sphereRadius,
											distance
										);

	// If the sphere intersects the polygon's plane, then we need to check further,
	// otherwise the sphere did NOT intersect the polygon.  Pretty fast so far huh?
	if(CLASSIFICATION == INTERSECTS) {
		// 2) STEP TWO - Finding the psuedo intersection point on the plane

		// Now we want to project the sphere's center onto the polygon's plane,
		// in the direction of the normal.  This is done by multiplying the "normal"
		// by the "distance" the sphere center is from the plane.  We got the distance
		// from the ClassifySphere() function call up above.  2 return values were given
		// through the "distance" variable being passed in as a reference.  If projecting
		// is confusing to you, just think of it as this: "I am starting at the center
		// of the sphere and I am going to just run into the plane.  I will move in the 
		// direction that is reverse from the normal.  When do I know when to stop?  Well,
		// I just go in that direction until my distance from the center is the same as
		// the distance the center of the sphere is from the plane."  By doing this
		// we get an offset to subtract from the center of the sphere.
		CVector3 vOffset = vNormal * distance;

		// Once we have the offset to the plane, we just subtract it from the center
		// of the sphere.  "vPosition" now a point that lies on the plane of the polygon.
		// Whether it is inside the polygon's perimeter is another story.  Usually it
		// is though, unless we get near the edges of the polygon.
		CVector3 vPosition = vSphereCenter - vOffset;

		// 3) STEP THREE - Check if the intersection point is inside the polygons perimeter

		// This is the same function used in our previous tutorial on Ray to Polygon Collision.
		// If the intersection point is inside the perimeter of the polygon, it returns true.
		// We pass in the intersection point, the list of vertices and vertex count of the poly.

		if(insidePolygon(vPosition, vPolygon, vertexCount)) {
			return true;	// We collided!
		}
		else {
			// 4) STEP FOUR - Check the sphere to see if it intersects the polygon edges

			// If we get here, we didn't find an intersection point in the perimeter.
			// There is still one more chance to redeem our sphere that it can hit the mark.
			// If any part of the sphere intersects the edges of the polygon, we collided.  
			// This is only checked if the sphere's center point is outside the edges of the
			// polygon. We pass in the center of the sphere, the list of verts, the polygon 
			// vertex count and the sphere's radius.  If this returns true we have a collision.
			if(edgeSphereCollision(vSphereCenter, vPolygon, vertexCount, sphereRadius)) {

				return true;	// We collided! "And you doubted me..." - Sphere
			}
		}
	}

	// If we get here, there is obviously no collision happening up in this crib
	return false;
}


///////////////////////////////// CLASSIFY SPHERE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This tells if a sphere is BEHIND, in FRONT, or INTERSECTS a plane, also it's distance
/////
///////////////////////////////// CLASSIFY SPHERE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

int classifySphere(	CVector3 &vSphereCenter, 
					CVector3 &vPlaneNormal, 
					CVector3 &vPointOnPlane, 
					float sphereRadius, 
					float &plane2SphereCenterDistance
) {

	// First we need to find the distance our polygon plane is from the origin.
	// We need this for the distance formula below.
	float d = (float)planeDistance(vPlaneNormal, vPointOnPlane);

	// Here we use the famous distance formula to find the distance the center point
	// of the sphere is from the polygon's plane.  
	// Remember that the formula is Ax + By + Cz + d = 0 with ABC = Normal, xyz = Point
	plane2SphereCenterDistance =	(vPlaneNormal.x * vSphereCenter.x) + 
									(vPlaneNormal.y * vSphereCenter.y) + 
									(vPlaneNormal.z * vSphereCenter.z) + d;
char logText[255] = {0};
sprintf(logText, "vPointOnPlane = %0.2f %0.2f %0.2f", vPointOnPlane.x, vPointOnPlane.y, vPointOnPlane.z);
//MessageBox(NULL, logText, "logText", MB_OK);

	// Now we query the information just gathered.  Here is how Sphere Plane Collision works:
	// If the distance the center is from the plane is less than the radius of the sphere,
	// we know that it must be intersecting the plane.  We take the absolute value of the
	// distance when we do this check because once the center of the sphere goes behind
	// the polygon, the distance turns into negative numbers (with 0 being that the center
	// is exactly on the plane).  What do I mean when I say "behind" the polygon?  How do
	// we know which side is the front or back side?  Well, the side with the normal pointing
	// out from it is the front side, the other side is the back side.  This is all dependant
	// on the direction the vertices stored.  I recommend drawing them counter-clockwise.
	// if you go clockwise the normal with then point the opposite way and will screw up
	// everything.
	// So, if we want to find if the sphere is in front of the plane, we just make sure
	// the distance is greater than or equal to the radius.  let's say we have a radius
	// of 5, and the distance the center is from the plane is 6; it's obvious that the
	// sphere is 1 unit away from the plane.
	// If the sphere isn't intersecting or in front of the plane, it HAS to be BEHIND it.

	// If the absolute value of the distance we just found is less than the radius, 
	// the sphere intersected the plane.
	if(absolute(plane2SphereCenterDistance) < sphereRadius)
		return INTERSECTS;
	// Else, if the distance is greater than or equal to the radius, the sphere is
	// completely in FRONT of the plane.
	else
	if(plane2SphereCenterDistance > sphereRadius)
		return FRONT;

	return BEHIND;
}


///////////////////////////////// EDGE SPHERE COLLSIION \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This returns true if the sphere is intersecting any of the edges of the polygon
/////
///////////////////////////////// EDGE SPHERE COLLSIION \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

bool edgeSphereCollision(	CVector3 &vSphereCenter, 
							CVector3 vPolygon[], 
							int vertexCount, 
							float sphereRadius
) {
	
	CVector3 vPoint;

	// This function takes in the sphere's center, the polygon's vertices, the vertex count
	// and the radius of the sphere.  We will return true from this function if the sphere
	// is intersecting any of the edges of the polygon.  How it works is, every edge line
	// segment finds the closest point on that line to the center of the sphere.  If the
	// distance from that closest point is less than the radius of the sphere, there was
	// a collision.  Otherwise, we are definately out of reach of the polygon.  This works
	// for triangles, quads, and any other convex polygons.

	// Go through all of the vertices in the polygon
	for(int i = 0; i < vertexCount; i++) {

		// This returns the closest point on the current edge to the center of the sphere.
		// Notice that we mod the second point of the edge by our vertex count.  This is
		// so that when we get to the last edge of the polygon, the second vertex of the
		// edge is the first vertex that we starting with.  
		
		vPoint = closestPointOnLine(vPolygon[i], vPolygon[ (i + 1) % vertexCount ], vSphereCenter);

		// Now, we want to calculate the distance between the closest point and the center
		float distance = getDistance(vPoint, vSphereCenter);

		// If the distance is less than the radius, there must be a collision so return true
		if(distance < sphereRadius)
			return true;

	}
	
	// The was no intersection of the sphere and the edges of the polygon
	return false;
}

// This returns the offset to move the center of the sphere off the collided polygon
CVector3 getCollisionOffset(CVector3 &vNormal, float radius, float distance) {
	CVector3 vOffset = CVector3(0, 0, 0);

	// Once we find if a collision has taken place, we need make sure the sphere
	// doesn't move into the wall.  In our app, the position will actually move into
	// the wall, but we check our collision detection before we render the scene, which
	// eliminates the bounce back effect it would cause.  The question is, how do we
	// know which direction to move the sphere back?  In our collision detection, we
	// account for collisions on both sides of the polygon.  Usually, you just need
	// to worry about the side with the normal vector and positive distance.  If 
	// you don't want to back face cull and have 2 sided planes, I check for both sides.
	//
	// Let me explain the math that is going on here.  First, we have the normal to
	// the plane, the radius of the sphere, as well as the distance the center of the
	// sphere is from the plane.  In the case of the sphere colliding in the front of
	// the polygon, we can just subtract the distance from the radius, then multiply
	// that new distance by the normal of the plane.  This projects that leftover
	// distance along the normal vector.  For instance, say we have these values:
	//
	//	vNormal = (1, 0, 0)		radius = 5		distance = 3
	//
	// If we subtract the distance from the radius we get: (5 - 3 = 2)
	// The number 2 tells us that our sphere is over the plane by a distance of 2.
	// So basically, we need to move the sphere back 2 units.  How do we know which
	// direction though?  This part is easy, we have a normal vector that tells us the
	// direction of the plane.  
	// If we multiply the normal by the left over distance we get:  (2, 0, 0)
	// This new offset vectors tells us which direction and how much to move back.
	// We then subtract this offset from the sphere's position, giving is the new
	// position that is lying right on top of the plane.  Ba da bing!
	// If we are colliding from behind the polygon (not usual), we do the opposite
	// signs as seen below:
	
	// If our distance is greater than zero, we are in front of the polygon
	if(distance > 0)
	{
		// Find the distance that our sphere is overlapping the plane, then
		// find the direction vector to move our sphere.
		float distanceOver = radius - distance;
		vOffset = vNormal * distanceOver;
	}
	else // Else colliding from behind the polygon
	{
		// Find the distance that our sphere is overlapping the plane, then
		// find the direction vector to move our sphere.
		float distanceOver = radius + distance;
		vOffset = vNormal * -distanceOver;
	}

	// There is one problem with check for collisions behind the polygon, and that
	// is if you are moving really fast and your center goes past the front of the
	// polygon, it will then assume you were colliding from behind and not let
	// you back in.  Most likely you will take out the if / else check, but I
	// figured I would show both ways in case someone didn't want to back face cull.

	// Return the offset we need to move back to not be intersecting the polygon.
	return vOffset;
}


/////////////////////////////////////////////////////////////////////////////////
//
// * QUICK NOTES * 
//
// If you haven't wrapped yourself around the porcelain god from all of the math,
// you are still reading this and passed!  Okay, so maybe it's a overstatement of
// all the math, but I thought I would make the weenies feel like I identify with
// them..... uh..... just kidding :)  But seriously, this isn't easy stuff to swallow.
//
// I won't attempt to explain everything again because it's taken too many hours to
// comment this baby, but I will go over the steps again:
//
//    1) STEP ONE - Finding the sphere's classification
//
//    2) STEP TWO - Finding the psuedo intersection point on the plane
//
//    3) STEP THREE - Check if the intersection point is inside the polygons perimeter
//
//    4) STEP FOUR - Check the sphere to see if it intersects the polygon edges
//
// It should be quite apparent while looking through this tutorial that there can be
// a bunch of optimizations.  For instance, we tend to calculate the normal and plane
// distances every frame, and also sometimes many times during that same frame.  We
// could precompute this useful information in the beginning of the program
// and pass in more data from our polygon.  It might even be good to create a polygon
// class that has a constructor that computes all this data and store it in the member variables.
// When dealing with thousands of polygons, every little bit helps.  Also, be sure you
// understand that you would not calculate this for every polygon in your world, only
// the polygons near you.  How you find these polygons is your problem :)  I suggest 
// an octree or BSP tree, depending on your level/world/scene.
//
// Once again, the next tutorial will demonstrate how to hook this up to our camera
// class and collide and slide against walls in a world.  
//
// Let us know at www.GameTutorials.com if this was helpful to you.  
// Any feedback is welcome .
// 
// 
// Ben Humphrey (DigiBen)
// Game Programmer
// DigiBen@GameTutorials.com
// Co-Web Host of www.GameTutorials.com
//
//
