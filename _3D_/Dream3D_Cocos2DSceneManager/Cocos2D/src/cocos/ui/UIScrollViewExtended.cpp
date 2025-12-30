#include "ui/UIScrollViewExtended.h"

NS_CC_BEGIN

namespace ui {

	//IMPLEMENT_CLASS_GUI_INFO(UIScrollViewExtended)

	UIScrollViewExtended::UIScrollViewExtended() :
		ScrollView()
	{
		_uiScrollViewObserver = NULL;
		_nextIndexToChange = -1;
		_numberOfElements = 0;
		_isInitialised = false;
		_numberOfVirtualElementsCanAccomodateOnWindow = 0;
		_lastLeftIndexUpdated = -1;
		_overrideVirtualItemAllocation = false;
		forceSpeedLimitForScrolling(true);
		_uiscrollview_extra_padding = UISCROLLVIEW_EXTRA_PADDING;
		//docking
		_uiScrollViewDockingObserver = NULL;
		_dockElemntIndex = -1;
		_dockElement = NULL;
		_currentDockingPosition = MUIScrollViewDockingObserver::eDockingPosition::kNone;
	}

	UIScrollViewExtended::~UIScrollViewExtended()
	{

	}

	UIScrollViewExtended* UIScrollViewExtended::createExtendedScrollView()
	{
		UIScrollViewExtended* widget = new (std::nothrow) UIScrollViewExtended();
		if (widget && widget->init())
		{
			widget->autorelease();
			return widget;
		}
		CC_SAFE_DELETE(widget);
		return nullptr;
	}

	void UIScrollViewExtended::initScrollViewExtended(MUIScrollViewObserver* observer, Size singleElementSize, Direction direction, MUIScrollViewDockingObserver* dockObserver, int dockElementIndex)
	{
		setUIScrollViewObserver(observer);
		setUIScrollViewDockingObserver(dockObserver);
		setDirection(direction);
		setSingleElementSize(singleElementSize);
		setDockingElementIndex(dockElementIndex);
	}

	void UIScrollViewExtended::setUIScrollViewObserver(MUIScrollViewObserver* observer)
	{
		_uiScrollViewObserver = observer;
	}

	void UIScrollViewExtended::setSingleElementSize(Size sz, bool overrideVirtualItemAllocation)
	{
		_overrideVirtualItemAllocation = overrideVirtualItemAllocation;
		_singleElementSize = sz;

		float noOfElementsInAWindow = 1.0f;
		if (_direction == Direction::VERTICAL)
			noOfElementsInAWindow = getContentSize().height / sz.height;
		else if (_direction == Direction::HORIZONTAL)
			noOfElementsInAWindow = getContentSize().width / sz.width;

		_numberOfVirtualElementsCanAccomodateOnWindow = noOfElementsInAWindow;

		if (!_overrideVirtualItemAllocation)	//don't allocate if the client is overriding allocation for extended scroll view.
		{
			float check_int_val = (int)noOfElementsInAWindow;
			bool isreminder = noOfElementsInAWindow > check_int_val;

			int start_index = _virtualElementList.size();
			int noOfVirtualElementsToAllocate = ((int)noOfElementsInAWindow + (isreminder ? 1 : 0) + _uiscrollview_extra_padding) - _virtualElementList.size();

			for (int x = start_index; x < start_index + noOfVirtualElementsToAllocate; x++)
			{
				Node* node = _uiScrollViewObserver->onAllocateVirtualElement(this, x);
				addVirtualElement(node);
			}
		}

		if (_uiScrollViewDockingObserver)
		{
			_dockElement = _uiScrollViewDockingObserver->onAllocateDockElement(this);
			Node::addChild(_dockElement, _dockElement->getLocalZOrder(), _dockElement->getName());
			setDockVisibility(false);
		}
	}

