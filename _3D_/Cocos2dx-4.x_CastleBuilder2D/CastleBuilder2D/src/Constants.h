#pragma once

#include "cocos2d.h"

/*Color constants*/
#define COLOR_3B_BROWN cocos2d::Color3B(235, 170, 57)

/*texture constants*/
#define TEXTURE_MAP "img/Map1.png"
#define TEXTURE_WHITE "img/white.png"
#define TEXTURE_SHOP "img/shop.png"
#define TEXTURE_PROFILE "img/profile.png"
#define TEXTURE_PREVIOUS "img/previous.png"
#define TEXTURE_NEXT "img/next.png"
#define TEXTURE_GOLD "img/coin.png"
#define TEXTURE_BRIDGE "img/decoration/bridge.png"
#define TEXTURE_BACKGROUND "img/background_with_border.png"
#define TEXTURE_TAB "img/tab_button.png"
#define TEXTURE_CLOSE "img/close.png"
#define TEXTURE_ZOOMIN "img/zoomin.png"
#define TEXTURE_ZOOMOUT "img/zoomout.png"

/*anchor points*/
#define ANCHOR_MID_POINT cocos2d::Vec2(0.5, 0.5)
#define ANCHOR_MID_BOTTOM_POINT cocos2d::Vec2(0.5, 0.0)
#define ANCHOR_MID_TOP_POINT cocos2d::Vec2(0.5, 1.0)
#define ANCHOR_RIGHT_TOP_POINT cocos2d::Vec2(1.0, 1.0)
#define ANCHOR_RIGHT_BOTTOM_POINT cocos2d::Vec2(1.0, 0.0)
#define ANCHOR_RIGHT_MID_POINT cocos2d::Vec2(1.0, 0.5)
#define ANCHOR_LEFT_BOTTOM_POINT cocos2d::Vec2(0.0, 0.0)
#define ANCHOR_LEFT_MID_POINT cocos2d::Vec2(0.0, 0.5)
#define ANCHOR_LEFT_TOP_POINT cocos2d::Vec2(0.0, 1.0)


/*Font name and font size consts*/
#define FONT_ARIAL "fonts/arial.ttf"
#define FONT_MARKER_FELT "fonts/Marker Felt.ttf"
#define FONT_SIZE_XX_SMALL 8
#define FONT_SIZE_X_SMALL 10
#define FONT_SIZE_SMALL 12
#define FONT_SIZE_MEDIUM 14
#define FONT_SIZE_LARGE 16
#define FONT_SIZE_X_LARGE 18
#define FONT_SIZE_XX_LARGE 20

/*scene z-order*/
#define SCENE_ZORDER_DEFAULT  0
#define SCENE_ZORDER_DIALOG  10
#define SCENE_ZORDER_TOAST  5

/*Shop tab items*/
enum SHOP_TABS{
WORK_SHOPS = 0,
DECORATIONS,
COUNT
};

/* string const*/
#define STR_SHOP_WORKSHOP "Work Shops"
#define STR_SHOP_DECORATIONS "Decorations"

/*zoon in out*/
#define ZOOM_MAX 2.0
#define ZOOM_MIN 1.0
#define ZOOM_STEP 0.2

/*grid Units*/
//#define MAX_GRID_UNITS 8
#define MAX_GRID_SIZE 256

/*custom event ids*/
#define EVT_SHOP_ITEM_SELECTED "shop_item_selcted"

/*xml constants*/
#define XML_CONFIG_FILENAME "config.xml"
#define XML_FRIEND_PROFILE_FILENAME "friend_profile.xml"
#define XML_SHOP_FILENAME "shop.xml"
#define XML_GRID_SAVE_FILENAME "grid_save.xml"
#define XML_CONFIG_ROOT "config"
#define XML_PROFILES_ROOT "profiles"
#define XML_CONFIG_RESOLUTION "resolution"
#define XML_CONFIG_WIDTH "width"
#define XML_CONFIG_HEIGHT "height"

#define XML_CONFIG_PROFILE "profile"
#define XML_CONFIG_ID "id"
#define XML_CONFIG_NAME "name"
#define XML_CONFIG_IMAGE "image"
#define XML_CONFIG_TITLE "title"
#define XML_CONFIG_COINS "coins"
#define XML_CONFIG_LEVEL "level"

#define XML_SHOP_ROOT "shop"
#define XML_WORKSHOP_ROOT "workshop"
#define XML_DECORATIONS_ROOT "decoration"
#define XML_CONFIG_SHOP_ITEM "shopitem"

#define XML_GRID_SAVE_ROOT "save"
#define XML_GRID_UNIT "gridunit"
#define XML_GRID_INDEX "gridindex"

/*assets search paths*/
#define ASSET_IMG_PATH "img"
#define ASSET_IMG_DECORATION_PATH "img/decoration"
#define ASSET_IMG_WORKSHOP_PATH "img/workshop"
