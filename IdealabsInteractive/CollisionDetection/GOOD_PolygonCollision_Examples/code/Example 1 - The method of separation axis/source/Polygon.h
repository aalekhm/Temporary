#pragma once

#include "Vector.h"

// basic polygon structure
struct Poly
{
public:
	Poly();
	Poly(int count, float radius);

	void transform(const Vector& position, float rotation);
	bool collide(const Poly& poly) const;
	void render(bool solid=true) const;

public:
	enum { MAX_VERTICES = 32 };
	Vector	m_vertices[MAX_VERTICES];
	int		m_count;

	// collision functions
private:
	void calculateInterval(const Vector& axis, float& min, float& max) const;
	bool intervalsSeparated(float mina, float maxa, float minb, float maxb) const;
	bool separatedByAxis(const Vector& axis, const Poly& poly) const;
};

