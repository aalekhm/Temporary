#pragma once

#include "Polygon.h"

struct Body
{
	Body();
	Body(float worldSize);
	bool collide(const Body& body) const;
	void render(bool collided) const;

	Poly	m_polygon;
	Vector  m_position;
	float   m_orientation;
};
