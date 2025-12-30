#include "FriendsLayout.h"
#include "ui/UIListView.h"
#include "Constants.h"
#include "ui/UIButton.h"
#include "ui/UIWidget.h"
#include "base/CCRef.h"
#include "ProfileView.h"
#include "XmlConfigManager.h"

USING_NS_CC;
using namespace ui;

CastleBuilder::FriendsLayout* CastleBuilder::FriendsLayout::create(const cocos2d::Size& size)
{
	FriendsLayout* widget = new (std::nothrow) FriendsLayout();
	widget->size = size;
	if (widget && widget->init())
	{
		widget->autorelease();
		return widget;
	}
	CC_SAFE_DELETE(widget);
	return nullptr;
}

bool CastleBuilder::FriendsLayout::init()
{
	//layout to hold left,right buttons and firends profile view
	auto layout = ui::Layout::create();
	layout->setContentSize(size);
	layout->setLayoutType(ui::Layout::Type::HORIZONTAL);

	//layout params to align center and provide left and right marign 
	auto layoutParameter = LinearLayoutParameter::create();
	layoutParameter->setMargin(Margin(10, 0, 10, 0));
	layoutParameter->setGravity(LinearLayoutParameter::LinearGravity::CENTER_VERTICAL);

	//left navigation button to scroll left 
	auto previousButton = ui::Button::create(TEXTURE_PREVIOUS, "", "");
	previousButton->setAnchorPoint(ANCHOR_MID_POINT);
	previousButton->setLayoutParameter(layoutParameter);
	layout->addChild(previousButton);

	//list view to add all the friends widgets
	auto friendsListView = ui::ListView::create();
	friendsListView->setContentSize(Size(size.width*.8, size.height));
	friendsListView->setLayoutParameter(layoutParameter);
	friendsListView->setItemsMargin(10);
	friendsListView->setDirection(ui::ScrollView::Direction::HORIZONTAL);
	for each (auto var in XmlConfigManager::Get().getFriendProfileList())
	{
		auto playerProfile = ProfileView::create();
		playerProfile->setData(var);
		friendsListView->pushBackCustomItem(playerProfile);
	}

	layout->addChild(friendsListView);

	//left navigation button to scroll right
	auto nextButton = ui::Button::create(TEXTURE_NEXT, "", "");
	nextButton->setAnchorPoint(ANCHOR_MID_POINT);
	nextButton->setLayoutParameter(layoutParameter);
	layout->addChild(nextButton);

	//set content size of the parent widget otherwise its always zero and causeing alignment issues
	this->setContentSize(layout->getContentSize());
	this->addChild(layout);

	//click lisenters
	previousButton->addClickEventListener([friendsListView](Ref*)
		{
			friendsListView->scrollToLeft(0.5, false);
		});

	nextButton->addClickEventListener([friendsListView](Ref*)
		{
			friendsListView->scrollToRight(0.5, false);
		});
	return true;
}
