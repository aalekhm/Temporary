#pragma once
#include "CocosCustomDefines.h"
#include "GameMessaging.h"

DECLARE_CUSTOM_VIEW_READER(SceneBGReader)
DECLARE_COCOS_CUSTOM_VIEW(SceneBG), public IGameMessageHandler
{
	public:
		DECLARE_CUSTOM_VIEW_PREREQUISITES(SceneBG)
		virtual void	HandleMessage(std::string sKey, const MessageData* pMessageData);
	protected:
		DECLARE_CUSTOM_VIEW_LIFECYCLE_ONATTACHONDETACH
		DECLARE_CUSTOM_VIEW_LIFECYCLE_ONPOSTATTACH
		DECLARE_CUSTOM_VIEW_LIFECYCLE_ONPREDETTACH
		DECLARE_CUSTOM_VIEW_LIFECYCLE_ONLOADONUNLOAD

		DECLARE_TOUCH_CALLBACK
	private:
		void			OnAnimationStart(std::string sAnimName);
		void			OnAnimationEnd(std::string sAnimName);

		DECLARE_COCOS_UI(creator::AnimationManager,		m_pAnimationManager);
};