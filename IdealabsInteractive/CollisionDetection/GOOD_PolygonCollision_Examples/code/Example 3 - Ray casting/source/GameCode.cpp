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
Vector rayStart, rayEnd;
RayCastingInfo info;
	
void GameShutdown()
{	
}

void GameInit(void)
{
	GameShutdown();

	A = Body(dbg_world_size);
	rayStart.randomise(Vector(dbg_world_size * 0.25f, dbg_world_size * 0.25f), Vector(dbg_world_size * 0.75f, dbg_world_size * 0.75f));
	rayEnd.randomise(Vector(dbg_world_size * 0.25f, dbg_world_size * 0.25f), Vector(dbg_world_size * 0.75f, dbg_world_size * 0.75f));
}

void GameUpdate(float dt)
{
	info = A.trace(rayStart, (rayEnd - rayStart));
}

void GameRender(void)
{
	A.render(info.m_intersected);

	renderSegment(rayStart, rayEnd, 0xFFFFFFFF);
	
	if(info.m_intersected)
	{
		Vector enter = rayStart + (rayEnd - rayStart) * info.m_tenter;
		renderArrow(enter, info.m_Nenter, 10, 0xFFFFFF80);

		Vector exit = rayStart + (rayEnd - rayStart) * info.m_tleave;
		renderArrow(exit, info.m_Nleave, 10, 0xFFFFFF80);

		renderDottedSegment(rayStart, rayEnd, 0xFFFF0000);
	}
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

