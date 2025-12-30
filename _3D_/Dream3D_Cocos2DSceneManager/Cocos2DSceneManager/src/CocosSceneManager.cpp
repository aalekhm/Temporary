
#include "CocosSceneManager.h"
#include "CocosCustomDefines.h"
#include "editor-support/cocostudio/CCSpriteFrameCacheHelper.h"
#include "ui/UIScrollViewExtended.h"
#include "CocosSceneGenericPopup.h"
#include "CocosPopupView.h"

#define MENU_STATE					"MenuState"
#define GAME_STATE					"GameplayState"
#define PLIST_HOLIDAY_TOURNAMENT	"HolidayTournament_Menu/HolidayTournament_Menu.plist"

///////////////////////////////////////////////////////// CocosSceneManager /////////////////////////////////////////////////////////

		CocosSceneManager::CocosSceneManager()
			: m_pRootSceneNode(NULL)
			, m_bIsViewTransiting(false)
			, m_sCurrentPushedView("")
			, m_sCurrentPopedView("")
			, m_sReplaceView("")
			, m_sPushView("")
			, m_sPrevStateID("")
			, m_sCurrentStateID(STATE_ID_SPLASH)
			, m_sNextStateID("")
			, m_eStateLoading(eState_LoadingWait)
			, m_sPushViewAfterLoading("")
			, mArePopupsSuppressed(false)
			, m_sPathConstructedFromViewName("")
		{

		}

		void CocosSceneManager::Initialize(int32_t iWidth, int32_t iHeight)
		{
			m_iWidth = iWidth;
			m_iHeight = iHeight;

			InitCocos2d();

#if CC_FORCE_PLIST_IMAGES
			LoadCommonSpriteSheet();
#endif
			LoadHandlers();
		}

		void CocosSceneManager::Finalize()
		{
			//CleanUp();
#if defined (EA_PLATFORM_BLAST_EMULATOR)
			DetachKeyboardListeners();
#endif
			UnloadHandlers();
		}

		void CocosSceneManager::Update()
		{
			cocos2d::Director* pDirector = cocos2d::Director::getInstance();
			if (pDirector != NULL)
				pDirector->mainLoop();
		}

		void CocosSceneManager::CleanUp()
		{
			m_ViewsHashMap.clear();
			m_CustomShaderPrograms.clear();
		}

		void CocosSceneManager::LoadHandlers()
		{
			REGISTER_MESSAGE_HANDLER(this, Message::eIdCocosPushView);
			REGISTER_MESSAGE_HANDLER(this, Message::eIdCocosPopView);
			REGISTER_MESSAGE_HANDLER(this, Message::eIdCocosReplaceView);
			REGISTER_MESSAGE_HANDLER(this, Message::eIdCocosTransitionToGame);
			REGISTER_MESSAGE_HANDLER(this, Message::eIdCocosTransitionFromGame);
			REGISTER_MESSAGE_HANDLER(this, Message::eIdCocosQuitFromGame);
			REGISTER_MESSAGE_HANDLER(this, Message::eIdCocosStartLoading);
			REGISTER_MESSAGE_HANDLER(this, Message::eIdCocosEndLoading);
			REGISTER_MESSAGE_HANDLER(this, Message::eidChangeLanguage);
			REGISTER_MESSAGE_HANDLER(this, Message::eIdBackKeyPressed);
			REGISTER_MESSAGE_HANDLER(this, Message::eIDNarc_appEngine_loadedServerFile);
			REGISTER_MESSAGE_HANDLER(this, Message::eIdCocosApplyUserData);
			REGISTER_MESSAGE_HANDLER(this, Message::eIdHandleCustomUIParameters);
			REGISTER_MESSAGE_HANDLER(this, Message::eIdGameSessionGameTypeChanged);
			REGISTER_MESSAGE_HANDLER(this, Message::eIDNarc_appEngine_playerProfileInvalid);
#if SWITCH_TETRIS_USE_DLC
			REGISTER_MESSAGE_HANDLER(this, Message::eIdDLCCompleted);
#endif
		}

		void CocosSceneManager::AttachKeyboardListeners()
		{
		}

		void CocosSceneManager::OnKeyDown(void* src, int32_t key)
		{
		}

		void CocosSceneManager::OnKeyRepeat(void* src, int32_t key)
		{

		}

		void CocosSceneManager::OnKeyUp(void* src, int32_t key)
		{

		}

		void CocosSceneManager::OnKeyCancel(void* src, int32_t key)
		{

		}

		void CocosSceneManager::OnCharacter(void* src, char16_t character)
		{

		}

		void CocosSceneManager::OnMouseDown(int mCode, int x, int y)
		{
			DispatchMouseEventToCocos(mCode, x, y, cocos2d::EventTouch::EventCode::BEGAN);
		}

		void CocosSceneManager::OnMouseMove(int mCode, int x, int y)
		{
			DispatchMouseEventToCocos(mCode, x, y, cocos2d::EventTouch::EventCode::MOVED);
		}

		void CocosSceneManager::OnMouseUp(int mCode, int x, int y)
		{
			DispatchMouseEventToCocos(mCode, x, y, cocos2d::EventTouch::EventCode::ENDED);
		}

		void CocosSceneManager::DetachKeyboardListeners()
		{
		}

		void CocosSceneManager::DispatchMouseEventToCocos(int mCode, int x, int y, cocos2d::EventTouch::EventCode eEventCode)
		{
			cocos2d::EventDispatcher* pDelegate = cocos2d::Director::getInstance()->getEventDispatcher();

			cocos2d::Touch pTouch;
			pTouch.setTouchInfo(mCode, x, y);

			cocos2d::EventTouch touchEvent;

			std::vector<cocos2d::Touch*> touches;
			touches.push_back(&pTouch);

			touchEvent.setTouches(touches);
			touchEvent.setEventCode(eEventCode);
			pDelegate->dispatchEvent(&touchEvent);
		}

		void CocosSceneManager::UnloadHandlers()
		{
			UNREGISTER_MESSAGE_HANDLER(this, Message::eIdCocosPushView);
			UNREGISTER_MESSAGE_HANDLER(this, Message::eIdCocosPopView);
			UNREGISTER_MESSAGE_HANDLER(this, Message::eIdCocosReplaceView);
		}

		void CocosSceneManager::InitCocos2d()
		{
			auto pScene = cocos2d::Scene::create();
			assert(pScene != NULL);

			m_pRootSceneNode = pScene;
			if (m_pRootSceneNode != NULL)
			{
				cocos2d::Application::getInstance();
				cocos2d::Director* pDirector = cocos2d::CCDirector::getInstance();

				cocos2d::GLView* view = cocos2d::GLViewImpl::create("cocos2dx");
				{
					cocos2d::Size designResolution(m_iWidth, m_iHeight);
					cocos2d::Size studioResolution(COCOSSTUDIO_DESIGN_RESOLUTION_WIDTH, COCOSSTUDIO_DESIGN_RESOLUTION_HEIGHT);

					view->setFrameSize((float)designResolution.width, (float)designResolution.height);
					view->setDesignResolutionSize((float)designResolution.width, (float)designResolution.height, ResolutionPolicy::NO_BORDER);
					pDirector->setOpenGLView(view);
					pDirector->setCocosStudioDesignResolution(studioResolution);
					pDirector->setAnimationInterval(1.0f / 60);
					pDirector->setContentScaleFactor(1.0f);
					pDirector->setDPI(72);
					pDirector->setClearColor(cocos2d::Color4F(0.0f, 0.0f, 0.0f, 1.0f));

					view->setFrameSize((float)designResolution.width, (float)designResolution.height);
					view->setDesignResolutionSize((float)designResolution.width, (float)designResolution.height, ResolutionPolicy::NO_BORDER);
				}

				pDirector->runWithScene(m_pRootSceneNode);
			}
		}

		void CocosSceneManager::AddSearchPath(std::string sPath)
		{
			cocos2d::FileUtils* pCCFileUtils = cocos2d::FileUtils::getInstance();
			pCCFileUtils->addSearchPath(sPath);
		}

		void CocosSceneManager::OnLifeCycleFocusLost()
		{
            Director::getInstance()->pause();
			CocosView* pTopMostView = GetTopMostView();
			if (pTopMostView != NULL)
			{
				pTopMostView->OnLifeCycleFocusLost();
			}
		}

		void CocosSceneManager::OnLifeCycleFocusGained()
		{
            Director::getInstance()->resume();
			CocosView* pTopMostView = GetTopMostView();
			if (pTopMostView != NULL)
			{
				pTopMostView->OnLifeCycleFocusGained();
			}
		}

		void CocosSceneManager::LoadCocosViewsJsonInfo(std::string sCocosViewFilePath)
		{
			std::string sCocosViewsJson = CCFileUtils::getInstance()->getStringFromFile(sCocosViewFilePath);
			nlohmann::json jCocosViewsJson = nlohmann::json::parse(sCocosViewsJson.c_str(), nullptr, false);
			if (jCocosViewsJson.type() != nlohmann::json::value_t::discarded)
			{
				nlohmann::json& jViews = jCocosViewsJson["Views"];
				int32_t iArraySize = jViews.size();
				for (uint32_t i = 0; i < iArraySize; i++)
				{
					nlohmann::json jView = jViews.at(i);
					if (jView != nlohmann::json::value_t::discarded)
					{
						std::string sViewName = jView["viewName"];
						CocosView* pCocosView = m_ViewsMap[sViewName].get();
						if (pCocosView == nullptr)
						{
							m_ViewsMap[sViewName] = std::move(std::make_unique<CocosView>());
							pCocosView = m_ViewsMap[sViewName].get();
						}
						
						pCocosView->RefreshDataFromJson(jView);
					}
				}
			}
		}

		const std::tuple<std::string, std::string>	CocosSceneManager::GetShaderPrgramByName(const std::string& sShaderProgramName)
		{
			std::tuple<std::string, std::string> tCustomShaderProgram;
			auto it = m_CustomShaderPrograms.find(sShaderProgramName);
			if (it != m_CustomShaderPrograms.end())
			{
				tCustomShaderProgram = it->second;
			}
			
			return tCustomShaderProgram;
		}

		bool CocosSceneManager::HandleMessage(int32_t messageId, void* pMessage)
		{
			//switch (messageId)
			//{
			//	case Message::eIdCocosPushView:
			//	{
			//		DECODE_MESSAGE_AND_CALL_COCOS_PUSHVIEW(pMessage);
			//	}
			//	break;
			//	case Message::eIdCocosPopView:
			//	{
			//		PopView();
			//	}
			//	break;
			//	case Message::eIdCocosReplaceView:
			//	{
			//		DECODE_MESSAGE_AND_CALL_COCOS_REPLACEVIEW(pMessage);
			//	
			//	}
			//	break;
			//}

			return true;
		}

		bool CocosSceneManager::HandleBackButtonForPopups()
		{
			if (HasValidPopUps())
			{
				CocosPopupQueueElement sCocosPopupQueueElement = m_vPopUpStack.front();
				CocosView* pCocosView = GetView(sCocosPopupQueueElement.m_sPopupName);
				if (pCocosView != NULL)
				{
					ICocosViewLifeCycle* pICocosViewLifeCycle = dynamic_cast<ICocosViewLifeCycle*>(pCocosView->GetViewNode());
					if (pICocosViewLifeCycle && !pICocosViewLifeCycle->HandleBackButton())
					{
						ClosePopUp();
					}
					return true;
				}
			}
			return false;
		}

		void CocosSceneManager::RefreshLocalization()
		{
			std::string fontName;

			for (auto itr = m_ViewsMap.begin(); itr != m_ViewsMap.end();itr++)
			{
				CocosView* view = itr->second.get();

				//if (GetCurrentStateID() == view->GetStateID()) //commenting to refresh current and commonstate views
				{
					cocos2d::Node *parent = view->GetViewNode();
					UpdateChildLocalization(parent,fontName);
				}

			}
		}

		void CocosSceneManager::UpdateChildLocalization(cocos2d::Node *parent, std::string fontName)
		{
            if (parent == NULL)
            {
                return;
            }
			auto childVector = parent->getChildren();
			for (const auto& child : childVector)
			{

				cocos2d::ui::Text *uiText = dynamic_cast<cocos2d::ui::Text*>(child);
				if (NULL != uiText && uiText->getFontName() != "")
				{
					uiText->setFontName(fontName);
					uiText->refreshLocalization();
				}
				
				cocos2d::Label *uiLabel = dynamic_cast<cocos2d::Label *>(child);
				if (NULL != uiLabel)
				{
					cocos2d::TTFConfig ttfconfig = uiLabel->getTTFConfig();
					ttfconfig.fontFilePath = fontName;
					uiLabel->setTTFConfig(ttfconfig);
					uiLabel->refreshLocalization();
				}

				cocos2d::ui::UIScrollViewExtended *uiScrollViewExtended = dynamic_cast<cocos2d::ui::UIScrollViewExtended *>(child);
				if (NULL != uiScrollViewExtended && uiScrollViewExtended->getDockingElement() != NULL)
				{
					UpdateChildLocalization(uiScrollViewExtended->getDockingElement(), fontName);
				}

				cocos2d::Node *uiNode = dynamic_cast<cocos2d::Node *>(child);
				if (NULL != uiNode)
				{
					UpdateChildLocalization(uiNode, fontName);
				}
				
			}
		}

		void CocosSceneManager::PushView(const std::string& sViewName)
		{
			if (HasValidPopUps())
				DismissAllPopUps();

			if (!IsViewTransiting() && IsValidView(sViewName))
			{
				// Show a 'Loading Scene' whenever there is a change in 'State'
				//if (IsChangeInState(sViewName))
				//{
				//	OpenLoading(sViewName);
				//}
				//else
				{
					if (HasValidViews())
					{
						std::string sTopScreen = GetTopOfViewStack();
						if (sTopScreen.compare(sViewName) != 0)
						{
							m_sPushView = sViewName;
							CloseView(sTopScreen);
						}
					}
					else
					{
						PushOnStackAndOpen(sViewName);
					}
				}
			}
		}

		void CocosSceneManager::PushViewFromRoot(const std::string& sViewName)
		{
			m_vViewStack.clear();
			OpenView(sViewName);
		}

		CocosView* CocosSceneManager::GetView(const std::string& sViewName)
		{
			CocosView* pCocosView = NULL;
			auto it = m_ViewsMap.find(sViewName);
			if (it != m_ViewsMap.end())
			{
				pCocosView = (CocosView*)it->second.get();
			}

			return pCocosView;
		}

		bool CocosSceneManager::IsViewOpen(const std::string& sViewName)
		{
			CocosView* pCocosView = GetView(sViewName);

			if (pCocosView && pCocosView->GetViewNode() && pCocosView->GetViewNode()->isVisible())
			{
				return true;
			}

			return false;
		}

		void CocosSceneManager::OpenLoading(const std::string& sViewName)
		{
			CocosView* pCocosView = GetView(sViewName);
			if (pCocosView != NULL)
			{
				m_sPrevStateID = m_sCurrentStateID;
				m_sNextStateID = pCocosView->GetStateID();

				m_sPushViewAfterLoading = sViewName;

				m_vViewStack.clear();
				m_eStateLoading = eState_LoadingProcess;

				OpenView(VIEW_LOADING);
			}
		}

		void CocosSceneManager::OpenViewPostLoading()
		{
			m_eStateLoading = eState_LoadingComplete;
			PushOnStackAndOpen(m_sPushViewAfterLoading);
		}

		void CocosSceneManager::PushOnStackAndOpen(const std::string& sViewName)
		{
			m_vViewStack.push_back(sViewName);
			OpenView(sViewName);
		}

		void CocosSceneManager::PostINAnimation()
			{
			m_sCurrentPushedView = "";
			if (m_eStateLoading == eState_LoadingComplete)
			{
				CloseView(VIEW_LOADING);
			}
		}

		bool CocosSceneManager::IsChangeInState(const std::string& sViewName)
		{
			CocosView* pCocosView = GetView(sViewName);
			if (pCocosView != NULL)
			{
				if (pCocosView->GetStateID() != m_sCurrentStateID)
				{
					return true;
				}
			}

			return false;
		}

		void CocosSceneManager::OpenViewExplicitly(const std::string& sViewName)
		{
			CocosView* pCocosView = GetView(sViewName);
			if (pCocosView != NULL)
			{
				pCocosView->OpenView(m_pRootSceneNode, true);
			}
		}

		void CocosSceneManager::OpenView(const std::string& sViewName, void* pViewData)
		{
			CocosView* pCocosView = GetView(sViewName);
			if (pCocosView != NULL)
			{
				m_sCurrentPushedView = sViewName;
				SetTransition();
				pCocosView->OpenView(m_pRootSceneNode, false, pViewData);
			}
		}

		void CocosSceneManager::ReplaceView(const std::string& sViewName)
		{
			if (HasValidPopUps())
				DismissAllPopUps();

			if (!IsViewTransiting() && HasValidViews() && IsValidView(sViewName))
			{
				m_sReplaceView = sViewName;
				PopView();
			}
		}

		void CocosSceneManager::PopView()
		{
			if (HasValidPopUps())
				DismissAllPopUps();

			if (CanPopView() && !IsViewTransiting() && HasValidViews())
			{
				std::string sTopScreen = GetTopOfViewStack();
				m_vViewStack.pop_back();

				CloseView(sTopScreen, true);
			}
		}

		void CocosSceneManager::CloseViewExplicitly(const std::string& sViewName)
		{
			CocosView* pCocosView = GetView(sViewName);
			if (pCocosView != NULL)
			{
				pCocosView->CloseView(true);
			}
		}

		void CocosSceneManager::CloseView(const std::string& sViewName, bool bIsViewPoped)
		{
				
			CocosView* pCocosView = GetView(sViewName);
			if (pCocosView != NULL)
			{
				SetTransition();
				m_sCurrentPopedView = sViewName;
				pCocosView->CloseView(false);
				if (bIsViewPoped)
				{
					pCocosView->OnViewPopped();
				}
			}
		}
		void CocosSceneManager::SetTransition()
		{
			m_bIsViewTransiting = true;
			Director::getInstance()->getEventDispatcher()->setEnabled(false);
		}
		void CocosSceneManager::EndTransition()
		{
			m_bIsViewTransiting = false;
			Director::getInstance()->getEventDispatcher()->setEnabled(true);
		}

		void CocosSceneManager::OpenPopUp(const std::string& sViewName, void* pViewData)
		{
			if (!(eState_LoadingProcess == m_eStateLoading) && IsValidView(sViewName) && !mArePopupsSuppressed)
			{
				if (IsQueueExclusivePopup(sViewName))
				{
					PutExclusivePopAtTheTopAndOpen(sViewName, pViewData);
				}
				else
				{
					if (NOT IsPopupAlreadyInQueue(sViewName))
					{
						m_vPopUpStack.push_back(CocosPopupQueueElement(sViewName, pViewData));
						if (m_vPopUpStack.size() == 1)
						{
							TryOpenNextPopupInQueue();
						}
					}
				}
			}
		}

		bool CocosSceneManager::IsPopupAlreadyInQueue(std::string sViewName)
		{
			CocosPopupView* pCocosPopupView = (CocosPopupView*)GetView(sViewName);
			if (pCocosPopupView != NULL && pCocosPopupView->HasMultipleInstances())
			{
				return false;
			}

			std::vector<CocosPopupQueueElement>::iterator itr = m_vPopUpStack.begin();
			std::vector<CocosPopupQueueElement>::iterator itrEnd = m_vPopUpStack.end();
			for (; itr != itrEnd; ++itr)
			{
				CocosPopupQueueElement& pCocosPopupQueueElement = *itr;
				if (pCocosPopupQueueElement.m_sPopupName == sViewName)
				{
					return true;
				}
			}

			return false;
		}

		void CocosSceneManager::PutExclusivePopAtTheTopAndOpen(std::string sViewName, void* pViewData)
		{
			CocosPopupView* pCocosPopupView = (CocosPopupView*)GetView(sViewName);
			assert(pCocosPopupView != NULL);
			if (pCocosPopupView != NULL)
			{
				CocosPopupQueueElement pCocosPopupQueueElement(sViewName, pViewData);

				if (m_vPopUpStack.size() > 0)
				{
					std::vector<CocosPopupQueueElement>::iterator itrFront = m_vPopUpStack.begin();
					m_vPopUpStack.insert(itrFront, pCocosPopupQueueElement);
				}
				else
				{
					m_vPopUpStack.push_back(CocosPopupQueueElement(sViewName, pViewData));
				}

				pCocosPopupView->OpenView(m_pRootSceneNode, true, pViewData);
			}
		}
		
		void CocosSceneManager::TryOpenNextPopupInQueue()
		{
			if (m_vPopUpStack.size() > 0)
			{
				CocosPopupQueueElement sNextPopup = m_vPopUpStack.front();
				{
					CocosPopupView* pCocosPopupView = (CocosPopupView*)GetView(sNextPopup.m_sPopupName);
					if (pCocosPopupView != NULL)
					{
						pCocosPopupView->OpenView(m_pRootSceneNode, true, sNextPopup.m_pPopupData);
					}
				}
			}
		}

		bool CocosSceneManager::IsQueueExclusivePopup(std::string sViewName)
		{
			return (sViewName == POPUP_GENERIC);
		}

		bool CocosSceneManager::IsPopupOpen(const std::string& sPopupName)
		{
			int32_t size = m_vPopUpStack.size();

			for (int32_t i = 0; i < size; i++)
			{
				if (sPopupName.compare(m_vPopUpStack.at(i).m_sPopupName) == 0)
					return true;
			}

			return false;
		}

		void CocosSceneManager::ClosePopUp(std::string sPopupName)
		{
			if (HasValidPopUps())
			{
				std::vector<CocosPopupQueueElement>::iterator itr		= m_vPopUpStack.begin();
				std::vector<CocosPopupQueueElement>::iterator itrEnd	= m_vPopUpStack.end();
				for (; itr != itrEnd; ++itr)
				{
					CocosPopupQueueElement sCocosPopupQueueElement = (*itr);
					if (sPopupName.compare(sCocosPopupQueueElement.m_sPopupName) == 0)
					{
						m_vPopUpStack.erase(itr);

						{
							CocosPopupView* pCocosPopupView = (CocosPopupView*)GetView(sPopupName);
							if (pCocosPopupView != NULL)
							{
								pCocosPopupView->CloseView(false);
							}

							CallOnPopupClosedTopView(sCocosPopupQueueElement.m_sPopupName);
						}
						break;
					}
				}
			}
		}


		void CocosSceneManager::ClosePopUp()
		{
			if (HasValidPopUps())
			{
				CocosPopupQueueElement sCocosPopupQueueElement = m_vPopUpStack.front();
				{
					std::vector<CocosPopupQueueElement>::iterator itrFront = m_vPopUpStack.begin();
					m_vPopUpStack.erase(itrFront);
				}

				{
					CocosPopupView* pCocosPopupView = (CocosPopupView*)GetView(sCocosPopupQueueElement.m_sPopupName);
					if (pCocosPopupView != NULL)
					{
						pCocosPopupView->CloseView(false);
					}

					CallOnPopupClosedTopView(sCocosPopupQueueElement.m_sPopupName);
				}
			}
		}

		void CocosSceneManager::CallOnPopupClosedTopView(std::string sPopupName)
		{
			if (HasValidViews())
			{
				std::string sTopScreen = m_vViewStack.back();
				ICocosViewLifeCycle* pICocosViewLifeCycle = dynamic_cast<ICocosViewLifeCycle*>(GetView(sTopScreen)->GetViewNode());
				if (pICocosViewLifeCycle != NULL)
				{
					pICocosViewLifeCycle->OnPopupClosed(sPopupName);
				}
			}
		}

		void CocosSceneManager::DismissAllPopUps()
		{
			while (m_vPopUpStack.size() > 0)
			{
				ClosePopUp();
			}
		}
		
		std::string CocosSceneManager::GetTopOfViewStack()
		{
			std::string sTopScreen = "";
			if (HasValidViews())
			{
				sTopScreen = m_vViewStack.back();
			}

			return sTopScreen;
		}

		bool CocosSceneManager::HasValidViews()
		{
			return (m_vViewStack.size() > 0);
		}

		bool CocosSceneManager::HasValidPopUps()
		{
			return (m_vPopUpStack.size() > 0);
		}
		
		bool CocosSceneManager::CanPopView()
		{
			bool bIsReplaceInProgress = (m_sReplaceView != "");
			return ( (m_vViewStack.size() > 1) || bIsReplaceInProgress );
		}

		bool CocosSceneManager::IsValidView(const std::string& sViewName)
		{
			auto it = m_ViewsMap.find(sViewName);
			return (it != m_ViewsMap.end());
		}

		void CocosSceneManager::OnINAnimationStart(const std::string& sAnimName)
		{
			CocosView* pCocosView = GetView(m_sCurrentPushedView);
			if (pCocosView != NULL)
			{
				pCocosView->OnINAnimationStart(sAnimName);
                
				if (m_sCurrentStateID == MENU_STATE)
                {
					PLAY_SFX("SFX_UI_MenuTransition");
                }
			}
		}

		void CocosSceneManager::OnINAnimationEnd(const std::string& sAnimName)
		{
			CocosView* pCocosView = GetView(m_sCurrentPushedView);
			if (pCocosView != NULL)
			{
				pCocosView->OnINAnimationEnd(sAnimName);

				EndTransition();

				PostINAnimation();
			}
		}

		void CocosSceneManager::OnOUTAnimationStart(const std::string& sAnimName)
		{
			CocosView* pCocosView = GetView(m_sCurrentPopedView);
			if (pCocosView != NULL)
			{
				pCocosView->OnOUTAnimationStart(sAnimName);
			}
		}

		void CocosSceneManager::OnOUTAnimationEnd(const std::string& sAnimName)
		{
			CocosView* pCocosView = GetView(m_sCurrentPopedView);
			if (pCocosView != NULL)
			{
				pCocosView->OnOUTAnimationEnd(sAnimName);

				EndTransition();

				PostOUTAnimation();
			}
		}

		void CocosSceneManager::PostOUTAnimation()
		{
			if (m_eStateLoading == eState_LoadingComplete)
			{
				m_sCurrentStateID = m_sNextStateID;
				m_sPrevStateID = m_sNextStateID = m_sPushViewAfterLoading = "";
				m_eStateLoading = eState_LoadingWait;
			}
			else
			if (m_sPushView.compare("") > 0)
			{
				PushOnStackAndOpen(m_sPushView);
				m_sPushView = "";
			}
			else
			if (m_sReplaceView.compare("") > 0)
			{
				PushOnStackAndOpen(m_sReplaceView);
				m_sReplaceView = "";
			}
			else
			if (m_sCurrentPopedView.compare("") > 0)
			{
				m_sCurrentPopedView = "";
				OpenView(GetTopOfViewStack());
			}
		}

		void CocosSceneManager::UnloadAllViews()
		{
			auto itr = m_ViewsMap.begin();
			while (itr != m_ViewsMap.end())
			{
				CocosView* pCocosView = itr->second.get();
				if (pCocosView)
				{
					cocos2d::Node* pSceneNode = pCocosView->GetViewNode();
					if (pSceneNode != NULL)
					{
						pCocosView->UnLoad();
					}
				}

				itr++;
			}

			m_vViewStack.clear();
			m_qLoadingTask.empty();
		}

		void CocosSceneManager::enqueueLoadingTask(std::function<void()> fLambda)
		{
			m_qLoadingTask.emplace(fLambda);
		}

		bool CocosSceneManager::IsLoadingQueueEmpty()
		{
			return m_qLoadingTask.empty();
		}

		int32_t CocosSceneManager::GetLoadTaskCount()
		{
			return m_qLoadingTask.size();
		}

		void CocosSceneManager::ProcessNextInLoadingQueue()
		{
			if (!m_qLoadingTask.empty())
			{
				std::function<void()> task = nullptr;
				task = m_qLoadingTask.front();
				m_qLoadingTask.pop();

				if (task != nullptr)
				{
					task();
				}
			}
		}

		void CocosSceneManager::UnLoadScenesWithStateID(const std::string& sStateID)
		{
			auto itr = m_ViewsMap.begin();
			while (itr != m_ViewsMap.end())
			{
				CocosView* pCocosView = (CocosView*)itr->second.get();
				if (pCocosView != NULL)
				{
					if (sStateID == pCocosView->GetStateID())
					{
						enqueueLoadingTask([this, pCocosView]()
						{
							cocos2d::Node* pSceneNode = pCocosView->GetViewNode();
							if (pSceneNode != NULL)
							{
								pCocosView->UnLoad();
							}
						}
						);
					}
				}

				itr++;
			}
		}

		void CocosSceneManager::UnLoadAssetsWithStateID(const std::string& sStateID)
		{
			UnLoadScenesWithStateID(sStateID);
			UnLoadAudioWithStateID(sStateID);
		}

		void CocosSceneManager::LoadAssetsWithStateID(const std::string& sStateID)
		{
			LoadScenesWithStateID(sStateID);
			LoadAudioWithStateID(sStateID);
		}

		void CocosSceneManager::UnLoadAudioWithStateID(const std::string& sStateID)
		{
			std::string sBankName = "";
			if (sStateID == MENU_STATE)
			{
				enqueueLoadingTask([]()
				{
					UNLOAD_AUDIO("SFX_UI_COMMON");
				});
			}
			else
			if (sStateID == GAME_STATE)
			{
				enqueueLoadingTask([]()
				{
					UNLOAD_AUDIO("SFX_INGAME");
					UNLOAD_AUDIO("BGM_BANK_INGAME");
				});
			}
		}

		void CocosSceneManager::LoadAudioWithStateID(const std::string& sStateID)
		{
			std::string sBankName = "";
			if (sStateID == MENU_STATE)
			{
				enqueueLoadingTask([]()
				{
					PRELOAD_AUDIO("SFX_UI_COMMON");
				});
			}
			else
			if (sStateID == GAME_STATE)
			{
				enqueueLoadingTask([]()
				{
					PRELOAD_AUDIO("SFX_INGAME");
					PRELOAD_AUDIO("BGM_BANK_INGAME");
				});
			}
		}
		void CocosSceneManager::LoadScenesWithStateID(const std::string& sStateID)
		{
			auto itr = m_ViewsMap.begin();
			while (itr != m_ViewsMap.end())
			{
				CocosView* pCocosView = (CocosView*)itr->second.get();
				if (pCocosView != NULL)
				{
					if (	sStateID == pCocosView->GetStateID() 
							&& 
							!pCocosView->IsLoadViewOnDemand()
					) {
						enqueueLoadingTask(	[this, pCocosView]()
											{
												pCocosView->Load();
											}
						);
					}
				}

				itr++;
			}
		}

