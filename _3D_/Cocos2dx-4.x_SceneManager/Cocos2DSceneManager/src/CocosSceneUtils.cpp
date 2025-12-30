#include "CocosSceneUtils.h"

#include "ui/CocosGUI.h"
#include "CocosSceneManager.h"
#include "editor-support/cocostudio/ActionTimeline/CSLoader.h"
#include "editor-support/cocostudio/CCComExtensionData.h"

//uint32_t CocosSceneUtils::GetViewHash(std::string sViewName)
//{
//	uint32_t iReturnHash = 0;
//	auto iStrLen = sViewName.length();
//	auto iCount = 0;
//	while (iCount < iStrLen)
//	{
//		char c = (char)sViewName.at(iCount);
//		iReturnHash = RotateLeft(iReturnHash, 13);
//		iReturnHash += c;
//		iCount++;
//	}
//
//	return iReturnHash;
//}

std::string CocosSceneUtils::GetExtentionName(const std::string& name)
{
	std::string path = name;
	size_t pos = path.find_last_of('.');
	std::string result = path.substr(pos + 1, path.length());

	return result;
}

cocos2d::Node* CocosSceneUtils::GetNodeFromParent(std::string sNodeName, cocos2d::Node* pParent /*= nullptr*/)
{
	cocos2d::Node* pRootNode = (pParent == nullptr) ? CocosSceneManager::Get().GetRootScene() : pParent;

	cocos2d::Node* pOutNode = nullptr;
	GET_CHILD_BY_NAME(pRootNode, pOutNode, cocos2d::Node, sNodeName);

	return pOutNode;
}

void CocosSceneUtils::RefreshCocosLayoutForComponent(cocos2d::Node* pNode)
{
	auto layoutComponent = cocos2d::ui::LayoutComponent::bindLayoutComponent(pNode);
	layoutComponent->refreshLayout();
}

std::string CocosSceneUtils::GetStrippedFileName(const std::string& sFileNameWithPath)
{
	std::string strippedfile = sFileNameWithPath;
	int indexOfSlash = sFileNameWithPath.find_last_of('/');
	int indexOfBackslash = sFileNameWithPath.find_last_of('\\');

	if (indexOfBackslash != std::string::npos)
	{
		strippedfile = sFileNameWithPath.substr(indexOfBackslash + 1);
	}
	else if (indexOfSlash != std::string::npos)
	{
		strippedfile = sFileNameWithPath.substr(indexOfSlash + 1);
	}

	return strippedfile;
}

bool CocosSceneUtils::RenameFileExtn(std::string& sFileNameWithExtn, const std::string& sExtnToChange, const std::string& sChangeToExtn)
{
	std::string::size_type pos = sFileNameWithExtn.rfind(sExtnToChange);
	if (pos != std::string::npos)
	{
		sFileNameWithExtn.replace(pos, sFileNameWithExtn.length(), sChangeToExtn);
		return true;
	}

	return false;
}

cocos2d::Node*	CocosSceneUtils::AllocateElementOfType(std::string sCSB, CocosView* pParentCocosView, cocos2d::Size& sContentSize)
{
	assert(pParentCocosView != NULL);

	if (pParentCocosView != NULL)
	{
		auto* pNode = cocos2d::CSLoader::createNode(sCSB, CC_CALLBACK_1(CocosView::OnNodeLoaded, pParentCocosView));

		if (pNode != NULL)
		{
			pNode->setContentSize(sContentSize);
			assert(pNode != NULL);

			return pNode;
		}
	}

	return NULL;
}

bool CocosSceneUtils::IsImageAvailable(std::string iconName, std::string directoryName)
{
	std::string filename = directoryName + iconName;
	if (SpriteFrameCache::getInstance()->getSpriteFrameByName(filename))
	{
		return true;
	}

	return false;
}

cocos2d::Size CocosSceneUtils::GetVisibleSize()
{
	cocos2d::Director* pDirector = cocos2d::Director::getInstance();
	return pDirector->getVisibleSize();
}

cocos2d::Vec2 CocosSceneUtils::GetWorldSpacePos(cocos2d::Node* pNode)
{
	return pNode->getParent()->convertToWorldSpace(pNode->getPosition());
}

cocos2d::Node* CocosSceneUtils::CreateRootNodeAtOrigin(cocos2d::Node* pParentNode)
{
	cocos2d::Node* pNode_Root = cocos2d::Node::create();
	pNode_Root->setPosition(0, 0);
	pParentNode->addChild(pNode_Root);

	return pNode_Root;
}

cocos2d::Layer* CocosSceneUtils::CreateRootLayerAtOrigin(cocos2d::Node* pParentNode)
{
	cocos2d::Layer* pLayer_Root = cocos2d::Layer::create();
	pLayer_Root->setContentSize(pParentNode->getContentSize());
	pLayer_Root->setPosition(0, 0);
	pParentNode->addChild(pLayer_Root);

	return pLayer_Root;
}

cocos2d::TargetedAction* CocosSceneUtils::CreateTargetedAction_MoveTo(cocos2d::Node* pNode, const cocos2d::Vec2& vDestPos, float fDuration)
{
	auto action_MoveTo = MoveTo::create(fDuration, vDestPos);
	cocos2d::TargetedAction* pTargetedAction = cocos2d::TargetedAction::create(pNode, action_MoveTo);

	return pTargetedAction;
}

cocos2d::TargetedAction* CocosSceneUtils::CreateAnimatedActionWithSpriteFrames(cocos2d::Node* pParent, const std::string& sFrameName, int32_t iMaxFrameCount, float fDuration, const cocos2d::Vec2& vAnchor, const cocos2d::Vec2& vPosition, float fAngle, float fOpacity/* = 255.0f*/)
{
	SpriteFrameCache* pSpriteFrameCache = SpriteFrameCache::getInstance();
	{
		cocos2d::Vector<cocos2d::SpriteFrame*> vAnimFrames;
		std::string sAnimFrameName = "";
		for (int32_t i = 1; i <= iMaxFrameCount; i++)
		{
			//sAnimFrameName.sprintf(sFrameName.c_str(), i);
			cocos2d::SpriteFrame* pSpriteFrame = pSpriteFrameCache->getSpriteFrameByName(sAnimFrameName);
			vAnimFrames.pushBack(pSpriteFrame);
		}

		auto pSprite = Sprite::createWithSpriteFrame(vAnimFrames.front());
		pSprite->setVisible(true);
		pSprite->setAnchorPoint(vAnchor);
		pSprite->setOpacity(fOpacity);
		pSprite->setPosition(0, 0);
		pSprite->setBlendFunc(BlendFunc::ADDITIVE);
		pSprite->setRotation(fAngle);
		pParent->addChild(pSprite);

		Animation* pAnimation = Animation::createWithSpriteFrames(vAnimFrames, fDuration);
		Animate* pAnimate = Animate::create(pAnimation);

		cocos2d::TargetedAction* pTargetedAction = cocos2d::TargetedAction::create(pSprite, pAnimate);

		return pTargetedAction;
	}
}

