
#include "Vector.h"
#include "Polygon.h"

CollisionInfo::CollisionInfo()
{
	m_mtdLengthSquared = 0.0f;
	m_mtd = Vector(0, 0);
	m_overlapped = false;
}	

RayCastingInfo::RayCastingInfo()
{
	m_intersected = false;
	m_tenter = 0.0f;
	m_tleave = 0.0f;
	m_Nenter = Vector(0, 0);
	m_Nleave = Vector(0, 0);
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

RayCastingInfo Poly::trace(const Vector& start, const Vector& delta) const
{
	RayCastingInfo info;

	// reset info to some weird values
	info.m_intersected = true;		 // we'll be converging tests from there
	info.m_tenter = 1.0f;			 // flags swept test as not being calculated yet
	info.m_tleave = 0.0f;			 // <--- ....
		
	// test separation axes of current polygon
	for(int j = m_count-1, i = 0; i < m_count; j = i, i ++)
	{
		Vector v0 = m_vertices[j];
		Vector v1 = m_vertices[i];

		Vector edge = v1 - v0; // edge
		Vector axis = edge.perp(); // sep axis is perpendicular ot the edge
		
		if(traceFailedOnAxis(axis, start, delta, info))
			return RayCastingInfo();
	}

	// sanity checks
	assert(!(info.m_intersected) || (info.m_tenter < info.m_tleave));
	info.m_intersected &= (info.m_tenter <= info.m_tleave);	

	// normalise normals
	info.m_Nenter.normalise();
	info.m_Nleave.normalise();
	return info;
}

bool Poly::traceFailedOnAxis(const Vector& axis, const Vector& start, const Vector& delta, RayCastingInfo& info) const
{
	float min, max;
	calculateInterval(axis, min, max);
	float s = start * axis;
	float d = delta * axis;
	return traceFailed(axis, min, max, s, d, info);
}

bool Poly::traceFailed(const Vector& axis, float min, float max, float start, float delta, RayCastingInfo& info) const
{
	if(!info.m_intersected)
		return true;

	if (fabs(delta) < 0.000001f) 
		return false;

	float t0 = (min - start) / delta;
	float t1 = (max - start) / delta;
	Vector n0 = -axis;
	Vector n1 = axis;
	
	if(t0 > t1)
	{
		swapf(t0, t1);
		n0.swap(n1);
	}

	if(t1 < 0.0f || t0 > 1.0f) 
		return true;

	if (info.m_tenter > info.m_tleave)
	{
		info.m_intersected = true;
		info.m_tenter = t0;
		info.m_tleave = t1;
		info.m_Nenter = n0;
		info.m_Nleave = n1;
		return false;
	}
	// else, make sure our current interval is in 
	// range [info.m_tenter, info.m_tleave];
	else
	{
		// separated.
		if(t0 > info.m_tleave || t1 < info.m_tenter)
		{
			info.m_intersected = false;
			return true;
		}

		// reduce the collison interval
		// to minima
		if (t0 > info.m_tenter)
		{
			info.m_tenter = t0;
			info.m_Nenter = n0;
		}
		if (t1 < info.m_tleave)
		{
			info.m_tleave = t1;
			info.m_Nleave = n1;
		}			
		return false;
	}
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
