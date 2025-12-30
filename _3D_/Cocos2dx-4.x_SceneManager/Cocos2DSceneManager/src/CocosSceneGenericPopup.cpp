#include"CocosSceneGenericPopup.h"
#include "CocosSceneManager.h"

DEFINE_CUSTOM_VIEW_READER("GenericPopup", CocosSceneGenericPopup, CocosSceneGenericPopupReader)

//DEFINE_CUSTOM_VIEW_LOCATE_TOUCH_CALLBACK(CocosSceneGenericPopup)

void CocosSceneGenericPopup::LoadMembers()
{
	GET_CHILD_BY_NAME(this, m_pPanel_Generic_PopUp, cocos2d::ui::Layout, "Panel_Generic_PopUp");
	GET_CHILD_BY_NAME(this, m_pPanel_Header, cocos2d::ui::Layout, "Panel_Header");
	GET_CHILD_BY_NAME(this, m_pText_Header, cocos2d::ui::Text, "Text_Header");
	GET_CHILD_BY_NAME(this, m_pText_Content, cocos2d::ui::Text, "Text_Content");
	GET_CHILD_BY_NAME(this, m_pButton_CloseButton, cocos2d::ui::Button, "Button_CloseButton");
	GET_CHILD_BY_NAME(this, m_pButton_YesButton, cocos2d::ui::Button, "Button_YES");
	GET_CHILD_BY_NAME(this, m_pText_Yes_Button, cocos2d::ui::Text, "Text_YES_Button");
	GET_CHILD_BY_NAME(this, m_pButton_NoButton, cocos2d::ui::Button, "Button_NO");
	GET_CHILD_BY_NAME(this, m_pText_No_Button, cocos2d::ui::Text, "Text_No_Button");
	GET_CHILD_BY_NAME(this, m_pImage_Title, cocos2d::ui::ImageView, "Image_TitleIcon");

	CREATE_ACTION_AND_RETAIN(CSB_LAYER, m_pProjectNode_Generic_popup_TimeLine)
	m_fLamdaFunc = nullptr;
	numButtons = 0;
	Update();
}

void CocosSceneGenericPopup::UnLoadMembers()
{
	STOP_ACTION_AND_RELEASE_CSB(m_pProjectNode_Generic_popup_TimeLine)

	CocosGenericPopUpInfo* pPopupInfo = (CocosGenericPopUpInfo*)(GetCocosView()->GetViewData());
	if (pPopupInfo)
	{
		std::unique_ptr<CocosGenericPopUpButtonInfo>& pButtonInfo = pPopupInfo->GetGenericPopUpButtonInfo();
		if (pButtonInfo)
			CocosGenericPopUpButtonInfo::DestroyPopUpInfo(pButtonInfo);

		pPopupInfo->DestroyPopUpInfo();
	}
				
}

void CocosSceneGenericPopup::AttachToScene()
{
}

void CocosSceneGenericPopup::DetachFromScene()
{
}

void CocosSceneGenericPopup::OnCloseButtonPressed()
{
	CocosSceneManager::Get().ClosePopUp();
}

void CocosSceneGenericPopup::OnYesButtonPressed()
{
	if (m_fLamdaFunc) {
		printf("[CocosSceneGenericPopup] OnTouch::m_pButton_YesButton In m_fLamdaFunc Intention = %s", GetButtonIntention(true).c_str());
		m_fLamdaFunc(GetButtonIntention(true));
	}

	CocosSceneManager::Get().ClosePopUp();
}

void CocosSceneGenericPopup::OnNoButtonPressed()
{
	if (m_fLamdaFunc)
	{
		printf("[CocosSceneGenericPopup] OnTouch::m_pButton_NoButton In m_fLamdaFunc");
		m_fLamdaFunc(GetButtonIntention(false));
	}

	CocosSceneManager::Get().ClosePopUp();
}

