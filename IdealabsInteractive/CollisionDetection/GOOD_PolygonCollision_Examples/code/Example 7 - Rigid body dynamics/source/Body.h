#pragma once

#include "Polygon.h"

struct Body
{
	Body();
	Body(const Vector& min, const Vector& max);
	Body(float worldSize);

	void render() const;
	void update();

	Poly	m_polygon;
	Vector  m_position;
	float   m_orientation;
	Vector  m_velocity;
	float	m_invmass;
	float	m_invinertia;
	float	m_angvelocity;
};

struct CollisionReport
{
	CollisionReport();
	
	CollisionReport(Body* a, Body* b);

	void render();

	void applyReponse(float cor, float cof);


	bool			m_collisionReported;
	Body*			m_body[2];
	Poly			m_poly[2];
	Vector			m_ncoll;
	Vector			m_mtd;
	float			m_tcoll;
	CollisionInfo	m_collisionInfo;
	ContactManifold	m_manifold;
	ContactPair		m_contact;
	Vector			m_vcoll;
};

