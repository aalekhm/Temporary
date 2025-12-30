#include "CSBSceneMainMenu.h"
#include "CocosSceneManager.h"

DEFINE_CUSTOM_VIEW_READER_CSB(CSBSceneMainMenu, CSBSceneMainMenuReader)

DEFINE_CUSTOM_VIEW_LOCATE_TOUCH_CALLBACK(CSBSceneMainMenu)

void CSBSceneMainMenu::LoadMembers()
{
	GET_CHILD_BY_NAME(this, m_pButtonNext, cocos2d::ui::Button, "Button_Next");
}

void CSBSceneMainMenu::AttachToScene()
{
	REGISTER_INGAME_TIME
	REGISTER_MESSAGE_HANDLER(this, "ListenEm")
}

void CSBSceneMainMenu::PostAttachToScene()
{
	CREATE_MESSAGE_AND_ADD_MESSAGEDATAPART(messageData, StringMessageData, "AnimName", "animation_Battles_In");
	MESSAGE_POST("RunBGAnimation", messageData);
}

void CSBSceneMainMenu::HandleMessage(std::string sKey, const MessageData* pMessageData)
{
}

void CSBSceneMainMenu::Update(uint32_t iDeltaTimeMs)
{

}

void CSBSceneMainMenu::OnTouch(cocos2d::Ref* pObject, cocos2d::ui::Widget::TouchEventType type)
{
	if (type == cocos2d::ui::Widget::TouchEventType::ENDED)
	{
		if (pObject == m_pButtonNext)
		{
			CocosSceneManager& pCocosSceneManager = CocosSceneManager::Get();
			pCocosSceneManager.PushView("CSBSceneMainMenuNext");
		}
	}
}

void CSBSceneMainMenu::PreDetachFromScene()
{
	CALL_DETACH_ANIMATION_DONE
}

void CSBSceneMainMenu::DetachFromScene()
{
	UNREGISTER_INGAME_TIME
	UNREGISTER_MESSAGE_HANDLER(this, "ListenEm")

	CREATE_MESSAGE_AND_ADD_MESSAGEDATAPART(messageData, StringMessageData, "AnimName", "animation_Battles_Out");
	MESSAGE_SEND("RunBGAnimation", messageData);
}

void CSBSceneMainMenu::UnLoadMembers()
{

}
