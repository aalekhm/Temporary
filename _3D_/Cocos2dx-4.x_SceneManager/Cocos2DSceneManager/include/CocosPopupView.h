#pragma once
#include "CocosView.h"

class CocosPopupView : public CocosView
{
	public:
		CocosPopupView();
		virtual									~CocosPopupView() { };

		void									OpenView(cocos2d::Scene* pRootScene, bool bOpenExplicit = false, void* pViewData = NULL);
		void									CloseView(bool bOpenExplicit = false);

		void									StartINAnimation(bool IsOpenExplicit);
		void									StartOUTAnimation(bool IsOpenExplicit);
		void									OnOUTAnimationEnd(const std::string& sAnimName);
	private:
};
