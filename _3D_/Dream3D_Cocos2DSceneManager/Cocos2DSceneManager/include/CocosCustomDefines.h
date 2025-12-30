#pragma once

#include <cocos2d.h>

#include "ui/CocosGUI.h"
#include "ActionTimeline/CCActionTimeline.h"
#include "editor-support/cocostudio/WidgetCallBackHandlerProtocol.h"
#include "editor-support/cocostudio/CocosStudioExport.h"
#include "editor-support/cocostudio/WidgetReader/NodeReader/NodeReader.h"
#include "editor-support/cocostudio/WidgetReader/NodeReaderDefine.h"
#include "CocosViewDefines.h"
#include "ICocosViewLifeCycle.h"

#define NOT					!
#define ICON_ERROR			"Common/Icon_Error.png"
#define ICON_SUCCESS		"Common/Icon_Success.png"
#define ICON_BATTLES		"Common/ActivityIcon_Battle.png"
#define ICON_TOURNAMENTS	"Common/ActivityIcon_Tournament.png"
#define ICON_SOCIAL			"Common/ActivityIcon_Social.png"
#define ICON_DEFAULT		""
#define ICON_SHARD			"Common/MinoShards.png"
#define ICON_CANCEL			"Common/Icon_Cancel.png"

#define TRY_GET_JSONVAL(__lval__, __json__, __typename__, __name__, __default__) \
__lval__ = (__json__.find(__name__) != __json__.end()) ? __json__[__name__].get<__typename__>() : __default__ \

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define DECLARE_CUSTOM_VIEW_READER(__classname__) \
class __classname__ : public cocostudio::NodeReader \
{ \
	DECLARE_CLASS_NODE_READER_INFO	\
	\
	public: \
	__classname__() { }; \
	~__classname__() { }; \
	\
	static __classname__* getInstance(); \
	\
	/** @deprecated Use method destroyInstance() instead */ \
	CC_DEPRECATED_ATTRIBUTE static void purge(); \
	static void destroyInstance(); \
	\
	cocos2d::Node* createNodeWithFlatBuffers(const flatbuffers::Table* nodeOptions); \
}; \
///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define DEFINE_CUSTOM_VIEW_READER(__classname__, __classnameReader__) \
		static __classnameReader__* instanceViewReader = nullptr; \
		IMPLEMENT_CLASS_NODE_READER_INFO(__classnameReader__) \
		__classnameReader__* __classnameReader__::getInstance() \
		{ \
				if (!instanceViewReader) \
				{  \
					instanceViewReader = new __classnameReader__(); \
				} \
				return instanceViewReader; \
			} \
\
		void __classnameReader__::purge() \
		{ \
			CC_SAFE_DELETE(instanceViewReader); \
		} \
\
		void __classnameReader__::destroyInstance() \
		{ \
			CC_SAFE_DELETE(instanceViewReader); \
		} \
\
		cocos2d::Node* __classnameReader__::createNodeWithFlatBuffers(const flatbuffers::Table* nodeOptions) \
		{ \
			__classname__* pCocosView = __classname__::create(); \
			setPropsWithFlatBuffers(pCocosView, (flatbuffers::Table*)nodeOptions); \
			return pCocosView; \
		} \
///////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////
#define DECLARE_CUSTOM_VIEW_LOCATE_TOUCH_CALLBACK \
cocos2d::ui::Widget::ccWidgetTouchCallback onLocateTouchCallback(const std::string &callBackName); \
void  OnTouch(cocos2d::Ref* pObject, cocos2d::ui::Widget::TouchEventType type); \
void  OnTouchBase(cocos2d::Ref* pObject, cocos2d::ui::Widget::TouchEventType type); \
///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define DECLARE_CUSTOM_VIEW_LOCATE_EVENT_CALLBACK \
cocos2d::ui::Widget::ccWidgetEventCallback onLocateEventCallback(const std::string &callBackName); \
void  OnEvent(cocos2d::Ref* pObject, int); \
///////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////
#define DEFINE_CUSTOM_VIEW_LOCATE_TOUCH_CALLBACK(__classname__) \
cocos2d::ui::Widget::ccWidgetTouchCallback __classname__::onLocateTouchCallback(const std::string &callBackName) \
{ \
if (callBackName == "OnTouch") \
{ \
return CC_CALLBACK_2(__classname__::OnTouchBase, this); \
} \
return  nullptr; \
} \
void __classname__::OnTouchBase(cocos2d::Ref* pObject, cocos2d::ui::Widget::TouchEventType type) \
{ \
OnTouch(pObject, type); \
if(type == cocos2d::ui::Widget::TouchEventType::ENDED) \
{ \
cocos2d::ui::Button* buttonNode = dynamic_cast<cocos2d::ui::Button*>(pObject); \
if (buttonNode != NULL) \
{ \
} \
} \
} \

