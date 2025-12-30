#pragma once

#include "Polygon.h"

struct Body
{
	Body();
	Body(float worldSize);
	void render() const;

	Poly	m_polygon;
	Vector  m_position;
	float   m_orientation;
	Vector  m_velocity;
};

struct CollisionReport
{
	CollisionReport();
	
	CollisionReport(Body* a, Body* b);

	void render();

	bool			m_collisionReported;
	Body*			m_body[2];
	Poly			m_poly[2];
	Vector			m_ncoll;
	Vector			m_mtd;
	float			m_tcoll;
	CollisionInfo	m_collisionInfo;
	ContactManifold	m_manifold;
};

