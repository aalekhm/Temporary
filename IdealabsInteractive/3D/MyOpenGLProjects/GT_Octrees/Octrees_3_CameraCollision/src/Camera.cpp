
#include "Camera.h"

void CCamera::setPosition(CVector3 position) {
	cameraPosition = position;
}

void CCamera::rotateX(float angle) {
	//Rotate viewdir around the right vector:
	viewVector = normalize(	viewVector*cos(angle * piOver180)
							+ 
							upVector*sin(angle * piOver180)
						);

	#if CAMERA_TYPE == CAMERA_AIRCRAFT
		//now compute the new UpVector (by cross product)
		upVector = cross(viewVector, rightVector)*-1;
	#endif
}

void CCamera::rotateY(float angle) {
	//Rotate viewdir around the right vector:
	viewVector = normalize(	viewVector*cos(angle * piOver180)
							-
							rightVector*sin(angle * piOver180)
						);

	//now compute the new UpVector (by cross product)
	rightVector = cross(viewVector, upVector);
}

void CCamera::rotateZ(float angle) {
	//Rotate viewdir around the right vector:
	rightVector = normalize(	rightVector*cos(angle * piOver180)
								+
								upVector*sin(angle * piOver180)
							);

	#if CAMERA_TYPE == CAMERA_AIRCRAFT
		//now compute the new UpVector (by cross product)
		upVector = cross(viewVector, rightVector)*-1;
	#endif
}

void CCamera::moveForward(float distance) {
	cameraPosition = cameraPosition + (viewVector*distance);
}

void CCamera::strafeRight(float distance) {
	cameraPosition = cameraPosition + (rightVector*distance);
}

void CCamera::moveUpward(float distance) {
	cameraPosition = cameraPosition + (upVector*distance);
}

/*PITCH - Rotation around X - axis */
void CCamera::cameraPitch(float rotation) {
	glRotatef(rotation, 1.0f, 0.0f, 0.0f);
}

/*YAW - Rotation around Y - axis */
void CCamera::cameraYaw(float rotation) {
	glRotatef(rotation, 0.0f, 1.0f, 0.0f);
}

/*ROLL - Rotation around Z - axis */
void CCamera::cameraRoll(float rotation) {
	glRotatef(rotation, 0.0f, 0.0f, 1.0f);
}

void CCamera::cameraMoveTo(CVector3 camPos) {
	glTranslatef(-camPos.x, -camPos.y /*+ CROUCH*0.25f*/, -camPos.z);
}

void CCamera::checkMouseMovementForGLULookAt() {
	int screenCenterX = SCREEN_WIDTH>>1;
	int screenCenterY = SCREEN_HEIGHT>>1;

	POINT mousePos;
	GetCursorPos(&mousePos);
	
	//Mouse not moved, hence return;
	if(mousePos.x == screenCenterX && mousePos.y == screenCenterY)
		return;

	SetCursorPos(screenCenterX, screenCenterY);
	
	//Move, Rotate as proportinal to the displacement
	float deltaX = (mousePos.x - screenCenterX)/5.0f;
	float deltaY = (mousePos.y - screenCenterY)/5.0f;
	///*
	bool yesRotateX = false;
	if(deltaY <= 0) {
		//if(cameraRotation.x < 80) {
			yesRotateX = true;
		//}
	}
	else 
	if(deltaY > 0) {
		//if(cameraRotation.x > -80) {
			yesRotateX = true;
		//}
	}
	//*/
	if(yesRotateX) {
		cameraRotation.x -= deltaY;
		rotateX(-deltaY);
	}

	cameraRotation.y -= deltaX;
	rotateY(-deltaX);
}

void CCamera::checkMouseMovementForTranslatef() {
	int screenCenterX = SCREEN_WIDTH>>1;
	int screenCenterY = SCREEN_HEIGHT>>1;

	POINT mousePos;
	GetCursorPos(&mousePos);
	
	//Mouse not moved, hence return;
	if(mousePos.x == screenCenterX && mousePos.y == screenCenterY)
		return;

	SetCursorPos(screenCenterX, screenCenterY);
	
	//Move, Rotate as proportinal to the displacement
	float deltaX = (mousePos.x - screenCenterX)/5.0f;
	float deltaY = (mousePos.y - screenCenterY)/5.0f;

	cameraRotation.x -= deltaY;
	cameraRotation.y -= deltaX;
}

