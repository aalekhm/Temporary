#include "SceneBG.h"
#include "CocosSceneManager.h"

DEFINE_CUSTOM_VIEW_READER("Background", SceneBG, SceneBGReader)
DEFINE_TOUCH_CALLBACK(SceneBG)

void SceneBG::LoadMembers()
{
	GET_CHILD_BY_NAME(this, m_pAnimationManager,	creator::AnimationManager,		"creator_animation_manager");
}

void SceneBG::AttachToScene()
{
	GameMessaging::Get().AddListener("RunBGAnimation", this);
}

void SceneBG::PostAttachToScene()
{

}

void SceneBG::HandleMessage(std::string sKey, const MessageData* pMessageData)
{
	if (sKey == "RunBGAnimation")
	{
		std::string sAnimName;
		GET_MESSAGEDATAPART(pMessageData, StringMessageData, "AnimName", sAnimName);
		if (!sAnimName.empty())
		{
			CocosView* pCocosView = ((ICocosViewLifeCycle*)this)->GetCocosView();
			CALL_COCOS_ANIMATION_BY_ACTION(pCocosView->GetViewNode(), pCocosView->GetAnimationManager(), sAnimName, SceneBG::OnAnimationStart, SceneBG::OnAnimationEnd, this, true, 0);
		}
	}
}

void SceneBG::OnAnimationStart(std::string sAnimName)
{

}

void SceneBG::OnAnimationEnd(std::string sAnimName)
{
}

void SceneBG::OnTouch(cocos2d::Ref* pObject, cocos2d::ui::Widget::TouchEventType type)
{
	if (type == cocos2d::ui::Widget::TouchEventType::ENDED)
	{
	}
}

void SceneBG::PreDetachFromScene()
{
	CALL_DETACH_ANIMATION_DONE
}

void SceneBG::DetachFromScene()
{
	GameMessaging::Get().RemoveListener("RunBGAnimation", this);
}

void SceneBG::UnLoadMembers()
{

}
