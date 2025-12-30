#ifndef UISCROLLVIEWEXTENDED_H
#define UISCROLLVIEWEXTENDED_H

#include "UIScrollView.h"

#define UISCROLLVIEW_EXTRA_PADDING	2
//TODO:
//1) This is an initial draft for development. So still need to finish of certain visual artifacts.
//2) Code cleanups if any.
//3) Error check have not yet done for now to test the stability of the code. Once the code is full proof we will do the error checks.
//4) Jump to position is not tested yet.
//

NS_CC_BEGIN
namespace ui {

	//Observer functions for clients who wants docking features.
	class CC_GUI_DLL MUIScrollViewDockingObserver
	{
	public:
		enum class eDockingPosition
		{
			kNone,
			kTopOrLeft,
			kBottomOrRight
		};
		virtual cocos2d::Node* onAllocateDockElement(cocos2d::Node* control) = 0;
		virtual void onUpdateDockElement(cocos2d::Node* control, Node* node, eDockingPosition dockingPosition) = 0;
	};

	//Observer functions for clients to get the extended scroll view events. These functions must be implemented by the clients.
	class CC_GUI_DLL MUIScrollViewObserver
	{
	public:
		virtual cocos2d::Node* onAllocateVirtualElement(cocos2d::Node* control, int index) = 0;
		virtual void onPostInit() {}
		virtual void onElementUpdate(cocos2d::Node* control, int index, Node* node) = 0;
	};
	//

	class CC_GUI_DLL  UIScrollViewExtended : public ScrollView
	{
		//DECLARE_CLASS_GUI_INFO

	public:
		UIScrollViewExtended();
		virtual ~UIScrollViewExtended();

		static UIScrollViewExtended* createExtendedScrollView();

		void SetUiscrollview_extra_padding(int val) { _uiscrollview_extra_padding = val; }
		//We can call this init function OR we can explicitly call setUIScrollViewObserver() & setSingleElementSize(). Which ever is convinient.
		void initScrollViewExtended(MUIScrollViewObserver* observer, Size singleElementSize, Direction direction, MUIScrollViewDockingObserver* dockObserver=NULL, int dockElementIndex=-1);

		//To get events for the client class. Client class must implement MUIScrollViewObserver. This variable must be initialized before setting the content or element size to scrollview.
		void setUIScrollViewObserver(MUIScrollViewObserver* observer);

		//This function has to be called prior to setNoOfElements(). It will internally call onAllocateVirtualElement() of the client class.
		void setSingleElementSize(Size sz, bool overrideVirtualItemAllocation = false);

		//This function is used to set the total no:of elements which the scroll view holds.
		//Note: Extended scroll view will reset its position to default (top/left) when you call this function.
		void setNoOfElements(int numberOfElements);
		int getNoOfElements()	{ return _numberOfElements;	}

		Size getSingleElementSize()		{ return _singleElementSize;	 }

		//docking functions
		void setUIScrollViewDockingObserver(MUIScrollViewDockingObserver* observer);
		void setDockingElementIndex(int dockElemntIndex);
		int getDockingElementIndex()	{ return _dockElemntIndex; }
		cocos2d::Node* getDockingElement()	{ return _dockElement; }

		void setDockVisibility(bool bVisibility) { _dockElement->setVisible(bVisibility); }

		void forceSpeedLimitForScrolling(bool flag)	{ _forceSpeedLimitForScrolling = flag;	 }
		void allocateIfOverrided(int numberOfElements);

	protected:
		virtual void moveInnerContainer(const Vec2& deltaMove, bool canStartBounceBack);

		//override functions
		virtual void addChild(Node* child)override;
		virtual void addChild(Node * child, int localZOrder)override;
		virtual void addChild(Node* child, int localZOrder, int tag) override;
		virtual void addChild(Node* child, int localZOrder, const std::string &name) override;

	private:
		void setNextIndex(int index)	{ _nextIndexToChange = index; }
		void upateElementsOnMove();
		void addVirtualElement(Node* child);

		float getNoOfElementsInWindow();

		void doDockOnMove(const Vec2& containerPos, const Size& windowSz);

		Size _singleElementSize;
		std::vector<Node*> _virtualElementList;	//virtual elements which the extended scrollview actually allocates for its use.
		bool _isInitialised;

		MUIScrollViewObserver* _uiScrollViewObserver;
		int _nextIndexToChange;			//-1 is not defined.
		int _numberOfElements;			//total no:of elements of this scrollview.
		float _numberOfVirtualElementsCanAccomodateOnWindow;

		Vec2 _initialInnerContainerPosition;
		Size _initialContentSize;

		//docking feature
		MUIScrollViewDockingObserver* _uiScrollViewDockingObserver;
		int _dockElemntIndex;			//-1 = Not set
		cocos2d::Node* _dockElement;
		MUIScrollViewDockingObserver::eDockingPosition _currentDockingPosition;

		int _lastLeftIndexUpdated;
		int _uiscrollview_extra_padding;

		bool _forceSpeedLimitForScrolling;
		bool _overrideVirtualItemAllocation;
	};
}
NS_CC_END
#endif