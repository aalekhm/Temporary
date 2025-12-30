#ifndef GAMEWORLD_H_DEF
#define GAMEWORLD_H_DEF

#include <vector>
#include <Vectors.h>
#include "Vehicle.h"

class GameWorld {

	public:
		GameWorld(int x, int y, int w, int h);
		~GameWorld();

		void		update(double dElapsedTimeMs, double dDeltaTime);
		void		render();
		void		addVehicle(Vehicle* pVehicle);
		int			cxClient();
		int			cyClient();
		int			cwClient();
		int			chClient();

		Vector2		getCrosshair();
		void		setCrosshair(int x, int y);

		int			getVehicleCount();
		Vehicle*	getVehicle(int i);
	private:
		//a container of all the moving entities
		std::vector<Vehicle*>         m_Vehicles;

		//the position of the crosshair
		Vector2                      m_vCrosshair;

		//local copy of client window dimensions
		int							m_cxClient,
									m_cyClient,
									m_cwClient,
									m_chClient;
};

#endif