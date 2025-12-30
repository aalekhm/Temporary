#include "XmlConfigManager.h"
#include "tinyxml2/tinyxml2.h"
#include "Constants.h"
#include "CCFileUtils.h"
#include <string.h>
USING_NS_CC;

namespace CastleBuilder
{
	XmlConfigManager::~XmlConfigManager()
	{
		friendProfileList.clear();
		workShopItemList.clear();
		decorationsItemList.clear();
	}
	void XmlConfigManager::init()
	{
		loadConfig();
		loadFriendProfileData();
		loadShopItemData();
	}
	void XmlConfigManager::saveGrid(std::vector<shared_ptr<GridData>> gridList)
	{
		tinyxml2::XMLDocument doc;
		tinyxml2::XMLElement* root = doc.NewElement(XML_GRID_SAVE_ROOT);
		for each (auto var in gridList)
		{
			tinyxml2::XMLElement* gridUnit = doc.NewElement(XML_GRID_UNIT);
			gridUnit->SetAttribute(XML_CONFIG_ID, var->itemId.c_str());
			gridUnit->SetAttribute(XML_GRID_INDEX, var->gridIndex);
			root->InsertEndChild(gridUnit);
		}
		doc.InsertEndChild(root);

		doc.SaveFile(getFileWriteableByName(XML_GRID_SAVE_FILENAME).c_str());
	}
	std::vector<shared_ptr<GridData>> XmlConfigManager::getSavedGridList()
	{
		auto list = std::vector<shared_ptr<GridData>>();
		tinyxml2::XMLDocument configDoc;
		configDoc.LoadFile(getFileWriteableByName(XML_GRID_SAVE_FILENAME).c_str());
		if (!configDoc.Error())
		{
			auto pRoot = configDoc.FirstChildElement(XML_GRID_SAVE_ROOT);
			if (pRoot)
			{
				auto element = pRoot->FirstChildElement(XML_GRID_UNIT);
				while (element)
				{
					auto value = make_shared<GridData>();
					value->gridIndex = element->IntAttribute(XML_GRID_INDEX);
					value->itemId = element->Attribute(XML_CONFIG_ID);
					list.push_back(value);
					element = element->NextSiblingElement(XML_GRID_UNIT);
				}
			}
		}
		return list;
	}
	shared_ptr<ShopItemData> XmlConfigManager::getShopItemById(std::string& id)
	{
		 auto shopItem = std::find_if(workShopItemList.begin(), workShopItemList.end(),
			[id](const shared_ptr<ShopItemData>& ele) { return ele->ID == id; });
		 if(shopItem == workShopItemList.end())
			 shopItem = std::find_if(decorationsItemList.begin(), decorationsItemList.end(),
				 [id](const shared_ptr<ShopItemData>& ele) { return ele->ID == id; });
		return *shopItem;
	}
	void XmlConfigManager::loadConfig()
	{
		tinyxml2::XMLDocument configDoc;
		configDoc.LoadFile(getFilePathByName(XML_CONFIG_FILENAME).c_str());
		if (!configDoc.Error())
		{
			auto pRoot = configDoc.FirstChildElement(XML_CONFIG_ROOT);
			if (pRoot)
			{
				auto xmlResolution = pRoot->FirstChildElement(XML_CONFIG_RESOLUTION);
				if (xmlResolution)
				{
					xmlResolution->FirstChildElement(XML_CONFIG_WIDTH)->QueryFloatText(&resolution.width);
					xmlResolution->FirstChildElement(XML_CONFIG_HEIGHT)->QueryFloatText(&resolution.height);
				}
			}
		}
	}
	void XmlConfigManager::loadFriendProfileData()
	{
		friendProfileList.clear();
		tinyxml2::XMLDocument configDoc;
		configDoc.LoadFile(getFilePathByName(XML_FRIEND_PROFILE_FILENAME).c_str());
		if (!configDoc.Error())
		{
			auto pRoot = configDoc.FirstChildElement(XML_PROFILES_ROOT);
			auto element = pRoot->FirstChildElement(XML_CONFIG_PROFILE);
			while (element)
			{
				auto profileData = make_shared<ProfileData>();
				profileData->ID = element->FirstChildElement(XML_CONFIG_ID)->GetText();
				profileData->name = element->FirstChildElement(XML_CONFIG_NAME)->GetText();
				profileData->image = element->FirstChildElement(XML_CONFIG_IMAGE)->GetText();
				element->FirstChildElement(XML_CONFIG_LEVEL)->QueryIntText(&profileData->level);
				friendProfileList.push_back(profileData);
				element = element->NextSiblingElement(XML_CONFIG_PROFILE);
			}
		}
	}
	void XmlConfigManager::loadShopItemData()
	{
		workShopItemList.clear();
		decorationsItemList.clear();

		tinyxml2::XMLDocument configDoc;
		configDoc.LoadFile(getFilePathByName(XML_SHOP_FILENAME).c_str());
		if (!configDoc.Error())
		{
			auto pRoot = configDoc.FirstChildElement(XML_SHOP_ROOT);
			populateShopItemLists(pRoot, SHOP_TABS::WORK_SHOPS);
			populateShopItemLists(pRoot, SHOP_TABS::DECORATIONS);
		}
	}

	void XmlConfigManager::populateShopItemLists(tinyxml2::XMLElement* pRoot, SHOP_TABS type)
	{
		std::string elementName = XML_WORKSHOP_ROOT;
		if (type == SHOP_TABS::DECORATIONS)
		{
			elementName = XML_DECORATIONS_ROOT;
		}
		auto elementRoot = pRoot->FirstChildElement(elementName.c_str());
		auto element = elementRoot->FirstChildElement(XML_CONFIG_SHOP_ITEM);
		while (element)
		{
			pushShopItemData(element, type);
			element = element->NextSiblingElement(XML_CONFIG_SHOP_ITEM);
		}
	}

	void XmlConfigManager::pushShopItemData(tinyxml2::XMLElement* element,SHOP_TABS type)
	{
		auto shopItemData = make_shared<ShopItemData>();
		shopItemData->ID = element->FirstChildElement(XML_CONFIG_ID)->GetText();
		shopItemData->title = element->FirstChildElement(XML_CONFIG_TITLE)->GetText();
		shopItemData->image = element->FirstChildElement(XML_CONFIG_IMAGE)->GetText();
		element->FirstChildElement(XML_CONFIG_COINS)->QueryIntText(&shopItemData->coins);
		if(type == SHOP_TABS::WORK_SHOPS)
			workShopItemList.push_back(shopItemData);
		else if (type == SHOP_TABS::DECORATIONS)
			decorationsItemList.push_back(shopItemData);
	}

	std::string XmlConfigManager::getFilePathByName(const std::string &fileName)
	{
		return FileUtils::getInstance()->fullPathForFilename(fileName);
	}
	std::string XmlConfigManager::getFileWriteableByName(const std::string& fileName)
	{
		return FileUtils::getInstance()->getWritablePath().append(fileName);
	}
}