void CCamera::checkKeyboardMovementForGLULookAt() {
	bool isWalking = false;
	if(mKeys[VK_UP]) {
		//if(cameraRotation.x < 80) {
			cameraRotation.x = ((int)cameraRotation.x + 1) % 360;
			rotateX(1.0);
		//}
	}
	else
	if(mKeys[VK_DOWN]) {
		//if(cameraRotation.x > -80) {
			cameraRotation.x = ((int)cameraRotation.x - 1) % 360;
			rotateX(-1.0);
		//}
	}
	
	if(mKeys['Q'] || mKeys[VK_LEFT]) {
		cameraRotation.y = ((int)cameraRotation.y + 1) % 360;
		rotateY(1.0);
	}
	else
	if(mKeys['E'] || mKeys[VK_RIGHT]) {
		cameraRotation.y = ((int)cameraRotation.y - 1) % 360;
		rotateY(-1.0);
	}

	#if CAMERA_TYPE == CAMERA_AIRCRAFT
		if(mKeys['Z']) {
			cameraRotation.z = ((int)cameraRotation.z + 1) % 360;
			rotateZ(-1.0);
		}
		if(mKeys['X']) {
			cameraRotation.z = ((int)cameraRotation.z - 1) % 360;
			rotateZ(1.0);
		}
	#endif

	if(mKeys['W'] || L_MOUSE_DOWN) {
		moveForward(WALK_SPEED + L_MOUSE_DOWN*WALK_SPEED);
		isWalking = true;
	}
	else
	if(mKeys['S'] || R_MOUSE_DOWN) {
		moveForward(-(WALK_SPEED + R_MOUSE_DOWN*WALK_SPEED));
		isWalking = true;
	}
	
	if(mKeys['A']) {
		strafeRight(-WALK_SPEED);
		isWalking = true;
	}
	else
	if(mKeys['D']) {
		strafeRight(WALK_SPEED);
		isWalking = true;
	}
	
	if(mKeys[VK_PRIOR]) {
		moveUpward(WALK_SPEED);
	}
	else
	if(mKeys[VK_NEXT]) {
		moveUpward(-WALK_SPEED);
	}

	if(isWalking)
		PLAYER_STATE = PLAYER_WALK;
}

void CCamera::checkKeyboardMovementForTranslatef() {
	bool isWalking = false;
	
	float sinTheta = sin( cameraRotation.y * piOver180 );
	float cosTheta = cos( cameraRotation.y * piOver180 );
	float SPEED = (WALK_SPEED + L_MOUSE_DOWN*WALK_SPEED);

	if(mKeys['Q'] || mKeys[VK_LEFT]) {
		cameraRotation.y = ((int)cameraRotation.y + 1) % 360;
	}
	else
	if(mKeys['E'] || mKeys[VK_RIGHT]) {
		cameraRotation.y = ((int)cameraRotation.y - 1) % 360;
	}
	
	if(mKeys['W'] || L_MOUSE_DOWN) {
		cameraPosition.x -= sinTheta * SPEED;
		cameraPosition.z -= cosTheta * SPEED;
		isWalking = true;
	}
	else
	if(mKeys['S'] || R_MOUSE_DOWN) {
		cameraPosition.x += sinTheta * SPEED;
		cameraPosition.z += cosTheta * SPEED;
		isWalking = true;
	}
	
	if(mKeys['A']) {
		cameraPosition.x += -cosTheta * SPEED;
		cameraPosition.z +=  sinTheta * SPEED;
		isWalking = true;
	}
	else
	if(mKeys['D']) {
		cameraPosition.x -= -cosTheta * SPEED;
		cameraPosition.z -=  sinTheta * SPEED;
		isWalking = true;
	}
	
	if(mKeys[VK_UP]) {
		cameraRotation.x = ((int)cameraRotation.x + 1) % 360;
	}
	else
	if(mKeys[VK_DOWN]) {
		cameraRotation.x = ((int)cameraRotation.x - 1) % 360;
	}
	
	if(mKeys[VK_PRIOR]) {
		moveUpward(WALK_SPEED);
	}
	else
	if(mKeys[VK_NEXT]) {
		moveUpward(-WALK_SPEED);
	}


	if(isWalking)
		PLAYER_STATE = PLAYER_WALK;
}

