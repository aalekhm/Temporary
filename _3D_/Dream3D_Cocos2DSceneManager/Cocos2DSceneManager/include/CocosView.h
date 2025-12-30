#pragma once
#include <cocos2d.h>
#include "ICocosViewLifeCycle.h"
#include "ActionTimeline/CCActionTimeline.h"
#include "json.hpp"

class CocosView
{
	public:
		CocosView();
		virtual									~CocosView() { };

		void									OnNodeLoaded(cocos2d::Ref* pObject);
		void									RefreshDataFromJson(nlohmann::json& jView);

		void									OpenView(cocos2d::Scene* pRootScene, bool bOpenExplicit = false, void* pViewData = NULL);
		void									CloseView(bool bOpenExplicit = false);
		void									OnViewPopped();

		const std::string&						GetViewName() { return m_sViewName; }

		void									OnINAnimationStart(const std::string& sAnimName);
		void									OnOUTAnimationStart(const std::string& sAnimName);

		void									OnINAnimationEnd(const std::string& sAnimName);
		void									OnOUTAnimationEnd(const std::string& sAnimName);

		void									OnDetachAnimationDone(ICocosViewLifeCycle* pICocosViewLifeCycle);

		const std::string&						GetStateID() { return m_sStateID; }
		cocos2d::Node*							GetViewNode() { return m_pViewNode; }
		void 									Load(bool partialLoad = true);
		void									UnLoad();
		void									OnLifeCycleFocusLost();
		void									OnLifeCycleFocusGained();
		void									ResetNode() { m_pViewNode = NULL; }

		const std::string&						GetCSBFileName() const { return m_sCSBFilename; }
		const std::string&						GetBGAnimTransitionIN() { return m_sBGTransitionIN; }
		const std::string&						GetBGAnimTransitionOUT() { return m_sBGTransitionOUT; }
		cocostudio::timeline::ActionTimeline*	GetActionTimeline() { return m_pActionTimeline; }

		const std::string&						GetAnimTransitionIN() { return m_sAnimIN; }
		const std::string&						GetAnimTransitionOUT() { return m_sAnimOUT; }

		bool									IsLoadViewOnDemand() { return m_bLoadOnDemand; }
		bool									IsUnLoadViewOnClose() { return m_bUnLoadOnClose; }
		bool									IsCacheViewOnDemand() { return m_bCacheFully; }
		bool									HasMultipleInstances() { return m_bHasMultipleInstances; }

		void									scheduleAllParticleUpdates();
		void									unscheduleAllParticleUpdates();

		void*									GetViewData(){ return m_ViewData; }
		void									SetViewData(void *viewData){ m_ViewData = viewData; }
		bool									GetBGHasUnifiedHeader() { return m_bBGHasUnfiedHeader; }
		const std::string&						GetSpriteSheetName() { return m_sSpriteSheetName; }
		void									SetUnLoadViewOnClose(bool unload) { m_bUnLoadOnClose = unload; }

		void									UpdateFonts();

        bool                                    IsPopup() const { return m_bIsPopup; }
		bool									HasCustomUIParameters() { return m_bHasCustomUIParameters; }
	protected:
		virtual void							OnShow();
		virtual void							OnHide();

		virtual void							OnLoad();
		virtual void							OnUnLoad();
			
		cocos2d::Node*							m_pViewNode;
		cocostudio::timeline::ActionTimeline*	m_pActionTimeline;

		std::string								m_sViewName;

		std::string								m_sAnimIN;
		std::string								m_sAnimOUT;
		std::string								m_sCSBFilename;
		std::string								m_sStateID;
		std::string								m_sZDepth;
		bool									m_bHasBG;
		std::string								m_sBGTransitionIN;
		std::string								m_sBGTransitionOUT;
		bool									m_bShowBack;
		bool									m_bIsPopup;
		bool									m_bLoadOnDemand;
		bool									m_bCacheFully;
		bool									m_bUnLoadOnClose;
		bool									m_bHasCustomUIParameters;
		bool									m_bHasMultipleInstances;

		void*									m_ViewData;
		std::string								m_sSpriteSheetName;
        std::string								m_sCustomClassName;
	private:
		void									StartINAnimation(bool IsOpenExplicit);
		void									StartOUTAnimation(bool IsOpenExplicit);

		void									HandleViewSpecifics();
		void									HandleBG();
		void									CleanUp();

		bool									CheckAndAddParticles(cocos2d::Ref* pObject);
		bool									CheckAndAddLifeCycleChild(cocos2d::Ref* pObject);
		bool									CheckAndUpdateFonts(cocos2d::Ref* pObject);

		std::vector<cocos2d::ParticleSystemQuad*>		m_vParticles;
		std::vector<ICocosViewLifeCycle*>				m_vLifeCycleChildrens;

		std::vector<bool>						m_vLifeCycleChildrensDetachAnimationFlag;
		bool									m_bIsCloseExplicit;
		bool									m_bIsNodeReadyForRender;
		int32_t									m_iZDepth;
		bool									m_bBGHasUnfiedHeader;

		void									HandleCustomUIParameters(cocos2d::Node* pObject);
		//std::map<std::string, EA::Json::JsonDomObject*>		m_mapCustomUINodeParameters_NameObject;
};