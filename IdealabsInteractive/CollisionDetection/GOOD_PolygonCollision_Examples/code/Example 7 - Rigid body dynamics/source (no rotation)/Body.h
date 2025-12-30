#pragma once

#include "Polygon.h"

struct Body
{
	Body();
	Body(const Vector& min, const Vector& max, float mass=0.0f);
	Body(float worldSize, float mass);

	void render() const;
	void update();

	Poly	m_polygon;
	Vector  m_position;
	float   m_orientation;
	Vector  m_velocity;
	float	m_invmass;
};

struct CollisionReport
{
	CollisionReport();
	
	CollisionReport(Body* a, Body* b);

	void render();

	void applyReponse(float CoR);

	bool			m_collisionReported;
	Body*			m_body[2];
	Poly			m_poly[2];
	Vector			m_ncoll;
	Vector			m_mtd;
	float			m_tcoll;
	CollisionInfo	m_collisionInfo;
	ContactManifold	m_manifold;

	Vector			m_vcoll;
};

