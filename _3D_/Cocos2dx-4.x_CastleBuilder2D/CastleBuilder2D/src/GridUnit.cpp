#include "GridUnit.h"
#include "Constants.h"
USING_NS_CC;
namespace CastleBuilder
{
	GridUnit* GridUnit::create(cocos2d::Size size)
	{
		GridUnit* gridUnit = new (std::nothrow) GridUnit();
		gridUnit->setContentSize(size);
		if (gridUnit && gridUnit->init())
		{
			gridUnit->autorelease();
			return gridUnit;
		}
		CC_SAFE_DELETE(gridUnit);
		return nullptr;
	}

	bool GridUnit::init()
	{
		highlight = ui::ImageView::create(TEXTURE_BACKGROUND);
		highlight->setContentSize(this->getContentSize());
		highlight->setAnchorPoint(ANCHOR_MID_POINT);
		highlight->ignoreContentAdaptWithSize(false);
		this->addChild(highlight);
		return true;
	}

	void GridUnit::highlightGridUint(bool gridUnitAvailable)
	{
		if (gridUnitAvailable)
			highlight->setColor(Color3B::GREEN);
		else
			highlight->setColor(Color3B::RED);
	}
}