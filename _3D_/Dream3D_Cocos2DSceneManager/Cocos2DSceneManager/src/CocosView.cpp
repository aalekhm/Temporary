#include "CocosView.h"
#include "CocosSceneManager.h"
#include "CocosCustomDefines.h"

CocosView::CocosView()
	: m_sViewName("")
	, m_sAnimIN("")
	, m_sAnimOUT("")
	, m_sCSBFilename("")
	, m_sStateID("")
	, m_pViewNode(NULL)
	, m_sZDepth(VIEW_Z_DEPTH_DEFAULT)
	, m_bHasBG(false)
	, m_sBGTransitionIN("")
	, m_sBGTransitionOUT("")
	, m_pActionTimeline(NULL)
	, m_bShowBack(true)
	, m_bIsPopup(false)
	, m_bLoadOnDemand(false)
	, m_bUnLoadOnClose(false)
	, m_bIsCloseExplicit(false)
	, m_bIsNodeReadyForRender(false)
	, m_iZDepth(0)
	, m_bBGHasUnfiedHeader(true)
	, m_sSpriteSheetName("")
    , m_sCustomClassName("")
	, m_bCacheFully(false)
	, m_bHasMultipleInstances(false)
	, m_bHasCustomUIParameters(false)
{

}

void CocosView::RefreshDataFromJson(nlohmann::json& jView)
{
	TRY_GET_JSONVAL(m_sViewName,				jView,	std::string, "viewName", "");
	TRY_GET_JSONVAL(m_sAnimIN,					jView,	std::string, "animIN", "");
	TRY_GET_JSONVAL(m_sAnimOUT,					jView,	std::string, "animOUT", "");
	TRY_GET_JSONVAL(m_sCSBFilename,				jView,	std::string, "csbFilename", "");
	TRY_GET_JSONVAL(m_sStateID,					jView,	std::string, "stateID", "");
	TRY_GET_JSONVAL(m_bHasBG,					jView,	bool, "hasBG", false);
	TRY_GET_JSONVAL(m_sZDepth,					jView,	std::string, "zDepth", "");
	TRY_GET_JSONVAL(m_sBGTransitionIN,			jView,	std::string, "bgTransitionIN", "");
	TRY_GET_JSONVAL(m_sBGTransitionOUT,			jView,	std::string, "bgTransitionOUT", "");
	TRY_GET_JSONVAL(m_bShowBack,				jView,	bool, "showBack", false);
	TRY_GET_JSONVAL(m_bIsPopup,					jView,	bool, "isPopup", false);
	TRY_GET_JSONVAL(m_bLoadOnDemand,			jView,	bool, "loadOnDemand", false);
	TRY_GET_JSONVAL(m_bUnLoadOnClose,			jView,	bool, "unloadOnClose", false);
	TRY_GET_JSONVAL(m_bBGHasUnfiedHeader,		jView,	bool, "bgHasUnifiedHeader", false);
	TRY_GET_JSONVAL(m_sSpriteSheetName,			jView,	std::string, "SpriteSheetName", "");
	TRY_GET_JSONVAL(m_sCustomClassName,			jView,	std::string, "customClassName", "");
	TRY_GET_JSONVAL(m_bCacheFully,				jView,	bool, "CacheFully", false);
	TRY_GET_JSONVAL(m_bHasMultipleInstances,	jView,	bool, "hasMultipleInstances", false);
}

void CocosView::Load(bool partialLoad /*= true*/)
{                
	bool bCacheFully = IsCacheViewOnDemand() || !partialLoad;
	if (m_pViewNode == NULL)
	{
		m_bIsNodeReadyForRender = false;
        if (m_sCustomClassName.empty())
        {
            m_pViewNode = cocos2d::CSLoader::createNode(m_sCSBFilename, CC_CALLBACK_1(CocosView::OnNodeLoaded, this));
        }
        else
        {
            // If custom class is provided in CocosView.json, use it to create an instance. This is
            // particularly useful if you sub class your Scene class.
            m_pViewNode = cocos2d::CSLoader::createNodeWithCustomClass(m_sCSBFilename, CC_CALLBACK_1(CocosView::OnNodeLoaded, this), m_sCustomClassName);
        }
		assert(m_pViewNode != NULL);

		m_pViewNode->setVisible(false);

		RETAIN_REF_POINTER(m_pViewNode);

		if (bCacheFully && m_pViewNode != NULL)
		{
			m_bIsNodeReadyForRender = true;
			CocosSceneManager::Get().AddCustomChild(m_pViewNode, m_sZDepth);
			CREATE_ACTION_AND_RETAIN(m_sCSBFilename, m_pActionTimeline)
			OnLoad();
		}
	}	
	else if (!m_bIsNodeReadyForRender)
	{
		m_bIsNodeReadyForRender = true;
		CocosSceneManager::Get().AddCustomChild(m_pViewNode, m_sZDepth);
		CREATE_ACTION_AND_RETAIN(m_sCSBFilename, m_pActionTimeline)
		OnLoad();
	}
}
		
