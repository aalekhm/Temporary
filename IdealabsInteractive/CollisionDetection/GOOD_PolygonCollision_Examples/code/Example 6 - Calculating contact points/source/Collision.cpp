
#include "Collision.h"

CollisionInfo::CollisionInfo()
{
	// swept test results
	m_tenter = 0.0f;            // <-- impossible value
	m_tleave = 0.0f;            // <-- impossible value
	m_Nenter = Vector(0, 0);	 // clear value
	m_Nleave = Vector(0, 0);	 // clear value
	m_collided = false;
	
	// overlap test results
	m_overlapped = false;
	m_mtd = Vector(0, 0);
	m_mtdLengthSquared = 0.0f;
}	

SupportPoints::SupportPoints()
{
	m_count = 0;
}

ContactPair::ContactPair()
{
	m_position[0] = m_position[1] = Vector(0, 0);
	m_distanceSquared = 0.0f;
}

ContactPair::ContactPair(const Vector& a, const Vector& b)
{
	Vector d(b - a);
	m_position[0] = a;
	m_position[1] = b;
	m_distanceSquared = (d * d);
}

int CompareContacts(const void* v0, const void* v1)
{ 
	ContactPair* V0 = (ContactPair*) v0;
	ContactPair* V1 = (ContactPair*) v1;
	return (V0->m_distanceSquared > V1->m_distanceSquared)? 1 : -1;
}

Vector closestPointOnEdge(const Vector* edge, const Vector& v)
{
	Vector e(edge[1] - edge[0]);
	Vector d(v - edge[0]);
	float t = (e * d) / (e * e);
	t = (t < 0.0f)? 0.0f : (t > 1.0f)? 1.0f : t;
	return edge[0] + e * t;
}

ContactManifold::ContactManifold()
{
	m_count = 0;
}

ContactManifold::ContactManifold(const SupportPoints& supports1, const SupportPoints& supports2)
{
	if(supports1.m_count == 1)
	{
		if (supports2.m_count == 1)
		{
			vertexVertex(supports1.m_support[0], supports2.m_support[0]);
		}
		else if(supports2.m_count == 2)
		{
			vertexEdge(supports1.m_support[0], supports2.m_support);
		}
		else
		{
			assertf(false, "invalid support point count");
		}
	}
	else if (supports1.m_count == 2)
	{
		if (supports2.m_count == 1)
		{
			edgeVertex(supports1.m_support, supports2.m_support[0]);
		}
		else if(supports2.m_count == 2)
		{
			edgeEdge(supports1.m_support, supports2.m_support);
		}
		else
		{
			assertf(false, "invalid support point count");
		}
	}
	else
	{
		assertf(false, "invalid support point count");
	}
}

void ContactManifold::edgeEdge(const Vector* edge1, const Vector* edge2)
{
	// setup all the potential 4 contact pairs
	m_contact[0] = ContactPair(edge1[0], closestPointOnEdge(edge2, edge1[0]));
	m_contact[1] = ContactPair(edge1[1], closestPointOnEdge(edge2, edge1[1]));
	m_contact[2] = ContactPair(closestPointOnEdge(edge1, edge2[0]), edge2[0]);
	m_contact[3] = ContactPair(closestPointOnEdge(edge1, edge2[1]), edge2[1]);

	// sort the contact pairs by distance value
	qsort(m_contact, 4, sizeof(m_contact[0]), CompareContacts);
	
	// take the closest two
	m_count = 2;
}

void ContactManifold::vertexVertex(const Vector& vertex1, const Vector& vertex2)
{
	m_contact[0] = ContactPair(vertex1, vertex2);
	m_count = 1;
}
	
void ContactManifold::edgeVertex(const Vector* edge, const Vector& vertex)
{
	m_contact[0] = ContactPair(closestPointOnEdge(edge, vertex), vertex);
	m_count = 1;
}

void ContactManifold::vertexEdge(const Vector& vertex, const Vector* edge)
{
	m_contact[0] = ContactPair(vertex, closestPointOnEdge(edge, vertex));
	m_count = 1;
}