cocos2d::ui::ImageView* CocosSceneUtils::CreateImageNode(cocos2d::Node* pParentNode, const std::string& sTexture, cocos2d::ui::Widget::TextureResType eTextureResType, const Vec2& vAnchor, const Vec2& vPosition, float fOpacity)
{
	cocos2d::ui::ImageView* pImageViewCenter = cocos2d::ui::ImageView::create();
	{
		pImageViewCenter->loadTexture(sTexture, eTextureResType);
		pImageViewCenter->setAnchorPoint(vAnchor);
		pImageViewCenter->setPosition(vPosition);
		pImageViewCenter->setOpacity(fOpacity);
	}

	pParentNode->addChild(pImageViewCenter);

	return pImageViewCenter;
}

cocos2d::TargetedAction* CocosSceneUtils::CreateTargetedAction_FadeIn(cocos2d::Node* pNode, float fDuration)
{
	auto action_FadeIn = FadeIn::create(fDuration);
	cocos2d::TargetedAction* pTargetedAction = cocos2d::TargetedAction::create(pNode, action_FadeIn);

	return pTargetedAction;
}

cocos2d::TargetedAction* CocosSceneUtils::CreateTargetedAction_FadeOut(cocos2d::Node* pNode, float fDuration)
{
	auto action_FadeOut = FadeOut::create(fDuration);
	cocos2d::TargetedAction* pTargetedAction = cocos2d::TargetedAction::create(pNode, action_FadeOut);

	return pTargetedAction;
}

cocos2d::TargetedAction* CocosSceneUtils::CreateTargetedAction_FadeTo(cocos2d::Node* pNode, float fDuration, int8_t iOpacity)
{
	auto action_FadeTo = FadeTo::create(fDuration, iOpacity);
	cocos2d::TargetedAction* pTargetedAction = cocos2d::TargetedAction::create(pNode, action_FadeTo);

	return pTargetedAction;
}

cocos2d::TargetedAction*	CocosSceneUtils::CreateTargetedAction_ScaleTo(cocos2d::Node* pNode, float fDuration, float fScaleX, float fScaleY)
{
	auto action_ScaleTo = cocos2d::ScaleTo::create(fDuration, fScaleX, fScaleY);
	cocos2d::TargetedAction* pTargetedAction = cocos2d::TargetedAction::create(pNode, action_ScaleTo);

	return pTargetedAction;
}

void CocosSceneUtils::CalcAndRotate(cocos2d::Node* pNode, const cocos2d::Vec2& vPoint1, const cocos2d::Vec2& vPoint2)
{
	pNode->setRotation(GetRotationInDegrees(vPoint1, vPoint2));
}

float_t CocosSceneUtils::GetRotationInDegrees(const cocos2d::Vec2& vPoint1, const cocos2d::Vec2& vPoint2)
{
	cocos2d::Vec2 pos = vPoint2 - vPoint1;
	float fAngle = atan2f(pos.y, pos.x);
	fAngle = -CC_RADIANS_TO_DEGREES(fAngle);
			
	return fAngle;
}

cocos2d::Sequence* CocosSceneUtils::CreateSequence(cocos2d::FiniteTimeAction* pFiniteTimeAction, ...)
{
	cocos2d::FiniteTimeAction* pAction = NULL;
	cocos2d::Vector<cocos2d::FiniteTimeAction*> vActions;
	vActions.reserve(8);

	va_list vList;
	va_start(vList, pFiniteTimeAction);
	pAction = va_arg(vList, cocos2d::FiniteTimeAction*);
	while (pAction)
	{	
		if (pAction)
			vActions.pushBack(pAction);

		pAction = va_arg(vList, cocos2d::FiniteTimeAction*);
	}
	va_end(vList);

	cocos2d::Sequence* pSequence = cocos2d::Sequence::create(vActions);
	return pSequence;
}

/*
(Widget In CocosStudio)->Properties->Advanced -> "User Data" will take the following form :
==> accessibilityTag:play;property1:value1;property2:value2 ...
The string might or might not end with a ';'
*/
bool CocosSceneUtils::HasCustomProperties(cocos2d::Node* pNode, std::string& sUserDataOUT)
{
	sUserDataOUT = "";
	if (pNode != NULL)
	{
		cocostudio::ComExtensionData* pComExtensionData = dynamic_cast<cocostudio::ComExtensionData*>(pNode->getComponent(cocostudio::ComExtensionData::COMPONENT_NAME));
		if (pComExtensionData)
		{
			sUserDataOUT = pComExtensionData->getCustomProperty();
			//sUserDataOUT.trim();

			return (sUserDataOUT.length() > 0);
		}
	}

	return false;
}

bool CocosSceneUtils::GetCustomPropertyValue(cocos2d::Node* pNode, const std::string& sPropertyNameIN, std::string& sPropertyValueOUT)
{
	std::string sUserData = "";
	bool bReturn = false;
	if (HasCustomProperties(pNode, sUserData))
	{	
		int32_t iStartPos = 0;
		int32_t iSemicolonPos = sUserData.find(";");
		iSemicolonPos = (iSemicolonPos < 0) ? sUserData.length() : iSemicolonPos;

		while (iSemicolonPos > 0)
		{
			std::string sUserDataTag;
			sUserDataTag = sUserData.substr(iStartPos, (iSemicolonPos - iStartPos));
			{
				int32_t iColonPos = sUserDataTag.find(":");

				std::string sTagName = sUserDataTag.substr(0, iColonPos);
				sPropertyValueOUT = sUserDataTag.substr(iColonPos + 1);

				bReturn = (sTagName.length() > 0 && sTagName == sPropertyNameIN);
				if (bReturn)
				{
					break;
				}

				iStartPos = iSemicolonPos + 1;
				iSemicolonPos = sUserData.find(";", iStartPos);

				// Sanity check if the "User Data" string doesn't end with a ';'
				if (iSemicolonPos < 0)
				{
					iSemicolonPos = (iStartPos < sUserDataTag.length()) ? sUserDataTag.length() : -1;
				}
			}
		}
	}

	return bReturn;
}

cocos2d::Node* CocosSceneUtils::ThemedUI_ReadParameterOverrides(cocos2d::Node* pNode/*, JsonDomObject* pJsonDomObject*/)
{
	//assert(pJsonDomObject != NULL);
	//if (pJsonDomObject != NULL)
	//{	
	//	std::string sString = "";
	//	JSON_TRY_GET_STRING("nodeName", sString, pJsonDomObject)
	//
	//	pNode = ThemedUI_ReadNodeAttributes(pNode, pJsonDomObject);
	//	ThemedUI_ReadCustomNodeAttributes(pNode, pJsonDomObject);
	//	ThemedUI_AddDynamicChildNodes(pNode, pJsonDomObject);
	//}

	return pNode;
}

