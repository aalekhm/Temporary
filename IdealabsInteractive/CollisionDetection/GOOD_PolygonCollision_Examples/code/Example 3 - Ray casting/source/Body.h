#pragma once

#include "Polygon.h"

struct Body
{
	Body();
	Body(float worldSize);
	CollisionInfo collide(const Body& body) const;
	RayCastingInfo trace(const Vector& start, const Vector& delta) const;
	void render(bool collided) const;

	Poly	m_polygon;
	Vector  m_position;
	float   m_orientation;
};


extern void renderARGB(int colour);
extern void renderDottedSegment(const Vector& start, const Vector& end, int colour);
extern void renderSegment(const Vector& start, const Vector& end, int colour);
extern void renderArrow(const Vector& start, const Vector& dir, float dist, int colour);
extern void renderArrow(const Vector& start, const Vector& end, int colour);
