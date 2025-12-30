#pragma once
#include "cocos2d.h"
#include "ui/UIWidget.h"
#include "ui/UIImageView.h"
#include "2d/CCLabel.h"

class cocos2d::ui::Widget;
namespace CastleBuilder {
	class GoldView : public cocos2d::ui::Widget
	{
	private:
		cocos2d::ui::ImageView* icon;
		cocos2d::Label* amount;
		void updateContentSize();
	public:
		static GoldView* create();
		virtual bool init();
		void setCoins(int coins);
	};
}