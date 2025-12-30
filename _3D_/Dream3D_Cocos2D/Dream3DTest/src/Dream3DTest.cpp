#include "Dream3DTest.h"
#include "ui/UIImageView.h"
#include "ui/UIWidget.h"
#include "math/Vec2.h"
#include "ActionTimeline/CSLoader.h"

#define ASSETS_PATH			"assets"
#define COCOS2D_PATH		"assets/Cocos2d"
#define SCENES_PATH			"assets/Cocos2d/Common"
#define RESOURCE_PATH		"assets/res"

Dream3DTest engine;

Dream3DTest::Dream3DTest()
{
}

Dream3DTest::~Dream3DTest()
{
}

void Dream3DTest::initialize()
{
	initMembers();
	initCocos2d();
	initScene();
}

void Dream3DTest::initMembers()
{
	char sDir[255];
	GetCurrentDirectory(255, sDir);
	m_sCurrentWorkingDirectory = sDir;
	m_sCurrentWorkingDirectory.append("/");
}

void Dream3DTest::initCocos2d()
{
	auto pScene = cocos2d::Scene::create();
	assert(pScene != NULL);

	m_pRootSceneNode = pScene;
	if (m_pRootSceneNode != NULL)
	{
		cocos2d::Application::sharedApplication();
		cocos2d::CCDirector* pDirector = cocos2d::CCDirector::getInstance();
		cocos2d::FileUtils* pCCFileUtils = cocos2d::FileUtils::getInstance();

		cocos2d::GLView* view = cocos2d::GLViewImpl::create("cocos2dx");

		cocos2d::Size designResolution(EngineManager::getInstance()->getWidth(), EngineManager::getInstance()->getHeight());
		cocos2d::Size studioResolution(COCOSSTUDIO_DESIGN_RESOLUTION_WIDTH, COCOSSTUDIO_DESIGN_RESOLUTION_HEIGHT);

		pDirector->setOpenGLView(view);
		pDirector->setCocosStudioDesignResolution(studioResolution);
		pDirector->setAnimationInterval(1.0 / 60);
		pDirector->setContentScaleFactor(1.0f);
		pDirector->setDPI(72);
		pDirector->setClearColor(cocos2d::Color4F(0.0f, 0.0f, 0.0f, 1.0f));

		view->setFrameSize((float)designResolution.width, (float)designResolution.height);
		view->setDesignResolutionSize((float)designResolution.width, (float)designResolution.height, ResolutionPolicy::NO_BORDER);

		pCCFileUtils->addSearchPath(std::string(m_sCurrentWorkingDirectory).append(SCENES_PATH));
		pCCFileUtils->addSearchPath(std::string(m_sCurrentWorkingDirectory).append(COCOS2D_PATH));
		pCCFileUtils->addSearchPath(std::string(m_sCurrentWorkingDirectory).append(RESOURCE_PATH));

		pDirector->runWithScene(m_pRootSceneNode);
	}
}

void Dream3DTest::initScene()
{
	cocos2d::ui::ImageView* pImageViewCenter = cocos2d::ui::ImageView::create();
	{
		pImageViewCenter->loadTexture("Pomogrenate.png", cocos2d::ui::Widget::TextureResType::LOCAL);
		pImageViewCenter->setAnchorPoint(cocos2d::Vec2(0.5f, 0.5f));
		pImageViewCenter->setPosition(cocos2d::Vec2(100.f, 100.0f));
		pImageViewCenter->setOpacity(255.0f);
		pImageViewCenter->setScale(0.5f);
	}

	cocos2d::Node* pGenericPopup = cocos2d::CSLoader::createNode("Scene_Generic_PopUp.csb", CC_CALLBACK_1(Dream3DTest::onNodeLoaded, this));

	m_pRootSceneNode->addChild(pGenericPopup);
	m_pRootSceneNode->addChild(pImageViewCenter);
}

void Dream3DTest::onNodeLoaded(cocos2d::Ref* pObject)
{

}

void Dream3DTest::update(float elapsedTime)
{

}

void Dream3DTest::render(float elapsedTime)
{
	if (cocos2d::Director::getInstance() != NULL)
		cocos2d::Director::getInstance()->mainLoop();
}
	 
void Dream3DTest::keyPressedEx(unsigned int iVirtualKeycode, unsigned short ch)
{

}

void Dream3DTest::keyReleasedEx(unsigned int iVirtualKeycode, unsigned short ch)
{

}
	 
void Dream3DTest::onMouseDownEx(int mCode, int x, int y)
{
	DispatchMouseEventToCocos(mCode, x, y, cocos2d::EventTouch::EventCode::BEGAN);
}

void Dream3DTest::onMouseMoveEx(int mCode, int x, int y)
{
	DispatchMouseEventToCocos(mCode, x, y, cocos2d::EventTouch::EventCode::MOVED);
}

void Dream3DTest::onMouseUpEx(int mCode, int x, int y)
{
	DispatchMouseEventToCocos(mCode, x, y, cocos2d::EventTouch::EventCode::ENDED);
}

void Dream3DTest::onMouseWheelEx(WPARAM wParam, LPARAM lParam)
{

}

void Dream3DTest::DispatchMouseEventToCocos(int mCode, int x, int y, cocos2d::EventTouch::EventCode eEventCode)
{
	cocos2d::EventDispatcher* pDelegate = cocos2d::Director::sharedDirector()->getEventDispatcher();

	cocos2d::Touch pTouch;
	pTouch.setTouchInfo(mCode, x, y);

	cocos2d::EventTouch touchEvent;

	std::vector<cocos2d::Touch*> touches;
	touches.push_back(&pTouch);

	touchEvent.setTouches(touches);
	touchEvent.setEventCode(eEventCode);
	pDelegate->dispatchEvent(&touchEvent);
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

//void main()
//{
//	Dream3DTest engine;
//}