void CCamera::updateCamera(void) {
	if(CAMERA_HAS_GRAVITY) {
			//Constant free fall
			float G = -GRAVITY;
			float Y = G;
			if(TRAMP > 0) {
				TRAMP += -GRAVITY;
				Y = TRAMP;
			}
			else
				TRAMP = 0;
			cameraPosition.y += Y;
	}
	
	if(mKeys['G']) {
		if(!mKeysPressed['G']) {
			mKeysPressed['G'] = true;
			CAMERA_HAS_GRAVITY = !CAMERA_HAS_GRAVITY;
		}
	}
	else
		mKeysPressed['G'] = false;


	if(mKeys['C']) {
		if(!mKeysPressed['C']) {
			mKeysPressed['C'] = true;

			cameraHasCollisions = !cameraHasCollisions;
		}
	}
	else
		mKeysPressed['C'] = false;

	if(mKeys['I']) {
		if(!mKeysPressed['I']) {
			mKeysPressed['I'] = true;
			CAMERA_IMPLEMENT_TYPE = (CAMERA_IMPLEMENT_TYPE == CAMERA_GLULOOKAT)?CAMERA_TRANSLATE:CAMERA_GLULOOKAT;
		}
	}
	else 
		mKeysPressed['I'] = false;

	if(CAMERA_IMPLEMENT_TYPE == CAMERA_GLULOOKAT) {
		checkKeyboardMovementForGLULookAt();
		checkMouseMovementForGLULookAt();
	}
	else {
		checkKeyboardMovementForTranslatef();
		checkMouseMovementForTranslatef();
	}
	
	//Aa Temp - Stay at a fixed level
	float FIXED_Y = 5.0f;
	//cameraPosition.y = FIXED_Y;
}

void CCamera::moveCamera(void) {
	
	if(CAMERA_IMPLEMENT_TYPE == CAMERA_GLULOOKAT) {
			CVector3 viewPoint = cameraPosition + viewVector;
			gluLookAt(	cameraPosition.x,	cameraPosition.y,	cameraPosition.z,
						viewPoint.x,		viewPoint.y,		viewPoint.z,
						upVector.x,			upVector.y,			upVector.z
					);
	}
	else 
	if(CAMERA_IMPLEMENT_TYPE == CAMERA_TRANSLATE) {
		//Player Camera - Rotation
		cameraPitch(-cameraRotation.x);
		cameraYaw(-cameraRotation.y);
		cameraRoll(-cameraRotation.z);
		
		//Player Camera - Translation
		cameraMoveTo(cameraPosition);
	}
}

void CCamera::cameraDebug() {
	glPrint(SCREEN_WIDTH - 300, SCREEN_HEIGHT - 10, 0, 255, 0, "'G' GRAVITY - 'G' == %d", CAMERA_HAS_GRAVITY);
	glPrint(SCREEN_WIDTH - 300, SCREEN_HEIGHT - 20, 0, 255, 0, "'C' CAMERA COLLISIONS == %d", cameraHasCollisions);
	glPrint(SCREEN_WIDTH - 300, SCREEN_HEIGHT - 30, 0, 255, 0, "'I' CAMERA TYPE = %s",(CAMERA_IMPLEMENT_TYPE == CAMERA_GLULOOKAT)?"CAMERA_GLULOOKAT":"CAMERA_TRANSLATE");

	//glPrint(SCREEN_WIDTH - 300, 40, "DEBUG : %s", debugString);
	glPrint(SCREEN_WIDTH - 300, 30, 0, 255, 0, "CAM POS : %0.2f %0.2f %0.2f", cameraPosition.x, cameraPosition.y, cameraPosition.z);
	glPrint(SCREEN_WIDTH - 300, 20, 0, 255, 0, "CAM ROT : %0.2f %0.2f %0.2f", cameraRotation.x, cameraRotation.y, cameraRotation.z);
	glPrint(SCREEN_WIDTH - 300, 10, 0, 255, 0, "VIEW VECT : %0.2f %0.2f %0.2f", viewVector.x, viewVector.y, viewVector.z);
}

