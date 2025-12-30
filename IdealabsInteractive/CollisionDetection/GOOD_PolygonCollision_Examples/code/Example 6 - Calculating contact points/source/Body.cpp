
#include "Body.h"

void renderARGB(int colour)
{
	glColor4ub(ARGB_R(colour), ARGB_G(colour), ARGB_B(colour), ARGB_A(colour));
}
void renderDottedSegment(const Vector& start, const Vector& end, int colour)
{
	renderARGB(colour);
	
	glLineStipple(1, 15);
	glBegin(GL_LINES);
	glVertex2f(start.x, start.y);
	glVertex2f(end.x, end.y);
	glEnd();
	glDisable(GL_LINE_STIPPLE);
}

void renderSegment(const Vector& start, const Vector& end, int colour)
{
	renderARGB(colour);
	
	glBegin(GL_LINES);
	glVertex2f(start.x, start.y);
	glVertex2f(end.x, end.y);
	glEnd();
}

void renderArrow(const Vector& start, const Vector& dir, float dist, int colour)
{
	renderARGB(colour);
	
	float angle = dir.angle(Vector(1, 0));
	glPushMatrix();
	glTranslatef(start.x, start.y, 0.0f);
	glRotatef(angle * 180.0f / pi(), 0.0f, 0.0f, -1.0f);
	glScalef(dist, dist, 1.0f);
	glColor4ub(ARGB_R(colour), ARGB_G(colour), ARGB_B(colour), ARGB_A(colour));
	glBegin(GL_LINES);
	glVertex2f(0.0f, 0.0f);
	glVertex2f(1.0f, 0.0f);
	glVertex2f(1.0f, 0.0f);
	glVertex2f(0.9f, 0.1f);
	glVertex2f(1.0f, 0.0f);
	glVertex2f(0.9f,-0.1f);
	glEnd();
	glPopMatrix();
}


void renderArrow(const Vector& start, const Vector& end, int colour)
{
	Vector dir = (end - start);
	float dist = dir.normalise();
	renderArrow(start, dir, dist, colour);
}

Body::Body()
{}

Body::Body(float worldSize)
{
	m_position.randomise(Vector(worldSize * 0.25f, worldSize * 0.25f), Vector(worldSize * 0.75f, worldSize * 0.75f));
	m_orientation = frand(pi());
	m_velocity.randomise(Vector(-worldSize * 0.15f, -worldSize * 0.15f), Vector(worldSize * 0.15f, worldSize * 0.15f));

	int count = rand() % 6 + 3;
	float rad = frand(worldSize * 0.05f) + worldSize * 0.1f;
	m_polygon = Poly(count, rad);
}

void Body::render() const
{
	Poly thisPoly = m_polygon;
	thisPoly.transform(m_position, m_orientation);

	renderARGB(0x808080FF);
	thisPoly.render(true);

	renderARGB(0xFFFFFFFF);
	thisPoly.render(false);

	renderArrow(m_position, m_position + m_velocity, 0xff80ff80);
}
	
CollisionReport::CollisionReport()
{
	m_collisionReported = false;
	m_body[0] = m_body[1] = NULL;
	m_ncoll = Vector(0, 0);
	m_mtd = Vector(0, 0);
	m_tcoll = 0.0f;
}

CollisionReport::CollisionReport(Body* a, Body* b)
{
	m_collisionReported = false;
	m_body[0]			= a;
	m_body[1]			= b;
	m_manifold			= ContactManifold();
	m_ncoll				= Vector(0, 0);
	m_mtd				= Vector(0, 0);
	m_tcoll				= 0.0f;

	// polygons in world space at the time of collision	
	m_poly[0] = a->m_polygon;
	m_poly[1] = b->m_polygon;
	m_poly[0].transform(a->m_position, a->m_orientation);
	m_poly[1].transform(b->m_position, b->m_orientation);
	
	// find collision
	Vector delta = (a->m_velocity - b->m_velocity);
	m_collisionInfo = m_poly[0].collide(m_poly[1], delta);
	m_collisionReported = (m_collisionInfo.m_overlapped || m_collisionInfo.m_collided);
		
	// convert collision info into collison plane info
	if(m_collisionInfo.m_overlapped)
	{
		if(m_collisionInfo.m_mtdLengthSquared <= 0.00001f)
		{
			assert(false);
			m_collisionReported = false;
			return;
		}

		m_ncoll = m_collisionInfo.m_mtd / sqrt(m_collisionInfo.m_mtdLengthSquared);
		m_tcoll = 0.0f;
		m_mtd   = m_collisionInfo.m_mtd;
	
	}
	else if(m_collisionInfo.m_collided)
	{
		m_ncoll = m_collisionInfo.m_Nenter;
		m_tcoll = m_collisionInfo.m_tenter;
		m_mtd   = Vector(0, 0);
	}	

	// find contact points at time of collision
	m_poly[0].translate(a->m_velocity * m_tcoll);
	m_poly[1].translate(b->m_velocity * m_tcoll);
	
	SupportPoints asup = m_poly[0].getSupports(m_ncoll);
	SupportPoints bsup = m_poly[1].getSupports(-m_ncoll);

	m_manifold = ContactManifold(asup, bsup);
};

void CollisionReport::render()
{
	if(!m_collisionReported)
		return;

	renderARGB(0x80FFA080);
	m_poly[0].render(true);
	m_poly[1].render(true);

	renderARGB(0xFFFFFFFF);
	m_poly[0].render(false);
	m_poly[1].render(false);

	for(int i = 0; i < m_manifold.m_count; i ++)
	{
		glLineWidth(4.0f);
		renderDottedSegment(m_manifold.m_contact[i].m_position[0], m_manifold.m_contact[i].m_position[1], 0xffff0000);
		glLineWidth(2.0f);
		renderArrow(m_manifold.m_contact[i].m_position[0], m_ncoll, 10, 0xffffff00);
		glLineWidth(1.0f);
	}
}