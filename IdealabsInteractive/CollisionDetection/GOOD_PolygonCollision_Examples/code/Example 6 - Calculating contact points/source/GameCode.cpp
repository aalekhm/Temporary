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

Body A;
Body B;
CollisionReport report;
	
void GameShutdown()
{	
}

void GameInit(void)
{
	GameShutdown();

	A = Body(dbg_world_size);
	B = Body(dbg_world_size);
}

void GameUpdate(float dt)
{
	report = CollisionReport(&A, &B);
}

void GameRender(void)
{
	A.render();
	B.render();
	report.render();
}

void GameOnKeyCallback(int keypressed)
{
	switch(keypressed)
	{
	case ' ':				GameInit();				break;
	case GLUT_KEY_DOWN:		A.m_position.y -= 2.0f;	break;
	case GLUT_KEY_UP:		A.m_position.y += 2.0f;	break;
	case GLUT_KEY_LEFT:		A.m_position.x -= 2.0f;	break;
	case GLUT_KEY_RIGHT:	A.m_position.x += 2.0f;	break;
	case '-':				A.m_orientation -= 0.1f;break;
	case '=':				A.m_orientation += 0.1f;break;
	}
}


void GameMouse(float x, float y, int buttons)
{
}

