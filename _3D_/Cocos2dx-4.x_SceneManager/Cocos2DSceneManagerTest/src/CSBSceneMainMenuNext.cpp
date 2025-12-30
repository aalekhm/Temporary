#include "CSBSceneMainMenuNext.h"
#include "CocosSceneManager.h"

DEFINE_CUSTOM_VIEW_READER_CSB(CSBSceneMainMenuNext, CSBSceneMainMenuNextReader)
DEFINE_CUSTOM_VIEW_LOCATE_TOUCH_CALLBACK(CSBSceneMainMenuNext)

void CSBSceneMainMenuNext::LoadMembers()
{
	GET_CHILD_BY_NAME(this, m_pButtonPrev, cocos2d::ui::Button, "Button_Prev");
}

void CSBSceneMainMenuNext::AttachToScene()
{
	REGISTER_INGAME_TIME
}

void CSBSceneMainMenuNext::PostAttachToScene()
{
}

void CSBSceneMainMenuNext::HandleMessage(std::string sKey, const MessageData* pMessageData)
{
}

void CSBSceneMainMenuNext::Update(uint32_t iDeltaTimeMs)
{

}

void CSBSceneMainMenuNext::OnTouch(cocos2d::Ref* pObject, cocos2d::ui::Widget::TouchEventType type)
{
	if (type == cocos2d::ui::Widget::TouchEventType::ENDED)
	{
		if (pObject == m_pButtonPrev)
		{
			CocosSceneManager::Get().PopView();
		}
	}
}

void CSBSceneMainMenuNext::PreDetachFromScene()
{
	CALL_DETACH_ANIMATION_DONE
}

void CSBSceneMainMenuNext::DetachFromScene()
{
	UNREGISTER_INGAME_TIME
}

void CSBSceneMainMenuNext::UnLoadMembers()
{

}
