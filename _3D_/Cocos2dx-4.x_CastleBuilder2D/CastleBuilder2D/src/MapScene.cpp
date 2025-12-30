#include "MapScene.h"
#include "ui/UIListView.h"
#include "Constants.h"
#include "ui/UIButton.h"
#include "ShopItemView.h"
#include "BaseDialog.h"
#include "BottomBarUI.h"
#include "ui/UIWidget.h"
#include "XmlConfigManager.h"
#include "2d/CCProgressTimer.h"

USING_NS_CC;

namespace CastleBuilder
{
	cocos2d::Scene* MapScene::createScene()
	{
		return MapScene::create();
	}

	bool MapScene::init()
	{
		CCLOG("Vector size %d", gridList.size());
		auto size = Director::getInstance()->getVisibleSize();
		bottomBarSize = Size(size.width, size.height * 0.25);
		mapOrigin = Vec2(0, bottomBarSize.height);
		Vec2 origin = Director::getInstance()->getVisibleOrigin();
		addMap(origin, size);
		addListeners();
		addBottomHud();
		addZoomBtns();
		addGridUnit();
		maxGridUnitsX = sprite->getContentSize().width / MAX_GRID_SIZE;
		maxGridUnitsY = sprite->getContentSize().height / MAX_GRID_SIZE;

		gridDataList = std::vector<shared_ptr<GridData>>(maxGridUnitsX * maxGridUnitsY, nullptr);
		gridList = std::vector<Node*>(maxGridUnitsX * maxGridUnitsY, nullptr);

		populateGirdFromSaveData();


		/*auto sprite = Sprite::create(TEXTURE_WHITE);
		sprite->setContentSize(Size(200,50));
		sprite->setAnchorPoint(ANCHOR_LEFT_MID_POINT);
		auto progressTimer = ProgressTimer::create(sprite);
		progressTimer->setType(ProgressTimer::Type::BAR);
		progressTimer->setMidpoint(ANCHOR_LEFT_MID_POINT);
		progressTimer->setAnchorPoint(ANCHOR_LEFT_MID_POINT);
		progressTimer->setContentSize(Size(200, 50));
		progressTimer->setPosition(size/2);
		progressTimer->setPercentage(75);
		this->addChild(progressTimer);*/
		return true;
	}

	void MapScene::populateGirdFromSaveData()
	{
		for each (auto gridData in XmlConfigManager::Get().getSavedGridList())
		{
			int xIndex = gridData->gridIndex % maxGridUnitsX;
			int yIndex = (gridData->gridIndex - xIndex) / maxGridUnitsY;
			currentShopItemData = XmlConfigManager::Get().getShopItemById(gridData->itemId).get();
			if (currentShopItemData != nullptr)
				snapItenWithIndex(xIndex, yIndex, gridData->gridIndex);
		}
	}

	void MapScene::saveGrid()
	{
		auto list = std::vector<shared_ptr<GridData>>();
		for each (auto var in gridDataList)
		{
			if (var != nullptr)
			{
				CCLOG("ID %s index %d", var->itemId.c_str(), var->gridIndex);
				list.push_back(var);
			}
		}
		if (list.size() > 0)
			XmlConfigManager::Get().saveGrid(list);
	}

	int MapScene::getGridUnitSize()
	{
		return MAX_GRID_SIZE;
	}
	
