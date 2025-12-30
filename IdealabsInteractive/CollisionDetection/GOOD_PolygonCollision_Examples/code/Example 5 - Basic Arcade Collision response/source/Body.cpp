
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

Body::Body(const Vector& min, const Vector& max, float mass)
{
	m_invmass		= (mass <= 0.0f)? 0.0f : 1.0f / mass;
	m_orientation	= 0.0f;
	m_position		= Vector(0, 0);
	m_polygon		= Poly(min, max);
}

Body::Body(float worldSize, float mass)
{
	m_invmass = (mass <= 0.0f)? 0.0f : 1.0f / mass;
	m_position.randomise(Vector(worldSize * 0.25f, worldSize * 0.25f), Vector(worldSize * 0.75f, worldSize * 0.75f));
	m_orientation = frand(pi());
	m_velocity.randomise(Vector(-worldSize * 0.01f, -worldSize * 0.01f), Vector(worldSize * 0.01f, worldSize * 0.01f));

	int count = rand() % 6 + 3;
	float rad = frand(worldSize * 0.02f) + worldSize * 0.02f;
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

void Body::render() const
{
	Poly thisPoly = m_polygon;
	thisPoly.transform(m_position, m_orientation);

	int fill = (m_invmass == 0.0f)? 0x80FFA080 : 0x808080FF;
	glColor4ub(ARGB_R(fill), ARGB_G(fill), ARGB_B(fill), ARGB_A(fill));
	thisPoly.render(true);

	int line = 0xFFFFFFFF;
	glColor4ub(ARGB_R(line), ARGB_G(line), ARGB_B(line), ARGB_A(line));
	thisPoly.render(false);

	renderArrow(m_position, m_position + m_velocity, 0xff80ff80);
}
	
void Body::update()
{
	if(m_invmass == 0.0f)
	{
		m_velocity = Vector(0, 0);
		return;
	}

	m_position += m_velocity;
}

void Body::respondToCollision(Body& body, const CollisionInfo& info)
{
	// overlapped. then separate the bodies.
	if(info.m_overlapped)
	{
		m_position += info.m_mtd * (m_invmass / (m_invmass + body.m_invmass));
		body.m_position -= info.m_mtd * (body.m_invmass / (m_invmass + body.m_invmass));
	}

	float tcoll = 0.0f;
	Vector ncoll(0.0f, 0.0f);

	// move bodies to collision time
	if(info.m_collided && info.m_tenter > 0.0f)
	{
		tcoll = info.m_tenter;
		ncoll = info.m_Nenter;
	}
	else
	{
		if(info.m_mtdLengthSquared < 0.00001f)
			return;

		tcoll = 0.0f;
		ncoll = info.m_mtd / sqrt(info.m_mtdLengthSquared);
	}
		
	m_position += m_velocity * tcoll;
	body.m_position += body.m_velocity * tcoll;

	Vector vcoll = (m_velocity - body.m_velocity);
	float vn = vcoll * ncoll;
	
	if(vn > 0.0f) 
		return;

	float CoR = 1.0f;
	float numer = -(1.0f + CoR) * vn;
	float denom = (m_invmass + body.m_invmass);
	Vector j = ncoll * (numer / denom);

	m_velocity += j * m_invmass;
	body.m_velocity -= j * body.m_invmass;
}

	