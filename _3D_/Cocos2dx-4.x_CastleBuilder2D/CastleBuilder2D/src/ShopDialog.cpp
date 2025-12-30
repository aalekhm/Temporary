#include "ShopDialog.h"
#include "Constants.h"
#include "ui/UIListView.h"
#include "ShopItemView.h"
#include "ui/UIWidget.h"
#include "ui/UILayoutParameter.h"
#include "XmlConfigManager.h"

USING_NS_CC;

namespace CastleBuilder
{
	ShopDialog* CastleBuilder::ShopDialog::create(cocos2d::Size size)
	{
		ShopDialog* dialog = new (std::nothrow) ShopDialog();
		dialog->rootNode = Node::create();
		dialog->rootNode->setContentSize(size);
		if (dialog && dialog->init())
		{
			dialog->autorelease();
			return dialog;
		}
		CC_SAFE_DELETE(dialog);
		return nullptr;
	}

	bool CastleBuilder::ShopDialog::init()
	{
		if (BaseDialog::init())
		{
			Size size = rootNode->getContentSize();
			auto tabSize = size * 0.80;
			auto tabBtnSize = Size(size.width * 0.25, size.height * 0.1);
		
			for (size_t i = 0; i < SHOP_TABS::COUNT; i++)
			{
				tabButtonList[i] = getTabHeaderByType((SHOP_TABS)i, tabBtnSize);
				tabButtonList[i]->setPosition(Vec2(-tabSize.width / 2 + tabButtonList[i]->getContentSize().width * i, size.height / 2));
				rootNode->addChild(tabButtonList[i]);
			}
			listView = getShopLayoutByTabType(SHOP_TABS::WORK_SHOPS, tabSize);
	
			rootNode->addChild(listView);
			selectTab(SHOP_TABS::WORK_SHOPS);
			BaseDialog::swallowTouches();
			return true;
		}
		return false;
	}

	void ShopDialog::selectTab(SHOP_TABS type)
	{
		populateShopByType(type);
		for (size_t i = 0; i < SHOP_TABS::COUNT; i++)
		{
			if (i == type)
			{
				tabButtonList[i]->setHighlighted(true);
			}
			else
			{
				tabButtonList[i]->setHighlighted(false);
			}
		}
	}
	ui::ListView* ShopDialog::getShopLayoutByTabType(SHOP_TABS type, Size tabSize)
	{
		auto shopItemlayout = ui::ListView::create();
		shopItemlayout->setBackGroundImage(TEXTURE_BACKGROUND);
		shopItemlayout->setGravity(ui::ListView::Gravity::TOP);
		shopItemlayout->setBackGroundImageScale9Enabled(true);
		shopItemlayout->setContentSize(tabSize);
		shopItemlayout->setAnchorPoint(ANCHOR_MID_POINT);
		shopItemlayout->setItemsMargin(2);
		shopItemlayout->setPadding(4, 4, 4, 4);
		shopItemlayout->setDirection(ui::ScrollView::Direction::HORIZONTAL);
		shopItemlayout->setSwallowTouches(false);
		return shopItemlayout;
	}
	void ShopDialog::populateShopByType(SHOP_TABS type)
	{
		listView->removeAllChildren();
		std::vector<shared_ptr<ShopItemData>> list;
		if (type == SHOP_TABS::WORK_SHOPS)
		{
			list = XmlConfigManager::Get().getWorkShopItemList();
		}
		else if (type == SHOP_TABS::DECORATIONS)
		{
			list = XmlConfigManager::Get().getDecorationsItemList();
		}
		for each (auto var in list)
		{
			auto shopItemView = ShopItemView::create();
			shopItemView->setData(var);
			listView->pushBackCustomItem(shopItemView);
		}

	}
	cocos2d::ui::Button* ShopDialog::getTabHeaderByType(SHOP_TABS type, cocos2d::Size btnSize)
	{
		auto tabHeader = ui::Button::create( TEXTURE_WHITE, TEXTURE_BACKGROUND);
		tabHeader->setTitleText(getTabTitleByType(type));
		tabHeader->setScale9Enabled(true);
		tabHeader->setContentSize(btnSize);
		tabHeader->setTitleColor(Color3B::BLACK);
		tabHeader->setAnchorPoint(ANCHOR_LEFT_TOP_POINT);
		tabHeader->addClickEventListener([this,type](Ref*) {
			selectTab(type);
			});
		return tabHeader;
	}

	std::string ShopDialog::getTabTitleByType(SHOP_TABS type)
	{
		std::string title = "";
		switch (type)
		{
		case SHOP_TABS::WORK_SHOPS:
			title = STR_SHOP_WORKSHOP;
			break;
		case SHOP_TABS::DECORATIONS:
			title = STR_SHOP_DECORATIONS;
			break;
		default:
			break;
		}
		return title;
	}
}