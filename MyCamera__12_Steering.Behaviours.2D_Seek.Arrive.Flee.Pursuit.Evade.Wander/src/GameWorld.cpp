#include "GameWorld.h"

GameWorld::GameWorld(int x, int y, int w, int h) {

	m_cxClient = x;
	m_cyClient = y;
	m_cwClient = w;
	m_chClient = h;
}

GameWorld::~GameWorld() {
	
}

void GameWorld::update(double dElapsedTimeMs, double dDeltaTime)
{
	Vehicle* pVehicle = NULL;
	for(int i = 0; i < m_Vehicles.size(); i++) {
		pVehicle = (Vehicle*) m_Vehicles[i];
		
		pVehicle->setTarget(getCrosshair());
		pVehicle->update(dElapsedTimeMs, dDeltaTime);
	}
}

void GameWorld::render()
{
	Vehicle* pVehicle = NULL;
	for(int i = 0; i < m_Vehicles.size(); i++) {
		pVehicle = (Vehicle*) m_Vehicles[i];

		pVehicle->render();
	}
}

void GameWorld::addVehicle(Vehicle* pVehicle) {
	m_Vehicles.push_back(pVehicle);
}

Vector2	GameWorld::getCrosshair() { 
	return m_vCrosshair;
}

void GameWorld::setCrosshair(int x, int y) {
	m_vCrosshair.x = x;
	m_vCrosshair.y = y;
}

int GameWorld::getVehicleCount() {
	return m_Vehicles.size();
}

Vehicle* GameWorld::getVehicle(int i) {
	int size = m_Vehicles.size();
	if(i < size) {
		return m_Vehicles[i];
	}
}

int	GameWorld::cxClient() { return m_cxClient;}
int	GameWorld::cyClient() { return m_cyClient;}
int	GameWorld::cwClient() { return m_cwClient;}
int	GameWorld::chClient() { return m_chClient;}
