#include "SceneMainMenu.h"
#include "CocosSceneManager.h"

DEFINE_CUSTOM_VIEW_READER(SceneMainMenu, SceneMainMenuReader)

DEFINE_CUSTOM_VIEW_LOCATE_TOUCH_CALLBACK(SceneMainMenu)

void SceneMainMenu::LoadMembers()
{
	GET_CHILD_BY_NAME(this, m_pButtonNext, cocos2d::ui::Button, "Button_Next");
}

void SceneMainMenu::AttachToScene()
{
	GameMessaging::Get().AddListener("ListenEm", this);
}

void SceneMainMenu::PostAttachToScene()
{
	CREATE_MESSAGE_AND_ADD_MESSAGEDATAPART(messageData, StringMessageData, "AnimName", "animation_Battles_In");
	MESSAGE_POST("RunBGAnimation", messageData);
}

void SceneMainMenu::HandleMessage(std::string sKey, const MessageData* pMessageData)
{
	if(sKey == "ListenEm")
	{
		bool b = true;
	}
}

void SceneMainMenu::OnTouch(cocos2d::Ref* pObject, cocos2d::ui::Widget::TouchEventType type)
{
	if (type == cocos2d::ui::Widget::TouchEventType::ENDED)
	{
		if (pObject == m_pButtonNext)
		{
			CocosSceneManager& pCocosSceneManager = CocosSceneManager::Get();
			pCocosSceneManager.PushView("MainMenuNext");
		}
	}
}

void SceneMainMenu::PreDetachFromScene()
{
	CALL_DETACH_ANIMATION_DONE
}

void SceneMainMenu::DetachFromScene()
{
	CREATE_MESSAGE_AND_ADD_MESSAGEDATAPART(messageData, StringMessageData, "AnimName", "animation_Battles_Out");
	MESSAGE_SEND("RunBGAnimation", messageData);

	GameMessaging::Get().RemoveListener("ListenEm", this);
}

void SceneMainMenu::UnLoadMembers()
{

}
