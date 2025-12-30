#pragma once
#include <string>

class CocosView;
class ICocosViewLifeCycle
{
	public:
		ICocosViewLifeCycle();
		virtual ~ICocosViewLifeCycle() {};

		void			SetCocosView(CocosView*	pCocosView) { m_pCocosView = pCocosView; }
		CocosView*		GetCocosView() { return m_pCocosView; }

				bool	HandleBackButton() { if (m_bIsBackButtonEnabled)  return OnBackButtonPressed(); else return true; }
		virtual bool	OnBackButtonPressed()	{ return false; }

		virtual void	AttachToScene() { };
		virtual void	DetachFromScene() { };

		virtual void	PostAttachToScene() { };
		virtual void	PreDetachFromScene();

		virtual void	OnLifeCycleFocusLost() { };
		virtual void	OnLifeCycleFocusGained() { };

		virtual void	LoadMembers() { };
		virtual void	UnLoadMembers() { };
				void	RemoveMessages();

		virtual void	OnViewPopped() { };
		virtual void	OnPopupClosed(std::string sPopupName) { };

		virtual void	SetBackButtonEnabled(bool enabled)	{ m_bIsBackButtonEnabled = enabled; }
	private:
		CocosView*		m_pCocosView;
		bool			m_bIsBackButtonEnabled;
};
///////////////////////////////////////////////////////////////////////////////////////////////////////