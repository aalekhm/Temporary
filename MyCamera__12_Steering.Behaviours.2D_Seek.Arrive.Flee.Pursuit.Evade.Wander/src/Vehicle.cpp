#include "Vehicle.h"
#include "Defines.h"

Vehicle::Vehicle(	Vector2 vPos,
					double	dMaxSpeed,
					double	dMaxForce,
					double	dMass,
					Summing_method  SummingMethod
) {
	m_vPos = vPos;

	m_dMaxSpeed = dMaxSpeed;
	m_dMaxForce = dMaxForce;
	m_dMass = dMass;

	setWeight(STEERING_BEHAVIOUR::SEEK, 200.0f);
	setWeight(STEERING_BEHAVIOUR::ARRIVE, 200.0f);
	setWeight(STEERING_BEHAVIOUR::FLEE, 200.0f);
	setWeight(STEERING_BEHAVIOUR::PURSUIT, 200.0f);
	setWeight(STEERING_BEHAVIOUR::EVADE, 200.0f);
	setWeight(STEERING_BEHAVIOUR::WANDER, 200.0f);

	m_SummingMethod = SummingMethod;
	m_iSteeringBehaviour = STEERING_BEHAVIOUR::SEEK;
	
	m_vVertices.push_back(Vector2(-8.0f, -7.0f));
	m_vVertices.push_back(Vector2(-8.0f, 6.0f));
	m_vVertices.push_back(Vector2(8.0f, 6.0f));
	m_vVertices.push_back(Vector2(8.0f, -7.0f));

	//stuff for the wander behavior
	double theta = randFloat() * (3.14159*2);
	m_dWanderRadius = 5.0f;
	m_dWanderDistance = 1.0f;
	//create a vector to a target position on the wander circle
	m_vWanderTarget = Vector2(m_dWanderRadius * cos(theta), m_dWanderRadius * sin(theta));
}

Vehicle::~Vehicle() {
}

void Vehicle::setWeight(STEERING_BEHAVIOUR st, double weight) {
	switch(st) {
		case STEERING_BEHAVIOUR::SEEK:
			m_dWeightSeek = weight;
		break;
		case STEERING_BEHAVIOUR::ARRIVE:
			m_dWeightArrive = weight;
		break;
		case STEERING_BEHAVIOUR::FLEE:
			m_dWeightFlee = weight;
		break;
		case STEERING_BEHAVIOUR::PURSUIT:
			m_dWeightPursuit = weight;
		break;
		case STEERING_BEHAVIOUR::EVADE:
			m_dWeightEvade = weight;
		break;
		case STEERING_BEHAVIOUR::WANDER:
			m_dWeightWander = weight;
		break;
	}	
}

//this behavior moves the agent towards a target position
Vector2 Vehicle::Seek(Vector2 TargetPos) {
	Vector2 vDesiredVelocity = (TargetPos - m_vPos);
	vDesiredVelocity.normalize();
	vDesiredVelocity *= m_dMaxSpeed;

	Vector2 retVector = (vDesiredVelocity - m_vVelocity);
	return retVector;
}

//this behavior returns a vector that moves the agent away
//from a target position
Vector2 Vehicle::Flee(Vector2 TargetPos) {
	Vector2 vDesiredVelocity = (m_vPos - TargetPos);

	//only flee if the target is within 'panic distance'. Work in distance
	//squared space.
	///* 
	const double PanicDistanceSq = 100.0f * 100.0;
	if(vDesiredVelocity.lengthSq() > PanicDistanceSq) {
		return Vector2(0,0);
	}
	//*/

	vDesiredVelocity.normalize();
	vDesiredVelocity *= m_dMaxSpeed;

	return (vDesiredVelocity - m_vVelocity);
}

//this behavior is similar to seek but it attempts to arrive 
//at the target position with a zero velocity
Vector2 Vehicle::Arrive(Vector2 TargetPos) {
	Vector2 vDesiredVelocity = (TargetPos - m_vPos);
	vDesiredVelocity.truncate(m_dMaxSpeed);

	Vector2 retVector = (vDesiredVelocity - m_vVelocity);
	return retVector;
}