	void UIScrollViewExtended::allocateIfOverrided(int numberOfElements)
	{
		if (!_overrideVirtualItemAllocation) return;

		removeAllChildren();

		for (int x = 0; x < numberOfElements; x++)
		{
			Node* node = _uiScrollViewObserver->onAllocateVirtualElement(this, x);
			addVirtualElement(node);
		}
	}

	float UIScrollViewExtended::getNoOfElementsInWindow()
	{
		float wnd_width = getContentSize().width;
		float wnd_height = getContentSize().height;
		float noOfElementsInAWindow = 0.0f;

		if (getDirection() == cocos2d::ui::ScrollView::Direction::VERTICAL)
		{
			noOfElementsInAWindow = (wnd_height / _singleElementSize.height) + _uiscrollview_extra_padding;
			if (noOfElementsInAWindow > (int)noOfElementsInAWindow)
				noOfElementsInAWindow += 1.0f;
		}
		else if (getDirection() == cocos2d::ui::ScrollView::Direction::HORIZONTAL)
		{
			noOfElementsInAWindow = (wnd_width / _singleElementSize.width) + _uiscrollview_extra_padding;
			if (noOfElementsInAWindow > (int)noOfElementsInAWindow)
				noOfElementsInAWindow += 1.0f;
		}

		return noOfElementsInAWindow;
	}

	void UIScrollViewExtended::setNoOfElements(int numberOfElements)
	{
		_virtualElementList.clear();

		_numberOfElements = numberOfElements;

		if (_numberOfElements > 0)
			_nextIndexToChange = 0;

		//hide all elements first.
		for (int x = 0; x < _innerContainer->getChildrenCount(); x++)
		{
			Node* node = _innerContainer->getChildren().at(x);
			node->setVisible(false);
			_virtualElementList.push_back(node);
		}
		//

		if (_isInitialised==false)	//First run
		{
			_initialInnerContainerPosition = _innerContainer->getPosition();
			_initialContentSize = _contentSize;
		}
		else
		{
			_innerContainer->setPosition(_initialInnerContainerPosition);
			_contentSize = _initialContentSize;
		}
		_isInitialised = false;	//Don't remove this from here. Else jumpToTop will be called and the indices will change.

		if (_direction == Direction::HORIZONTAL)
			jumpToLeft();
		else if (_direction == Direction::VERTICAL)
			jumpToTop();

		int nChilds = (int)_innerContainer->getChildren().size();
		if (nChilds>0)	//safety check
		{
			Node* tempElement = _innerContainer->getChildren().at(0);
			float deltaX = tempElement->getPositionX();
			float deltaY = tempElement->getPositionY();

			//initialize the elements
			int elementsToUpdate = (_numberOfElements < _innerContainer->getChildrenCount()) ? _numberOfElements : _innerContainer->getChildrenCount();
			int noOfElementsInWindow = (int)getNoOfElementsInWindow();
            if (_overrideVirtualItemAllocation)
            {
                //re-assign the no : of elements, if we are overriding the feature.
                _numberOfElements = elementsToUpdate;
            }
            
			for (int x = 0; x < elementsToUpdate; x++)
			{
				Node* node = _innerContainer->getChildren().at(x);
				if (_direction == Direction::HORIZONTAL)
					node->setPosition(x*_singleElementSize.width, deltaY);
				else if (_direction == Direction::VERTICAL)
				{
					if ((float)_numberOfElements < _numberOfVirtualElementsCanAccomodateOnWindow)
						node->setPosition(deltaX, ((_numberOfVirtualElementsCanAccomodateOnWindow - 1.0f) - x)*_singleElementSize.height);
					else
						node->setPosition(deltaX, ((_numberOfElements - 1) - x)*_singleElementSize.height);
				}
				node->setVisible(true);
				_lastLeftIndexUpdated = 0;
				if (x <= noOfElementsInWindow)	//only update those items inside the visible window.
				{
					_uiScrollViewObserver->onElementUpdate(this, x, node);
				}
			}
			//
		}

		//docking
		if (_uiScrollViewDockingObserver && _dockElemntIndex > -1)
		{
			_currentDockingPosition = MUIScrollViewDockingObserver::eDockingPosition::kNone;
			setDockVisibility(false);	//reset to false.
			float wnd_width = getContentSize().width;
			float wnd_height = getContentSize().height;

			if (getDirection() == cocos2d::ui::ScrollView::Direction::VERTICAL)
			{
				//check if the element is below the window.
				if ((_dockElemntIndex * _singleElementSize.height) + _singleElementSize.height > getContentSize().height)
				{
					_currentDockingPosition = MUIScrollViewDockingObserver::eDockingPosition::kBottomOrRight;
					_dockElement->setPositionY(0);
					setDockVisibility(true);
					if(getDockingElementIndex() >= 0)
						_uiScrollViewDockingObserver->onUpdateDockElement(this, _dockElement, _currentDockingPosition);
				}

				//check if the element is above the window.
				if (_dockElemntIndex*_singleElementSize.height < 0)
				{
					_currentDockingPosition = MUIScrollViewDockingObserver::eDockingPosition::kTopOrLeft;
					_dockElement->setPositionY(wnd_height - getSingleElementSize().height);
					setDockVisibility(true);
					_uiScrollViewDockingObserver->onUpdateDockElement(this, _dockElement, _currentDockingPosition);
				}
			}
		}
		//

		if (_direction == Direction::HORIZONTAL)
		{
			setInnerContainerSize(Size(_numberOfElements*_singleElementSize.width, getContentSize().height));
			jumpToLeft();
		}
		else if (_direction == Direction::VERTICAL)
		{
			setInnerContainerSize(Size(getContentSize().width, _numberOfElements*_singleElementSize.height));
			jumpToTop();
		}

		_isInitialised = true;
		_uiScrollViewObserver->onPostInit();
	}

