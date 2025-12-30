#pragma once

#include "Vector.h"

struct CollisionInfo
{
	CollisionInfo();

	// overlaps
	bool	m_overlapped;
	float	m_mtdLengthSquared;
	Vector	m_mtd;

	// swept
	bool	m_collided; 
	Vector	m_Nenter;
	Vector	m_Nleave;
	float	m_tenter;
	float	m_tleave;
};

struct SupportPoints
{
	SupportPoints();
	
	enum { MAX_SUPPORTS = 4 };
	Vector m_support[MAX_SUPPORTS];
	int m_count;
};

struct ContactPair
{
	ContactPair();
	ContactPair(const Vector& a, const Vector& b);
	
	Vector m_position[2];
	float  m_distanceSquared;
};

struct ContactManifold
{
	ContactManifold();

	ContactManifold(const SupportPoints& supports1, const SupportPoints& supports2);
	void edgeEdge(const Vector* edge1 , const Vector* edge2);
	void edgeVertex(const Vector* edge, const Vector& vertex);
	void vertexEdge(const Vector& vertex, const Vector* edge);
	void vertexVertex(const Vector& vertex1, const Vector& vertex2);

	enum { MAX_CONTACTS = 8 };
	ContactPair m_contact[MAX_CONTACTS];
	int m_count;

};