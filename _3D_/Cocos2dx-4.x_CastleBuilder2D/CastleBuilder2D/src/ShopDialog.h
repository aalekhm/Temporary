#pragma once
#include "cocos2d.h"
#include "BaseDialog.h"
#include "Constants.h"
#include "ui/UIListView.h"
#include "ui/UITabControl.h"
#include <string.h>
#include "ui/UIButton.h"
#include "ui/UIListView.h"
#include <algorithm>

namespace CastleBuilder
{
	class ShopDialog : public BaseDialog
	{
	public:
		static ShopDialog* create(cocos2d::Size size);
		virtual bool init();
	private:
		cocos2d::ui::Button* tabButtonList[SHOP_TABS::COUNT];
		cocos2d::ui::ListView* listView;

		void selectTab(SHOP_TABS type);
		cocos2d::ui::ListView* getShopLayoutByTabType(SHOP_TABS type, cocos2d::Size tabSize);
		void populateShopByType(SHOP_TABS type);
		cocos2d::ui::Button* getTabHeaderByType(SHOP_TABS type, cocos2d::Size btnSize);
		std::string getTabTitleByType(SHOP_TABS type);
	};
}