	//docking
	void UIScrollViewExtended::setUIScrollViewDockingObserver(MUIScrollViewDockingObserver* observer)
	{
		_uiScrollViewDockingObserver = observer;
	}

	void UIScrollViewExtended::setDockingElementIndex(int dockElemntIndex)
	{
		_dockElemntIndex = dockElemntIndex;
	}

	void UIScrollViewExtended::doDockOnMove(const Vec2& containerPos, const Size& windowSz)
	{
		//docking
		if (_uiScrollViewDockingObserver && _dockElemntIndex > -1)
		{
			float tPos = 0;
			if (getDirection() == cocos2d::ui::ScrollView::Direction::VERTICAL)
			{
				//check if the element is below the window.
				tPos = -(containerPos.y - windowSz.height);

				if (tPos + (_dockElemntIndex * _singleElementSize.height) + _singleElementSize.height > getContentSize().height)
				{
					setDockVisibility(true);
					if (_currentDockingPosition != MUIScrollViewDockingObserver::eDockingPosition::kBottomOrRight)
					{
						_currentDockingPosition = MUIScrollViewDockingObserver::eDockingPosition::kBottomOrRight;
						_dockElement->setPositionY(0);
						_uiScrollViewDockingObserver->onUpdateDockElement(this, _dockElement, _currentDockingPosition);
					}
				}
				else
					//check if the element is above the window.
					if (tPos + _dockElemntIndex*_singleElementSize.height < 0)
					{
						setDockVisibility(true);
						if (_currentDockingPosition != MUIScrollViewDockingObserver::eDockingPosition::kTopOrLeft)
						{
							_currentDockingPosition = MUIScrollViewDockingObserver::eDockingPosition::kTopOrLeft;
							_dockElement->setPositionY(windowSz.height - _singleElementSize.height);
							_uiScrollViewDockingObserver->onUpdateDockElement(this, _dockElement, _currentDockingPosition);
						}
					}
					else
					{
						setDockVisibility(false);	//reset to false.
						_currentDockingPosition = MUIScrollViewDockingObserver::eDockingPosition::kNone;
					}
			}
		}
		//
	}
	//~docking