void CCamera::setGravity(bool bHasGravity) {
	CAMERA_HAS_GRAVITY = bHasGravity;
}

//void CCamera::setCollision(bool bHasCollision) {
//	CAMERA_HAS_COLLISIONS = bHasCollision;
//}

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
//This checks all the polygons in our list and offsets the camera if collided
void CCamera::checkCameraCollision(SCENE* scene) {

	// This function is pretty much a direct rip off of SpherePolygonCollision()
	// We needed to tweak it a bit though, to handle the collision detection once 
	// it was found, along with checking every triangle in the list if we collided.  
	// pVertices is the world data. If we have space partitioning, we would pass in 
	// the vertices that were closest to the camera. What happens in this function 
	// is that we go through every triangle in the list and check if the camera's 
	// sphere collided with it.  If it did, we don't stop there.  We can have 
	// multiple collisions so it's important to check them all.  One a collision 
	// is found, we calculate the offset to move the sphere off of the collided plane.
	
	CVector3 LEG_POS = cameraPosition;	//Camera is our eyes
	LEG_POS.y -= 0.15f;					//so go to the Legs, the invisible collision sphere is at the legs

	// Go through all the triangles
	for(int i = 0; i < scene->maxFaces; i++) {
		int vertex0 = scene->faces[i].vertexID[0] - 1;
		int vertex1 = scene->faces[i].vertexID[1] - 1;
		int vertex2 = scene->faces[i].vertexID[2] - 1;
		int vertex3 = scene->faces[i].vertexID[3] - 1;
		
		CVector3 vVertex0 = CVector3(scene->vertices[vertex0].x, scene->vertices[vertex0].y, scene->vertices[vertex0].z);
		CVector3 vVertex1 = CVector3(scene->vertices[vertex1].x, scene->vertices[vertex1].y, scene->vertices[vertex1].z);
		CVector3 vVertex2 = CVector3(scene->vertices[vertex2].x, scene->vertices[vertex2].y, scene->vertices[vertex2].z);
		CVector3 vVertex3 = CVector3(scene->vertices[vertex3].x, scene->vertices[vertex3].y, scene->vertices[vertex3].z);

		// Store of the current triangle we testing
		CVector3 vPolygon[4] = {vVertex3, vVertex2, vVertex1, vVertex0};//Give in clockwise direction

		// 1) STEP ONE - Finding the sphere's classification
	
		// We want the normal to the current polygon being checked
		CVector3 vNormal = normal(vPolygon);

		// This will store the distance our sphere is from the plane
		float distance = 0.0f;

		// This is where we determine if the sphere is in FRONT, BEHIND, or INTERSECTS the plane
		int classification = classifySphere(LEG_POS, vNormal, vPolygon[0], camRadius, distance);
char logText[255] = {0};
sprintf(debugString, "class = %d %0.2f %0.2f %0.2f ", classification, vNormal.x, vNormal.y, vNormal.z);
//MessageBox(NULL, logText, "logText", MB_OK);

		// If the sphere intersects the polygon's plane, then we need to check further
		if(classification == OBJECT_INTERSECTS_PLANE) {
			// 2) STEP TWO - Finding the psuedo intersection point on the plane

			// Now we want to project the sphere's center onto the triangle's plane
			CVector3 vOffset = vNormal * distance;

			// Once we have the offset to the plane, we just subtract it from the center
			// of the sphere.  "vIntersection" is now a point that lies on the plane of the triangle.
			CVector3 vIntersection = LEG_POS - vOffset;

			// 3) STEP THREE - Check if the intersection point is inside the triangles perimeter

			// We first check if our intersection point is inside the triangle, if not,
			// the algorithm goes to step 4 where we check the sphere again the polygon's edges.

			// We do one thing different in the parameters for EdgeSphereCollision though.
			// Since we have a bulky sphere for our camera, it makes it so that we have to 
			// go an extra distance to pass around a corner. This is because the edges of 
			// the polygons are colliding with our peripheral view (the sides of the sphere).  
			// So it looks likes we should be able to go forward, but we are stuck and considered 
			// to be colliding.  To fix this, we just pass in the radius / 2.  Remember, this
			// is only for the check of the polygon's edges.  It just makes it look a bit more
			// realistic when colliding around corners.  Ideally, if we were using bounding box 
			// collision, cylinder or ellipses, this wouldn't really be a problem.

			if(	insidePolygon(vIntersection, vPolygon, 4) 
				||
				edgeSphereCollision(LEG_POS, vPolygon, 4, camRadius / 2)
			) {

				// If we get here, we have collided!  To handle the collision detection
				// all it takes is to find how far we need to push the sphere back.
				// GetCollisionOffset() returns us that offset according to the normal,
				// radius, and current distance the center of the sphere is from the plane.
				vOffset = getCollisionOffset(vNormal, camRadius, distance);
//sprintf(debugString, "vNormal = %0.2f %0.2f %0.2f ", vNormal.x, vNormal.y, vNormal.z);
				// Now that we have the offset, we want to ADD it to the position and
				// view vector in our camera.  This pushes us back off of the plane.  We
				// don't see this happening because we check collision before we render
				// the scene.
				cameraPosition = cameraPosition + vOffset;

			}
		}
	}
}

