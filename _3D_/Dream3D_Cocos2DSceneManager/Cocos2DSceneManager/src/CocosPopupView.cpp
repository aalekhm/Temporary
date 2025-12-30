#include "CocosPopupView.h"
#include "CocosSceneManager.h"
#include "CocosCustomDefines.h"

CocosPopupView::CocosPopupView()
{

}

void CocosPopupView::OpenView(cocos2d::Scene* pRootScene, bool bOpenExplicit, void* pViewData)
{
	m_ViewData = pViewData;
	Load(false);

	StartINAnimation(bOpenExplicit);
}

void CocosPopupView::StartINAnimation(bool IsOpenExplicit)
{
	if (m_pViewNode != NULL && !m_pViewNode->isVisible())
	{
		m_pViewNode->setVisible(true);
		CALL_COCOS_ANIMATION_BY_ACTION(m_pViewNode, m_pActionTimeline, m_sAnimIN, CocosPopupView::OnINAnimationStart, CocosPopupView::OnINAnimationEnd, this, true, 0);
	}
}

void CocosPopupView::CloseView(bool bOpenExplicit)
{
	StartOUTAnimation(bOpenExplicit);
}

void CocosPopupView::StartOUTAnimation(bool IsOpenExplicit)
{
	if (m_pViewNode != NULL && m_pViewNode->isVisible())
	{
		CALL_COCOS_ANIMATION_BY_ACTION(m_pViewNode, m_pActionTimeline, m_sAnimOUT, CocosPopupView::OnOUTAnimationStart, CocosPopupView::OnOUTAnimationEnd, this, true, 0);
	}
}

void CocosPopupView::OnOUTAnimationEnd(const std::string& sAnimName)
{
	CocosView::OnOUTAnimationEnd(sAnimName);
	CocosSceneManager::Get().TryOpenNextPopupInQueue();
}