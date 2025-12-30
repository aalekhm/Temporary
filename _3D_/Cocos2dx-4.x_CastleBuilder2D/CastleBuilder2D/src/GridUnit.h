#pragma once
#include "cocos2d.h"
#include "ui/UIImageView.h"

namespace CastleBuilder
{
	class GridUnit :public cocos2d::Node
	{
	private:
		cocos2d::ui::ImageView* highlight;
	public:
		static GridUnit* create(cocos2d::Size size);
		virtual bool init();
		void highlightGridUint(bool gridUnitAvailable);
	};
}