cocos2d::Node* CocosSceneUtils::ThemedUI_ReadNodeAttributes(cocos2d::Node* pNode/*, JsonDomObject* pJsonDomObject*/)
{
	/*
	CoefficientsManager& pCoefficientsManager = CoefficientsManager::Get();
	if (pCoefficientsManager.DoesVariableExist(CHAR16("attributes"), pJsonDomObject))
	{
		JsonDomObject* pJsonDomObject_Attributes = pCoefficientsManager.GetJsonDomObject(CHAR16("attributes"), pJsonDomObject);

		assert(pJsonDomObject_Attributes != NULL);
		if (pJsonDomObject_Attributes != NULL)
		{
			std::string sStringValue = "";
			int32_t iIntValue = 0;

			if (pCoefficientsManager.DoesVariableExist(EA_CHAR16("FileNodeResource"), pJsonDomObject_Attributes))
			{
				JSON_TRY_GET_STRING("FileNodeResource", sStringValue, pJsonDomObject_Attributes)
				{
					pNode = cocos2d::CSLoader::createNode(sStringValue);
				}

				if (pCoefficientsManager.DoesVariableExist(EA_CHAR16("AnimName"), pJsonDomObject_Attributes))
				{
					std::string sCSBName = sStringValue;
					JSON_TRY_GET_STRING("AnimName", sStringValue, pJsonDomObject_Attributes)
					{
						cocostudio::timeline::ActionTimeline* pActionTimeline = nullptr;
						CREATE_ACTION_AND_RETAIN(sCSBName, pActionTimeline);
						CALL_COCOS_ANIMATION_BY_ACTION_WITHOUT_CALLBACK(pNode, pActionTimeline, sStringValue, false);
					}
				}
			}

			if (pCoefficientsManager.DoesVariableExist(EA_CHAR16("Visibility"), pJsonDomObject_Attributes))
			{
				bool bBoolValue = false;
				JSON_TRY_GET_BOOL("Visibility", bBoolValue, pJsonDomObject_Attributes)
				{
					//pNode->setThemeParameter(THEMED_VISIBILITY, &bBoolValue);
					ThemedUI_SetThemeParameter(pNode, THEMED_VISIBILITY, &bBoolValue);
				}
			}

			if (pCoefficientsManager.DoesVariableExist(EA_CHAR16("RealColour"), pJsonDomObject_Attributes))
			{
				JSON_TRY_GET_STRING("RealColour", sStringValue, pJsonDomObject_Attributes)
				{
					ThemedUI_SetThemeParameter(pNode, THEMED_REAL_COLOUR, (void*)sStringValue.c_str());
				}
			}

			if (pCoefficientsManager.DoesVariableExist(EA_CHAR16("Opacity"), pJsonDomObject_Attributes))
			{
				JSON_TRY_GET_STRING("Opacity", sStringValue, pJsonDomObject_Attributes)
				{
					ThemedUI_SetThemeParameter(pNode, THEMED_OPACITY, (void*)sStringValue.c_str());
				}
			}

			if (pCoefficientsManager.DoesVariableExist(EA_CHAR16("Anchor"), pJsonDomObject_Attributes))
			{
				JSON_TRY_GET_STRING("Anchor", sStringValue, pJsonDomObject_Attributes)
				{
					ThemedUI_SetThemeParameter(pNode, THEMED_ANCHOR, (void*)sStringValue.c_str());
				}
			}

			if (pCoefficientsManager.DoesVariableExist(EA_CHAR16("Position"), pJsonDomObject_Attributes))
			{
				JSON_TRY_GET_STRING("Position", sStringValue, pJsonDomObject_Attributes)
				{
					ThemedUI_SetThemeParameter(pNode, THEMED_POSITION, (void*)sStringValue.c_str());
				}
			}

			if (pCoefficientsManager.DoesVariableExist(EA_CHAR16("Rotation"), pJsonDomObject_Attributes))
			{
				JSON_TRY_GET_INT("Rotation", iIntValue, pJsonDomObject_Attributes)
				{
					ThemedUI_SetThemeParameter(pNode, THEMED_ROTATION, (void*)&iIntValue);
				}
			}

			if (pCoefficientsManager.DoesVariableExist(EA_CHAR16("ContentSize"), pJsonDomObject_Attributes))
			{
				JSON_TRY_GET_STRING("ContentSize", sStringValue, pJsonDomObject_Attributes)
				{
					ThemedUI_SetThemeParameter(pNode, THEMED_CONTENTSIZE, (void*)sStringValue.c_str());
				}
			}

			if (pCoefficientsManager.DoesVariableExist(EA_CHAR16("Scale"), pJsonDomObject_Attributes))
			{
				JSON_TRY_GET_STRING("Scale", sStringValue, pJsonDomObject_Attributes)
				{
					ThemedUI_SetThemeParameter(pNode, THEMED_SCALING, (void*)sStringValue.c_str());
				}
			}

			if (pCoefficientsManager.DoesVariableExist(EA_CHAR16("Texture"), pJsonDomObject_Attributes))
			{
				JSON_TRY_GET_STRING("Texture", sStringValue, pJsonDomObject_Attributes)
				{
					ThemedUI_SetThemeParameter(pNode, THEMED_TEXTURE, (void*)sStringValue.c_str());
				}
			}

			if (pCoefficientsManager.DoesVariableExist(EA_CHAR16("FontResource"), pJsonDomObject_Attributes))
			{
				JSON_TRY_GET_STRING("FontResource", sStringValue, pJsonDomObject_Attributes)
				{
					ThemedUI_SetThemeParameter(pNode, THEMED_TEXT_FONTRESOURCE, (void*)sStringValue.c_str());
				}
			}

			if (pCoefficientsManager.DoesVariableExist(EA_CHAR16("FontSize"), pJsonDomObject_Attributes))
			{
				JSON_TRY_GET_INT("FontSize", iIntValue, pJsonDomObject_Attributes)
				{
					ThemedUI_SetThemeParameter(pNode, THEMED_TEXT_FONTSIZE, (void*)&iIntValue);
				}
			}

			if (pCoefficientsManager.DoesVariableExist(EA_CHAR16("Text"), pJsonDomObject_Attributes))
			{
				JSON_TRY_GET_STRING("Text", sStringValue, pJsonDomObject_Attributes)
				{
					ThemedUI_SetThemeParameter(pNode, THEMED_TEXT_FONTTEXT, (void*)sStringValue.c_str());
				}
			}

			if (pCoefficientsManager.DoesVariableExist(EA_CHAR16("FontColour"), pJsonDomObject_Attributes))
			{
				JSON_TRY_GET_STRING("FontColour", sStringValue, pJsonDomObject_Attributes)
				{
					ThemedUI_SetThemeParameter(pNode, THEMED_TEXT_FONTCOLOUR, (void*)sStringValue.c_str());
				}
			}

			if (pCoefficientsManager.DoesVariableExist(EA_CHAR16("FontAlign"), pJsonDomObject_Attributes))
			{
				JSON_TRY_GET_STRING("FontAlign", sStringValue, pJsonDomObject_Attributes)
				{
					ThemedUI_SetThemeParameter(pNode, THEMED_TEXT_FONTALIGN, (void*)sStringValue.c_str());
				}
			}

			if (pCoefficientsManager.DoesVariableExist(EA_CHAR16("TextArea"), pJsonDomObject_Attributes))
			{
				JSON_TRY_GET_STRING("TextArea", sStringValue, pJsonDomObject_Attributes)
				{
					ThemedUI_SetThemeParameter(pNode, THEMED_TEXT_FONTTEXTAREA, (void*)sStringValue.c_str());
				}
			}

			if (pCoefficientsManager.DoesVariableExist(EA_CHAR16("UserData"), pJsonDomObject_Attributes))
			{
				JSON_TRY_GET_STRING("UserData", sStringValue, pJsonDomObject_Attributes)
				{
					ThemedUI_SetThemeParameter(pNode, THEMED_USERDATA, (void*)sStringValue.c_str());
				}
			}
		}
	}
	*/
	return pNode;
}