#if CC_FORCE_PLIST_IMAGES
		void CocosSceneManager::LoadCommonSpriteSheet()
		{
			cocos2d::SpriteFrameCache::getInstance()->LoadSpriteSheetWithFile(PLIST_COMMON);
			cocos2d::SpriteFrameCache::getInstance()->LoadSpriteSheetWithFile(PLIST_HOLIDAY_TOURNAMENT);
			std::string str = "CommonState";
			LoadSpriteSheetsForState(str);
		}

		void CocosSceneManager::UnLoadCommonSpriteSheet()
		{
			cocos2d::SpriteFrameCache::getInstance()->UnLoadSpriteSheetWithFile(PLIST_COMMON);
			cocos2d::SpriteFrameCache::getInstance()->UnLoadSpriteSheetWithFile(PLIST_HOLIDAY_TOURNAMENT);
			std::string str = "CommonState";
			UnLoadSpriteSheetsForState(str);
		}

		void CocosSceneManager::LoadSpriteSheetsForState(std::string& sStateID)
		{
			auto itr = m_ViewsMap.begin();
			while (itr != m_ViewsMap.end())
			{
				CocosView* pCocosView = (CocosView*)itr->second;
				if (pCocosView != NULL)
				{
					if (sStateID == pCocosView->GetStateID())
					{
						std::string sFileName = "";
						if (pCocosView->GetSpriteSheetName() != "")
						{
							sFileName = pCocosView->GetSpriteSheetName();
							cocos2d::SpriteFrameCache::getInstance()->LoadSpriteSheetWithFile(sFileName);
						}
					}
				}

				itr++;
			}
		}

		void CocosSceneManager::UnLoadSpriteSheetsForState(std::string& sStateID)
		{
			auto itr = m_ViewsMap.begin();
			while (itr != m_ViewsMap.end())
			{
				CocosView* pCocosView = (CocosView*)itr->second;
				if (pCocosView != NULL)
				{
					if (sStateID == pCocosView->GetStateID())
					{
						std::string sFileName = "";
						if (pCocosView->GetSpriteSheetName() != "")
						{
							sFileName = pCocosView->GetSpriteSheetName();
							SpriteFrameCache::getInstance()->UnLoadSpriteSheetWithFile(sFileName);
						}
					}
				}

				itr++;
			}
		}
