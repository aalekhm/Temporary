#pragma once

#include <cocos2d.h>
//#include "CocosSceneUtils.h"
#include "editor-support/cocostudio/cocostudio.h"
#include "CocosCustomDefines.h"
#include "CocosView.h"
#include "Singleton.h"
#include <map>
#include "json.hpp"

#define VIEW_Z_DEPTH_NEAR			"Near" //1
#define VIEW_Z_DEPTH_FAR			"Far" // -1
#define VIEW_Z_DEPTH_DEFAULT		"Default" //0
#define VIEW_Z_DEPTH_NEAREST		"Nearest"//10 

#define PLIST_COMMON				"Common/Common.plist"
#define PLIST_SCENE_BATTLES			"Scene_Battles/Scene_Battles.plist"

#define STATE_ID_SPLASH				"SplashState"
#define STATE_ID_COMMON				"CommonState"
#define STATE_ID_MENU				"MenuState"
#define STATE_ID_GAMEPLAY			"GameplayState"

using namespace cocos2d;

class ICocosViewLifeCycle;

typedef std::map<std::string, std::unique_ptr<CocosView>>			VIEW_MAP;
typedef std::unordered_map<uint32_t, std::string>					VIEW_HASH_MAP;


struct CocosPopupQueueElement
{
	CocosPopupQueueElement(std::string sPopupName, void* pPopupData)
	: m_sPopupName(sPopupName)
	, m_pPopupData(pPopupData)
	{ }

	std::string	m_sPopupName;
	void*			m_pPopupData;
};

class CocosSceneManager : public Singleton<CocosSceneManager>
{
	enum STATE_LOADING
	{
		eState_LoadingWait,
		eState_LoadingProcess,
		eState_LoadingComplete,
	};

	public:
												CocosSceneManager();
		virtual									~CocosSceneManager() = default;

		void									Initialize(int32_t iWidth, int32_t iHeight);
		void									Finalize();

		void									AddSearchPath(std::string sPath);
		void									LoadCocosViewsJsonInfo(std::string sCocosViewFilePath);

		void									OnLifeCycleFocusLost();
		void									OnLifeCycleFocusGained();

		void									Update();

		// IKeyListener
		virtual void							OnKeyDown(void* src, int32_t key);
		virtual void							OnKeyRepeat(void* src, int32_t key);
		virtual void							OnKeyUp(void* src, int32_t key);
		virtual void							OnKeyCancel(void* src, int32_t key);

		virtual void							OnMouseDown(int mCode, int x, int y);
		virtual void							OnMouseMove(int mCode, int x, int y);
		virtual void							OnMouseUp(int mCode, int x, int y);

		// ICharacterListener
		virtual void							OnCharacter(void* src, char16_t character);

		void									AddCustomChild(cocos2d::Node* pNode, std::string sViewDepth);
		cocos2d::Scene*							GetRootScene() { return m_pRootSceneNode; };

		void									PushView(const std::string& sViewName);
		void									PopView();
		void									ReplaceView(const std::string& sViewName);

		void									OpenViewExplicitly(const std::string& sSceneName);
		void									CloseViewExplicitly(const std::string& sViewName);
			
		void									ProcessNextInLoadingQueue();
		bool									IsLoadingQueueEmpty();
		int32_t									GetLoadTaskCount();
		int32_t									GetViewStackSize() { return m_vViewStack.size(); }

		void									OpenPopUp(const std::string& sViewName, void* pViewData = NULL);
		void									ClosePopUp();
		void									ClosePopUp(std::string sPopupName);
		void									CallOnPopupClosedTopView(std::string sPopupName);
		void									TryOpenNextPopupInQueue();

		const std::string&						GetPrevStateID() { return m_sPrevStateID; }
		const std::string&						GetCurrentStateID() { return m_sCurrentStateID; }
		const std::string&						GetNextStateID() { return m_sNextStateID; }

		void									OnINAnimationStart(const std::string& sAnimName);
		void									OnOUTAnimationStart(const std::string& sAnimName);
		void									OnINAnimationEnd(const std::string& sAnimName);
		void									OnOUTAnimationEnd(const std::string& sAnimName);

		void									enqueueLoadingTask(std::function<void()> fLambda);

