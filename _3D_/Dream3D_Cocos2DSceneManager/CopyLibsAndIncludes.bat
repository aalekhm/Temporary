@echo copy scenes

set currentPath=%~dp0
cd %currentPath%

xcopy Debug\Dream3D_d.lib external-deps\Dream3D\lib /s /y
xcopy Debug\Cocos2D_d.lib external-deps\Cocos2D\lib /s /y
xcopy Debug\Cocos2DSceneManager_d.lib external-deps\Cocos2DSceneManager\lib /s /y

xcopy Dream3D\include external-deps\Dream3D\include /s /y
xcopy Cocos2D\include external-deps\Cocos2D\include /s /y
xcopy Cocos2DSceneManager\include external-deps\Cocos2DSceneManager\include /s /y

pause