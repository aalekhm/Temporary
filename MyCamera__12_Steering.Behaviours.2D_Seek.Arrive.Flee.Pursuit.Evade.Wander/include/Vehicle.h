#ifndef VEHICLE_H_DEF
#define VEHICLE_H_DEF

#include <vector>
#include <Vectors.h>
#include "Matrices.h"

class Vehicle {

	public:
		enum Summing_method{weighted_average, prioritized, dithered};
		enum Deceleration{slow = 3, normal = 2, fast = 1};

		enum STEERING_BEHAVIOUR {
			SEEK = 1,
			ARRIVE = 2,
			FLEE = 4,
			PURSUIT = 8,
			EVADE = 16,
			WANDER = 32
		};

		Vehicle(	Vector2			vPos,
					double			dMaxSpeed,
					double			dMaxForce,
					double			dMass,
					Summing_method  SummingMethod
				);
		~Vehicle();

		/* .......................................................
						BEGIN BEHAVIOR DECLARATIONS
		.......................................................*/
		STEERING_BEHAVIOUR	getSteerBehaviour() { return m_iSteeringBehaviour; }
		void		setSteerBehaviour(STEERING_BEHAVIOUR st) { m_iSteeringBehaviour = st; }
		bool		isOn(STEERING_BEHAVIOUR st) { return (m_iSteeringBehaviour & st) > 0; }

		//this behavior moves the agent towards a target position
		Vector2		Seek(Vector2 TargetPos);

		//this behavior returns a vector that moves the agent away
		//from a target position
		Vector2		Flee(Vector2 TargetPos);

		//this behavior is similar to seek but it attempts to arrive 
		//at the target position with a zero velocity
		Vector2		Arrive(Vector2 TargetPos);

		//this behavior predicts where an agent will be in time T and seeks
		//towards that point to intercept it.
		Vector2		Pursuit(const Vehicle* evader);

		//this behavior attempts to evade a pursuer
		Vector2		Evade(const Vehicle* pursuer);

		//this behavior makes the agent wander about randomly
		Vector2		Wander();

		bool		AccumulateForce(Vector2 &RunningTot, Vector2 ForceToAdd);
		Vector2		CalculateDithered();
		Vector2		CalculateWeightedSum();
		Vector2		CalculatePrioritized();
		Vector2		Calculate();
		void		update(double dElapsedTime, double dDeltaTime);
		void		render();

		Vector2		getPosition() { return m_vPos; }
		Vector2		getHeading() { return m_vHeading; }
		Vector2		getRight() { return m_vRight; }

		int			getPosX();
		int			getPosY();
		void		setTarget(Vector2 vTarget);
		void		checkPositionBounds();
		std::vector<Vector2> getTransformedVertices();
		void		setWeight(STEERING_BEHAVIOUR st, double weight);
		
		double		randFloat();
		int			randInt(int x, int y);
		double		randomClamped();
		Vector2		pointToWorldSpace(const Vector2 &vPoint, const Vector2 &vAgentHeading,const Vector2 &vAgentSide, const Vector2 &vAgentPosition);

		Vector2					m_vPos;
		Vector2					m_vHeading;
		Vector2					m_vRight;
		std::vector<Vector2>	m_vVertices;
		Vehicle*				m_pTargetAgent1;
	private:
		Vector2			m_vVelocity;
		Vector2			m_vSteeringForce;
		Vector2			m_vTargetPos;
		double			m_ElapsedTimeMs;

		Vector2			m_vWanderTarget;
		double			m_dWanderRadius;
		double			m_dWanderDistance;

		double			m_dMaxSpeed;
		double			m_dMaxForce;
		double			m_dMass;
		double			m_dWeightSeek;
		double			m_dWeightArrive;
		double			m_dWeightFlee;
		double			m_dWeightPursuit;
		double			m_dWeightEvade;
		double			m_dWeightWander;

		Summing_method  m_SummingMethod;
		Deceleration	m_Deceleration;

		STEERING_BEHAVIOUR	m_iSteeringBehaviour;
};

#endif