void CocosSceneUtils::ThemedUI_ReadCustomNodeAttributes(cocos2d::Node* pNode/*, JsonDomObject* pJsonDomObject*/)
{
	//CoefficientsManager& pCoefficientsManager = CoefficientsManager::Get();
	//if (pCoefficientsManager.DoesVariableExist(CHAR16("CustomNodeAttributes"), pJsonDomObject))
	//{
	//	JsonDomArray* pJsonDomArray_CutomNodeAttributes = pCoefficientsManager.GetJsonDomArray(CHAR16("CustomNodeAttributes"), pJsonDomObject);
	//
	//	assert(pJsonDomArray_CutomNodeAttributes != NULL);
	//	if (pJsonDomArray_CutomNodeAttributes != NULL)
	//	{
	//		uint32_t iArraySize = GameFoundation::Json::Util::GetSize(pJsonDomArray_CutomNodeAttributes);
	//		for (uint32_t i = 0; i < iArraySize; i++)
	//		{
	//			Json::JsonDomObject* pJsonDomChildObj = GameFoundation::Json::Util::GetObjectAtIndex(pJsonDomArray_CutomNodeAttributes, i);
	//			if (pJsonDomChildObj != NULL)
	//			{
	//				std::string sString = "";
	//				JSON_TRY_GET_STRING("nodeName", sString, pJsonDomChildObj)
	//
	//				if (sString.length() > 0)
	//				{
	//					cocos2d::Node* pChildNode = nullptr;
	//					pChildNode = dynamic_cast<cocos2d::Node*>(pNode->getChildByName(sString));
	//
	//					if (pChildNode != nullptr)
	//					{
	//						ThemedUI_ReadParameterOverrides(pChildNode, pJsonDomChildObj);
	//					}
	//				}
	//			}
	//		}
	//	}
	//}
}

void CocosSceneUtils::ThemedUI_AddDynamicChildNodes(cocos2d::Node* pParent/*, JsonDomObject* pJsonDomObject*/)
{
	//CoefficientsManager& pCoefficientsManager = CoefficientsManager::Get();
	//if (pCoefficientsManager.DoesVariableExist(CHAR16("DynamicChildren"), pJsonDomObject))
	//{
	//	JsonDomArray* pJsonDomArray_DynamicChildren = pCoefficientsManager.GetJsonDomArray(CHAR16("DynamicChildren"), pJsonDomObject);
	//
	//	assert(pJsonDomArray_DynamicChildren != NULL);
	//	if (pJsonDomArray_DynamicChildren != NULL)
	//	{
	//		uint32_t iArraySize = GameFoundation::Json::Util::GetSize(pJsonDomArray_DynamicChildren);
	//		std::string sClassType = "";
	//
	//		for (uint32_t i = 0; i < iArraySize; i++)
	//		{
	//			Json::JsonDomObject* pJsonDomChildObj = GameFoundation::Json::Util::GetObjectAtIndex(pJsonDomArray_DynamicChildren, i);
	//			if (pJsonDomChildObj != NULL)
	//			{
	//				if (pCoefficientsManager.DoesVariableExist(CHAR16("ClassType"), pJsonDomChildObj))
	//				{
	//					JSON_TRY_GET_STRING("ClassType", sClassType, pJsonDomChildObj)
	//
	//					cocos2d::Node* pDynamicChildNode = nullptr;
	//					if (sClassType == "ImageView")
	//					{
	//						pDynamicChildNode = cocos2d::ui::ImageView::create();
	//					}
	//					else
	//					if (sClassType == "Layout")
	//					{
	//						pDynamicChildNode = cocos2d::ui::Layout::create();
	//					}
	//					else
	//					if (sClassType == "Text")
	//					{
	//						pDynamicChildNode = cocos2d::ui::Text::create();
	//					}
	//
	//					pDynamicChildNode = ThemedUI_ReadParameterOverrides(pDynamicChildNode, pJsonDomChildObj);
	//					pParent->addChild(pDynamicChildNode);
	//				}
	//			}
	//		}
	//	}
	//}
}

void CocosSceneUtils::ThemedUI_SetThemeParameter(cocos2d::Node* pNode, THEMED_PARAMETER eTHEMED_PARAMETER, void* pValue)
{
	bool* boolPtr = static_cast<bool*>(pValue);
	int32_t* int32_tPtr = static_cast<int32_t*>(pValue);
	const std::string sString = (char*)pValue;

	auto layoutComponent = cocos2d::ui::LayoutComponent::bindLayoutComponent(pNode);
	layoutComponent->setActiveEnabled(true);

	switch (eTHEMED_PARAMETER)
	{
		case THEMED_POSITION:
		case THEMED_SCALING:
		case THEMED_ROTATION:
		case THEMED_ANCHOR:
		case THEMED_VISIBILITY:
		case THEMED_REAL_COLOUR:
		case THEMED_OPACITY:
			ThemedUI_HandleNode(pNode, eTHEMED_PARAMETER, pValue);
		break;
		case THEMED_CONTENTSIZE:
			ThemedUI_HandleWidget(pNode, eTHEMED_PARAMETER, pValue);
		break;
		case THEMED_TEXTURE:
			ThemedUI_HandleSprite(pNode, eTHEMED_PARAMETER, pValue);
		break;
		case THEMED_TEXT_FONTRESOURCE:
		case THEMED_TEXT_FONTSIZE:
		case THEMED_TEXT_FONTTEXT:
		case THEMED_TEXT_FONTCOLOUR:
		case THEMED_TEXT_FONTALIGN:
		case THEMED_TEXT_FONTTEXTAREA:
			ThemedUI_HandleText(pNode, eTHEMED_PARAMETER, pValue);
		break;
		case THEMED_USERDATA:
			ThemedUI_UserData(pNode, eTHEMED_PARAMETER, pValue);
		break;
	}
}

