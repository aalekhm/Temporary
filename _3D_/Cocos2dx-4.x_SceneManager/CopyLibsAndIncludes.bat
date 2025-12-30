@echo copy scenes

set currentPath=%~dp0
cd %currentPath%

xcopy Debug\Cocos2D_d.lib external-deps\Cocos2D\lib /s /y
xcopy Debug\Cocos2DSceneManager_d.lib external-deps\Cocos2DSceneManager\lib /s /y

xcopy Cocos2DSceneManager\include external-deps\Cocos2DSceneManager\include /s /y

pause