
#include "Vector.h"
#include "Polygon.h"

CollisionInfo::CollisionInfo()
{
	m_mtdLengthSquared = 0.0f;
	m_mtd = Vector(0, 0);
	m_overlapped = false;
}	

Poly::Poly()
{
	m_count = 0;
}

Poly::Poly(int count, float radius)
{
	m_count = count;
	
	for(int i = 0; i < m_count; i ++)
	{
		float a = 2.0f * pi() * (i / (float) m_count);

		m_vertices[i] = Vector(cos(a), sin(a)) * radius;
	}
}

void Poly::transform(const Vector& position, float rotation)
{
	for(int i = 0; i < m_count; i ++)
		m_vertices[i].transform(position, rotation);
}

void Poly::render(bool solid) const
{
	if(solid)
	{
		glBegin(GL_TRIANGLE_FAN);
	}
	else
	{
		glBegin(GL_LINE_LOOP);
	}
	for(int i = 0; i < m_count; i ++)
	{
		glVertex2f(m_vertices[i].x, m_vertices[i].y);
	}
	glVertex2f(m_vertices[0].x, m_vertices[0].y);
	glEnd();
}

CollisionInfo Poly::collide(const Poly& poly) const
{
	// invalidate the mtd stored.
	CollisionInfo info;
	info.m_mtdLengthSquared = -1.0f; // flag mtd as invalid
		
	// test separation axes of current polygon
	for(int j = m_count-1, i = 0; i < m_count; j = i, i ++)
	{
		Vector v0 = m_vertices[j];
		Vector v1 = m_vertices[i];

		Vector edge = v1 - v0; // edge
		Vector axis = edge.perp(); // sep axis is perpendicular ot the edge

		if(separatedByAxis(axis, poly, info))
			return CollisionInfo();
	}

	// test separation axes of other polygon
	for(int j = poly.m_count-1, i = 0; i < poly.m_count; j = i, i ++)
	{
		Vector v0 = poly.m_vertices[j];
		Vector v1 = poly.m_vertices[i];

		Vector edge = v1 - v0; // edge
		Vector axis = edge.perp(); // sep axis is perpendicular ot the edge

		if(separatedByAxis(axis, poly, info))
			return CollisionInfo();
	}
	info.m_overlapped = true;
	return info;
}

void Poly::calculateInterval(const Vector& axis, float& min, float& max) const
{
	min = max = (m_vertices[0] * axis);

	for(int i = 1; i < m_count; i ++)
	{
		float d = (m_vertices[i] * axis);
		if (d < min) 
			min = d; 
		else if (d > max) 
			max = d;
	}
}

bool Poly::separatedByAxis(const Vector& axis, const Poly& poly, CollisionInfo& info) const
{
	float mina, maxa;
	float minb, maxb;

	// calculate both polygon intervals along the axis we are testing
	calculateInterval(axis, mina, maxa);
	poly.calculateInterval(axis, minb, maxb);

	// calculate the two possible overlap ranges.
	// either we overlap on the left or right of the polygon.
	float d0 = (maxb - mina);
	float d1 = (minb - maxa);

	// no overlap detected
	if(d0 < 0.0f || d1 > 0.0f) return true;

	// find out if we overlap on the 'right' or 'left' of the polygon.
	float overlap = (d0 < -d1)? d0 : d1;

	// the axis length squared
	float axis_length_squared = (axis * axis);
	assert(axis_length_squared > 0.00001f);

	// the mtd vector for that axis
	Vector sep = axis * (overlap / axis_length_squared);

	// the mtd vector length squared.
	float sep_length_squared = (sep * sep);

	// if that vector is smaller than our computed MTD (or the mtd hasn't been computed yet)
	// use that vector as our current mtd.
	if(sep_length_squared < info.m_mtdLengthSquared || info.m_mtdLengthSquared < 0.0f)
	{
		info.m_mtdLengthSquared = sep_length_squared;
		info.m_mtd = sep;
	}
	return false;
}
