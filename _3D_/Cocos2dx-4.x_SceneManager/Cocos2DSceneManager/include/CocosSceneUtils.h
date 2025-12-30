#pragma once

#include <cocos2d.h>
#include "ui/UIImageView.h"

enum THEMED_PARAMETER
{
	THEMED_POSITION = 0,
	THEMED_SCALING,
	THEMED_ROTATION,
	THEMED_ANCHOR,
	THEMED_CONTENTSIZE,
	THEMED_VISIBILITY,
	THEMED_REAL_COLOUR,
	THEMED_OPACITY,
	THEMED_TEXTURE,

	THEMED_TEXT_FONTRESOURCE,
	THEMED_TEXT_FONTSIZE,
	THEMED_TEXT_FONTTEXT,
	THEMED_TEXT_FONTCOLOUR,
	THEMED_TEXT_FONTALIGN,
	THEMED_TEXT_FONTTEXTAREA,

	THEMED_USERDATA
};

class CocosView;
class BlitzHelper;
class ProfilePic;

class CocosSceneUtils
{
	public:
		//static uint32_t					GetViewHash(std::string sScreenName);
		static std::string					GetExtentionName(const std::string& name);
		static cocos2d::Node*				GetNodeFromParent(std::string sNodeName, cocos2d::Node* pParent = nullptr);

		static void							RefreshCocosLayoutForComponent(cocos2d::Node* pNode);

		static std::string					GetStrippedFileName(const std::string& sFileNameWithPath);
		static bool							RenameFileExtn(std::string& sFileNameWithExtn, const std::string& sExtnToChange, const std::string& sChangeToExtn);

		static cocos2d::Node*				AllocateElementOfType(std::string sCSB, CocosView* pParentCocosView, cocos2d::Size& sContentSize);

		static bool							IsImageAvailable(std::string iconName, std::string directoryName);

		static cocos2d::Size				GetVisibleSize();
		static cocos2d::Vec2				GetWorldSpacePos(cocos2d::Node* pNode);

		static float_t						GetRotationInDegrees(const cocos2d::Vec2& vPoint1, const cocos2d::Vec2& vPoint2);

		static cocos2d::Node*				CreateRootNodeAtOrigin(cocos2d::Node* pParentNode);
		static cocos2d::Layer*				CreateRootLayerAtOrigin(cocos2d::Node* pParentNode);
		static cocos2d::TargetedAction*		CreateTargetedAction_MoveTo(cocos2d::Node* pNode, const cocos2d::Vec2& vDestPos, float fDuration);
		static cocos2d::TargetedAction*		CreateAnimatedActionWithSpriteFrames(cocos2d::Node* pParent, const std::string& sFrameName, int32_t iMaxFrameCount, float fDuration, const cocos2d::Vec2& vAnchor, const cocos2d::Vec2& vPosition, float fAngle, float fOpacity = 255.0f);
		static cocos2d::ui::ImageView*		CreateImageNode(cocos2d::Node* pParentNode, const std::string& sTexture, cocos2d::ui::Widget::TextureResType eTextureResType, const cocos2d::Vec2& vAnchor, const cocos2d::Vec2& vPosition, float fOpacity);
		static cocos2d::TargetedAction*		CreateTargetedAction_FadeIn(cocos2d::Node* pNode, float fDuration);
		static cocos2d::TargetedAction*		CreateTargetedAction_FadeOut(cocos2d::Node* pNode, float fDuration);
		static cocos2d::TargetedAction*		CreateTargetedAction_FadeTo(cocos2d::Node* pNode, float fDuration, int8_t iOpacity);
		static cocos2d::TargetedAction*		CreateTargetedAction_ScaleTo(cocos2d::Node* pNode, float fDuration, float fScaleX, float fScaleY);
		static void							CalcAndRotate(cocos2d::Node* pNode, const cocos2d::Vec2& vPoint1, const cocos2d::Vec2& vPoint2);

		static cocos2d::Sequence*			CreateSequence(cocos2d::FiniteTimeAction* pFiniteTimeAction, ...);

		static bool							HasCustomProperties(cocos2d::Node* pNode, std::string& sUserDataOUT);
		static bool							GetCustomPropertyValue(cocos2d::Node* pNode, const std::string& sPropertyNameIN, std::string& sPropertyValueOUT);

		static cocos2d::Node*				ThemedUI_ReadParameterOverrides(cocos2d::Node* pNode/*, Json::JsonDomObject* pJsonDomObject*/);

		static void							handleCustomUIParameters(CocosView* pCocosView, cocos2d::Node* pNode);
		static void							handleCustomUIParameters(std::string& sViewName, cocos2d::Node* pNode);
		static void							readAndApplyUserData(cocos2d::Node* pNode);
		static bool							readNameValuePairsFromString(std::string sInString, std::string sDelimiterMain, std::string sDelimiterNameValue, std::map<std::string, std::string>& mapUserDataOut);
		static std::string					getValueForNameFromMap(std::string sName, std::map<std::string, std::string>& mapNameValuePair);

		static bool							ReadCustomUIParameters(/*std::map<std::string, JsonDomObject*>& m_mapCustomUINodeParameters_NameObject, JsonDomObject* pJsonDomObject*/);
private:
	static cocos2d::Node* 					ThemedUI_ReadNodeAttributes(cocos2d::Node* pNode/*, JsonDomObject* pJsonDomObject*/);
		static void							ThemedUI_ReadCustomNodeAttributes(cocos2d::Node* pNode/*, JsonDomObject* pJsonDomObject*/);
		static void							ThemedUI_AddDynamicChildNodes(cocos2d::Node* pParent/*, JsonDomObject* pJsonDomObject*/);
		static void							ThemedUI_SetThemeParameter(cocos2d::Node*, THEMED_PARAMETER eTHEMED_PARAMETER, void* pValue);

		static void							ThemedUI_HandleNode(cocos2d::Node* pNode, THEMED_PARAMETER eTHEMED_PARAMETER, void* pValue);
		static void							ThemedUI_HandleWidget(cocos2d::Node* pNode, THEMED_PARAMETER eTHEMED_PARAMETER, void* pValue);
		static void							ThemedUI_HandleLayout(cocos2d::Node* pNode, THEMED_PARAMETER eTHEMED_PARAMETER, void* pValue);
		static void							ThemedUI_HandleImageView(cocos2d::Node* pNode, THEMED_PARAMETER eTHEMED_PARAMETER, void* pValue);
		static void							ThemedUI_HandleSprite(cocos2d::Node* pNode, THEMED_PARAMETER eTHEMED_PARAMETER, void* pValue);
		static void							ThemedUI_HandleText(cocos2d::Node* pNode, THEMED_PARAMETER eTHEMED_PARAMETER, void* pValue);
		static void							ThemedUI_UserData(cocos2d::Node* pNode, THEMED_PARAMETER eTHEMED_PARAMETER, void* pValue);

		static bool							readUserData(cocos2d::Node* pNode, std::map<std::string, std::string>& mapUserDataOut);
		static void							applyUserData(cocos2d::Node* pNode, std::map<std::string, std::string>& mapUserDataOut);
};