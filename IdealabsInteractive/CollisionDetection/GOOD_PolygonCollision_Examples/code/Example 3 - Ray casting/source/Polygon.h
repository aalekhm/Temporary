#pragma once

#include "Vector.h"

struct CollisionInfo
{
	CollisionInfo();
	float			m_mtdLengthSquared;
	Vector			m_mtd;
	bool			m_overlapped;
};

struct RayCastingInfo
{
	RayCastingInfo();

	bool m_intersected;
	float m_tenter;
	float m_tleave;
	Vector m_Nenter;
	Vector m_Nleave;
};

// basic polygon structure
struct Poly
{
public:
	Poly();
	Poly(int count, float radius);

	void transform(const Vector& position, float rotation);
	CollisionInfo collide(const Poly& poly) const;
	RayCastingInfo trace(const Vector& start, const Vector& delta) const;

	void render(bool solid=true) const;

public:
	enum { MAX_VERTICES = 32 };
	Vector	m_vertices[MAX_VERTICES];
	int		m_count;

	// collision functions
private:
	void calculateInterval(const Vector& axis, float& min, float& max) const;
	bool separatedByAxis(const Vector& axis, const Poly& poly, CollisionInfo& info) const;

	bool traceFailed(const Vector& axis, float min, float max, float start, float delta, RayCastingInfo& info) const;
	bool traceFailedOnAxis(const Vector& axis, const Vector& start, const Vector& delta, RayCastingInfo& info) const;
};