//------------------------------ Pursuit ---------------------------------
//
//  this behavior creates a force that steers the agent towards the 
//  evader
//------------------------------------------------------------------------
Vector2 Vehicle::Pursuit(const Vehicle* evader) {
	//if the evader is ahead and facing the agent then we can just seek
	//for the evader's current position.
	Vector2 ToEvader = evader->m_vPos - m_vPos;

	double RelativeHeading = m_vHeading.dot(evader->m_vHeading);

	if ( (ToEvader.dot(m_vHeading) > 0) &&  
		(RelativeHeading < -0.95))  //acos(0.95)=18 degs
	{
		return Seek(evader->m_vPos);
	}

	//Not considered ahead so we predict where the evader will be.

	//the lookahead time is propotional to the distance between the evader
	//and the pursuer; and is inversely proportional to the sum of the
	//agent's velocities
	double evadersSpeed = evader->m_vVelocity.length();
	double LookAheadTime = ToEvader.length() / (m_dMaxSpeed + evadersSpeed);

	//now seek to the predicted future position of the evader
	return Seek(evader->m_vPos + evader->m_vVelocity * LookAheadTime);
}

//----------------------------- Evade ------------------------------------
//
//  similar to pursuit except the agent Flees from the estimated future
//  position of the pursuer
//------------------------------------------------------------------------
Vector2 Vehicle::Evade(const Vehicle* pursuer) {
	/* Not necessary to include the check for facing direction this time */

	Vector2 ToPursuer = pursuer->m_vPos - m_vPos;

	//uncomment the following two lines to have Evade only consider pursuers 
	//within a 'threat range'
	const double ThreatRange = 100.0;
	if (ToPursuer.lengthSq() > ThreatRange * ThreatRange) return Vector2(0, 0);

	//the lookahead time is propotional to the distance between the pursuer
	//and the pursuer; and is inversely proportional to the sum of the
	//agents' velocities
	double pursuersSpeed = pursuer->m_vVelocity.length();
	double LookAheadTime = ToPursuer.length() / (m_dMaxSpeed + pursuersSpeed);

	//now flee away from predicted future position of the pursuer
	return Flee(pursuer->m_vPos + pursuer->m_vVelocity * LookAheadTime);
}

//--------------------------- Wander -------------------------------------
//
//  This behavior makes the agent wander about randomly
//------------------------------------------------------------------------
Vector2 Vehicle::Wander() {
	//this behavior is dependent on the update rate, so this line must
	//be included when using time independent framerate.
	double JitterThisTimeSlice = 1.0;//m_dWanderJitter * m_pVehicle->TimeElapsed();

	//first, add a small random vector to the target's position
	m_vWanderTarget += Vector2(randomClamped() * JitterThisTimeSlice, randomClamped() * JitterThisTimeSlice);

	//reproject this new vector back on to a unit circle
	m_vWanderTarget.normalize();

	//increase the length of the vector to the same as the radius
	//of the wander circle
	m_vWanderTarget *= m_dWanderRadius;

	//move the target into a position WanderDist in front of the agent
	Vector2 vTarget = m_vWanderTarget + Vector2(m_dWanderDistance, 0);
	
	//and steer towards it
	return Seek(m_vPos + vTarget);
}

bool Vehicle::AccumulateForce(Vector2 &RunningTot, Vector2 ForceToAdd)
{
	//calculate how much steering force the vehicle has used so far
	double MagnitudeSoFar = RunningTot.length();

	//calculate how much steering force remains to be used by this vehicle
	double MagnitudeRemaining = m_dMaxForce - MagnitudeSoFar;

	//return false if there is no more force left to use
	if (MagnitudeRemaining <= 0.0) return false;

	//calculate the magnitude of the force we want to add
	double MagnitudeToAdd = ForceToAdd.length();

	//if the magnitude of the sum of ForceToAdd and the running total
	//does not exceed the maximum force available to this vehicle, just
	//add together. Otherwise add as much of the ForceToAdd vector is
	//possible without going over the max.
	if (MagnitudeToAdd < MagnitudeRemaining)
	{
		RunningTot += ForceToAdd;
	}
	else
	{
		//add it to the steering force
		RunningTot += (Vec2DNormalize(ForceToAdd) * MagnitudeRemaining); 
	}

	return true;
}

Vector2 Vehicle::CalculateDithered()
{
	return Vector2(0, 0);
}