void CocosSceneUtils::ThemedUI_HandleNode(cocos2d::Node* pNode, THEMED_PARAMETER eTHEMED_PARAMETER, void* pValue)
{
	bool* boolPtr = static_cast<bool*>(pValue);
	int32_t* int32_tPtr = static_cast<int32_t*>(pValue);
	const std::string sString = (char*)pValue;

	auto layoutComponent = cocos2d::ui::LayoutComponent::bindLayoutComponent(pNode);
	layoutComponent->setActiveEnabled(true);

	switch (eTHEMED_PARAMETER)
	{
		case THEMED_POSITION:
		{
			cocos2d::Vec2 vPosition;
			char sBuf[4];
			sscanf_s(sString.c_str(), "%s %f, %f", sBuf, &vPosition.x, &vPosition.y);

			if (strcmp(sBuf, "PER") == 0)
			{
				cocos2d::Vec2 vOriginalPositionPercent = cocos2d::Vec2(layoutComponent->getPositionPercentX(), layoutComponent->getPositionPercentY());
				if (vPosition.x < 0.0f)
					vPosition.x = vOriginalPositionPercent.x;
				if (vPosition.y < 0.0f)
					vPosition.y = vOriginalPositionPercent.y;

				layoutComponent->setPositionPercentXEnabled(true);
				layoutComponent->setPositionPercentYEnabled(true);

				layoutComponent->setPositionPercentX(vPosition.x);
				layoutComponent->setPositionPercentY(vPosition.y);
			}
			else
			if (strcmp(sBuf, "ABS") == 0)
			{
				cocos2d::Vec2 vOriginalPosition = pNode->getPosition();
				if (vPosition.x < 0.0f)
					vPosition.x = vOriginalPosition.x;
				if (vPosition.y < 0.0f)
					vPosition.y = vOriginalPosition.y;

				layoutComponent->setPositionPercentXEnabled(false);
				layoutComponent->setPositionPercentYEnabled(false);

				layoutComponent->setPosition(vPosition);
			}
			else
			{
				assert("Invalid Position Type specified.");
			}

			ui::Helper::doLayout(pNode);
		}
		break;
		case THEMED_CONTENTSIZE:
		{
			cocos2d::ui::Widget* pWidget = dynamic_cast<cocos2d::ui::Widget*>(pNode);
			cocos2d::Size vSize;
			char sBuf[4];
			sscanf_s(sString.c_str(), "%s %f, %f", sBuf, &vSize.width, &vSize.height);

			if (strcmp(sBuf, "PER") == 0)
			{
				cocos2d::Vec2 vOriginalSizePercent = layoutComponent->getPercentContentSize();

				if (vSize.width < 0.0f)
					vSize.width = vOriginalSizePercent.x;
				if (vSize.height < 0.0f)
					vSize.height = vOriginalSizePercent.y;

				layoutComponent->setPercentWidthEnabled(true);
				layoutComponent->setPercentHeightEnabled(true);

				layoutComponent->setPercentContentSize(vSize);
			}
			else
			if (strcmp(sBuf, "ABS") == 0)
			{
				cocos2d::Size vOriginalSize = pNode->getContentSize();

				if (vSize.width < 0.0f)
					vSize.width = vOriginalSize.width;
				if (vSize.height < 0.0f)
					vSize.height = vOriginalSize.height;

				layoutComponent->setPercentWidthEnabled(false);
				layoutComponent->setPercentHeightEnabled(false);

				layoutComponent->setSize(vSize);
			}
			else
			{
				assert("Invalid Size Type specified.");
			}

			ui::Helper::doLayout(pNode);
		}
		break;
		case THEMED_SCALING:
		{
			float_t fScaleX = 1.0f, fScaleY = 1.0f;
			sscanf_s(sString.c_str(), "%f, %f", &fScaleX, &fScaleY);

			if (fScaleX > 0.0f)
				pNode->setScaleX(fScaleX);
			if (fScaleY > 0.0f)
				pNode->setScaleY(fScaleY);
		}
		break;
		case THEMED_ROTATION:
		{
			int32_t iAngle = *int32_tPtr;
			pNode->setRotation(iAngle);
		}
		break;
		case THEMED_ANCHOR:
		{
			cocos2d::Vec2 vAnchor;
			sscanf_s(sString.c_str(), "%f, %f", &vAnchor.x, &vAnchor.y);

			cocos2d::Vec2 vOriginalAnchor = pNode->getAnchorPoint();
			if (vAnchor.x < 0.0f)
				vAnchor.x = vOriginalAnchor.x;
			if (vAnchor.y < 0.0f)
				vAnchor.y = vOriginalAnchor.y;

			pNode->setAnchorPoint(vAnchor);
		}
			break;
		case THEMED_VISIBILITY:
			if (boolPtr != NULL)
			{
				pNode->setVisible((*boolPtr));
			}
		break;
		case THEMED_REAL_COLOUR:
		{
			int32_t byteRed = 255, byteGreen = 255, byteBlue = 255;
			sscanf_s(sString.c_str(), "%d, %d, %d", &byteRed, &byteGreen, &byteBlue);

			cocos2d::Color3B originalColor = pNode->getColor();
			if (byteRed < 0)
				byteRed = originalColor.r;
			if (byteGreen < 0)
				byteGreen = originalColor.g;
			if (byteBlue < 0)
				byteBlue = originalColor.b;

			Color3B cRealColor = cocos2d::Color3B((GLubyte)byteRed, (GLubyte)byteGreen, (GLubyte)byteBlue);
			pNode->setColor(cRealColor);
		}
		break;
		case THEMED_OPACITY:
		{
			uint32_t byteOpacity = 255;
			sscanf_s(sString.c_str(), "0x%x", &byteOpacity);
			pNode->setOpacity((GLubyte)byteOpacity);
		}
		break;
	}
}

