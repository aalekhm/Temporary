#include "BaseDialog.h"
#include "ui/UIButton.h"
#include "ui/UIImageView.h"
#include "Constants.h"
#include "SceneManager.h"

USING_NS_CC;

namespace CastleBuilder
{
	BaseDialog::BaseDialog():rootNode(nullptr)
	{
	}
	BaseDialog::~BaseDialog()
	{
	}
	BaseDialog* BaseDialog::create(cocos2d::Size size)
	{
		BaseDialog* dialog = new (std::nothrow) BaseDialog();
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
	bool BaseDialog::init()
	{
		auto director = cocos2d::Director::getInstance();
		auto visibleSize = director->getVisibleSize();
		this->setContentSize(visibleSize);
		this->addChild(rootNode);
		//bottom bar ui background
		auto* background = ui::ImageView::create(TEXTURE_BACKGROUND);
		background->setScale9Enabled(true);
		background->setContentSize(rootNode->getContentSize());
		background->setAnchorPoint(ANCHOR_MID_POINT);
		background->setColor(COLOR_3B_BROWN);
		rootNode->addChild(background);

		//close button
		auto closeBtn = ui::Button::create(TEXTURE_CLOSE, "", "");
		closeBtn->setAnchorPoint(ANCHOR_RIGHT_TOP_POINT);
		closeBtn->setPosition(rootNode->getContentSize());
		background->addChild(closeBtn,1);
		//add click event lisenter to close btn
		closeBtn->addClickEventListener([this](Ref*) {
			close();
			});

		//always position the dialog in the center of the screen
		this->setPosition(visibleSize/2);
		return true;
	}
	bool BaseDialog::swallowTouches()
	{
		auto touchlisenters = EventListenerTouchOneByOne::create();
		touchlisenters->setSwallowTouches(true);
		touchlisenters->onTouchBegan = [](Touch* touch, Event* event) {
			return true;
		};

		cocos2d::Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(touchlisenters, this);
		return true;
	}
	void BaseDialog::close()
	{
		SceneManager::Get().popDialog();
	}
}