Vector2 Vehicle::CalculateWeightedSum()
{
	if(isOn(STEERING_BEHAVIOUR::SEEK)) {
		m_vSteeringForce += Seek(m_vTargetPos) * m_dWeightSeek;
		m_vSteeringForce.truncate(m_dMaxForce);
	}

	if(isOn(STEERING_BEHAVIOUR::ARRIVE)) {
		m_vSteeringForce += Arrive(m_vTargetPos) * m_dWeightArrive;
		m_vSteeringForce.truncate(m_dMaxForce);
	}

	if(isOn(STEERING_BEHAVIOUR::FLEE)) {
		m_vSteeringForce += Flee(m_vTargetPos) * m_dWeightFlee;
		m_vSteeringForce.truncate(m_dMaxForce);
	}

	if(isOn(STEERING_BEHAVIOUR::PURSUIT)) {
		m_vSteeringForce += Pursuit(m_pTargetAgent1) * m_dWeightPursuit;
		m_vSteeringForce.truncate(m_dMaxForce);
	}

	if(isOn(STEERING_BEHAVIOUR::EVADE)) {
		m_vSteeringForce += Evade(m_pTargetAgent1) * m_dWeightEvade;
		m_vSteeringForce.truncate(m_dMaxForce);
	}

	if(isOn(STEERING_BEHAVIOUR::WANDER)) {
		m_vSteeringForce += Wander() * m_dWeightWander;
		m_vSteeringForce.truncate(m_dMaxForce);
	}

	return m_vSteeringForce;
}

Vector2 Vehicle::CalculatePrioritized()
{
	Vector2 force;

	if(isOn(STEERING_BEHAVIOUR::SEEK)) {
		force = Seek(m_vTargetPos) * m_dWeightSeek;
	}

	if(isOn(STEERING_BEHAVIOUR::ARRIVE)) {
		force = Arrive(m_vTargetPos) * m_dWeightSeek;
	}

	if(isOn(STEERING_BEHAVIOUR::FLEE)) {
		force = Flee(m_vTargetPos) * m_dWeightSeek;
	}

	if(isOn(STEERING_BEHAVIOUR::PURSUIT)) {
		force = Pursuit(m_pTargetAgent1) * m_dWeightPursuit;
	}

	if(isOn(STEERING_BEHAVIOUR::EVADE)) {
		force = Evade(m_pTargetAgent1) * m_dWeightEvade;
	}

	if(isOn(STEERING_BEHAVIOUR::WANDER)) {
		force = Wander() * m_dWeightWander;
	}

	if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;

	return m_vSteeringForce;
}

Vector2 Vehicle::Calculate()
{
	//reset the steering force
	m_vSteeringForce.zero();

	switch (m_SummingMethod)
	{
		case weighted_average:
			m_vSteeringForce = CalculateWeightedSum(); break;
		case prioritized:
			m_vSteeringForce = CalculatePrioritized(); break;
		case dithered:
			m_vSteeringForce = CalculateDithered();break;
		default:
			m_vSteeringForce = Vector2(0,0); 
	}//end switch

	return m_vSteeringForce;
}

void Vehicle::update(double dElapsedTime, double dDeltaTime) {
	m_ElapsedTimeMs = dElapsedTime;

	Vector2 SteeringForce;

	////calculate the combined force from each steering behavior in the 
	////vehicle's list
	SteeringForce = Calculate();

	////Acceleration = Force/Mass
	Vector2 acceleration = SteeringForce / m_dMass;

	////update velocity
	m_vVelocity += acceleration * (dDeltaTime/1000.0f);

	////make sure vehicle does not exceed maximum velocity
	m_vVelocity.truncate(m_dMaxSpeed);

	//update the position
	m_vPos += m_vVelocity * (dDeltaTime/1000.0f);

	const float EPSILON = 0.00000001f;
	if(m_vVelocity.lengthSq() > EPSILON) 
	{
		m_vHeading = m_vVelocity;
		m_vRight = m_vHeading.perpendicular();
	}

	checkPositionBounds();
}

