#include "SceneMainMenuNext.h"
#include "CocosSceneManager.h"
#include "CocosCustomDefines.h"

DEFINE_CUSTOM_VIEW_READER("MainMenuNext", SceneMainMenuNext, SceneMainMenuNextReader)
DEFINE_TOUCH_CALLBACK(SceneMainMenuNext)

void SceneMainMenuNext::LoadMembers()
{
	GET_CHILD_BY_NAME(this, m_pButtonPrev, cocos2d::ui::Button, "Button_Prev");
	{
		GLUE_BTN_EVENT_WITH_TOUCH_CALLBACK(SceneMainMenuNext, m_pButtonPrev, this);
		m_pButtonPrev->setScale9Enabled(true);
	}
}

void SceneMainMenuNext::AttachToScene()
{
	REGISTER_INGAME_TIME
}

void SceneMainMenuNext::PostAttachToScene()
{
}

void SceneMainMenuNext::HandleMessage(std::string sKey, const MessageData* pMessageData)
{
}

void SceneMainMenuNext::Update(uint32_t iDeltaTimeMs)
{

}

void SceneMainMenuNext::OnTouch(cocos2d::Ref* pObject, cocos2d::ui::Widget::TouchEventType type)
{
	if (type == cocos2d::ui::Widget::TouchEventType::ENDED)
	{
		if (pObject == m_pButtonPrev)
		{
			CocosSceneManager::Get().PopView();
		}
	}
}

void SceneMainMenuNext::PreDetachFromScene()
{
	CALL_DETACH_ANIMATION_DONE
}

void SceneMainMenuNext::DetachFromScene()
{
	UNREGISTER_INGAME_TIME
	UNREGISTER_MESSAGE_HANDLER(this, "ListenEm")
}

void SceneMainMenuNext::UnLoadMembers()
{
}