//void CocosSceneGenericPopup::OnTouch(cocos2d::Ref* pObject, cocos2d::ui::Widget::TouchEventType type)
//{
//	if (type == cocos2d::ui::Widget::TouchEventType::ENDED)
//	{
//		if (m_pButton_CloseButton == pObject)
//		{
//			OnCloseButtonPressed();
//		}
//		else if (m_pButton_YesButton == pObject)
//		{
//			OnYesButtonPressed();
//		}
//		else if (m_pButton_NoButton == pObject)
//		{
//			OnNoButtonPressed();
//		}
//	}
//}

std::string CocosSceneGenericPopup::GetButtonIntention(bool yesButton)
{
	CocosGenericPopUpInfo* pPopupInfo = (CocosGenericPopUpInfo*)(GetCocosView()->GetViewData());
	if (pPopupInfo)
	{
		CocosGenericPopUpButtonInfo* buttoninfo = pPopupInfo->GetGenericPopUpButtonInfo().get();
		if (buttoninfo)
		{
			if (yesButton)
			{
				return buttoninfo->GetYesButtonIntention();
			}
			else
			{
				return buttoninfo->GetNoButtonIntention();
			}
		}
	}
	return "";
			
}

void CocosSceneGenericPopup::Update()
{
	CocosGenericPopUpInfo* pPopupInfo = (CocosGenericPopUpInfo*)(GetCocosView()->GetViewData());
	if (pPopupInfo)
	{				
		//Update Content
		if (pPopupInfo->GetContentText() != "")
		{
			m_pText_Content->setTextAreaSize(Size(m_pPanel_Generic_PopUp->getContentSize().width, 0.0f));
			m_pText_Content->setString(pPopupInfo->GetContentText()/*, pPopupInfo->IsContentLocalized()*/);

			cocos2d::Size sizeWinSize = cocos2d::Director::getInstance()->getWinSize();
			cocos2d::Vec2 sizePanelGenericPercent = m_pPanel_Generic_PopUp->getSizePercent();
			float fsizePanelGenericPixels = (sizeWinSize.height * sizePanelGenericPercent.y);
					
			float_t newPanelHeightPixels = m_pPanel_Generic_PopUp->getContentSize().height + (m_pText_Content->getContentSize().height * m_pText_Content->getScaleY()); // getContentSize always returns the actual size and it doesn't consider the scale so we need to multiply the scale here 
			float fNewPanelGenericPercentY = newPanelHeightPixels / sizeWinSize.height;
			sizePanelGenericPercent.y = fNewPanelGenericPercentY;
			m_pPanel_Generic_PopUp->setSizePercent(sizePanelGenericPercent);

			auto layoutComponent = ui::LayoutComponent::bindLayoutComponent(m_pPanel_Generic_PopUp);
			layoutComponent->refreshLayout();
		}

		//Update Title Icon
		if (pPopupInfo->GetTitleIconPath() != "")
		{
			//Update Title Text
			m_pImage_Title->loadTexture(pPopupInfo->GetTitleIconPath(), Widget::TextureResType::PLIST);
		}

		//Update Title
		if (pPopupInfo->GetTitleText() != "")
		{
			//Update Title Text
			m_pText_Header->setString(pPopupInfo->GetTitleText()/*, pPopupInfo->IsTitleLocalized()*/);

			//Update Title Position
			auto layoutComponent = ui::LayoutComponent::bindLayoutComponent(m_pPanel_Header);
			layoutComponent->refreshLayout();
		}

		//Update Close Button Visibility
		m_pButton_CloseButton->setVisible(pPopupInfo->IsCloseButtonVisible());

		CocosGenericPopUpButtonInfo* buttoninfo = pPopupInfo->GetGenericPopUpButtonInfo().get();
		numButtons = 0;
		if (buttoninfo != NULL)
		{
			if (buttoninfo->GetNoButtonText() != "") 
			{
				m_pText_Yes_Button->setString(buttoninfo->GetYesButtonText()/*, buttoninfo->IsYesButtonLocalized()*/);
				m_pText_No_Button->setString(buttoninfo->GetNoButtonText()/*, buttoninfo->IsNoButtonLocalised()*/);
				numButtons = 2;
			}
			else
			{
				m_pText_Yes_Button->setString(buttoninfo->GetYesButtonText()/*, buttoninfo->IsYesButtonLocalized()*/);
				numButtons = 1;
			}
		}
		char animName[255];
		sprintf_s(animName, "animation_%d_button", numButtons);

		CALL_COCOS_ANIMATION_BY_ACTION_WITHOUT_CALLBACK(this, m_pProjectNode_Generic_popup_TimeLine, animName, false);
		m_fLamdaFunc = pPopupInfo->GetTargetLamda();
	}
}

