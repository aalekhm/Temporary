#pragma once
#include "cocos2d.h"
#include "ui/UILayout.h"

namespace CastleBuilder
{
	class BottomBarUI :public cocos2d::Node
	{
	public:
		static cocos2d::Node* create(cocos2d::Size size);
		virtual bool init();
	private:
		void setUpBottomBackgroundLayout();
	};
}