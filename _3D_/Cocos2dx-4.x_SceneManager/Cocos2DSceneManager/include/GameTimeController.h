#pragma once
#include "Singleton.h"
#include <vector>
#include <chrono>

#define SEND_TICK(_handler__, __iDeltaTimeMs__) _handler__->Update(__iDeltaTimeMs__);

class IGameTimeHandler
{
	public:
		virtual void	Update(unsigned int iDeltaTimeMs) = 0;
};

typedef	std::vector<IGameTimeHandler*>		GAMETIMEHANDLERLIST;

class GameTimeController : public Singleton<GameTimeController>
{
	public:
															GameTimeController();
		void												Register(IGameTimeHandler* pGameTimeHandler);
		void												UnRegister(IGameTimeHandler* pGameTimeHandler);
		bool												IsRegistered(IGameTimeHandler* pGameTimeHandler);

		void												Process();
	protected:

	private:
		GAMETIMEHANDLERLIST									m_HandlerList;

		uint32_t											m_iDeltaTimeMs;
		std::chrono::high_resolution_clock::time_point		m_tpLastUpdate;
};