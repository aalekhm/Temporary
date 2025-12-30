#pragma once

#include "cocos2d.h"
#include "ui/UIWidget.h"

class cocos2d::ui::Widget;
namespace CastleBuilder {
	class FriendsLayout : public cocos2d::ui::Widget
	{
	private:
		cocos2d::Size size;
	public:
		static FriendsLayout* create(const cocos2d::Size &size);
		virtual bool init();
	};
}

