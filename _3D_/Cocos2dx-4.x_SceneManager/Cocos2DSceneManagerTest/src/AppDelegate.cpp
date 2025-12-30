/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "AppDelegate.h"
#include "CocosSceneManager.h"
#include "CocosSceneUtils.h"
#include "CreatorReader.h"
#include "GameMessaging.h"

#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		960

#define DATA_PATH			"/Resources/data"
#define CREATOR_ROOT		"/Resources/creator/"
#define SCENES_PATH			"/Resources/creator/Scenes/"
#define SPRITESHEET_COMMON	"/Resources/creator/Common/"
#define COMMON_PLIST		"Common.plist"

#define JSON_COCOSVIEWS		"CocosViews.json"

#define SCENE(__creatorSceneFile__) __creatorSceneFile__

USING_NS_CC;
AppDelegate::AppDelegate()
{
}

AppDelegate::~AppDelegate() 
{
	CocosSceneManager::Get().Release();
}

bool AppDelegate::applicationDidFinishLaunching()
{
	// Get CWD
	std::string sCurrentWorkingDirectory;
	{
		char sDir[255];
		GetCurrentDirectory(255, sDir);
		sCurrentWorkingDirectory = sDir;
		sCurrentWorkingDirectory.append("/");
	}

	CocosSceneManager& pCocosSceneManager = CocosSceneManager::Get();
	{
		pCocosSceneManager.Initialize(SCREEN_WIDTH, SCREEN_HEIGHT);

		pCocosSceneManager.AddSearchPath(std::string(sCurrentWorkingDirectory).append(DATA_PATH));
		pCocosSceneManager.AddSearchPath(std::string(sCurrentWorkingDirectory).append(CREATOR_ROOT));
		pCocosSceneManager.AddSearchPath(std::string(sCurrentWorkingDirectory).append(SCENES_PATH));
		pCocosSceneManager.AddSearchPath(std::string(sCurrentWorkingDirectory).append(SPRITESHEET_COMMON));
		
		pCocosSceneManager.LoadCocosViewsJsonInfo(JSON_COCOSVIEWS);

		SpriteFrameCache::getInstance()->addSpriteFramesWithFile(COMMON_PLIST);
	}

	initScene();

    return true;
}

void AppDelegate::initScene()
{
	CocosSceneManager& pCocosSceneManager = CocosSceneManager::Get();
	{
		pCocosSceneManager.SetBackgroundView("Background");
		pCocosSceneManager.PushView("MainMenu");
	}

	cocos2d::Scene* pRootSceneNode = pCocosSceneManager.GetRootScene();
	{
		cocos2d::ui::ImageView* pPomogranate = CocosSceneUtils::CreateImageNode(pRootSceneNode, "Pomogrenate.png", cocos2d::ui::Widget::TextureResType::LOCAL, cocos2d::Vec2(0.5f, 0.5f), cocos2d::Vec2(SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.9f), 255.0f);
		pPomogranate->setScale(0.5f);
		pPomogranate->setLocalZOrder(10);
	}
}

void AppDelegate::onUpdate()
{
	CocosSceneManager::Get().Update();
	GameMessaging().Get().ProcessQueue();
}

void AppDelegate::applicationDidEnterBackground() 
{
    Director::getInstance()->stopAnimation();
}

void AppDelegate::applicationWillEnterForeground() 
{
    Director::getInstance()->startAnimation();
}