	void UIScrollViewExtended::upateElementsOnMove()
	{
		Vec2 singleElementSize = getSingleElementSize();
		if (singleElementSize.x == 0.0f || singleElementSize.y == 0.0f || !_isInitialised)
			return;

		Vec2 containerPos = _innerContainer->getPosition();
		if (getDirection() == cocos2d::ui::ScrollView::Direction::VERTICAL)
			containerPos.y = _innerContainer->getSize().height + containerPos.y;

		const Size& windowSz = getContentSize();

		//docking
		doDockOnMove(containerPos, windowSz);

		//check if there is any need to update the left/right element of both borders.
		float noOfElementsInAWindow = 0;
		float lowerBound_float = 0.0f;

		if (getDirection() == cocos2d::ui::ScrollView::Direction::VERTICAL)
		{
			noOfElementsInAWindow = (windowSz.height / singleElementSize.y) + _uiscrollview_extra_padding;
			if (noOfElementsInAWindow > (int)noOfElementsInAWindow)
				noOfElementsInAWindow += 1.0f;

			lowerBound_float = (fabsf(containerPos.y) - windowSz.height) / singleElementSize.y;
		}
		else if (getDirection() == cocos2d::ui::ScrollView::Direction::HORIZONTAL)
		{
			noOfElementsInAWindow = (windowSz.width / singleElementSize.x) + _uiscrollview_extra_padding;
			if (noOfElementsInAWindow > (int)noOfElementsInAWindow)
				noOfElementsInAWindow += 1.0f;

			lowerBound_float = fabsf(containerPos.x) / singleElementSize.x;
		}

		int lowerBound = (int)lowerBound_float;
		if (lowerBound_float <= -1.0f || (lowerBound + (int)noOfElementsInAWindow) > _numberOfElements) return;
        
		if (lowerBound != _nextIndexToChange && lowerBound < _numberOfElements)
		{
			bool isMovingLeft = _nextIndexToChange < lowerBound;
			bool isMovingRight = _nextIndexToChange > lowerBound;
			setNextIndex(lowerBound);

			Node* tempElement = _virtualElementList.at(0);
			float deltaX = tempElement->getPositionX();
			float deltaY = tempElement->getPositionY();

			if (isMovingLeft)
			{
				int noOfElementsToUpdate = abs(lowerBound - _lastLeftIndexUpdated);

				for (int x = noOfElementsToUpdate-1; x >= 0; x--)
				{
					if (getDirection() == cocos2d::ui::ScrollView::Direction::VERTICAL)
						deltaY = (_numberOfElements - ((lowerBound-x) + (int)noOfElementsInAWindow))*singleElementSize.y;
					else if (getDirection() == cocos2d::ui::ScrollView::Direction::HORIZONTAL)
						deltaX = (((lowerBound-x-1) + (int)noOfElementsInAWindow))*singleElementSize.x;

					if (!_overrideVirtualItemAllocation)	//Don't need to update the position or visibility for normal scroll view.
					{
						Node* firstNode = _virtualElementList.at(0);
						_virtualElementList.erase(std::remove(_virtualElementList.begin(), _virtualElementList.end(), firstNode));
						_virtualElementList.push_back(firstNode);
						firstNode->setPosition(deltaX, deltaY);
						firstNode->setVisible(false);
						//recalculate indexToUpdate
						int indexToUpdate = ((lowerBound - x) - 1) + (int)noOfElementsInAWindow;
						if (indexToUpdate < _numberOfElements)
						{
							firstNode->setVisible(true);
							_lastLeftIndexUpdated = lowerBound;
							_uiScrollViewObserver->onElementUpdate(this, indexToUpdate, firstNode);
						}
					}
					else
					{
						int indexToUpdate = ((lowerBound - x) - 1) + (int)noOfElementsInAWindow;
						if (indexToUpdate < _numberOfElements)
						{
							_lastLeftIndexUpdated = lowerBound;
							_uiScrollViewObserver->onElementUpdate(this, indexToUpdate, _virtualElementList[indexToUpdate]);
						}
					}
				}
			}

			if (isMovingRight)
			{
				int noOfElementsToUpdate = abs(lowerBound - _lastLeftIndexUpdated);

				for (int x = 0; x < noOfElementsToUpdate; x++)
				{
					if (getDirection() == cocos2d::ui::ScrollView::Direction::VERTICAL)
						deltaY = (_numberOfElements - ((lowerBound+x) + 1))*singleElementSize.y;
					else if (getDirection() == cocos2d::ui::ScrollView::Direction::HORIZONTAL)
						deltaX = ((lowerBound+x))*singleElementSize.x;

					if (!_overrideVirtualItemAllocation)
					{
						Node* lastNode = _virtualElementList.at(_virtualElementList.size() - 1);
						_virtualElementList.erase(std::remove(_virtualElementList.begin(), _virtualElementList.end(), lastNode));
						_virtualElementList.insert(_virtualElementList.begin(), lastNode);
						lastNode->setPosition(deltaX, deltaY);
						lastNode->setVisible(false);
						if ((lowerBound + x) < _numberOfElements)
						{
							lastNode->setVisible(true);
							_lastLeftIndexUpdated = lowerBound;
#if 0
							char buffer[256];
							sprintf(buffer, "Right lastLeftIndexUpdated = %d lowerBound = %d nUpdates(%d)\n", _lastLeftIndexUpdated, lowerBound, noOfElementsToUpdate);
							OutputDebugString(buffer);
#endif
							_uiScrollViewObserver->onElementUpdate(this, lowerBound + x, lastNode);
						}
					}
					else
					{
						if ((lowerBound + x) < _numberOfElements)
						{
							_lastLeftIndexUpdated = lowerBound;
							_uiScrollViewObserver->onElementUpdate(this, lowerBound + x, _virtualElementList[lowerBound+x]);
						}
					}
				}
			}
		}
	}