void CocosSceneGenericPopup::SetContentText(const std::string& text, bool localized)
{
	m_pText_Content->setString(text/*, localized*/);
}

bool CocosSceneGenericPopup::OnBackButtonPressed()
{
	switch (numButtons)
	{
		case 0:
		{
			if (m_pButton_CloseButton->isVisible())
			{
				OnCloseButtonPressed();
			}

			return true;
		}
		break;
		case 1:
		{
			if(m_pButton_CloseButton->isVisible())
			{
				OnCloseButtonPressed();
			}
			else
			{
				OnYesButtonPressed();
			}

			return true;
		}
		break;
		case 2:
		{
			if (m_pButton_CloseButton->isVisible())
			{
				OnCloseButtonPressed();
			}
			else
			{
				OnNoButtonPressed();
			}

			return true;
		}
		break;
	}

	return false;
}

//CocosGenericPopUpInfo
CocosGenericPopUpInfo::CocosGenericPopUpInfo()
	: mTitleIconPath("")
	, mTitleText("")
	, mContentText("")
	, mButtonsInfo(nullptr)
{

}
CocosGenericPopUpInfo* CocosGenericPopUpInfo::CreatePopUpInfo()
{
	CocosGenericPopUpInfo* pInfo = std::make_unique<CocosGenericPopUpInfo>().get();
	return pInfo;
}

void CocosGenericPopUpInfo::DestroyPopUpInfo()
{
	
}

void CocosGenericPopUpInfo::SetTitleIcon(const std::string& path)
{
	mTitleIconPath = path;
}

void CocosGenericPopUpInfo::SetTitleText(const std::string& text, bool localized)
{
	mTitleText = text;
	mIsTitleLocalized = localized;
}

void CocosGenericPopUpInfo::SetContentText(const std::string& text, bool localized)
{
	mContentText = text;
	mIsContentLocalized = localized;
}

void CocosGenericPopUpInfo::SetGenericPopupButtonInfo(std::unique_ptr<CocosGenericPopUpButtonInfo>& pInfo)
{
	mButtonsInfo = std::move(pInfo);
}

void CocosGenericPopUpInfo::SetCloseButtonVisibility(bool visible)
{
	mIsCloseButtonVisible = visible;
}

//Button Info
CocosGenericPopUpButtonInfo::CocosGenericPopUpButtonInfo()
	:mYesButtonText("")
	, mYesButtonIntention("")
	, mYesButtonTextLocalized(false)
	, mNoButtonText("")
	, mNoButtonIntention("")
	, mNoButtonTextLocalized(false)
{

}
std::unique_ptr<CocosGenericPopUpButtonInfo>& CocosGenericPopUpButtonInfo::CreateCocosGenericPopUpButtonInfo()
{
	std::unique_ptr<CocosGenericPopUpButtonInfo>& pInfo = std::make_unique<CocosGenericPopUpButtonInfo>();
	return pInfo;
}

void CocosGenericPopUpButtonInfo::DestroyPopUpInfo(std::unique_ptr<CocosGenericPopUpButtonInfo>& pInfo)
{
	pInfo.reset();
}

void CocosGenericPopUpButtonInfo::SetYesButtonTextWithIntension(const std::string& text, bool localized, const std::string& intention)
{
	mYesButtonText			= text;
	mYesButtonIntention		= intention;
	mYesButtonTextLocalized = localized;
}

void CocosGenericPopUpButtonInfo::SetNoButtonTextWithIntension(const std::string& text, bool localized, const std::string& intention)
{
	mNoButtonText			= text;
	mNoButtonIntention		= intention;
	mNoButtonTextLocalized	= localized;
}