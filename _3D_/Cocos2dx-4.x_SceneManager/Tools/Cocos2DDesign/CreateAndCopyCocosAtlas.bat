@echo CreateAtlas
cd %~dp0

set currentPath=%~dp0
cd %currentPath%
set cmd="dir "..\..\..\build\TetrisBlitzApp\" /b"
for /F "tokens=*" %%i in (' %cmd% ') DO SET buildPath=%%i

..\..\..\scripts\CocosTexturePacker.py ..\..\..\data\GameAssets\Cocos2dxImages\size200\ ..\..\..\build\data\AssetsOutput\PC\Debug\Assets\Cocos2dxImages\size200\ 1.0
..\..\..\scripts\CocosTexturePacker.py ..\..\..\data\GameAssets\Cocos2dxImages\size200\ ..\..\..\build\data\AssetsOutput\PC\Debug\Assets\Cocos2dxImages\size150\ 0.75
..\..\..\scripts\CocosTexturePacker.py ..\..\..\data\GameAssets\Cocos2dxImages\size200\ ..\..\..\build\data\AssetsOutput\PC\Debug\Assets\Cocos2dxImages\size100\ 0.5

xcopy ..\..\..\build\data\AssetsOutput\PC\Debug\Assets\Cocos2dxImages ..\..\..\build\TetrisBlitzApp\%buildPath%\blast-x86-vc-dev-debug\bin\TetrisBlitzApp\assets\Assets\Cocos2dxImages\ /s /y
xcopy ..\..\..\build\data\AssetsOutput\PC\Debug\Assets\Cocos2dxImages ..\..\..\build\TetrisBlitzApp\%buildPath%\blast-x86-vc-dev-debug\bin\assets\Assets\Cocos2dxImages\ /s /y