		std::string								GetTopOfViewStack();
		std::string								GetTopMostViewName();
		CocosView*								GetTopMostView();

		bool									HasValidPopUps();

		CocosView*								GetView(const std::string& sViewName);
		bool									IsViewOpen(const std::string& sViewName);
		bool									IsPopupOpen(const std::string& sPopupName);

		bool									IsViewTransiting() { return m_bIsViewTransiting; }

		bool									ArePopupsSuppressed()	{ return mArePopupsSuppressed; }
        bool									IsValidView(const std::string& sViewName);

		void									UpdateChildLocalization(cocos2d::Node *parent, std::string fontName);

		bool									IsInLoadingState() { return (m_eStateLoading > eState_LoadingWait && m_eStateLoading <= eState_LoadingComplete); };
		const std::tuple<std::string, std::string>	GetShaderPrgramByName(const std::string& sShaderProgramName);
	protected:
		void									ForceSuppressPopups(bool shouldSuppress);

	private:
		void									CleanUp();
		virtual bool							HandleMessage(int32_t messageId, void* pMessage);

		void									InitCocos2d();
		void									DispatchMouseEventToCocos(int mCode, int x, int y, cocos2d::EventTouch::EventCode eEventCode);

		void									OpenView(const std::string& sSceneName, void* pViewData = NULL);
		void									CloseView(const std::string& sViewName, bool bIsViewPoped = false);

		int32_t									GetZOrderByName(const std::string& sViewDepth);
		void									LoadHandlers();
		void									AttachKeyboardListeners();
		void									DetachKeyboardListeners();
		void									UnloadHandlers();

		void									PushViewFromRoot(const std::string& viewName);
				
		bool									CanPopView();

		bool									HasValidViews();

		void									SetTransition();
		void									EndTransition();

		void									UnLoadAssetsWithStateID(const std::string& sStateID);
		void									LoadAssetsWithStateID(const std::string& sStateID);
		void									UnLoadScenesWithStateID(const std::string& sStateID);
		void									LoadScenesWithStateID(const std::string& sStateID);
		void									UnLoadAudioWithStateID(const std::string& sStateID);
		void									LoadAudioWithStateID(const std::string& sStateID);

		void									OpenLoading(const std::string& sViewName);
		void									OpenViewPostLoading();
		void									PostINAnimation();

		bool									IsChangeInState(const std::string& sSceneName);
		void									PostOUTAnimation();

		void									PushOnStackAndOpen(const std::string& sViewName);

		void									UnloadAllViews();
		void									DismissAllPopUps();

		void									RefreshLocalization();

		bool									IsPopupAlreadyInQueue(std::string sViewName);
		bool									IsQueueExclusivePopup(std::string sViewName);
		void									PutExclusivePopAtTheTopAndOpen(std::string sViewName, void* pViewData);

#if CC_FORCE_PLIST_IMAGES
		void									LoadCommonSpriteSheet();
		void									UnLoadCommonSpriteSheet();
		void									LoadSpriteSheetsForState(std::string& sStateID);
		void									UnLoadSpriteSheetsForState(std::string& sStateID);
#endif
		bool									HandleBackButtonForPopups();

		cocos2d::Scene*							m_pRootSceneNode;

		bool									m_bIsViewTransiting;

		VIEW_MAP						m_ViewsMap;
		VIEW_HASH_MAP					m_ViewsHashMap;
		std::map<std::string, std::tuple<std::string, std::string>>	m_CustomShaderPrograms;

		std::vector<std::string>			m_vViewStack;
		std::vector<CocosPopupQueueElement>	m_vPopUpStack;

		std::string							m_sCurrentPushedView;
		std::string							m_sCurrentPopedView;

		std::string							m_sReplaceView;
		std::string							m_sPushView;

		std::string							m_sPrevStateID;
		std::string							m_sCurrentStateID;
		std::string							m_sNextStateID;

		std::string							m_sPushViewAfterLoading;

		std::queue < std::function<void()>>	m_qLoadingTask;
		STATE_LOADING						m_eStateLoading;

		bool								mArePopupsSuppressed;

		std::string							m_sPathConstructedFromViewName;

		int32_t								m_iWidth;
		int32_t								m_iHeight;
};