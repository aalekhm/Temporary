#include "ProfileView.h"
#include "ui/UIWidget.h"
#include "base/CCRef.h"
#include "ui/UIImageView.h"
#include "Constants.h"
#include "ui/UILayout.h"

USING_NS_CC;
namespace CastleBuilder {

	ProfileView* ProfileView::create()
	{
		ProfileView* widget = new (std::nothrow) ProfileView();
		if (widget && widget->init())
		{
			widget->autorelease();
			return widget;
		}
		CC_SAFE_DELETE(widget);
		return nullptr;
	}

	bool ProfileView::init()
	{
		//player profile pic
		playerPic = ui::ImageView::create(TEXTURE_PROFILE);
		playerPic->setAnchorPoint(ANCHOR_MID_POINT);
		this->addChild(playerPic);

		//player name
		playerName = Label::createWithTTF("PlayerName", FONT_MARKER_FELT, FONT_SIZE_SMALL);
		playerName->setPosition(playerPic->getContentSize().width/2, - playerName->getContentSize().height/2);
		playerPic->addChild(playerName);

		//player Level
		playerLevel = Label::createWithTTF("Player Level", FONT_ARIAL, FONT_SIZE_SMALL);
		playerLevel->setPosition(playerPic->getContentSize().width / 2, -playerName->getContentSize().height / 2 -playerLevel->getContentSize().height);
		playerPic->addChild(playerLevel);

		//position the entire widget in the center
		playerPic->setPosition(Vec2(playerPic->getContentSize().width / 2, 
			(playerPic->getContentSize().height + playerName->getContentSize().height+ playerLevel->getContentSize().height)/2 ));
		this->setContentSize(playerPic->getContentSize());
		return true;
	}

	void ProfileView::setData(shared_ptr<ProfileData> data)
	{
		playerPic->loadTexture(data->image);
		playerName->setString(data->name);
		char level[25];
		sprintf(level, "Level%d", data->level);
		playerLevel->setString(level);
	}
}
