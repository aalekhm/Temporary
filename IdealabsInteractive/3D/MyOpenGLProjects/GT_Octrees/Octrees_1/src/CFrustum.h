
#include "main.h"

#ifndef _CFRUSTUM_H_
#define _CFRUSTUM_H_

class CFrustum {
	
	public :
		float FOV;
		float nearPlaneDistance;
		float farPlaneDistance;
		float whRatio;

		float nearPlaneHeight, farPlaneHeight;
		float nearPlaneWidth, farPlaneWidth;

		CFrustum() {};

		void setFrustum(float fieldOfView, float WH_RATIO, float npd, float fpd) {
			FOV = fieldOfView;
			nearPlaneDistance = npd;
			farPlaneDistance = fpd;
			whRatio = WH_RATIO;
		};

		// Call this every time the camera moves to update the frustum
		void calculateFrustum();

		// This takes a 3D point and returns TRUE if it's inside of the frustum
		bool isPointInFrustum(float x, float y, float z);

		// This takes a 3D point and a radius and returns TRUE if the sphere is inside of the frustum
		bool isSphereInFrustum(float x, float y, float z, float radius);

		// This takes the center and half the length of the cube.
		bool isCubeInFrustum(float x, float y, float z, float size);

	private:
		// This holds the A B C and D values for each side of our frustum.
		float mFrustum[6][4];
};

#endif