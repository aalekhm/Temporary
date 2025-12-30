#include "SceneManager.h"
#include "MapScene.h"
#include "Constants.h"
#include "XmlConfigManager.h"

USING_NS_CC;

namespace CastleBuilder
{
	SceneManager::SceneManager():rootSceneNode(nullptr)
		,currentSceneNode(nullptr)
		,currentDialogSceneNode(nullptr)
	{
	}
	void CastleBuilder::SceneManager::init()
	{
		auto director = Director::getInstance();
		auto glview = director->getOpenGLView();
		//init configs so that we can read resolution from config
		XmlConfigManager::Get().init();
		// Set the design resolution
		Size resolution = XmlConfigManager::Get().getResolution();
		if (!glview) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
			glview = GLViewImpl::createWithRect("CastleBuilder", cocos2d::Rect(0, 0, resolution.width, resolution.height));
#else
			glview = GLViewImpl::create("CastleBuilder");
#endif
			director->setOpenGLView(glview);
		}
		glview->setDesignResolutionSize(designResolutionSize.width, designResolutionSize.height, ResolutionPolicy::NO_BORDER);
		auto frameSize = glview->getFrameSize();
		//director->setContentScaleFactor(min(resolution.height / designResolutionSize.height, resolution.width / designResolutionSize.width));

		// turn on display FPS
		director->setDisplayStats(true);

		// set FPS. the default value is 1.0/60 if you don't call this
		director->setAnimationInterval(1.0f / 60);

		//add search path
		cocos2d::FileUtils* pCCFileUtils = cocos2d::FileUtils::getInstance();
		pCCFileUtils->addSearchPath(ASSET_IMG_PATH);
		pCCFileUtils->addSearchPath(ASSET_IMG_DECORATION_PATH);
		pCCFileUtils->addSearchPath(ASSET_IMG_WORKSHOP_PATH);

		auto pScene = cocos2d::Scene::create();
		rootSceneNode = pScene;
		director->runWithScene(rootSceneNode);
		loadMapScene();
	}
	void SceneManager::loadMapScene()
	{
		auto scene = CastleBuilder::MapScene::createScene();
		showScene(scene);
	}
	void SceneManager::showScene(cocos2d::Scene* scene)
	{
		unloadCurrentScene();
		currentSceneNode = scene;
		rootSceneNode->addChild(currentSceneNode, SCENE_ZORDER_DEFAULT);
	}

	void SceneManager::unloadCurrentScene()
	{
		if (currentSceneNode != nullptr)
		{
			currentSceneNode->removeFromParent();
			currentSceneNode = nullptr;
		}
	}

	void SceneManager::pushDialog(cocos2d::Node * dialog)
	{
		popDialog();
		currentDialogSceneNode = dialog;
		rootSceneNode->addChild(currentDialogSceneNode, SCENE_ZORDER_DIALOG);
	}

	void SceneManager::popDialog()
	{
		if (currentDialogSceneNode != nullptr)
		{
			currentDialogSceneNode->removeFromParent();
			currentDialogSceneNode = nullptr;
		}
	}
}
