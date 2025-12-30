/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org
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

#include "platform/CCPlatformConfig.h"
#if defined(EA_PLATFORM_ANDROID)

#include "platform/CCDevice.h"
#include <string>
#include <android/log.h>
#include <jni.h>
#include "base/ccTypes.h"
//#include "platform/eamt/android/jni/DPIJni.h"
//#include "platform/eamt/android/jni/Java_org_cocos2dx_lib_Cocos2dxHelper.h"
//#include "platform/eamt/android/jni/JniHelper.h"
#include "platform/CCFileUtils.h"
#include "base/CCDirector.h"
#include "../source/Main/Android/JniContext.h"
NS_CC_BEGIN

int Device::getDPI()
{
    static int dpi = -1;
    if (dpi == -1)
    {
        dpi = Director::sharedDirector()->getDPI();
    }
    return dpi;
}

void Device::setAccelerometerEnabled(bool isEnabled)
{
    if (isEnabled)
    {
        //enableAccelerometerJni();
    }
    else
    {
       // disableAccelerometerJni();
    }
}

void Device::setAccelerometerInterval(float interval)
{
	//setAccelerometerIntervalJni(interval);
}

class BitmapDC
{
public:

    BitmapDC()
    : _data(nullptr)
    , _width(0)
    , _height(0)
    {
		jniEnv = EA::Blast::JniContext::GetEnv();
		if (jniEnv != NULL)
		{
			jclass LocalRefJClass = jniEnv->FindClass("org/cocos2dx/lib/Cocos2dxBitmap");
			if (LocalRefJClass != NULL)
			{
				BitMapCls = (jclass)jniEnv->NewGlobalRef(LocalRefJClass);
			}
			if (BitMapCls != NULL)
			{
				BitMapMethod = jniEnv->GetStaticMethodID(BitMapCls, "createTextBitmapShadowStroke", "([BLjava/lang/String;IIIIIIIIZFFFFZIIIIFZI)Z");
			}
			jniEnv->DeleteLocalRef(LocalRefJClass);
		}
    }

    ~BitmapDC(void)
    {
		if (jniEnv != NULL)
		{
			jniEnv->DeleteGlobalRef(BitMapCls);
		}
    }

    bool getBitmapFromJavaShadowStroke(	const char *text,
    									int nWidth,
    									int nHeight,
    									Device::TextAlign eAlignMask,
                      const FontDefinition& textDefinition )
    {
		if (jniEnv == NULL || BitMapCls == NULL || BitMapMethod == NULL)
		{
			return false;
		}

        // Do a full lookup for the font path using FileUtils in case the given font name is a relative path to a font file asset,
        // or the path has been mapped to a different location in the app package:
		std::string fullPathOrFontName = textDefinition._fontName;
		if(FileUtils::getInstance()->isFileExist(fullPathOrFontName)) {
			fullPathOrFontName = FileUtils::getInstance()->fullPathForFilename(textDefinition._fontName);

			// If the path name returned includes the 'assets' dir then that needs to be removed, because the android.content.Context
			// requires this portion of the path to be omitted for assets inside the app package.
			if (fullPathOrFontName.find("assets/") == 0)
			{
				fullPathOrFontName = fullPathOrFontName.substr(strlen("assets/"));	// Chop out the 'assets/' portion of the path.
			}
		}

        /**create bitmap
        * this method call Cococs2dx.createBitmap()(java code) to create the bitmap, the java code
        * will call Java_org_cocos2dx_lib_Cocos2dxBitmap_nativeInitBitmapDC() to init the width, height
        * and data.
        * use this approach to decrease the jni call number
        */
		   
        int count = strlen(text);
        jbyteArray strArray = jniEnv->NewByteArray(count);
		   
        jniEnv->SetByteArrayRegion(strArray, 0, count, reinterpret_cast<const jbyte*>(text));
		   
        jstring jstrFont = jniEnv->NewStringUTF(fullPathOrFontName.c_str());

		jniEnv->CallStaticBooleanMethod(BitMapCls, BitMapMethod, strArray,
			jstrFont, textDefinition._fontSize, textDefinition._fontFillColor.r, textDefinition._fontFillColor.g,
			textDefinition._fontFillColor.b, textDefinition._fontAlpha,
			eAlignMask, nWidth, nHeight,
			textDefinition._shadow._shadowEnabled, textDefinition._shadow._shadowOffset.width, -textDefinition._shadow._shadowOffset.height,
			textDefinition._shadow._shadowBlur, textDefinition._shadow._shadowOpacity,
			textDefinition._stroke._strokeEnabled, textDefinition._stroke._strokeColor.r, textDefinition._stroke._strokeColor.g,
			textDefinition._stroke._strokeColor.b, textDefinition._stroke._strokeAlpha, textDefinition._stroke._strokeSize,
			textDefinition._enableWrap, textDefinition._overflow);
		   
           jniEnv->DeleteLocalRef(strArray);
           jniEnv->DeleteLocalRef(jstrFont);
		   
           return true;
    }

public:
    int _width;
    int _height;
    unsigned char *_data;

	JNIEnv*  jniEnv;
	jclass BitMapCls;
	jmethodID BitMapMethod;
};

static BitmapDC& sharedBitmapDC()
{
    static BitmapDC s_BmpDC;
    return s_BmpDC;
}

Data Device::getTextureDataForText(const char * text, const FontDefinition& textDefinition, TextAlign align, int &width, int &height, bool& hasPremultipliedAlpha)
{
    Data ret;
    do 
    {
        BitmapDC &dc = sharedBitmapDC();

        if(! dc.getBitmapFromJavaShadowStroke(text, 
            (int)textDefinition._dimensions.width, 
            (int)textDefinition._dimensions.height, 
            align, textDefinition )) { break;};

        width = dc._width;
        height = dc._height;
        ret.fastSet(dc._data,width * height * 4);
        hasPremultipliedAlpha = true;
    } while (0);

    return ret;
}


void Device::setKeepScreenOn(bool value)
{
    //setKeepScreenOnJni(value);
}

NS_CC_END



// this method is called by Cocos2dxBitmap
extern "C"
{
    /**
    * this method is called by java code to init width, height and pixels data
    */
    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxBitmap_nativeInitBitmapDC(JNIEnv*  env, jobject thiz, int width, int height, jbyteArray pixels)
    {
        int size = width * height * 4;
        cocos2d::BitmapDC& bitmapDC = cocos2d::sharedBitmapDC();
        bitmapDC._width = width;
        bitmapDC._height = height;
        bitmapDC._data = (unsigned char*)malloc(sizeof(unsigned char) * size);
        env->GetByteArrayRegion(pixels, 0, size, (jbyte*)bitmapDC._data);
    }
};

#endif // defined(EA_PLATFORM_ANDROID)