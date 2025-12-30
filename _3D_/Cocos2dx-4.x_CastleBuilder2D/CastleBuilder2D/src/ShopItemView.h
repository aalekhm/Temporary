#pragma once
#include "cocos2d.h"
#include "ui/UIWidget.h"
#include "2d/CCLabel.h"
#include "GoldView.h"
#include "ui/UIImageView.h"
#include "Model/Model.h"

class cocos2d::ui::Widget;
namespace CastleBuilder {
	class ShopItemView : public cocos2d::ui::Widget
	{
	private:
		cocos2d::ui::ImageView* image;
		cocos2d::Label* title;
		GoldView* goldView;
		shared_ptr<ShopItemData> data;
	public:
		static ShopItemView* create();
		virtual bool init();
		void setData(shared_ptr<ShopItemData> data);
	};
}