void CocosView::OnNodeLoaded(Ref* pObject)
{
	bool bFallThrough = true;

	if (bFallThrough) bFallThrough = (CheckAndAddParticles(pObject) == false);
	if (bFallThrough) bFallThrough = (CheckAndUpdateFonts(pObject) == false);
	if (bFallThrough) bFallThrough = (CheckAndAddLifeCycleChild(pObject) == false);

	// Handle Custom UI Parameters
	{
		std::tuple<void*, void*> tData = std::make_tuple(this, pObject);
		//GameMessaging::GetServer().MessageSend(EA::TetrisApp::Message::eIdHandleCustomUIParameters, (void*)&tData);
	}
	//GameMessaging::GetServer().MessageSend(EA::TetrisApp::Message::eIdCocosApplyUserData, (void*)pObject);
}

void CocosView::HandleCustomUIParameters(cocos2d::Node* pObject)
{
	//if (m_bHasCustomUIParameters)
	//{
	//	std::string sNodeName = ((cocos2d::Node*)pObject)->getName();
	//
	//	auto it = m_mapCustomUINodeParameters_NameObject.find(sNodeName);
	//	if (it != m_mapCustomUINodeParameters_NameObject.end())
	//	{
	//		EA::Json::JsonDomObject* pJsonDomObject = m_mapCustomUINodeParameters_NameObject[sNodeName];
	//
	//		assert(pJsonDomObject != NULL);
	//		//CocosSceneUtils::ThemedUI_ReadParameterOverrides((cocos2d::Node*)pObject, pJsonDomObject);
	//	}
	//}
}

void CocosView::UpdateFonts()
{
	std::string fontName;
	CocosSceneManager::Get().UpdateChildLocalization(GetViewNode(), fontName);
}

bool CocosView::CheckAndAddParticles(Ref* pObject)
{
    cocos2d::ParticleSystemQuad* particleSystem = dynamic_cast<cocos2d::ParticleSystemQuad*>(pObject);
	if (particleSystem)
	{
		m_vParticles.push_back(particleSystem);
		return true;
	}

	return false;
}

bool CocosView::CheckAndUpdateFonts(Ref* pObject)
{
	std::string fontName;
	cocos2d::ui::Text *uiText = dynamic_cast<cocos2d::ui::Text*>(pObject);
	if (NULL != uiText && uiText->getFontName() != "")
	{
		uiText->setFontName(fontName);
		uiText->refreshLocalization();
		return true;
	}

	cocos2d::Label *uiLabel = dynamic_cast<cocos2d::Label *>(pObject);
	if (NULL != uiLabel)
	{
		cocos2d::TTFConfig ttfconfig = uiLabel->getTTFConfig();
		ttfconfig.fontFilePath = fontName;
		uiLabel->setTTFConfig(ttfconfig);
		uiLabel->refreshLocalization();
		return true;
	}
	return false;
}

bool CocosView::CheckAndAddLifeCycleChild(Ref* pObject)
{
	ICocosViewLifeCycle* pICocosViewLifeCycle = dynamic_cast<ICocosViewLifeCycle*>(pObject);
	if (pICocosViewLifeCycle)
	{
		pICocosViewLifeCycle->SetCocosView(this);
		m_vLifeCycleChildrens.push_back(pICocosViewLifeCycle);
		m_vLifeCycleChildrensDetachAnimationFlag.push_back(false);
		return true;
	}

	return false;
}

void CocosView::scheduleAllParticleUpdates()
{
	for (auto particle : m_vParticles)
	{
		particle->scheduleUpdateWithPriority(1);
	}
}

void CocosView::unscheduleAllParticleUpdates()
{
	for (auto particle : m_vParticles)
	{
		particle->unscheduleUpdate();
	}
}

void CocosView::OpenView(cocos2d::Scene* pRootScene, bool bOpenExplicit, void* pViewData)
{
	m_ViewData = pViewData;
	Load(false);
	StartINAnimation(bOpenExplicit);            
}

void CocosView::StartINAnimation(bool IsOpenExplicit)
{
	if (m_pViewNode != NULL && !m_pViewNode->isVisible())
	{
		m_pViewNode->setVisible(true);
		HandleViewSpecifics();

		if (IsOpenExplicit)
		{
			CALL_COCOS_ANIMATION_BY_ACTION(m_pViewNode, m_pActionTimeline, m_sAnimIN, CocosView::OnINAnimationStart, CocosView::OnINAnimationEnd, this, true, 0);
		}
		else
		{
			CALL_COCOS_ANIMATION_BY_ACTION(m_pViewNode, m_pActionTimeline, m_sAnimIN, CocosSceneManager::OnINAnimationStart, CocosSceneManager::OnINAnimationEnd, &CocosSceneManager::Get(), true, 0);
		}
	}
}

void CocosView::HandleViewSpecifics()
{
	HandleBG();
}

