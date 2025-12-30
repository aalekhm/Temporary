#include "SceneMainMenuNext.h"
#include "CocosSceneManager.h"

DEFINE_CUSTOM_VIEW_READER(SceneMainMenuNext, SceneMainMenuNextReader)

DEFINE_CUSTOM_VIEW_LOCATE_TOUCH_CALLBACK(SceneMainMenuNext)

void SceneMainMenuNext::LoadMembers()
{
	GET_CHILD_BY_NAME(this, m_pButtonPrev, cocos2d::ui::Button, "Button_Prev");
}

void SceneMainMenuNext::AttachToScene()
{
}

void SceneMainMenuNext::PostAttachToScene()
{
}

void SceneMainMenuNext::HandleMessage(std::string sKey, const MessageData* pMessageData)
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
}

void SceneMainMenuNext::UnLoadMembers()
{

}
