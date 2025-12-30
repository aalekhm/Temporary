#pragma once

#include "cocos2d.h"
#include "GridUnit.h"
#include "Constants.h"
#include "Model/Model.h"

namespace CastleBuilder
{
	class MapScene :public cocos2d::Scene
	{
	private:
		MapScene():sprite(nullptr)
			,gridUnit(nullptr)
			,bottomBarSize(cocos2d::Size(0,0))
			,currentShopItemData(nullptr)
			,mapOrigin() {};
		~MapScene() {};
		Node* sprite;
		GridUnit* gridUnit;
		std::vector<Node*> gridList;
		std::vector<shared_ptr<GridData>> gridDataList;

		cocos2d::Size bottomBarSize;
		cocos2d::Vec2 mapOrigin;
		int maxGridUnitsX;
		int maxGridUnitsY;
		ShopItemData* currentShopItemData;
		CREATE_FUNC(MapScene);
		void addListeners();
		void addTouchEventListeners();
		void addMouseEventListeners();
		void addCustomEventListeners();
		int getlistIndexFromXY(int xIndex, int yIndex);
		void snapGridUnit(cocos2d::EventMouse* event);
		void snapItenWithIndex(int xIndex, int yIndex, int index);
		void spwanItemOnGrid(cocos2d::Vec2& snapPos, int index);
		void highlightGridUnit(cocos2d::EventMouse* event);
		void updateMapPosition(cocos2d::Vec2 delta = cocos2d::Vec2());
		void addZoomBtns();
		void addBottomHud();
		void addMap(cocos2d::Vec2& origin, cocos2d::Size& visibleSize);
		void addGridUnit();
	public:
		static cocos2d::Scene* createScene();
		virtual bool init();
		void populateGirdFromSaveData();
		void saveGrid();
		int getGridUnitSize();
	};

}

