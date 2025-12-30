#pragma once

#include "Polygon.h"

struct Body
{
	Body();
	Body(const Vector& min, const Vector& max, float mass=0.0f);
	Body(float worldSize, float mass);
	CollisionInfo collide(const Body& body) const;
	void respondToCollision(Body& body, const CollisionInfo& info);
	void render() const;
	void update();

	Poly	m_polygon;
	Vector  m_position;
	Vector	m_velocity;
	float   m_orientation;
	float	m_invmass;
};