///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define DEFINE_CUSTOM_VIEW_PROPAGATE_TOUCH_CALLBACK(__classname__) \
void __classname__::OnTouchBase(cocos2d::Ref* pObject, cocos2d::ui::Widget::TouchEventType type) \
{ \
OnTouch(pObject, type); \
cocos2d::ui::Button* buttonNode = dynamic_cast<cocos2d::ui::Button*>(pObject); \
if (buttonNode != NULL) \
{ \
} \
} \
///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define DEFINE_CUSTOM_VIEW_LOCATE_EVENT_CALLBACK(__classname__) \
cocos2d::ui::Widget::ccWidgetEventCallback __classname__::onLocateEventCallback(const std::string &callBackName) \
{ \
if (callBackName == "OnEvent") \
{ \
return CC_CALLBACK_2(__classname__::OnEvent, this); \
} \
return  nullptr; \
} \
///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define DECLARE_CUSTOM_VIEW_LIFECYCLE_ONATTACHONDETACH \
virtual void			AttachToScene(); \
virtual void			DetachFromScene(); \

#define DECLARE_CUSTOM_VIEW_LIFECYCLE_ONLOADONUNLOAD \
virtual void			LoadMembers(); \
virtual void			UnLoadMembers(); \

#define DECLARE_CUSTOM_VIEW_LIFECYCLE_ONPOSTATTACH \
virtual void			PostAttachToScene(); \

#define DECLARE_CUSTOM_VIEW_LIFECYCLE_ONPREDETTACH \
virtual void			PreDetachFromScene(); \
///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define DECLARE_CUSTOM_VIEW_PREREQUISITES(__classname__) \
__classname__() { }; \
DECLARE_CUSTOM_VIEW_LOCATE_TOUCH_CALLBACK \
CREATE_FUNC(__classname__) \
///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define DECLARE_CUSTOM_VIEW_TOUCH_PREREQUISITES \
DECLARE_CUSTOM_VIEW_LOCATE_TOUCH_CALLBACK \
///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define DECLARE_COCOS_CUSTOM_VIEW(__classname__) \
using namespace cocos2d; \
using namespace cocos2d::ui; \
class __classname__ : public cocos2d::ui::Widget, public cocostudio::WidgetCallBackHandlerProtocol, public ICocosViewLifeCycle \
///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define DECLARE_COCOS_CUSTOM_VIEW_SUBCLASS(__classname__, __baseclassname__) \
using namespace cocos2d; \
using namespace cocos2d::ui; \
class __classname__ : public __baseclassname__
///////////////////////////////////////////////////////////////////////////////////////////////////////

#define ANIM_ONCE			0
#define ANIM_INFINITE_LOOP	-1

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define CALL_COCOS_ANIMATION_BY_ACTION(__node__, __action__, __animationName__, __animStartCallbackFunc__, __animEndCallbackFunc__, __callbackObjRef__, __callCallbacksIfNoAnims__, __loopCount__) \
{ \
	if (__action__->hasAnimationInfos() && __action__->IsAnimationInfoExists(__animationName__)) \
	{ \
		__node__->stopAllActions(); \
		__node__->runAction(__action__); \
		__action__->play(__animationName__, __loopCount__); \
	\
		__action__->setOnAnimationStartCallFunc(CC_CALLBACK_1(__animStartCallbackFunc__, __callbackObjRef__)); \
		__action__->setOnAnimationEndCallFunc(CC_CALLBACK_1(__animEndCallbackFunc__, __callbackObjRef__)); \
	} \
	else \
	{ \
		if(__callCallbacksIfNoAnims__) \
		{ \
			(*__callbackObjRef__).__animStartCallbackFunc__(__animationName__); \
			(*__callbackObjRef__).__animEndCallbackFunc__(__animationName__); \
		} \
	} \
} \

