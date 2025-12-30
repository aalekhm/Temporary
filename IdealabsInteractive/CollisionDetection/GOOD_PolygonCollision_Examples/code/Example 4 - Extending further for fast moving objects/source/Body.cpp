
#include "Body.h"

void renderArrow(const Vector& start, const Vector& dir, float dist, int colour)
{
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

CollisionInfo Body::collide(const Body& body) const
{
	// compute transformed polygons
	Poly	thisPoly  = m_polygon;
	Poly	otherPoly = body.m_polygon;
	thisPoly.transform(m_position, m_orientation);
	otherPoly.transform(body.m_position, body.m_orientation);

	// the relative velocity used to compute
	// swept collisions
	Vector delta = (m_velocity - body.m_velocity);

	return thisPoly.collide(otherPoly, delta);
}

void Body::render(bool collided) const
{
	Poly thisPoly = m_polygon;
	thisPoly.transform(m_position, m_orientation);

	int fill = (collided)? 0x80FFA080 : 0x808080FF;
	glColor4ub(ARGB_R(fill), ARGB_G(fill), ARGB_B(fill), ARGB_A(fill));
	thisPoly.render(true);

	int line = 0xFFFFFFFF;
	glColor4ub(ARGB_R(line), ARGB_G(line), ARGB_B(line), ARGB_A(line));
	thisPoly.render(false);

	renderArrow(m_position, m_position + m_velocity, 0xff80ff80);
}
	
