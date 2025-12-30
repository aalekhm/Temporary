#pragma once
#include "Singleton.h"
#include "cocos2d.h"
#include "math/CCGeometry.h"
#include "Model/Model.h"
#include "tinyxml2.h"
#include "Constants.h"

namespace CastleBuilder
{
	class XmlConfigManager:public Singleton<XmlConfigManager>
	{
	public:
		~XmlConfigManager();
		void init();
		const cocos2d::Size getResolution() { return resolution; }
		const std::vector<shared_ptr<ProfileData>> getFriendProfileList() { return friendProfileList;}
		const std::vector<shared_ptr<ShopItemData>> getWorkShopItemList() { return workShopItemList; }
		const std::vector<shared_ptr<ShopItemData>> getDecorationsItemList() { return decorationsItemList; }
		void saveGrid(std::vector<shared_ptr<GridData>> gridList);
		std::vector<shared_ptr<GridData>> getSavedGridList();
		shared_ptr<ShopItemData> getShopItemById(std::string &id);
	private:
		cocos2d::Size resolution;
		std::vector<shared_ptr<ProfileData>> friendProfileList;
		std::vector<shared_ptr<ShopItemData>> workShopItemList;
		std::vector<shared_ptr<ShopItemData>> decorationsItemList;

		void loadConfig();
		void loadFriendProfileData();
		void loadShopItemData();
		void populateShopItemLists(tinyxml2::XMLElement* pRoot, SHOP_TABS type);
		void pushShopItemData(tinyxml2::XMLElement* element, SHOP_TABS type);
		std::string getFilePathByName(const std::string& fileName);
		std::string getFileWriteableByName(const std::string& fileName);
	};
}


