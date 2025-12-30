#pragma once
#include "CocosCustomDefines.h"
#include "GameMessaging.h"
#include "GameTimeController.h"

DECLARE_CUSTOM_VIEW_READER_CSB(CSBSceneMainMenuReader)
DECLARE_COCOS_CUSTOM_VIEW_CSB(CSBSceneMainMenu), public IGameMessageHandler, public IGameTimeHandler
{
	public:
		DECLARE_CUSTOM_VIEW_PREREQUISITES_CSB(CSBSceneMainMenu)
		virtual void	HandleMessage(std::string sKey, const MessageData* pMessageData);
		virtual void	Update(uint32_t iDeltaTimeMs);
	protected:
		DECLARE_CUSTOM_VIEW_LIFECYCLE_ONATTACHONDETACH
		DECLARE_CUSTOM_VIEW_LIFECYCLE_ONPOSTATTACH
		DECLARE_CUSTOM_VIEW_LIFECYCLE_ONPREDETTACH
		DECLARE_CUSTOM_VIEW_LIFECYCLE_ONLOADONUNLOAD
	private:
		DECLARE_COCOS_UI(cocos2d::ui::Button,	m_pButtonNext);
};