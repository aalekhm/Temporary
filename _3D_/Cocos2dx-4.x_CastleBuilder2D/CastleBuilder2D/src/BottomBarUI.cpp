#include "BottomBarUI.h"
#include "Constants.h"
#include "ui/UIListView.h"
#include "ui/UIScrollView.h"
#include "ui/UIImageView.h"
#include "ui/UIButton.h"
#include "FriendsLayout.h"
#include "GoldView.h"
#include "SceneManager.h"
#include "ShopDialog.h"

USING_NS_CC;

namespace CastleBuilder
{
	cocos2d::Node* BottomBarUI::create(cocos2d::Size size)
	{
		BottomBarUI* node = new (std::nothrow) BottomBarUI();
		node->setContentSize(size);
		if (node && node->init())
		{
			node->autorelease();
			return node;
		}
		CC_SAFE_DELETE(node);
		return nullptr;
	}
	bool BottomBarUI::init()
	{
		setUpBottomBackgroundLayout();
		return true;
	}
	void BottomBarUI::setUpBottomBackgroundLayout()
	{
		Size size = Director::getInstance()->getVisibleSize();
		const Size bottomBar = this->getContentSize();
		SpriteFrame* spriteFrame = SpriteFrame::create(TEXTURE_WHITE,
		Rect(0.0f, 0.0f, bottomBar.width, bottomBar.height));

		this->setColor(Color3B(0,0,0));
		//this->setContentSize(bottomBar);
		this->setAnchorPoint(ANCHOR_LEFT_BOTTOM_POINT);

		//bottom bar ui background
		auto* background = ui::ImageView::create(TEXTURE_WHITE);
		background->setScale9Enabled(true);
		background->setContentSize(bottomBar);
		background->setAnchorPoint(ANCHOR_LEFT_BOTTOM_POINT);
		background->setColor(COLOR_3B_BROWN);
		this->addChild(background);

		//layout to holder friends profile layout and shop button
		//friends profile layout 
		auto* friendsProfileLayout = FriendsLayout::create(Size(bottomBar.width * 0.8, bottomBar.height));
		friendsProfileLayout->setAnchorPoint(ANCHOR_MID_POINT);
		friendsProfileLayout->setPosition(Vec2(bottomBar.width * 0.45, bottomBar.height / 2));
		background->addChild(friendsProfileLayout);

		//shop button
		auto button = ui::Button::create(TEXTURE_SHOP, "", "");
		button->setAnchorPoint(ANCHOR_MID_POINT);
		button->setPosition(Vec2(bottomBar.width * 0.9, bottomBar.height/2));
		background->addChild(button);
		button->addClickEventListener([](Ref*) {
			Size size = Director::getInstance()->getVisibleSize();
			SceneManager::Get().pushDialog(static_cast<Node*>(ShopDialog::create(size *0.6)));
			});
	}
}