#define CALL_COCOS_ANIMATION_BY_ACTION_WITH_FRAMEEVENT(__node__, __action__, __animationName__, __animStartCallbackFunc__, __animEndCallbackFunc__, __animFrameEventCallback__, __callbackObjRef__, __callCallbacksIfNoAnims__, __loopCount__) \
{ \
	if (__action__->hasAnimationInfos() && __action__->IsAnimationInfoExists(__animationName__)) \
	{ \
		__node__->stopAllActions(); \
		__node__->runAction(__action__); \
		__action__->play(__animationName__, __loopCount__); \
	\
		__action__->setOnAnimationStartCallFunc(CC_CALLBACK_1(__animStartCallbackFunc__, __callbackObjRef__)); \
		__action__->setOnAnimationEndCallFunc(CC_CALLBACK_1(__animEndCallbackFunc__, __callbackObjRef__)); \
		__action__->setFrameEventCallFunc(CC_CALLBACK_1(__animFrameEventCallback__, __callbackObjRef__)); \
	} \
	else \
	{ \
		if(__callCallbacksIfNoAnims__) \
		{ \
			(*__callbackObjRef__).__animStartCallbackFunc__(__animationName__); \
			(*__callbackObjRef__).__animEndCallbackFunc__(__animationName__); \
			(*__callbackObjRef__).__animFrameEventCallback__(nullptr); \
		} \
	} \
} \
///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define CALL_COCOS_ANIMATION_BY_ACTION_LAMBDACALLBACK(__node__, __action__, __animationName__, __animStartCallbackFunc__, __animEndCallbackFunc__, __loopCount__) \
{ \
if (__action__->hasAnimationInfos() && __action__->IsAnimationInfoExists(__animationName__)) \
	{ \
	__node__->stopAllActions(); \
	__node__->runAction(__action__); \
	__action__->play(__animationName__, __loopCount__); \
	\
	__action__->setOnAnimationStartCallFunc(__animStartCallbackFunc__); \
	__action__->setOnAnimationEndCallFunc(__animEndCallbackFunc__); \
	} \
	else \
	{ \
		if (__animStartCallbackFunc__ != nullptr) \
			__animStartCallbackFunc__(__animationName__); \
		if (__animEndCallbackFunc__ != nullptr) \
			__animEndCallbackFunc__(__animationName__); \
	} \
} \
///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define COCOS_ANIMATION_NO_LOOP		1
#define COCOS_ANIMATION_LOOP		-1
///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define CALL_COCOS_ANIMATION_BY_ACTION_WITHOUT_CALLBACK(__node__, __action__, __animationName__, __loopCount__) \
{ \
	if (__action__->hasAnimationInfos() && __action__->IsAnimationInfoExists(__animationName__)) \
	{ \
		__node__->stopAllActions(); \
		__node__->runAction(__action__); \
		__action__->play(__animationName__, __loopCount__); \
	} \
	else\
	{\
	}\
} \
///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define STOP_COCOS_ANIMATION(__actionTimeline__) \
__actionTimeline__->stop(); \
///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define JSON_TRY_GET_STRING(__strName__, __outStr__, __jsonDOMObject__) \
CoefficientsManager::Get().TryGetString8(EA_CHAR16(__strName__), __outStr__, __jsonDOMObject__); \

#define JSON_TRY_GET_BOOL(__strName__, __outBool__, __jsonDOMObject__) \
CoefficientsManager::Get().TryGetBool(EA_CHAR16(__strName__), __outBool__, __jsonDOMObject__); \

#define JSON_TRY_GET_INT(__strName__, __outInt__, __jsonDOMObject__) \
CoefficientsManager::Get().TryGetInt(EA_CHAR16(__strName__), __outInt__, __jsonDOMObject__); \
///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define CALL_LIFECYCLE(__functionName__) \
{ \
	for (int32_t i = 0; i < m_vLifeCycleChildrens.size(); i++) \
	{ \
		ICocosViewLifeCycle* pICocosViewLifeCycleNode = dynamic_cast<ICocosViewLifeCycle*>(m_vLifeCycleChildrens[i]); \
		if (pICocosViewLifeCycleNode != NULL) \
		{ \
			pICocosViewLifeCycleNode->__functionName__(); \
		} \
	} \
} \
///////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////
#define CALL_COCOS_PUSHVIEW_MESSAGE(__screenName__) \
EA::Messaging::MessageBasicRC<1>* msg = new EA::Messaging::MessageBasicRC<1>; \
msg->SetUint32(0, CocosSceneUtils::GetViewHash(__screenName__)); \
EA::GameFoundation::GameMessaging::GetServer().MessagePost(Message::eIdCocosPushView, msg); \
///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define CALL_COCOS_POPVIEW_MESSAGE \
EA::GameFoundation::GameMessaging::GetServer().MessagePost(Message::eIdCocosPopView, NULL); \
///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define CALL_COCOS_REPLACEVIEW_MESSAGE(__screenName__) \
EA::Messaging::MessageBasicRC<1>* msg = new EA::Messaging::MessageBasicRC<1>; \
msg->SetUint32(0, CocosSceneUtils::GetViewHash(__screenName__)); \
EA::GameFoundation::GameMessaging::GetServer().MessagePost(Message::eIdCocosReplaceView, msg); \
///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define DECODE_MESSAGE_AND_CALL_COCOS_PUSHVIEW(sViewName) PushView(sViewName);
///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define DECODE_MESSAGE_AND_CALL_COCOS_REPLACEVIEW(sViewName) ReplaceView(sViewName);
///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define COCOS_DEBUGLOG(__log__)\
EA_LOG("TetrisBlitzApp", EA::Trace::kLevelDebug, (__log__));\
EA_LOG("", EA::Trace::kLevelDebug, ("\n"));\
///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define SEND_STRING_MESSAGE(__messageID__, __stringMessage__) \
if (__stringMessage__ != "") \
{ \
	EA::GameFoundation::GameMessaging::GetServer().MessageSend(__messageID__, (void*)__stringMessage__.c_str()); \
} \