bool CCamera::checkCameraCollision(CVector3* vPolygon, long verticeCount) {

	CVector3 LEG_POS = cameraPosition;	//Camera is our eyes
	LEG_POS.y -= 50.15f;					//so go to the Legs, the invisible collision sphere is at the legs
	
	// 1) STEP ONE - Finding the sphere's classification

	// We want the normal to the current polygon being checked
	CVector3 vNormal = normal(vPolygon);

	// This will store the distance our sphere is from the plane
	float distance = 0.0f;

	// This is where we determine if the sphere is in FRONT, BEHIND, or INTERSECTS the plane
	int classification = classifySphere(LEG_POS, vNormal, vPolygon[0], camRadius, distance);

char logText[255] = {0};
sprintf(debugString, "class = %d %0.2f %0.2f %0.2f ", classification, vNormal.x, vNormal.y, vNormal.z);
//MessageBox(NULL, logText, "logText", MB_OK);

	// If the sphere intersects the polygon's plane, then we need to check further
	if(classification == OBJECT_INTERSECTS_PLANE) {
		// 2) STEP TWO - Finding the psuedo intersection point on the plane

		// Now we want to project the sphere's center onto the triangle's plane
		CVector3 vOffset = vNormal * distance;

		// Once we have the offset to the plane, we just subtract it from the center
		// of the sphere.  "vIntersection" is now a point that lies on the plane of the triangle.
		CVector3 vIntersection = LEG_POS - vOffset;

		// 3) STEP THREE - Check if the intersection point is inside the triangles perimeter

		// We first check if our intersection point is inside the triangle, if not,
		// the algorithm goes to step 4 where we check the sphere again the polygon's edges.

		// We do one thing different in the parameters for EdgeSphereCollision though.
		// Since we have a bulky sphere for our camera, it makes it so that we have to 
		// go an extra distance to pass around a corner. This is because the edges of 
		// the polygons are colliding with our peripheral view (the sides of the sphere).  
		// So it looks likes we should be able to go forward, but we are stuck and considered 
		// to be colliding.  To fix this, we just pass in the radius / 2.  Remember, this
		// is only for the check of the polygon's edges.  It just makes it look a bit more
		// realistic when colliding around corners.  Ideally, if we were using bounding box 
		// collision, cylinder or ellipses, this wouldn't really be a problem.

		if(	insidePolygon(vIntersection, vPolygon, verticeCount) 
			||
			edgeSphereCollision(LEG_POS, vPolygon, verticeCount, camRadius / 2)
		) {

			// If we get here, we have collided!  To handle the collision detection
			// all it takes is to find how far we need to push the sphere back.
			// GetCollisionOffset() returns us that offset according to the normal,
			// radius, and current distance the center of the sphere is from the plane.
			vOffset = getCollisionOffset(vNormal, camRadius, distance);
//sprintf(debugString, "vNormal = %0.2f %0.2f %0.2f ", vNormal.x, vNormal.y, vNormal.z);
			// Now that we have the offset, we want to ADD it to the position and
			// view vector in our camera.  This pushes us back off of the plane.  We
			// don't see this happening because we check collision before we render
			// the scene.
			cameraPosition = cameraPosition + vOffset;
			return true;
		}
	}

	return false;
}
