#include "GameMessaging.h"

void GameMessaging::AddListener(std::string sMessage, IGameMessageHandler* pGameMessageHandler)
{
	GAMEMESSAGEHANDLERS& vHandlers = m_HandlerList[sMessage];
	if (vHandlers.empty())
	{
		vHandlers = std::vector<IGameMessageHandler*>();
		vHandlers.push_back(pGameMessageHandler);
	}
	else
	{
		auto pHandlerItr = std::find(vHandlers.begin(), vHandlers.end(), pGameMessageHandler);
		if (pHandlerItr == vHandlers.end())
		{
			vHandlers.push_back(pGameMessageHandler);
		}
	}	
}

void GameMessaging::RemoveListener(std::string sMessage, IGameMessageHandler* pGameMessageHandler)
{
	GAMEMESSAGEHANDLERS& vHandlers = m_HandlerList[sMessage];
	if (!vHandlers.empty())
	{
		auto pHandler = std::find(vHandlers.begin(), vHandlers.end(), pGameMessageHandler);
		if (pHandler != vHandlers.end())
		{
			vHandlers.erase(pHandler);
		}
	}
}

bool GameMessaging::IsRegistered(std::string sMessage, IGameMessageHandler* pGameMessageHandler)
{
	GAMEMESSAGEHANDLERS& vHandlers = m_HandlerList[sMessage];
	if (vHandlers.empty())
	{
		return false;
	}

	auto pHandler = std::find(vHandlers.begin(), vHandlers.end(), pGameMessageHandler);
	return (pHandler != vHandlers.end());
}

void GameMessaging::MessageSend(std::string sMessage, std::unique_ptr<MessageData> pData)
{
	GAMEMESSAGEHANDLERS& vHandlers = m_HandlerList[sMessage];
	if (!vHandlers.empty())
	{
		for (IGameMessageHandler* pHandler : vHandlers)
		{
			if (pHandler != nullptr)
			{
				pHandler->HandleMessage(sMessage, pData.get());
			}
		}

		pData.reset();
	}
}

void GameMessaging::MessagePost(std::string sMessage, std::unique_ptr<MessageData> pData)
{
	m_PostDataList[sMessage] = std::move(pData);
}

void GameMessaging::ProcessQueue()
{
	if (!m_PostDataList.empty())
	{
		GAMEMESSAGE_POSTDATA::iterator mapItr = m_PostDataList.begin();
		while (mapItr != m_PostDataList.end())
		{
			MESSAGE_SEND(mapItr->first, mapItr->second);
			mapItr++;
		}

		m_PostDataList.clear();
	}
}