#define SEND_BOOL_MESSAGE(__messageID__, __boolValue__) \
EA::GameFoundation::GameMessaging::GetServer().MessageSend(__messageID__, (void*)__boolValue__); \
///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define RETAIN_REF_POINTER(__node__) __node__->retain();
#define RELEASE_REF_POINTER(__node__) __node__->release();

#define	CREATE_ACTION_AND_RETAIN(__csbFileName__, __actionTimeline__) \
__actionTimeline__ = cocos2d::CSLoader::getInstance()->createTimeline(__csbFileName__); \
assert(__actionTimeline__); \
RETAIN_REF_POINTER(__actionTimeline__); \

#define STOP_ACTION_AND_RELEASE(__actionTimeline__) \
STOP_COCOS_ANIMATION(__actionTimeline__) \
RELEASE_REF_POINTER(__actionTimeline__) \
///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define GET_CHILD_BY_NAME(__parentNode__, __outNode__, __classType__, __childName) \
{ \
	__outNode__ = dynamic_cast<__classType__*>(__parentNode__->getChildByName(__childName)); \
	assert(__outNode__ != NULL); \
} \
///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define DECLARE_COCOS_UI(__className, __memberName__) __className*	__memberName__;
///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define DECLARE_COCOS_UI_ARRAY(__className, __memberName__, __arrayCount__) __className*	__memberName__[__arrayCount__];
///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define REGISTER_INGAME_TIME() \
if (!IsRegisteredInGameTime()) \
{ \
	RegisterInGameTime(); \
} \
///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define UNREGISTER_INGAME_TIME() \
if (IsRegisteredInGameTime()) \
{ \
	UnRegisterInGameTime(); \
} \
///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define REGISTER_MESSAGE_HANDLER(__handler__, __messageID__) \
//GameMessaging::GetServer().AddHandler(__handler__, __messageID__, false); \
///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define UNREGISTER_MESSAGE_HANDLER(__handler__, __messageID__) \
//GameMessaging::GetServer().RemoveHandler(__handler__, __messageID__); \
///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define SAFE_REGISTER_TETRISCORE_MESSAGE_HANDLER(__handler__, __messageID__) \
if (!CoreMessaging::GetServer().IsHandlerRegistered(__handler__, __messageID__)) \
{ \
	CoreMessaging::GetServer().AddHandler(__handler__, __messageID__, false); \
} \

#define SAFE_REGISTER_MESSAGE_HANDLER(__handler__, __messageID__) \
if (!GameMessaging::GetServer().IsHandlerRegistered(__handler__, __messageID__)) \
{ \
	GameMessaging::GetServer().AddHandler(__handler__, __messageID__, false); \
} \
///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define SAFE_UNREGISTER_TETRISCORE_MESSAGE_HANDLER(__handler__, __messageID__) \
if (CoreMessaging::GetServer().IsHandlerRegistered(__handler__, __messageID__)) \
{ \
	CoreMessaging::GetServer().RemoveHandler(__handler__, __messageID__); \
} \