#endif

		void CocosSceneManager::ForceSuppressPopups(bool shouldSuppress)
		{
			if (shouldSuppress)
			{
				DismissAllPopUps();
			}

			mArePopupsSuppressed = shouldSuppress;
		}

		void CocosSceneManager::AddCustomChild(cocos2d::Node* pNode, std::string sViewDepth)
		{
			auto zOrder = GetZOrderByName(sViewDepth);
			CocosSceneManager::Get().GetRootScene()->addChild(pNode, zOrder);
		}

		int32_t CocosSceneManager::GetZOrderByName(const std::string& sViewDepth)
		{
			int32_t zOrder = 0;
			if (sViewDepth.compare(VIEW_Z_DEPTH_DEFAULT) == 0)
			{
				zOrder = 0;
			}
			else if(sViewDepth.compare(VIEW_Z_DEPTH_NEAREST) == 0)
			{
				zOrder = 10;
			}
			else if(sViewDepth.compare(VIEW_Z_DEPTH_NEAR) == 0)
			{
				zOrder = 1;
			}
			else if(sViewDepth.compare(VIEW_Z_DEPTH_FAR) == 0)
			{
				zOrder = -1;
			}
			return zOrder;
		}

		std::string CocosSceneManager::GetTopMostViewName()
		{
			static std::string topMostViewName;
			topMostViewName = "";
			if (eState_LoadingProcess == m_eStateLoading)
			{
				if (m_sPrevStateID.compare(STATE_ID_SPLASH) != 0)// don't show banner on splash to cover-flow loading screen
				{
					topMostViewName = VIEW_LOADING;
				}
			}
			else
			{
				if(HasValidPopUps())
				{
					topMostViewName = m_vPopUpStack.front().m_sPopupName;
				}
				else if(HasValidViews())
				{
					topMostViewName = m_vViewStack.back();
				}
			}
			
			return topMostViewName;
		}

		CocosView* CocosSceneManager::GetTopMostView()
		{
			std::string sTopMostViewName = GetTopMostViewName();
			CocosView* pCocosView = GetView(sTopMostViewName);
            return pCocosView;
		}
		///////////////////////////////////////////////////////// CocosSceneManager /////////////////////////////////////////////////////////