#pragma once

#include "Engine/EngineManager.h"
#include <cocos2d.h>

class Dream3DTest : EngineManager, public cocos2d::Application
{
	public:
												Dream3DTest();
		virtual									~Dream3DTest();

		virtual void							onSetViewport(int& iWidth, int& iHeight);
		virtual void							onInit();
		virtual void							update(float elapsedTime);
		virtual void							render(float elapsedTime);

		virtual void							keyPressedEx(unsigned int iVirtualKeycode, unsigned short ch);
		virtual void							keyReleasedEx(unsigned int iVirtualKeycode, unsigned short ch);

		virtual void							onMouseDownEx(int mCode, int x, int y);
		virtual void							onMouseMoveEx(int mCode, int x, int y);
		virtual void							onMouseUpEx(int mCode, int x, int y);
		virtual void							onMouseWheelEx(WPARAM wParam, LPARAM lParam);

		bool									applicationDidFinishLaunching();
		void									applicationDidEnterBackground();
		void									applicationWillEnterForeground();
	protected:
	private:
		void									initMembers();
		void									initScene();

		void									onNodeLoaded(cocos2d::Ref* pObject);

		std::string								m_sCurrentWorkingDirectory;
};

