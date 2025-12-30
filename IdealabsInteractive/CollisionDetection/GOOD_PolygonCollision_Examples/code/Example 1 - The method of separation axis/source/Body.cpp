
#include "Body.h"

Body::Body()
{}

Body::Body(float worldSize)
{
	m_position.randomise(Vector(worldSize * 0.25f, worldSize * 0.25f), Vector(worldSize * 0.75f, worldSize * 0.75f));
	m_orientation = frand(pi());

	int count = rand() % 6 + 3;
	float rad = frand(worldSize * 0.05f) + worldSize * 0.1f;
	m_polygon = Poly(count, rad);
}

bool Body::collide(const Body& body) const
{
	Poly thisPoly = m_polygon;
	Poly otherPoly = body.m_polygon;
	thisPoly.transform(m_position, m_orientation);
	otherPoly.transform(body.m_position, body.m_orientation);

	return thisPoly.collide(otherPoly);
}

void Body::render(bool collided) const
{
	Poly thisPoly = m_polygon;
	thisPoly.transform(m_position, m_orientation);

	int fill = (collided)? 0x80FF8080 : 0x808080FF;
	glColor4ub(ARGB_R(fill), ARGB_G(fill), ARGB_B(fill), ARGB_A(fill));
	thisPoly.render(true);

	int line = 0xFFFFFFFF;
	glColor4ub(ARGB_R(line), ARGB_G(line), ARGB_B(line), ARGB_A(line));
	thisPoly.render(false);
}
	
