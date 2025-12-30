#pragma once
#include <string>
using namespace std;
namespace CastleBuilder {
	struct ShopItemData
	{
	public:
		string ID;
		string image;
		string title;
		int coins;
	};

	struct ProfileData
	{
	public:
		string ID;
		string name;
		string image;
		int level;
	};

	struct GridData
	{
	public:
		int gridIndex;
		std::string itemId;
	};
}