#include "ICocosViewLifeCycle.h"
#include "CocosSceneManager.h"

ICocosViewLifeCycle::ICocosViewLifeCycle()
: m_pCocosView(NULL)
, m_bIsBackButtonEnabled(true)
{

}

void ICocosViewLifeCycle::RemoveMessages()
{			
}

void ICocosViewLifeCycle::PreDetachFromScene() 
{ 
	m_pCocosView->OnDetachAnimationDone(this); 
}
