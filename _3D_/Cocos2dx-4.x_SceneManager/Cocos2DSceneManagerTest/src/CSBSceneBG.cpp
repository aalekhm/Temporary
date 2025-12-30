#include "CSBSceneBG.h"
#include "CocosSceneManager.h"

DEFINE_CUSTOM_VIEW_READER_CSB(CSBSceneBG, CSBSceneBGReader)
DEFINE_CUSTOM_VIEW_LOCATE_TOUCH_CALLBACK(CSBSceneBG)

void CSBSceneBG::LoadMembers()
{
}

void CSBSceneBG::AttachToScene()
{
	REGISTER_MESSAGE_HANDLER(this, "RunBGAnimation")
}

void CSBSceneBG::PostAttachToScene()
{

}

void CSBSceneBG::HandleMessage(std::string sKey, const MessageData* pMessageData)
{
	if (sKey == "RunBGAnimation")
	{
		std::string sAnimName;
		GET_MESSAGEDATAPART(pMessageData, StringMessageData, "AnimName", sAnimName);
		if (!sAnimName.empty())
		{
			CocosView* pCocosView = ((ICocosViewLifeCycle*)this)->GetCocosView();
			CALL_COCOS_ANIMATION_BY_ACTION_CSB(pCocosView->GetViewNode(), pCocosView->GetActionTimeline(), sAnimName, CSBSceneBG::OnAnimationStart, CSBSceneBG::OnAnimationEnd, this, false, 0);
		}
	}
}

void CSBSceneBG::OnAnimationStart(std::string sAnimName)
{

}

void CSBSceneBG::OnAnimationEnd(std::string sAnimName)
{
}

void CSBSceneBG::OnTouch(cocos2d::Ref* pObject, cocos2d::ui::Widget::TouchEventType type)
{
	if (type == cocos2d::ui::Widget::TouchEventType::ENDED)
	{
	}
}

void CSBSceneBG::PreDetachFromScene()
{
	CALL_DETACH_ANIMATION_DONE
}

void CSBSceneBG::DetachFromScene()
{
	UNREGISTER_MESSAGE_HANDLER(this, "RunBGAnimation")
}

void CSBSceneBG::UnLoadMembers()
{

}