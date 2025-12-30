#pragma once
#include "cocos2d.h"

namespace CastleBuilder
{
	class BaseDialog :public cocos2d::Node
	{
	protected:
		cocos2d::Node* rootNode;
		bool swallowTouches();
	public:
		BaseDialog();
		~BaseDialog();
		static BaseDialog* create(cocos2d::Size size);
		virtual bool init();

		void close();

	private:
	};
}