void CocosView::HandleBG()
{
	if (m_bHasBG)
	{
		CocosSceneManager::Get().OpenViewExplicitly(VIEW_BG);
	}
	else
	{
		if (GetStateID() != "CommonState")
		{
			CocosSceneManager::Get().CloseViewExplicitly(VIEW_BG);
		}
	}
}

void CocosView::OnLoad()
{
	unscheduleAllParticleUpdates();
	CALL_LIFECYCLE(LoadMembers)
}

void CocosView::OnINAnimationStart(const std::string& sAnimName)
{
	OnShow();

}

void CocosView::OnShow()
{
	scheduleAllParticleUpdates();
	CALL_LIFECYCLE(AttachToScene)

	std::vector<ICocosViewLifeCycle*>::iterator itr = m_vLifeCycleChildrens.begin();
	for (int32_t iPosition = 0; itr != m_vLifeCycleChildrens.end(); itr++, iPosition++)
	{
		m_vLifeCycleChildrensDetachAnimationFlag[iPosition] = false;			
	}
}

void CocosView::OnLifeCycleFocusLost()
{
	CALL_LIFECYCLE(OnLifeCycleFocusLost)
}

void CocosView::OnLifeCycleFocusGained()
{
	CALL_LIFECYCLE(OnLifeCycleFocusGained)
}

void CocosView::OnINAnimationEnd(const std::string& sAnimName)
{
	CALL_LIFECYCLE(PostAttachToScene)
}

void CocosView::CloseView(bool bOpenExplicit)
{
	m_bIsCloseExplicit = bOpenExplicit;
	CALL_LIFECYCLE(PreDetachFromScene);
}

void CocosView::OnViewPopped()
{
	CALL_LIFECYCLE(OnViewPopped);
}

void CocosView::StartOUTAnimation(bool IsOpenExplicit)
{
	if (m_pViewNode != NULL && m_pViewNode->isVisible())
	{
		if (IsOpenExplicit)
		{
			CALL_COCOS_ANIMATION_BY_ACTION(m_pViewNode, m_pActionTimeline, m_sAnimOUT, CocosView::OnOUTAnimationStart, CocosView::OnOUTAnimationEnd, this, true, 0);
		}
		else
		{
			CALL_COCOS_ANIMATION_BY_ACTION(m_pViewNode, m_pActionTimeline, m_sAnimOUT, CocosSceneManager::OnOUTAnimationStart, CocosSceneManager::OnOUTAnimationEnd, &CocosSceneManager::Get(), true, 0);
		}
	}
}

void CocosView::OnDetachAnimationDone(ICocosViewLifeCycle* pICocosViewLifeCycle)
{
	std::vector<ICocosViewLifeCycle*>::iterator itr = m_vLifeCycleChildrens.begin();
	int32_t iDetachFinishedCount = 0;
	for (int32_t iPosition = 0; itr != m_vLifeCycleChildrens.end(); itr++, iPosition++)
	{
		ICocosViewLifeCycle* pICocosView = *itr;
		if (pICocosViewLifeCycle == pICocosView)
		{
			m_vLifeCycleChildrensDetachAnimationFlag[iPosition] = true;
		}

		if (m_vLifeCycleChildrensDetachAnimationFlag[iPosition])
		{
			iDetachFinishedCount++;
			if (iDetachFinishedCount == m_vLifeCycleChildrensDetachAnimationFlag.size())
			{
				StartOUTAnimation(m_bIsCloseExplicit);
				m_bIsCloseExplicit = false;
				break;
			}
		}
	}
}

void CocosView::OnOUTAnimationStart(const std::string& sAnimName)
{
			
}

void CocosView::OnOUTAnimationEnd(const std::string& sAnimName)
{
	if (IsUnLoadViewOnClose())
	{
		UnLoad();
	}
	else
	{
		OnHide();
	}
}

void CocosView::OnHide()
{
	if (m_pViewNode != NULL && m_pViewNode->isVisible())
	{
		unscheduleAllParticleUpdates();
		m_pViewNode->setVisible(false);
		CALL_LIFECYCLE(DetachFromScene);
	}		
}

void CocosView::OnUnLoad()
{
	if (m_bIsNodeReadyForRender)
    {
		CALL_LIFECYCLE(UnLoadMembers)
		CALL_LIFECYCLE(RemoveMessages);
        STOP_ACTION_AND_RELEASE(m_pActionTimeline)
    }
	CleanUp();
}
		
void CocosView::UnLoad()
{
	if (m_pViewNode != NULL)
	{
		OnHide();
		OnUnLoad();
	}
}

void CocosView::CleanUp()
{
	m_vParticles.clear();
	m_vLifeCycleChildrens.clear();
	m_vLifeCycleChildrensDetachAnimationFlag.clear();
	m_pViewNode->removeFromParent();
	RELEASE_REF_POINTER(m_pViewNode)
	m_pViewNode = NULL;
}