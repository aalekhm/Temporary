#pragma once
#include "cocos2d.h"
#include "ui/UIWidget.h"
#include "ui/UIImageView.h"
#include "2d/CCLabel.h"
#include "Model/Model.h"

class cocos2d::ui::Widget;
class cocos2d::ui::ImageView;
namespace CastleBuilder {
	class ProfileView : public cocos2d::ui::Widget
	{
	private:
		cocos2d::ui::ImageView* playerPic;
		cocos2d::Label* playerName;
		cocos2d::Label* playerLevel;
	public:
		static ProfileView* create();
		virtual bool init();
		void setData(shared_ptr<ProfileData> data);
	};
}