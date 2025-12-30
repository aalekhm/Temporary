#include "ShopItemView.h"
#include "ui/UIWidget.h"
#include "base/CCRef.h"
#include "Constants.h"
#include "ui/UIButton.h"
#include "SceneManager.h"
#include <memory>
#include "Model/Model.h"

USING_NS_CC;

namespace CastleBuilder {

	ShopItemView* ShopItemView::create()
	{
		ShopItemView* widget = new (std::nothrow) ShopItemView();
		if (widget && widget->init())
		{
			widget->autorelease();
			return widget;
		}
		CC_SAFE_DELETE(widget);
		return nullptr;
	}

	bool ShopItemView::init()
	{
		this->data = nullptr;
		//background
		auto background = ui::Button::create(TEXTURE_BACKGROUND);
		background->setAnchorPoint(ANCHOR_MID_POINT);
		background->setScale9Enabled(true);
		background->addClickEventListener([=](Ref*) {
			Size size = Director::getInstance()->getVisibleSize();
			EventCustom event(EVT_SHOP_ITEM_SELECTED);
			event.setUserData(static_cast<void*>(data.get()));
			background->getEventDispatcher()->dispatchEvent(&event);
			SceneManager::Get().popDialog();
			});
		//item title
		title = Label::createWithTTF("title", FONT_ARIAL, FONT_SIZE_SMALL);
		title->setTextColor(Color4B::BLACK);
		title->setAnchorPoint(ANCHOR_MID_TOP_POINT);

		//item image
		image = ui::ImageView::create(TEXTURE_BRIDGE);
		image->setContentSize(Size(108,108));
		image->setAnchorPoint(ANCHOR_MID_POINT);
		image->ignoreContentAdaptWithSize(false);

		//gold view
		goldView = GoldView::create();
		goldView->setAnchorPoint(ANCHOR_LEFT_BOTTOM_POINT);

		background->addChild(title);
		background->addChild(image);
		background->addChild(goldView);

		//position it

		background->setContentSize(Size(image->getContentSize().width, 
			image->getContentSize().height + title->getContentSize().height + goldView->getContentSize().height));
		this->setContentSize(background->getContentSize());


		image->setPosition(background->getContentSize() / 2);
		title->setPosition(background->getContentSize().width / 2,
			background->getContentSize().height * 0.9);

		this->addChild(background);
		background->setPosition(background->getContentSize()/2);
		//position the entire widget in the center
		return true;
	}

	void ShopItemView::setData(shared_ptr<ShopItemData> data)
	{
		this->data = data;
		image->loadTexture(data->image);
		title->setString(data->title);
		goldView->setCoins(data->coins);
	}
}
