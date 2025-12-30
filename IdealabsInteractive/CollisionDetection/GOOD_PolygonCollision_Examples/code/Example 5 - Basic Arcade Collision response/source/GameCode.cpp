/*
------------------------------------------------------------------
File: dynamics.cpp
Started: 18/01/2004 14:15:44
  
$Header: $
$Revision: $
$Locker: $
$Date: $
  
Author: Olivier renault
------------------------------------------------------------------
Module: 
Description: 
------------------------------------------------------------------
$History: $
------------------------------------------------------------------
*/

//-----------------------------------------------------------
// Verlet rigid body dynamics
//-----------------------------------------------------------
#include "GameCode.h"
#include "Body.h"

float  dbg_world_size	= 100;

enum { MAX_BODIES = 200 };
int bodycount = 0;
int player = 0;
Body body[MAX_BODIES];
	
void GameShutdown()
{	
}

void GameInit(void)
{
	GameShutdown();

	bodycount = 0;
	player = 0;
	body[bodycount++] = Body(Vector(-dbg_world_size, -dbg_world_size), Vector(0.0f, dbg_world_size * 2));
	body[bodycount++] = Body(Vector( dbg_world_size, -dbg_world_size), Vector(dbg_world_size * 2, dbg_world_size));
	body[bodycount++] = Body(Vector(-dbg_world_size, -dbg_world_size), Vector(dbg_world_size * 2, 0.0f));
	body[bodycount++] = Body(Vector(-dbg_world_size,  dbg_world_size), Vector(dbg_world_size * 2, dbg_world_size * 2));

	player = bodycount;
	for(int i  =0; i < 10; i ++)
	{
		body[bodycount++] = Body(dbg_world_size, frand(3.0f) + 1.0f);
	}
}

void GameUpdate(float dt)
{
	for(int i = 0; i < bodycount; i ++)
	{
		body[i].update();
	}

	for(int i = 0; i < bodycount; i ++)
	{
		for(int j = i+1; j < bodycount; j ++)
		{
			// both bodies static. skip
			if(body[i].m_invmass == 0.0f && body[j].m_invmass == 0.0f)
				continue;

			CollisionInfo info = body[i].collide(body[j]);

			if(info.m_collided || info.m_overlapped)
			{
				body[i].respondToCollision(body[j], info);
			}
		}
	}
}

void GameRender(void)
{
	for(int i = 0; i < bodycount; i ++)
	{
		body[i].render();
	}
}

void GameOnKeyCallback(int keypressed)
{
	switch(keypressed)
	{
	case ' ':				GameInit();							break;
	case GLUT_KEY_DOWN:		body[player].m_velocity.y -= 0.2f;	break;
	case GLUT_KEY_UP:		body[player].m_velocity.y += 0.2f;	break;
	case GLUT_KEY_LEFT:		body[player].m_velocity.x -= 0.2f;	break;
	case GLUT_KEY_RIGHT:	body[player].m_velocity.x += 0.2f;	break;
	case '-':				if(bodycount > 5) bodycount--;		break;
	case '=':				if(bodycount < MAX_BODIES) body[bodycount++] = Body(dbg_world_size, frand(3.0f) + 1.0f); break;
	}
}


void GameMouse(float x, float y, int buttons)
{
}