void CocosSceneUtils::ThemedUI_HandleWidget(cocos2d::Node* pNode, THEMED_PARAMETER eTHEMED_PARAMETER, void* pValue)
{
	bool* boolPtr = static_cast<bool*>(pValue);
	int32_t* int32_tPtr = static_cast<int32_t*>(pValue);
	const std::string sString = (char*)pValue;

	auto layoutComponent = cocos2d::ui::LayoutComponent::bindLayoutComponent(pNode);
	layoutComponent->setActiveEnabled(true);

	bool bPassToParent = true;

	switch (eTHEMED_PARAMETER)
	{
		case THEMED_CONTENTSIZE:
		{
			cocos2d::ui::Widget* pWidget = dynamic_cast<cocos2d::ui::Widget*>(pNode);
			cocos2d::Size vSize;
			char sBuf[4];
			sscanf_s(sString.c_str(), "%s %f, %f", sBuf, &vSize.width, &vSize.height);

			if (pWidget != NULL)
			{
				pWidget->setUnifySizeEnabled(false);
				pWidget->ignoreContentAdaptWithSize(false);

				pWidget->setLayoutComponentEnabled(true);

				if (strcmp(sBuf, "PER") == 0)
				{
					cocos2d::Vec2 vOriginalSizePercent = pWidget->getSizePercent();

					if (vSize.width < 0.0f)
						vSize.width = vOriginalSizePercent.x;
					if (vSize.height < 0.0f)
						vSize.height = vOriginalSizePercent.y;

					pWidget->setSizeType(cocos2d::ui::Widget::SizeType::PERCENT);
					pWidget->setSizePercent(vSize);
				}
				else
				if (strcmp(sBuf, "ABS") == 0)
				{
					cocos2d::Size vOriginalSize = pWidget->getContentSize();

					if (vSize.width < 0.0f)
						vSize.width = vOriginalSize.width;
					if (vSize.height < 0.0f)
						vSize.height = vOriginalSize.height;

					pWidget->setSizeType(cocos2d::ui::Widget::SizeType::ABSOLUTE);
					pWidget->setContentSize(vSize);
				}
				else
				{
					assert("Invalid Size Type specified.");
				}

				bPassToParent = false;
			}
		}
	}

	if (bPassToParent)
		ThemedUI_HandleNode(pNode, eTHEMED_PARAMETER, pValue);
}

void CocosSceneUtils::ThemedUI_HandleLayout(cocos2d::Node* pNode, THEMED_PARAMETER eTHEMED_PARAMETER, void* pValue)
{
	switch (eTHEMED_PARAMETER)
	{
		case THEMED_TEXTURE:
		{
			cocos2d::ui::Layout* pLayout = dynamic_cast<cocos2d::ui::Layout*>(pNode);
			if (pLayout != NULL)
			{
				std::string sTextureName = (char*)pValue;
				pLayout->setBackGroundImage(sTextureName, cocos2d::ui::Widget::TextureResType::PLIST);
			}
		}
	}
}

void CocosSceneUtils::ThemedUI_HandleImageView(cocos2d::Node* pNode, THEMED_PARAMETER eTHEMED_PARAMETER, void* pValue)
{
	bool bPassOn = true;

	switch (eTHEMED_PARAMETER)
	{
		case THEMED_TEXTURE:
		{
			cocos2d::ui::ImageView* pImageView = dynamic_cast<cocos2d::ui::ImageView*>(pNode);
			if (pImageView != NULL)
			{
				std::string sTextureName = (char*)pValue;
				pImageView->loadTexture(sTextureName, cocos2d::ui::Widget::TextureResType::PLIST);
				bPassOn = false;
			}
		}
	}

	if (bPassOn)
		ThemedUI_HandleLayout(pNode, eTHEMED_PARAMETER, pValue);
}

void CocosSceneUtils::ThemedUI_HandleSprite(cocos2d::Node* pNode, THEMED_PARAMETER eTHEMED_PARAMETER, void* pValue)
{
	bool bPassOn = true;

	switch (eTHEMED_PARAMETER)
	{
		case THEMED_TEXTURE:
		{
			cocos2d::Sprite* pSprite = dynamic_cast<cocos2d::Sprite*>(pNode);
			if (pSprite != NULL)
			{
				std::string sTextureName = (char*)pValue;
				cocos2d::Size sSize = pSprite->getContentSize();

				pSprite->setTexture(sTextureName, cocos2d::ui::Widget::TextureResType::PLIST);
						
				cocos2d::Rect rTextureRect = pSprite->getTextureRect();
				float fScaleX = sSize.width / rTextureRect.size.width;
				float fScaleY = sSize.height / rTextureRect.size.height;
				pSprite->setScale(fScaleX, fScaleY);

				bPassOn = false;
			}
		}
	}

	if (bPassOn)
		ThemedUI_HandleImageView(pNode, eTHEMED_PARAMETER, pValue);
}

void CocosSceneUtils::ThemedUI_HandleText(cocos2d::Node* pNode, THEMED_PARAMETER eTHEMED_PARAMETER, void* pValue)
{
	bool* boolPtr = static_cast<bool*>(pValue);
	int32_t* int32_tPtr = static_cast<int32_t*>(pValue);
	const std::string sString = (char*)pValue;

	switch (eTHEMED_PARAMETER)
	{
		case THEMED_TEXT_FONTRESOURCE:
		{
			//assert_MSG(sString.length() > 0, "No FontFile specified");
			if (sString.length() > 0)
			{
				cocos2d::ui::Text* pText = dynamic_cast<cocos2d::ui::Text*>(pNode);
				pText->setFontName(sString);
			}
		}
		break;
		case THEMED_TEXT_FONTSIZE:
		{
			cocos2d::ui::Text* pText = dynamic_cast<cocos2d::ui::Text*>(pNode);
			int32_t* iFontSize = static_cast<int32_t*>(pValue);
			pText->setFontSize(*iFontSize);
		}
		break;
		case THEMED_TEXT_FONTTEXT:
		{
			cocos2d::ui::Text* pText = dynamic_cast<cocos2d::ui::Text*>(pNode);
			pText->setString(sString);
		}
		break;
		case THEMED_TEXT_FONTCOLOUR:
		{
			cocos2d::ui::Text* pText = dynamic_cast<cocos2d::ui::Text*>(pNode);
			int32_t byteRed = 255, byteGreen = 255, byteBlue = 255, byteAlpha = 255;
			sscanf_s(sString.c_str(), "%d, %d, %d, %d", &byteRed, &byteGreen, &byteBlue, &byteAlpha);
					
			cocos2d::Color4B originalColor = pText->getTextColor();
			if (byteRed < 0)
				byteRed = originalColor.r;
			if (byteGreen < 0)
				byteGreen = originalColor.g;
			if (byteBlue < 0)
				byteBlue = originalColor.b;
			if (byteAlpha < 0)
				byteAlpha = originalColor.a;
			
			Color4B realCOlour = cocos2d::Color4B((GLubyte)byteRed, (GLubyte)byteGreen, (GLubyte)byteBlue, (GLubyte)byteAlpha);
			pText->setTextColor(realCOlour);
		}
		break;
		case  THEMED_TEXT_FONTALIGN:
		{
			char sHAlign[7];
			char sHAlignVal[6];
			char sVAlign[7];
			char sVAlignVal[6];
			sscanf_s(sString.c_str(), "%s %s %s %s",	&sHAlign,
													&sHAlignVal, 
													&sVAlign,
													&sVAlignVal);

			cocos2d::ui::Text* pText = dynamic_cast<cocos2d::ui::Text*>(pNode);
			if (strcmp(sHAlign, "HALIGN") == 0)
			{
				TextHAlignment hAlignment = TextHAlignment::CENTER;
				if (strcmp(sHAlignVal, "LEFT") == 0)
					hAlignment = TextHAlignment::LEFT;
				else
				if (strcmp(sHAlignVal, "CENTER") == 0)
					hAlignment = TextHAlignment::CENTER;
				else
				if (strcmp(sHAlignVal, "RIGHT") == 0)
					hAlignment = TextHAlignment::RIGHT;
				
				pText->setTextHorizontalAlignment(hAlignment);
			}

			if (strcmp(sVAlign, "VALIGN") == 0)
			{
				TextVAlignment vAlignment = TextVAlignment::CENTER;
				if (strcmp(sVAlignVal, "TOP") == 0)
					vAlignment = TextVAlignment::TOP;
				else
				if (strcmp(sVAlignVal, "CENTER") == 0)
					vAlignment = TextVAlignment::CENTER;
				else
				if (strcmp(sVAlignVal, "BOTTOM") == 0)
					vAlignment = TextVAlignment::BOTTOM;

				pText->setTextVerticalAlignment(vAlignment);
			}
		}
		break;
		case THEMED_TEXT_FONTTEXTAREA:
		{
			cocos2d::Size sAreaSize;
			sscanf_s(sString.c_str(), "%f, %f", &sAreaSize.width, &sAreaSize.height);
			cocos2d::ui::Text* pText = dynamic_cast<cocos2d::ui::Text*>(pNode);

			cocos2d::Size vOriginalSize = pText->getTextAreaSize();
			if (sAreaSize.width < 0.0f)
				sAreaSize.width = vOriginalSize.width;
			if (sAreaSize.height < 0.0f)
				sAreaSize.height = vOriginalSize.height;

			pText->setTextAreaSize(sAreaSize);
		}
		break;
	}
}

