#pragma once
#include "Singleton.h"
#include "cocos2d.h"

static cocos2d::Size designResolutionSize = cocos2d::Size(1280, 800);

namespace CastleBuilder
{
	class SceneManager :public Singleton<SceneManager>
	{
	private:
		cocos2d::Scene* rootSceneNode;
		cocos2d::Scene* currentSceneNode;
		cocos2d::Node* currentDialogSceneNode;
	public:
		SceneManager();
		virtual ~SceneManager() { };
		void init();
		void loadMapScene();
		void showScene(cocos2d::Scene* scene);
		void unloadCurrentScene();
		void pushDialog(cocos2d::Node * dialog);
		void popDialog();
	};
}


