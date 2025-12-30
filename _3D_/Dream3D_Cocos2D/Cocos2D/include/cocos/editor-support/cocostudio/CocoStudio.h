/****************************************************************************
Copyright (c) 2013-2014 Chukong Technologies Inc.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#ifndef __EDITOR_SUPPORT_COCOSTUDIO_H__
#define __EDITOR_SUPPORT_COCOSTUDIO_H__

#include "CCActionFrame.h"
#include "CCActionFrameEasing.h"
#include "CCActionManagerEx.h"
#include "CCActionNode.h"
#include "CCActionObject.h"
#include "CCArmature.h"
#include "CCBone.h"
#include "CCArmatureAnimation.h"
#include "CCProcessBase.h"
#include "CCTween.h"
#include "CCDatas.h"
#include "CCBatchNode.h"
#include "CCDecorativeDisplay.h"
#include "CCDisplayFactory.h"
#include "CCDisplayManager.h"
#include "CCSkin.h"
#include "CCColliderDetector.h"
#include "CCArmatureDataManager.h"
#include "CCArmatureDefine.h"
#include "CCDataReaderHelper.h"
#include "CCTransformHelp.h"
#include "CCUtilMath.h"
#include "CCComBase.h"
#include "CCComAttribute.h"
#include "CCComAudio.h"
#include "CCComController.h"
#include "CCComRender.h"
#include "CCInputDelegate.h"
#include "DictionaryHelper.h"
#include "CCSGUIReader.h"
#include "CCSSceneReader.h"
#include "TriggerBase.h"
#include "ActionTimeline/CCActionTimelineCache.h"
#include "ActionTimeline/CCFrame.h"
#include "ActionTimeline/CCTimeLine.h"
#include "ActionTimeline/CCActionTimeline.h"
#include "ActionTimeline/CCActionTimelineNode.h"
#include "ActionTimeline/CCBoneNode.h"
#include "ActionTimeline/CCSkeletonNode.h"
#include "ActionTimeline/CSLoader.h"
#include "CocosStudioExport.h"

namespace cocostudio
{
    void CC_STUDIO_DLL destroyCocosStudio();
}

#endif
