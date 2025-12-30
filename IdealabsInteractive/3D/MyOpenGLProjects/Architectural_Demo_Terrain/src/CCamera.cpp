
#include "CCamera.h"

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
		if(cameraRotation.x < 80) {
			yesRotateX = true;
		}
	}
	else 
	if(deltaY > 0) {
		if(cameraRotation.x > -80) {
			yesRotateX = true;
		}
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
		if(cameraRotation.x < 80) {
			cameraRotation.x = ((int)cameraRotation.x + 1) % 360;
			rotateX(1.0);
		}
	}
	if(mKeys[VK_DOWN]) {
		if(cameraRotation.x > -80) {
			cameraRotation.x = ((int)cameraRotation.x - 1) % 360;
			rotateX(-1.0);
		}
	}
	if(mKeys['Q'] || mKeys[VK_LEFT]) {
		cameraRotation.y = ((int)cameraRotation.y + 1) % 360;
		rotateY(1.0);
	}
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
	if(mKeys['S'] || R_MOUSE_DOWN) {
		moveForward(-(WALK_SPEED + R_MOUSE_DOWN*WALK_SPEED));
		isWalking = true;
	}
	if(mKeys['A']) {
		strafeRight(-WALK_SPEED);
		isWalking = true;
	}
	if(mKeys['D']) {
		strafeRight(WALK_SPEED);
		isWalking = true;
	}
	if(mKeys[VK_PRIOR]) {
		moveUpward(WALK_SPEED);
	}
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
	if(mKeys['E'] || mKeys[VK_RIGHT]) {
		cameraRotation.y = ((int)cameraRotation.y - 1) % 360;
	}
	if(mKeys['W'] || L_MOUSE_DOWN) {
		cameraPosition.x -= sinTheta * SPEED;
		cameraPosition.z -= cosTheta * SPEED;
		isWalking = true;
	}
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
	if(mKeys['D']) {
		cameraPosition.x -= -cosTheta * SPEED;
		cameraPosition.z -=  sinTheta * SPEED;
		isWalking = true;
	}
	if(mKeys[VK_UP]) {
		cameraRotation.x = ((int)cameraRotation.x + 1) % 360;
	}
	if(mKeys[VK_DOWN]) {
		cameraRotation.x = ((int)cameraRotation.x - 1) % 360;
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

			CAMERA_HAS_COLLISIONS = !CAMERA_HAS_COLLISIONS;
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
		//BUGGY
		//Player Camera - Rotation
		cameraPitch(-cameraRotation.x);
		cameraYaw(-cameraRotation.y);
		cameraRoll(-cameraRotation.z);
		
		//Player Camera - Translation
		cameraMoveTo(cameraPosition);
	}
}

void CCamera::cameraDebug() {
	glPrint(SCREEN_WIDTH - 300, SCREEN_HEIGHT - 10, "'G' GRAVITY - 'G' == %d", CAMERA_HAS_GRAVITY);
	glPrint(SCREEN_WIDTH - 300, SCREEN_HEIGHT - 20, "'C' CAMERA COLLISIONS == %d", CAMERA_HAS_COLLISIONS);
	glPrint(SCREEN_WIDTH - 300, SCREEN_HEIGHT - 30, "'I' CAMERA TYPE = %s",(CAMERA_IMPLEMENT_TYPE == CAMERA_GLULOOKAT)?"CAMERA_GLULOOKAT":"CAMERA_TRANSLATE");

	glPrint(SCREEN_WIDTH - 300, 30, "CAM POS : %0.2f %0.2f %0.2f", cameraPosition.x, cameraPosition.y, cameraPosition.z);
	glPrint(SCREEN_WIDTH - 300, 20, "CAM ROT : %0.2f %0.2f %0.2f", cameraRotation.x, cameraRotation.y, cameraRotation.z);
	glPrint(SCREEN_WIDTH - 300, 10, "VIEW VECT : %0.2f %0.2f %0.2f", viewVector.x, viewVector.y, viewVector.z);
}

void CCamera::setGravity(bool bHasGravity) {
	CAMERA_HAS_GRAVITY = bHasGravity;
}

void CCamera::setCollision(bool bHasCollision) {
	CAMERA_HAS_COLLISIONS = bHasCollision;
}
