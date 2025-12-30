#include "GoldView.h"
#include "ui/UIWidget.h"
#include "base/CCRef.h"
#include "ui/UIImageView.h"
#include "Constants.h"
USING_NS_CC;

namespace CastleBuilder {

	void GoldView::updateContentSize()
	{
		Size size = Size(icon->getContentSize().width + amount->getContentSize().width, icon->getContentSize().height);
		setContentSize(size);
	}

	GoldView* GoldView::create()
	{
		GoldView* widget = new (std::nothrow) GoldView();
		if (widget && widget->init())
		{
			widget->autorelease();
			return widget;
		}
		CC_SAFE_DELETE(widget);
		return nullptr;
	}

	bool GoldView::init()
	{
		//gold icon
		icon = ui::ImageView::create(TEXTURE_GOLD);
		icon->setAnchorPoint(ANCHOR_LEFT_BOTTOM_POINT);

		//gold amount text
		amount = Label::createWithTTF("10000", FONT_MARKER_FELT, FONT_SIZE_SMALL);
		amount->setTextColor(Color4B::BLACK);
		amount->setAnchorPoint(ANCHOR_LEFT_MID_POINT);
		amount->setPosition(icon->getContentSize().width, icon->getContentSize().height / 2);
		icon->addChild(amount);

		this->addChild(icon);
		updateContentSize();
		return true;
	}

	void GoldView::setCoins(int coins)
	{
		amount->setString(std::to_string(coins));
	}
	
}
