
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

Body::Body(const Vector& min, const Vector& max)
{
	m_invinertia	= 0.0f;
	m_invmass		= 0.0f;
	m_orientation	= 0.0f;
	m_position		= Vector(0, 0);
	m_polygon		= Poly(min, max);
}

Body::Body(float worldSize)
{
	m_position.randomise(Vector(worldSize * 0.25f, worldSize * 0.25f), Vector(worldSize * 0.75f, worldSize * 0.75f));
	m_orientation = frand(pi());
	m_velocity.randomise(Vector(-worldSize * 0.01f, -worldSize * 0.01f), Vector(worldSize * 0.01f, worldSize * 0.01f));
	m_angvelocity = frand(0.01f);

	int count = rand() % 6 + 3;
	float rad = frand(worldSize * 0.05f) + worldSize * 0.05f;
	m_polygon = Poly(count, rad);

	float density  = (frand(1.0f) < 0.3f)? 0.0f : frand(0.3f) + 0.7f;
	float inertia  = (density == 0.0f)? 0.0f : m_polygon.calculateInertia();
	float mass     = (density == 0.0f)? 0.0f : m_polygon.calculateMass(density);

	m_invmass = (mass > 0.0f)? 1.0f / mass : 0.0f;
	m_invinertia = (inertia > 0.0f)? 1.0f / (inertia * mass) : 0.0f; 
}


void Body::update()
{
	if(m_invmass == 0.0f)
	{
		m_velocity = Vector(0, 0);
		m_angvelocity = 0.0f;
		return;
	}

	m_velocity += Vector(0.0f, -0.01f);

	//m_velocity.y -= 0.01f;
	m_orientation += m_angvelocity;	
	m_position += m_velocity;
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
	m_contact			= ContactPair();
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
		
	if(!m_collisionReported)
		return;

	// convert collision info into collison plane info
	if(m_collisionInfo.m_overlapped)
	{
		if(m_collisionInfo.m_mtdLengthSquared <= 0.00001f)
		{
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
	}	

	// find contact points at time of collision
	m_poly[0].translate(a->m_velocity * m_tcoll);
	m_poly[1].translate(b->m_velocity * m_tcoll);
	
	// support pointys of the two objects
	SupportPoints asup = m_poly[0].getSupports(m_ncoll);
	SupportPoints bsup = m_poly[1].getSupports(-m_ncoll);

	// the contact patch .
	m_manifold = ContactManifold(asup, bsup);

	// approximate the contact patch to a single contact pair.
	m_contact  = m_manifold.reduction();
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

void CollisionReport::applyReponse(float cor, float cof)
{
	if(!m_collisionReported)
		return;

	Body* a = m_body[0];
	Body* b = m_body[1];

	// overlapped. then separate the bodies.
	a->m_position += m_mtd * (a->m_invmass / (a->m_invmass + b->m_invmass));
	b->m_position -= m_mtd * (b->m_invmass / (a->m_invmass + b->m_invmass));
	
	// move to time of collision
	a->m_position += a->m_velocity * m_tcoll;
	b->m_position += b->m_velocity * m_tcoll;

	// apply friction impulses at contacts
	Vector pa = m_contact.m_position[0];
	Vector pb = m_contact.m_position[1];
	Vector ra = pa - a->m_position;
	Vector rb = pb - b->m_position;
	Vector va = a->m_velocity + ra.perp() * a->m_angvelocity;
	Vector vb = b->m_velocity + rb.perp() * b->m_angvelocity;
	Vector v  = (va - vb);
	Vector vt = v - (v * m_ncoll) * m_ncoll;
	Vector nf = -vt; nf.normalise(); // friction normal
	Vector nc = m_ncoll; // collision normal

	// contact points separating, no impulses.
	if(v * nc > 0.0f) return;
	
	// collision impulse
	float jc	= (v * nc) / (	(a->m_invmass + b->m_invmass) + 
								(ra ^ nc) * (ra ^ nc) * a->m_invinertia + 
								(rb ^ nc) * (rb ^ nc) * b->m_invinertia);

	// friction impulse
	float jf	= (v * nf)	/ (	(a->m_invmass + b->m_invmass) + 
								(ra ^ nf) * (ra ^ nf) * a->m_invinertia + 
								(rb ^ nf) * (rb ^ nf) * b->m_invinertia);

	// clamp friction. 
	if(fabs(jf) > fabs(jc * cof)) 
		jf = fabs(cof) * sign(jc);
	
	// total impulse restituted
	Vector impulse = nc * (jc * -(1.0f + cor)) + nf * (jf * -1.0f);

	a->m_velocity += impulse * a->m_invmass;
	b->m_velocity -= impulse * b->m_invmass;

	a->m_angvelocity += (ra ^ impulse) * a->m_invinertia;
	b->m_angvelocity -= (rb ^ impulse) * b->m_invinertia;
}