	void MapScene::addGridUnit()
	{
		int gridUnitSize = getGridUnitSize();
		gridUnit = GridUnit::create(Size(gridUnitSize, gridUnitSize));
		gridUnit->setPosition(sprite->getContentSize() / 2);
		gridUnit->setVisible(false);
		sprite->addChild(gridUnit);
	}
	void MapScene::addZoomBtns()
	{
		auto visibleSize = Director::getInstance()->getVisibleSize();
		//zoom button
		auto zoomIn = ui::Button::create(TEXTURE_ZOOMIN, "", "");
		zoomIn->setAnchorPoint(ANCHOR_RIGHT_MID_POINT);
		zoomIn->ignoreContentAdaptWithSize(false);
		zoomIn->setContentSize(Size(64,64));
		zoomIn->setPosition(Vec2(visibleSize.width, visibleSize.height *.5));
		zoomIn->addClickEventListener([this](Ref*) {
			sprite->setScale(std::min(sprite->getScale() + ZOOM_STEP, ZOOM_MAX));
			updateMapPosition();
			});

		//zoom button
		auto zoomOut = ui::Button::create(TEXTURE_ZOOMOUT, "", "");
		zoomOut->setAnchorPoint(ANCHOR_RIGHT_MID_POINT);
		zoomOut->ignoreContentAdaptWithSize(false);
		zoomOut->setContentSize(Size(64, 64));
		zoomOut->setPosition(Vec2(visibleSize.width, visibleSize.height * .5 - zoomOut->getContentSize().height));
		zoomOut->addClickEventListener([this](Ref*) {
			sprite->setScale(std::max(sprite->getScale() - ZOOM_STEP,ZOOM_MIN));
			updateMapPosition();
			});
		this->addChild(zoomIn);
		this->addChild(zoomOut);
	}

	void MapScene::addBottomHud()
	{
		auto* node = BottomBarUI::create(bottomBarSize);
		this->addChild(node);
	}
	void MapScene::addMap(cocos2d::Vec2& origin, cocos2d::Size& visibleSize)
	{
		sprite = Sprite::create("img/Map1.jpg");
		sprite->setAnchorPoint(ANCHOR_LEFT_BOTTOM_POINT);
		sprite->setPosition(mapOrigin);
		this->addChild(sprite);
	}
	void MapScene::addListeners()
	{
		addCustomEventListeners();
		addMouseEventListeners();
		addTouchEventListeners();
	}
	void MapScene::addTouchEventListeners()
	{
		auto listeners = EventListenerTouchOneByOne::create();
		listeners->onTouchMoved = [this](Touch* touch, Event* event)
		{
			//CCLOG("touched %s", "yes");
			updateMapPosition(touch->getDelta());
		};

		// trigger when you push down
		listeners->onTouchBegan = [](Touch* touch, Event* event)
		{
			//CCLOG("touched %s", "yes");
			return true; // if you are consuming it
		};

		// trigger when you push down
		listeners->onTouchEnded = [this](Touch* touch, Event* event)
		{
			//CCLOG("touched %s", "yes");
			return true; // if you are consuming it
		};
		_eventDispatcher->addEventListenerWithSceneGraphPriority(listeners, sprite);
	}
	void MapScene::addMouseEventListeners()
	{
		auto mouseLisenters = EventListenerMouse::create();
		mouseLisenters->onMouseMove = [this](EventMouse* event)
		{
			if (nullptr != currentShopItemData)
			{
				highlightGridUnit(event);
			}
		};
		mouseLisenters->onMouseDown = [=](EventMouse* event) {
			if (nullptr != currentShopItemData)
			{
				snapGridUnit(event);
			}
		};
		_eventDispatcher->addEventListenerWithSceneGraphPriority(mouseLisenters, sprite);
	}

	void MapScene::addCustomEventListeners()
	{
		auto _listener = EventListenerCustom::create(EVT_SHOP_ITEM_SELECTED, [=](EventCustom* event) {
			currentShopItemData = (static_cast<ShopItemData*>(event->getUserData()));
			auto* shopItem = ui::ImageView::create(currentShopItemData->image);
			shopItem->setAnchorPoint(ANCHOR_MID_POINT);
			shopItem->setTag(10);
			gridUnit->addChild(shopItem);
			});
		_eventDispatcher->addEventListenerWithSceneGraphPriority(_listener, this);
	}