#define SAFE_UNREGISTER_MESSAGE_HANDLER(__handler__, __messageID__) \
if (GameMessaging::GetServer().IsHandlerRegistered(__handler__, __messageID__)) \
{ \
	GameMessaging::GetServer().RemoveHandler(__handler__, __messageID__); \
} \
///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define CREATE_AND_OPEN_GENERIC_POPUP(__screenName__, __iconPath__, __titleText__, __titleLocalized__, __contentText__, __contentLocalized__) \
{ \
	CocosGenericPopUpInfo* pPopupInfo = CocosGenericPopUpInfo::CreatePopUpInfo(); \
	pPopupInfo->SetGenericPopupButtonInfo(NULL); \
	pPopupInfo->SetTitleIcon(__iconPath__); \
	pPopupInfo->SetTitleText(__titleText__, __titleLocalized__); \
	pPopupInfo->SetContentText(__contentText__, __contentLocalized__); \
	pPopupInfo->SetCloseButtonVisibility(true); \
	CocosSceneManager::Get().OpenPopUp(POPUP_GENERIC, (void*)pPopupInfo); \
} \
///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define CREATE_AND_OPEN_GENERIC_POPUP_WITHOUT_CLOSE_BUTTON(__screenName__, __iconPath__, __titleText__, __titleLocalized__, __contentText__, __contentLocalized__) \
{ \
	CocosGenericPopUpInfo* pPopupInfo = CocosGenericPopUpInfo::CreatePopUpInfo(); \
	pPopupInfo->SetGenericPopupButtonInfo(NULL); \
	pPopupInfo->SetTitleIcon(__iconPath__); \
	pPopupInfo->SetTitleText(__titleText__, __titleLocalized__); \
	pPopupInfo->SetContentText(__contentText__, __contentLocalized__); \
	pPopupInfo->SetCloseButtonVisibility(false); \
	CocosSceneManager::Get().OpenPopUp(POPUP_GENERIC, (void*)pPopupInfo); \
} \
///////////////////////////////////////////////////////////////////////////////////////////////////////

#define CREATE_AND_OPEN_GENERIC_POPUP_WITH_BUTTONS(		__STRING_YES_BTN_TEXT__, __BOOL_YES_BTN_TEXT_LOC__, __STRING_YES_BTN_INTENTION__, \
														__STRING_NO_BTN_TEXT__, __BOOL_NO_BTN_TEXT_LOC__, __STRING_NO_BTN_INTENTION__, \
														__STRING_ICON_PATH__,\
														__STRING_TITLE__, __BOOL_TITLE_LOC__, \
														__STRING_CONTEXT__, __BOOL_CONTEXT_LOC__, \
														__LAMDA_CALLBACK__) \
{ \
	CocosGenericPopUpButtonInfo* pInfo = CocosGenericPopUpButtonInfo::CreateCocosGenericPopUpButtonInfo(); \
	pInfo->SetYesButtonTextWithIntension(__STRING_YES_BTN_TEXT__, __BOOL_YES_BTN_TEXT_LOC__, __STRING_YES_BTN_INTENTION__); \
	pInfo->SetNoButtonTextWithIntension(__STRING_NO_BTN_TEXT__, __BOOL_NO_BTN_TEXT_LOC__, __STRING_NO_BTN_INTENTION__); \
	\
	CocosGenericPopUpInfo* pPopupInfo = CocosGenericPopUpInfo::CreatePopUpInfo(); \
	pPopupInfo->SetTitleIcon(__STRING_ICON_PATH__); \
	pPopupInfo->SetTitleText(__STRING_TITLE__, __BOOL_TITLE_LOC__); \
	pPopupInfo->SetContentText(__STRING_CONTEXT__, __BOOL_CONTEXT_LOC__); \
	pPopupInfo->SetCloseButtonVisibility(false); \
	pPopupInfo->SetGenericPopupButtonInfo(pInfo); \
	pPopupInfo->SetTargetLamda(__LAMDA_CALLBACK__); \
	\
	CocosSceneManager::Get().OpenPopUp(POPUP_GENERIC, (void*)pPopupInfo); \
} \
	
///////////////////////////////////////////////////////////////////////////////////////////////////////
#define SHOW_NETWORK_LOADER(__bool__) \
{ \
	m_pProjectNode_NetworkLoader->setVisible(__bool__); \
	CocosLayerNetworkLoader* pCocosLayerNetworkLoader = dynamic_cast<CocosLayerNetworkLoader*>(m_pProjectNode_NetworkLoader); \
	if (__bool__) \
		pCocosLayerNetworkLoader->StartAnimation(); \
	else \
		pCocosLayerNetworkLoader->StopAnimation(); \
} \
///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define CALL_DETACH_ANIMATION_DONE \
{ \
	CocosView* pCocosView = ((ICocosViewLifeCycle*)this)->GetCocosView(); \
	pCocosView->OnDetachAnimationDone(this); \
} \

#define PRELOAD_AUDIO(STR_BANK_NAME)
#define UNLOAD_AUDIO(STR_BANK_NAME)
#define PLAY_SFX(STR_FILE_NAME)
///////////////////////////////////////////////////////////////////////////////////////////////////////