void CocosSceneUtils::ThemedUI_UserData(cocos2d::Node* pNode, THEMED_PARAMETER eTHEMED_PARAMETER, void* pValue)
{
	bool* boolPtr = static_cast<bool*>(pValue);
	int32_t* int32_tPtr = static_cast<int32_t*>(pValue);
	const std::string sString = (char*)pValue;

	if (eTHEMED_PARAMETER == THEMED_USERDATA)
	{
		cocostudio::ComExtensionData* pThisComExtensionData = dynamic_cast<cocostudio::ComExtensionData*>(pNode->getComponent("ComExtensionData"));
		if (pThisComExtensionData == nullptr && sString.length() > 0)
		{
			pThisComExtensionData = cocostudio::ComExtensionData::create();
			pNode->addComponent(pThisComExtensionData);
		}

		if (pThisComExtensionData)
		{
			pThisComExtensionData->setCustomProperty(sString);
			//GameMessaging::GetServer().MessageSend(eIdCocosApplyUserData, (void*)pNode);
		}
	}
}

bool CocosSceneUtils::ReadCustomUIParameters(/*std::map<std::string, JsonDomObject*>& m_mapCustomUINodeParameters_NameObject, JsonDomObject* pJsonDomObject*/)
{
	/*
	bool bHasCustomUIParameters = CoefficientsManager::Get().DoesVariableExist(CHAR16("CustomNodeAttributesTag"), pJsonDomObject);
	if (bHasCustomUIParameters)
	{
		std::string16 sString16 = CHAR16("");
		std::string sString = "";
		JSON_TRY_GET_STRING("CustomNodeAttributesTag", sString, pJsonDomObject)
			ConvertString(sString, sString16);

		Json::JsonDomArray* pCustomNodesArray = CoefficientsManager::Get().GetJsonDomArray(CHAR16("CocosViews.CustomNodes"));
		if (pCustomNodesArray != NULL)
		{
			uint32_t iArraySize = GameFoundation::Json::Util::GetSize(pCustomNodesArray);

			for (uint32_t i = 0; i < iArraySize; i++)
			{
				Json::JsonDomObject* pJsonDomObject = GameFoundation::Json::Util::GetObjectAtIndex(pCustomNodesArray, i);
				if (pJsonDomObject != NULL)
				{
					if (CoefficientsManager::Get().DoesVariableExist(sString16, pJsonDomObject))
					{
						Json::JsonDomArray* pThisNodeArray = CoefficientsManager::Get().GetJsonDomArray(sString16, pJsonDomObject);
						if (pThisNodeArray != NULL)
						{
							uint32_t iArraySize = GameFoundation::Json::Util::GetSize(pThisNodeArray);

							for (uint32_t i = 0; i < iArraySize; i++)
							{
								Json::JsonDomObject* pJsonDomObject = GameFoundation::Json::Util::GetObjectAtIndex(pThisNodeArray, i);
								if (pJsonDomObject != NULL)
								{
									std::string sNodeName = "";
									JSON_TRY_GET_STRING("nodeName", sNodeName, pJsonDomObject);

									m_mapCustomUINodeParameters_NameObject[sNodeName] = pJsonDomObject;
								}
							}
						}
					}
				}
			}
		}
	}
			
	return bHasCustomUIParameters;
	*/
	return false;
}

void CocosSceneUtils::handleCustomUIParameters(CocosView* pCocosView, Node* pNode)
{
	//std::string sNodeName = ((cocos2d::Node*)pNode)->getName();
	//std::map<std::string, JsonDomObject*> m_pMapCustomUINodeParameters_NameObject = *pCocosView->GetCustomUIParameterMap();
	//
	//auto it = m_pMapCustomUINodeParameters_NameObject.find(sNodeName);
	//if (it != m_pMapCustomUINodeParameters_NameObject.end())
	//{
	//	JsonDomObject* pJsonDomObject = m_pMapCustomUINodeParameters_NameObject[sNodeName];
	//
	//	assert(pJsonDomObject != NULL);
	//	ThemedUI_ReadParameterOverrides((cocos2d::Node*)pNode, pJsonDomObject);
	//}
}

void CocosSceneUtils::handleCustomUIParameters(std::string& sViewName, Node* pNode)
{	
	CocosView* pCocosView = CocosSceneManager::Get().GetView("CocosSceneGame");
	handleCustomUIParameters(pCocosView, pNode);
}

void CocosSceneUtils::readAndApplyUserData(Node* pNode)
{
	std::map<std::string, std::string> mapUserDataOut;
	if (readUserData(pNode, mapUserDataOut))
	{
		applyUserData(pNode, mapUserDataOut);
	}
}

/*
(Widget In CocosStudio) -> Properties -> Advanced -> "User Data" will take the following form:
==> accessibilityTag:play;property1:value1;property2:value2 ...
The string might or might not end with a ';'
*/
bool CocosSceneUtils::readUserData(Node* pNode, std::map<std::string, std::string>& mapUserDataOut)
{
	bool bReturn = false;
	cocostudio::ComExtensionData* pComExtensionData = dynamic_cast<cocostudio::ComExtensionData*>(pNode->getComponent("ComExtensionData"));
	if (pComExtensionData)
	{
		auto pUserData = pComExtensionData->getCustomProperty();
		std::string sUserData = pUserData;
		//sUserData.sprintf("%s", pUserData.c_str());

		bReturn = readNameValuePairsFromString(sUserData, ";", ":", mapUserDataOut);
	}

	return bReturn;
}

