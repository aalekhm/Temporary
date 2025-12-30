#pragma once
#include "CocosCustomDefines.h"
#include "GameMessaging.h"

DECLARE_CUSTOM_VIEW_READER(SceneMainMenuReader)
DECLARE_COCOS_CUSTOM_VIEW(SceneMainMenu), public IGameMessageHandler
{
	public:
		DECLARE_CUSTOM_VIEW_PREREQUISITES(SceneMainMenu)
		virtual void	HandleMessage(std::string sKey, const MessageData* pMessageData);
	protected:
		DECLARE_CUSTOM_VIEW_LIFECYCLE_ONATTACHONDETACH
		DECLARE_CUSTOM_VIEW_LIFECYCLE_ONPOSTATTACH
		DECLARE_CUSTOM_VIEW_LIFECYCLE_ONPREDETTACH
		DECLARE_CUSTOM_VIEW_LIFECYCLE_ONLOADONUNLOAD
	private:
		DECLARE_COCOS_UI(cocos2d::ui::Button,	m_pButtonNext);
};