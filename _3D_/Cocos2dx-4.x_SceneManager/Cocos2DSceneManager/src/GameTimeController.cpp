#pragma once
#include "GameTimeController.h"

#define TICK \
auto now = std::chrono::high_resolution_clock::now(); \
m_iDeltaTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_tpLastUpdate).count(); \
m_tpLastUpdate = now; \

GameTimeController::GameTimeController()
: m_iDeltaTimeMs(0)
{
	m_tpLastUpdate = std::chrono::high_resolution_clock::now();
}

void GameTimeController::Register(IGameTimeHandler* pGameTimeHandler)
{
	if (m_HandlerList.empty())
	{
		m_HandlerList.push_back(pGameTimeHandler);
	}
	else
	{
		auto pHandlerItr = std::find(m_HandlerList.begin(), m_HandlerList.end(), pGameTimeHandler);
		if (pHandlerItr == m_HandlerList.end())
		{
			m_HandlerList.push_back(pGameTimeHandler);
		}
	}	
}

void GameTimeController::UnRegister(IGameTimeHandler* pGameTimeHandler)
{
	if (!m_HandlerList.empty())
	{
		auto pHandler = std::find(m_HandlerList.begin(), m_HandlerList.end(), pGameTimeHandler);
		if (pHandler != m_HandlerList.end())
		{
			m_HandlerList.erase(pHandler);
		}
	}
}

bool GameTimeController::IsRegistered(IGameTimeHandler* pGameTimeHandler)
{
	if (m_HandlerList.empty())
	{
		return false;
	}

	auto pHandler = std::find(m_HandlerList.begin(), m_HandlerList.end(), pGameTimeHandler);
	return (pHandler != m_HandlerList.end());
}

void GameTimeController::Process()
{
	TICK

	if (!m_HandlerList.empty())
	{
		for (IGameTimeHandler* pIGameTimeHandler : m_HandlerList)
		{
			SEND_TICK(pIGameTimeHandler, m_iDeltaTimeMs);
		}
	}
}