bool CocosSceneUtils::readNameValuePairsFromString(std::string sInString, std::string sDelimiterMain, std::string sDelimiterNameValue, std::map<std::string, std::string>& mapUserDataOut)
{
	bool bReturn = false;

	std::string sTagValue;
	int32_t iTotalLength = sInString.length();

	int32_t iStartPos = 0;
	int32_t iSemicolonPos = sInString.find(sDelimiterMain);
	iSemicolonPos = (iSemicolonPos == std::string::npos) ? sInString.length() : iSemicolonPos;
	while (iSemicolonPos > 0)
	{
		std::string sUserDataTag;
		sUserDataTag = sInString.substr(iStartPos, (iSemicolonPos - iStartPos));
		{
			int32_t iColonPos = sUserDataTag.find(sDelimiterNameValue);

			std::string sTagName = sUserDataTag.substr(0, iColonPos);
			sTagValue = sUserDataTag.substr(iColonPos + 1);

			mapUserDataOut[sTagName] = sTagValue;

			iStartPos = iSemicolonPos + 1;
			iSemicolonPos = sInString.find(sDelimiterMain, iStartPos);

			// Sanity check if the "User Data" string doesn't end with a sDelimiter(eg: ';')
			if (iSemicolonPos < 0)
			{
				iSemicolonPos = (iStartPos < iTotalLength) ? iTotalLength : -1;
			}

			bReturn = true;
		}
	}

	return bReturn;
}

std::string CocosSceneUtils::getValueForNameFromMap(std::string sSearchName, std::map<std::string, std::string>& mapNameValuePair)
{
	std::string sReturnValue = "";
	std::map<std::string, std::string>::const_iterator itr = mapNameValuePair.begin();
	for (; itr != mapNameValuePair.end(); ++itr)
	{
		std::string sName = itr->first;
		std::string sValue = itr->second;

		if (strcmp(sName.c_str(), sSearchName.c_str()) == 0)
		{
			sReturnValue = sValue;
			break;
		}
	}

	return sReturnValue;
}

void CocosSceneUtils::applyUserData(Node* pNode, std::map<std::string, std::string>& mapUserDataOut)
{
	std::map<std::string, std::string>::const_iterator itr = mapUserDataOut.begin();
	for (; itr != mapUserDataOut.end(); ++itr)
	{
		std::string sName = itr->first;
		std::string sValue = itr->second;

		//if (sName.rfind("Json") != std::string::npos)
		//{
		//	bool b = true;
		//	std::string16 sName16, sValue16;
		//	ConvertString(sName, sName16);
		//	ConvertString(sValue, sValue16);
		//	JsonReaderContainer* newReaderContainer = JsonReaderContainer::CreateNewFromString(sValue16, sName16);
		//	if (newReaderContainer != NULL)
		//	{
		//		JsonDomObject* pJsonDomObject = newReaderContainer->GetReader()->GetRootObject();
		//		if (pJsonDomObject != NULL)
		//		{
		//		}
		//	}
		//}
		//else
		{
			if (sName.find("setParticleSystemPositionType") != std::string::npos)
			{
				if(sValue == "FREE")
					((cocos2d::ParticleSystemQuad*)pNode)->setPositionType(cocos2d::ParticleSystemQuad::PositionType::FREE);
				else
				if (sValue == "RELATIVE")
					((cocos2d::ParticleSystemQuad*)pNode)->setPositionType(cocos2d::ParticleSystemQuad::PositionType::RELATIVE);
				else
				if (sValue == "GROUPED")
					((cocos2d::ParticleSystemQuad*)pNode)->setPositionType(cocos2d::ParticleSystemQuad::PositionType::GROUPED);
			}

#ifdef USE_RETRO_SHADERS
			else
			if (sName.find("SetShaderUniforms") != std::string::npos)
			{
				GLProgramState *glState = nullptr;
				std::map<std::string, std::string> mapShaderData;
				if (readNameValuePairsFromString(sValue, "&", "=", mapShaderData))
				{
					std::map<std::string, std::string>::const_iterator itrShaderData = mapShaderData.begin();
					int32_t iCount = 0;

					auto it = mapShaderData.find("shaderProgram");
					if (it != mapShaderData.end())
					{
						std::string sShaderProgram = (it->second).c_str();
						const std::tuple<std::string, std::string>& tShaderProgram = CocosSceneManager::Get().GetShaderPrgramByName(sShaderProgram);

						std::string sVertexShader = std::get<0>(tShaderProgram);
						std::string sFragmentShader = std::get<1>(tShaderProgram);

						glState = GLProgramState::getOrCreateWithByteArrays(sShaderProgram.c_str(), sValue, sVertexShader.c_str(), sFragmentShader.c_str(), "");
					}

					for (; iCount < mapShaderData.size(); ++itrShaderData, iCount++)
					{
						std::string sSubName = itrShaderData->first;
						std::string sSubValue = itrShaderData->second;

						pNode->addCustomShaderUniform(sSubName.c_str(), sSubValue.c_str());

						if (sSubName.find("condition") != std::string::npos)
						{
							if (sSubValue == "u_IfRetroI")
							{
							}
						}
						else
						if (sSubName.rfind("2f") != std::string::npos)
						{
							Vec2 fValue;
							sscanf_s(sSubValue.c_str(), "%f, %f", &fValue.x, &fValue.y);
							glState->setUniformVec2(sSubName, fValue);
						}
						else
						if (sSubName.rfind("3f") != std::string::npos)
						{
							Vec3 fValue;
							sscanf_s(sSubValue.c_str(), "%f, %f, %f", &fValue.x, &fValue.y, &fValue.z);
							glState->setUniformVec3(sSubName, fValue);
						}
						else
						if (sSubName.rfind("4f") != std::string::npos)
						{
							Vec4 fValue;
							sscanf_s(sSubValue.c_str(), "%f,%f,%f,%f", &fValue.x, &fValue.y, &fValue.z, &fValue.w);
							glState->setUniformVec4(sSubName, fValue);
								
							glState->setUniformVec4(sSubName, fValue);
						}
						else
						if (sSubName.rfind("f") != std::string::npos)
						{
							float_t fValue = 0.0f;
							sscanf_s(sSubValue.c_str(), "%f", &fValue);
							glState->setUniformFloat(sSubName, fValue);
								
							glState->setUniformFloat(sSubName, fValue);
						}
						else
						if (sSubName.rfind("i") != std::string::npos)
						{
							int32_t iValue = 0;
							sscanf_s(sSubValue.c_str(), "%d", &iValue);
							glState->setUniformInt(sSubName, iValue);
						}
					}
				}

				pNode->setGLProgramState(glState);
			}
#endif
		}
	}
}