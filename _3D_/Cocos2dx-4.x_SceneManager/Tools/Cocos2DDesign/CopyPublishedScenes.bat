@echo copy scenes

set currentPath=%~dp0
cd %currentPath%
set cmd="dir "..\..\..\build\TetrisBlitzApp\" /b"
for /F "tokens=*" %%i in (' %cmd% ') DO SET buildPath=%%i


xcopy ..\..\..\data\Assets\CocosScenes ..\..\..\build\TetrisBlitzApp\%buildPath%\blast-x86-vc-dev-debug\bin\TetrisBlitzApp\assets\Assets\CocosScenes /s /y
xcopy ..\..\..\data\Assets\CocosScenes ..\..\..\build\data\AssetsOutput\PC\Debug\Assets\CocosScenes /s /y