	int MapScene::getlistIndexFromXY(int xIndex,int yIndex)
	{
		return xIndex + yIndex * maxGridUnitsX;
	}
	void MapScene::snapGridUnit(cocos2d::EventMouse* event)
	{
		gridUnit->setVisible(true);
		Size visibleSize = Director::getInstance()->getVisibleSize();
		Size size = sprite->getContentSize() / 2;
		Size offset = (sprite->getContentSize() * sprite->getScale() - visibleSize);
		Vec2 pos = convertToWindowSpace(event->getLocation()) - sprite->getPosition();
		int gridUnitSize = getGridUnitSize();
		int xIndex = pos.x / gridUnitSize;
		int yIndex = pos.y / gridUnitSize;
		int index = getlistIndexFromXY(xIndex,yIndex);
		snapItenWithIndex(xIndex, yIndex, index);
	}
	void MapScene::snapItenWithIndex(int xIndex, int yIndex, int index)
	{
		int gridUnitSize = getGridUnitSize();
		Vec2 snapPos = Vec2(xIndex * gridUnitSize + gridUnitSize / 2, yIndex * gridUnitSize + gridUnitSize / 2);
		if (gridList.at(index) == nullptr)
		{
			spwanItemOnGrid(snapPos, index);
		}
	}
	void MapScene::spwanItemOnGrid(cocos2d::Vec2& snapPos, int index)
	{
		auto gridNode = ui::Widget::create();
		gridNode->setAnchorPoint(ANCHOR_MID_POINT);
		int gridUnitSize = getGridUnitSize();
		gridNode->setContentSize(Size(gridUnitSize, gridUnitSize));
		gridNode->setPosition(snapPos);
		auto shopItem = ui::ImageView::create(currentShopItemData->image);
		shopItem->setAnchorPoint(ANCHOR_MID_POINT);
		shopItem->setPosition(gridNode->getContentSize() / 2);
		gridNode->addChild(shopItem);
		sprite->addChild(gridNode);
		gridList.at(index) = gridNode;
		{
			auto gridData = make_shared<GridData>();
			gridData->gridIndex = index;
			gridData->itemId = currentShopItemData->ID;
			gridDataList.push_back(gridData);
		}
		gridUnit->setVisible(false);
		gridUnit->removeChildByTag(10);
		currentShopItemData = nullptr;

		saveGrid();
		//for delete functionality
		//gridNode->addClickEventListener([=](Ref* ref) {
		//	ui::Widget* widget = static_cast<ui::Widget*>(ref);
		//	widget->getUserData();
		//	});
	}
	void MapScene::highlightGridUnit(cocos2d::EventMouse* event)
	{
		gridUnit->setVisible(true);
		Size visibleSize = Director::getInstance()->getVisibleSize();
		Size size = sprite->getContentSize() / 2;
		Size offset = (sprite->getContentSize() * sprite->getScale() - visibleSize);
		Vec2 pos = convertToWindowSpace(event->getLocation()) - sprite->getPosition();
		int gridUnitSize = getGridUnitSize();
		gridUnit->setContentSize(Size(gridUnitSize, gridUnitSize));
		int xIndex = pos.x / gridUnitSize;
		int yIndex = pos.y / gridUnitSize;
		Vec2 snapPos = Vec2(xIndex * gridUnitSize + gridUnitSize / 2, yIndex * gridUnitSize + gridUnitSize / 2);
		gridUnit->setPosition(snapPos);
		int index = getlistIndexFromXY(xIndex, yIndex);
		CCLOG("gridIndex x:%d y:%d index:%d", xIndex,yIndex,index);
		gridUnit->highlightGridUint(gridList.at(index)==nullptr);
	}

	void MapScene::updateMapPosition(cocos2d::Vec2 delta)
	{
		Size visibleSize = Director::getInstance()->getVisibleSize();
		Size offset = (sprite->getContentSize() * sprite->getScale() - visibleSize);
		Vec2 position = sprite->getPosition() + delta;
		position = position.getClampPoint(Vec2(-offset.width,-offset.height),Vec2(0,mapOrigin.y));
		sprite->setPosition(position);
	}
	
}
