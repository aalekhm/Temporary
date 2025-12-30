#pragma once

#include "cocos2d.h"
#include "ui/UIWidget.h"
#include "editor-support/cocostudio/WidgetCallBackHandlerProtocol.h"
#include "CocosCustomDefines.h"

#define TEXT_WIDTH_RATIO 0.9f //This value is multiplied with panel width to get the left and right margins

DECLARE_CUSTOM_VIEW_READER(CocosSceneGenericPopupReader)

typedef	std::function<void(std::string)>	fPopupCallback;

DECLARE_COCOS_CUSTOM_VIEW(CocosSceneGenericPopup)
{
public:
											DECLARE_CUSTOM_VIEW_PREREQUISITES(CocosSceneGenericPopup)
	void									SetTitleText(const std::string& text, bool localized = false);
	void									SetContentText(const std::string& text, bool localized = false);
	bool									OnBackButtonPressed();

protected:
	DECLARE_CUSTOM_VIEW_LIFECYCLE_ONLOADONUNLOAD
	DECLARE_CUSTOM_VIEW_LIFECYCLE_ONATTACHONDETACH

	void									Update();

private:
	DECLARE_COCOS_UI(cocos2d::ui::ImageView,m_pImage_Title);
	DECLARE_COCOS_UI(cocos2d::ui::Layout,	m_pPanel_Generic_PopUp);
	DECLARE_COCOS_UI(cocos2d::ui::Layout,	m_pPanel_Header);
	DECLARE_COCOS_UI(cocos2d::ui::Text,		m_pText_Header);
	DECLARE_COCOS_UI(cocos2d::ui::Text,		m_pText_Content);
	DECLARE_COCOS_UI(cocos2d::ui::Button,	m_pButton_CloseButton);
	DECLARE_COCOS_UI(cocos2d::ui::Button,	m_pButton_YesButton);
	DECLARE_COCOS_UI(cocos2d::ui::Text,		m_pText_Yes_Button);
	DECLARE_COCOS_UI(cocos2d::ui::Button,	m_pButton_NoButton);
	DECLARE_COCOS_UI(cocos2d::ui::Text,		m_pText_No_Button);


	std::string								GetButtonIntention(bool yesbutton);

	void									OnCloseButtonPressed();
	void									OnYesButtonPressed();
	void									OnNoButtonPressed();

	const string CSB_LAYER			=		"Common/Scene_Generic_PopUp.csb";
	cocostudio::timeline::ActionTimeline*	m_pProjectNode_Generic_popup_TimeLine;
	fPopupCallback							m_fLamdaFunc;
	int32_t									numButtons;
};

class CocosGenericPopUpButtonInfo
{
public:
	static CocosGenericPopUpButtonInfo*		CreateCocosGenericPopUpButtonInfo();
	virtual									~CocosGenericPopUpButtonInfo() { };
	CocosGenericPopUpButtonInfo();
	static void								DestroyPopUpInfo(CocosGenericPopUpButtonInfo* info);
	void									SetYesButtonTextWithIntension(const std::string& text, bool localized, const std::string& intention);
	void									SetNoButtonTextWithIntension(const std::string& text, bool localized, const std::string& intention);
	std::string								GetYesButtonText() { return mYesButtonText; }
	bool									IsYesButtonLocalized() { return mYesButtonTextLocalized; }
	std::string								GetYesButtonIntention() { return mYesButtonIntention; }
	std::string								GetNoButtonText() { return mNoButtonText; }
	bool									IsNoButtonLocalised() { return mNoButtonTextLocalized; }
	std::string								GetNoButtonIntention() { return mNoButtonIntention; }


private:
	std::string								mYesButtonText;
	std::string								mYesButtonIntention;
	bool									mYesButtonTextLocalized;
	std::string								mNoButtonText;
	std::string								mNoButtonIntention;
	bool									mNoButtonTextLocalized;
};

class CocosGenericPopUpInfo
{
	public:	
		CocosGenericPopUpInfo();
		virtual									~CocosGenericPopUpInfo() { };

		static CocosGenericPopUpInfo*			CreatePopUpInfo();
		void									DestroyPopUpInfo();
		void									SetTitleIcon(const std::string& path);
		void									SetTitleText(const std::string& text, bool localized);
		void									SetContentText(const std::string& text, bool localized);
		void									SetCloseButtonVisibility(bool visible);
		std::string      				  	    GetTitleIconPath() { return mTitleIconPath; }
		std::string      				  	    GetTitleText() { return mTitleText; }
		std::string      				  	    GetContentText() { return mContentText; }
		bool		      				  	    IsTitleLocalized() { return mIsTitleLocalized; }
		bool		      				  	    IsContentLocalized() { return mIsContentLocalized; }
		bool									IsCloseButtonVisible() { return mIsCloseButtonVisible; }
		void									SetGenericPopupButtonInfo(CocosGenericPopUpButtonInfo* info);
		CocosGenericPopUpButtonInfo*			GetGenericPopUpButtonInfo() { return mButtonsInfo; }

		void									SetTargetLamda(fPopupCallback targetLamda) { m_fLamdaFunc = targetLamda; }
		fPopupCallback							GetTargetLamda() { return m_fLamdaFunc; }

	private:
		std::string								mTitleIconPath;
		std::string								mTitleText;
		std::string								mContentText;
		bool									mIsTitleLocalized;
		bool 									mIsContentLocalized;
		bool									mIsCloseButtonVisible;
		CocosGenericPopUpButtonInfo*			mButtonsInfo;
		fPopupCallback							m_fLamdaFunc;
};