#pragma once
#include "CocosCustomDefines.h"
#include "GameMessaging.h"

DECLARE_CUSTOM_VIEW_READER_CSB(CSBSceneBGReader)
DECLARE_COCOS_CUSTOM_VIEW_CSB(CSBSceneBG), public IGameMessageHandler
{
	public:
		DECLARE_CUSTOM_VIEW_PREREQUISITES_CSB(CSBSceneBG)
		virtual void	HandleMessage(std::string sKey, const MessageData* pMessageData);
	protected:
		DECLARE_CUSTOM_VIEW_LIFECYCLE_ONATTACHONDETACH
		DECLARE_CUSTOM_VIEW_LIFECYCLE_ONPOSTATTACH
		DECLARE_CUSTOM_VIEW_LIFECYCLE_ONPREDETTACH
		DECLARE_CUSTOM_VIEW_LIFECYCLE_ONLOADONUNLOAD
	private:
		void			OnAnimationStart(std::string sAnimName);
		void			OnAnimationEnd(std::string sAnimName);
};