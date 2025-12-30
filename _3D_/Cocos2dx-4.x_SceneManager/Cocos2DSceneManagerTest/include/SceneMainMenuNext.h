#pragma once
#include "CocosCustomDefines.h"
#include "GameMessaging.h"
#include "GameTimeController.h"

DECLARE_CUSTOM_VIEW_READER(SceneMainMenuNextReader)
DECLARE_COCOS_CUSTOM_VIEW(SceneMainMenuNext), public IGameMessageHandler, public IGameTimeHandler
{
	public:
		DECLARE_CUSTOM_VIEW_PREREQUISITES(SceneMainMenuNext)
		virtual void	HandleMessage(std::string sKey, const MessageData* pMessageData);
		virtual void	Update(uint32_t iDeltaTimeMs);
	protected:
		DECLARE_CUSTOM_VIEW_LIFECYCLE_ONATTACHONDETACH
		DECLARE_CUSTOM_VIEW_LIFECYCLE_ONPOSTATTACH
		DECLARE_CUSTOM_VIEW_LIFECYCLE_ONPREDETTACH
		DECLARE_CUSTOM_VIEW_LIFECYCLE_ONLOADONUNLOAD

		DECLARE_TOUCH_CALLBACK
	private:
		DECLARE_COCOS_UI(cocos2d::ui::Button, m_pButtonPrev);
};