	void UIScrollViewExtended::moveInnerContainer(const Vec2& deltaMove, bool canStartBounceBack)
	{
		Vec2 tmp(deltaMove);

		if (_forceSpeedLimitForScrolling)
		{
			//clamp the value to single element sz if the delta is greater.
			if (getDirection() == cocos2d::ui::ScrollView::Direction::VERTICAL)
			{
				tmp.y = (fabsf(deltaMove.y) > _singleElementSize.height) ? ((deltaMove.y >= 0.0f) ? _singleElementSize.height : -_singleElementSize.height) : deltaMove.y;
			}
			else if (getDirection() == cocos2d::ui::ScrollView::Direction::HORIZONTAL)
			{
				tmp.x = (fabsf(deltaMove.x) > _singleElementSize.width) ? ((deltaMove.x >= 0.0f) ? _singleElementSize.width : -_singleElementSize.width) : deltaMove.x;
			}
			else
			{
				//Not implemented
			}
		}

		ScrollView::moveInnerContainer(tmp, canStartBounceBack);
		if (_uiScrollViewObserver)
			upateElementsOnMove();
	}

	void UIScrollViewExtended::addVirtualElement(Node* child)
	{
		addChild(child);
	}

	void UIScrollViewExtended::addChild(Node* child)
	{
		ScrollView::addChild(child);
	}

	void UIScrollViewExtended::addChild(Node * child, int localZOrder)
	{
		ScrollView::addChild(child, localZOrder);
	}

	void UIScrollViewExtended::addChild(Node *child, int zOrder, int tag)
	{
		ScrollView::addChild(child, zOrder, tag);
	}

	void UIScrollViewExtended::addChild(Node* child, int zOrder, const std::string &name)
	{
		ScrollView::addChild(child, zOrder, name);
	}
}

NS_CC_END