void Vehicle::render() {
		std::vector<Vector2> vVehicleVertices = getTransformedVertices();

		STEERING_BEHAVIOUR eSteer = getSteerBehaviour();
		RectF uvRectF(0.0f, 0.0f, 0.0f, 0.0f);
		switch(eSteer)
		{
			case SEEK:
			{
				uvRectF.X = 22.0f;
				uvRectF.X = 251.0f;
				uvRectF.X = 16.0f;
				uvRectF.X = 13.0f;
			}
			break;
			case ARRIVE:
			{
				uvRectF.X = 22.0f;
				uvRectF.X = 266.0f;
				uvRectF.X = 16.0f;
				uvRectF.X = 13.0f;
			}
			break;
			case FLEE:
			{
				uvRectF.X = 22.0f;
				uvRectF.X = 251.0f;
				uvRectF.X = 16.0f;
				uvRectF.X = 13.0f;
			}
			break;
			case PURSUIT:
			{
				uvRectF.X = 40.0f;
				uvRectF.X = 266.0f;
				uvRectF.X = 16.0f;
				uvRectF.X = 13.0f;
			}
			break;
			case EVADE:
			{
				uvRectF.X = 4.0f;
				uvRectF.X = 266.0f;
				uvRectF.X = 16.0f;
				uvRectF.X = 13.0f;
			}
			break;
			case WANDER:
			{
				uvRectF.X = 4.0f;
				uvRectF.X = 251.0f;
				uvRectF.X = 16.0f;
				uvRectF.X = 13.0f;
			}
			break;
		}
		
		//drawQuadU(TEXTURE_CORE, vVehicleVertices, uvRectF.X, uvRectF.Y, uvRectF.Width, uvRectF.Height);			
}

int Vehicle::getPosX() {
	return m_vPos.x;
}

int Vehicle::getPosY() {
	return m_vPos.y;
}

void Vehicle::setTarget(Vector2 vTarget) {
	m_vTargetPos = vTarget;
}

void Vehicle::checkPositionBounds() {
	Vector2 vWorldPos = Vector2(1024>>1, 768>>1);
	Vector2 vWorldSize = Vector2(1024>>1, 768>>1);

	if(m_vPos.x > vWorldPos.x + vWorldSize.x)
		m_vPos.x = vWorldPos.x;
	else
	if(m_vPos.x < vWorldPos.x)
		m_vPos.x = vWorldPos.x + vWorldSize.x;

	if(m_vPos.y > vWorldPos.y + vWorldSize.y)
		m_vPos.y = vWorldPos.y;
	else
	if(m_vPos.y < vWorldPos.y)
		m_vPos.y = vWorldPos.y + vWorldSize.y;
}

std::vector<Vector2> Vehicle::getTransformedVertices() {
	//copy the original vertices into the buffer about to be transformed
	std::vector<Vector2> vVehicleVertices = m_vVertices;

	//create a transformation matrix
	Matrix3 matTransform;

	//scale
	matTransform.scale(1.0, 1.0);

	//rotate
	matTransform.rotate(m_vHeading.normalize(), m_vRight.normalize());

	//and translate
	matTransform.translate(m_vPos.x, m_vPos.y);

	//now transform the object's vertices
	matTransform.transformVector2Ds(vVehicleVertices);
	
	return vVehicleVertices;
}

//returns a random integer between x and y
int Vehicle::randInt(int x, int y)
{
	return rand() % (y-x+1) + x;
}

//returns a random double between zero and 1
double Vehicle::randFloat()      {return ((rand())/(RAND_MAX+1.0));}

//returns a random double in the range -1 < n < 1
double Vehicle::randomClamped()    {
	return randFloat() - randFloat();
}

//--------------------- PointToWorldSpace --------------------------------
//
//  Transforms a point from the agent's local space into world space
//------------------------------------------------------------------------
Vector2 Vehicle::pointToWorldSpace(	const Vector2 &vPoint,
									const Vector2 &vAgentHeading,
									const Vector2 &vAgentSide,
									const Vector2 &vAgentPosition)
{
	//make a copy of the point
	Vector2 vTransPoint = vPoint;

	//create a transformation matrix
	Matrix3 matTransform;

	//rotate
	matTransform.rotate(vAgentHeading, vAgentSide);

	//and translate
	matTransform.translate(vAgentPosition.x, vAgentPosition.y);

	//now transform the vertices
	matTransform.transformVector2D(vTransPoint);

	return vTransPoint;
}