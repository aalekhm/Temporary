#pragma once
#include "2d/CCScene.h"

class ICocosReader
{
	public:
		virtual ~ICocosReader() {}
		virtual cocos2d::Scene* createWithFilename(std::string& sSceneCreatorFile) = 0;
};