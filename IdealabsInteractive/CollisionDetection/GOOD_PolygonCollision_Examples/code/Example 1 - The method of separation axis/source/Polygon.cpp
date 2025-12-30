
#include "Vector.h"
#include "Polygon.h"

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

bool Poly::collide(const Poly& poly) const
{
	// test separation axes of current polygon
	for(int j = m_count-1, i = 0; i < m_count; j = i, i ++)
	{
		Vector v0 = m_vertices[j];
		Vector v1 = m_vertices[i];

		Vector edge = v1 - v0; // edge
		Vector axis = edge.perp(); // sep axis is perpendicular ot the edge

		if(separatedByAxis(axis, poly))
			return false;
	}

	// test separation axes of other polygon
	for(int j = poly.m_count-1, i = 0; i < poly.m_count; j = i, i ++)
	{
		Vector v0 = poly.m_vertices[j];
		Vector v1 = poly.m_vertices[i];

		Vector edge = v1 - v0; // edge
		Vector axis = edge.perp(); // sep axis is perpendicular ot the edge

		if(separatedByAxis(axis, poly))
			return false;
	}
	return true;
}

void Poly::calculateInterval(const Vector& axis, float& min, float& max) const
{
	min = max = (m_vertices[0] * axis);

	for(int i = 1; i < m_count; i ++)
	{
		float d = (m_vertices[i] * axis);//Dot Product
		if (d < min) 
			min = d; 
		else if (d > max) 
			max = d;
	}
}

bool Poly::intervalsSeparated(float mina, float maxa, float minb, float maxb) const
{
	return (mina > maxb) || (minb > maxa);
}

bool Poly::separatedByAxis(const Vector& axis, const Poly& poly) const
{
	float mina, maxa;
	float minb, maxb;

	calculateInterval(axis, mina, maxa);
	
	poly.calculateInterval(axis, minb, maxb);
	
	return intervalsSeparated(mina, maxa, minb, maxb);
}
