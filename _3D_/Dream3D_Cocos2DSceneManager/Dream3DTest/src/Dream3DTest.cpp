#include "Dream3DTest.h"
#include <cocos2d.h>
#include "math/Vec2.h"
#include "ActionTimeline/CSLoader.h"
#include <CocosSceneManager.h>
#include "CocosSceneUtils.h"
#include "GameMessaging.h"

#define ASSETS_PATH			"assets"
#define COCOS2D_PATH		"assets/Cocos2d"
#define SCENES_PATH			"assets/Cocos2d/Common"
#define RESOURCE_PATH		"assets/res"
#define DATA_PATH			"assets/data"
#define JSON_COCOSVIEWS		"CocosViews.json"

Dream3DTest engine;

IMessageDataWrapper(CocosVec, cocos2d::Vec2)
IMessageDataWrapper(CocosSize, cocos2d::Size)

std::map<std::string, std::unique_ptr<MessageData>> MessageDataMap;

Dream3DTest::Dream3DTest()
{
}

Dream3DTest::~Dream3DTest()
{
}

void Dream3DTest::onSetViewport(int& iWidth, int& iHeight)
{
	iWidth = 640;
	iHeight = 960;
}

void Dream3DTest::onInit()
{
	initMembers();
	initScene();

	{
		CREATE_MESSAGE_AND_ADD_MESSAGEDATAPART(messageData, CocosVec, "CocosVector", cocos2d::Vec2(10, 10));
		ADDITIONAL_MESSAGEDATAPART(messageData, CocosSize, "CocosSize", cocos2d::Size(50, 50));

		//MESSAGE_SEND("ListenEm", messageData);
		MESSAGE_POST("ListenEm", messageData);
	}
}

void Dream3DTest::initMembers()
{
	// Get CWD
	{
		char sDir[255];
		GetCurrentDirectory(255, sDir);
		m_sCurrentWorkingDirectory = sDir;
		m_sCurrentWorkingDirectory.append("/");
	}

	CocosSceneManager& pCocosSceneManager = CocosSceneManager::Get();
	{
		pCocosSceneManager.Initialize(EngineManager::getInstance()->getWidth(), EngineManager::getInstance()->getHeight());

		pCocosSceneManager.AddSearchPath(std::string(m_sCurrentWorkingDirectory).append(DATA_PATH));
		pCocosSceneManager.AddSearchPath(std::string(m_sCurrentWorkingDirectory).append(SCENES_PATH));
		pCocosSceneManager.AddSearchPath(std::string(m_sCurrentWorkingDirectory).append(COCOS2D_PATH));
		pCocosSceneManager.AddSearchPath(std::string(m_sCurrentWorkingDirectory).append(RESOURCE_PATH));
		pCocosSceneManager.LoadCocosViewsJsonInfo(JSON_COCOSVIEWS);
	}
}

void Dream3DTest::initScene()
{
	CocosSceneManager& pCocosSceneManager = CocosSceneManager::Get();
	cocos2d::Scene* pRootSceneNode = pCocosSceneManager.GetRootScene();

	cocos2d::ui::ImageView* pPomogranate = CocosSceneUtils::CreateImageNode(pRootSceneNode, "Pomogrenate.png", cocos2d::ui::Widget::TextureResType::LOCAL, cocos2d::Vec2(0.5f, 0.5f), cocos2d::Vec2(EngineManager::getInstance()->getWidth() * 0.5f, EngineManager::getInstance()->getHeight() * 0.9f), 255.0f);
	pPomogranate->setScale(0.5f);
	
	pCocosSceneManager.OpenViewExplicitly("Background");
	pCocosSceneManager.PushView("MainMenu");
}

void Dream3DTest::onNodeLoaded(cocos2d::Ref* pObject)
{

}

void Dream3DTest::update(float elapsedTime)
{
	CocosSceneManager::Get().Update();
	GameMessaging().Get().ProcessQueue();
}

void Dream3DTest::render(float elapsedTime)
{
}
	 
void Dream3DTest::keyPressedEx(unsigned int iVirtualKeycode, unsigned short ch)
{
	CocosSceneManager::Get().OnKeyDown(nullptr, ch);
}

void Dream3DTest::keyReleasedEx(unsigned int iVirtualKeycode, unsigned short ch)
{
	CocosSceneManager::Get().OnKeyUp(nullptr, ch);
}
	 
void Dream3DTest::onMouseDownEx(int mCode, int x, int y)
{
	CocosSceneManager::Get().OnMouseDown(mCode, x, y);
}

void Dream3DTest::onMouseMoveEx(int mCode, int x, int y)
{
	CocosSceneManager::Get().OnMouseMove(mCode, x, y);
}

void Dream3DTest::onMouseUpEx(int mCode, int x, int y)
{
	CocosSceneManager::Get().OnMouseUp(mCode, x, y);
}

void Dream3DTest::onMouseWheelEx(WPARAM wParam, LPARAM lParam)
{

}

bool Dream3DTest::applicationDidFinishLaunching()
{
	return true;
}

void Dream3DTest::applicationDidEnterBackground()
{

}

void Dream3DTest::applicationWillEnterForeground()
{

}
