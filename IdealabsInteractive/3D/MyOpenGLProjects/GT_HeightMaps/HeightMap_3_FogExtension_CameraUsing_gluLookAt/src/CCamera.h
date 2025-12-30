
#include "main.h"

#ifndef _CCAMERA_H_
#define _CCAMERA_H_

#define	CAMERA_HUMAN		0
#define	CAMERA_AIRCRAFT		1
#define	CAMERA_TYPE			CAMERA_HUMAN

#define CAMERA_GLULOOKAT		0
#define CAMERA_TRANSLATE		1

class CCamera {

	public:
		CVector3 cameraPosition;
		CVector3 cameraRotation;
		CVector3 viewVector;
		CVector3 rightVector;
		CVector3 upVector;
		bool CAMERA_HAS_COLLISIONS;
		bool CAMERA_HAS_GRAVITY;
		byte CAMERA_IMPLEMENT_TYPE;
		float camRadius;

		CCamera() {};
		CCamera(CVector3 VIEW_VECT, CVector3 RIGHT_VECT, CVector3 UP_VECT) {
			viewVector = VIEW_VECT;
			rightVector = RIGHT_VECT;
			upVector = UP_VECT;

			CAMERA_HAS_COLLISIONS	= false;
			CAMERA_HAS_GRAVITY		= false;
			CAMERA_IMPLEMENT_TYPE  = CAMERA_GLULOOKAT;

			cameraRotation = CVector3(0.0, 0.0, 0.0);
			camRadius = 0.20f;
		};

		void setPosition(CVector3 position);
		void setGravity(bool bHasGravity);
		void setCollision(bool bHasCollision);

		void rotateX(float angle);
		void rotateY(float angle);
		void rotateZ(float angle);
		void moveForward(float distance);
		void strafeRight(float distance);
		void moveUpward(float distance);

		void cameraPitch(float rotation);
		void cameraYaw(float rotation);
		void cameraRoll(float rotation);
		void cameraMoveTo(CVector3 camPos);

		void checkMouseMovementForGLULookAt();
		void checkKeyboardMovementForGLULookAt();

		void checkMouseMovementForTranslatef();
		void checkKeyboardMovementForTranslatef();

		void updateCamera(void);
		void moveCamera(void);
		
		void cameraDebug();
};

#endif