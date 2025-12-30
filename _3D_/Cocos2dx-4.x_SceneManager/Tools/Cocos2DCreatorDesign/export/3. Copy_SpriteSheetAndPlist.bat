@echo off

SET SRC_PLIST_FOLDER=plist
SET DEST_PLIST_FOLDER=..\..\..\Cocos2DSceneManagerTest\Resources\creator\Common

SET SRC_SCENE_FOLDER=Resources\creator\Scenes
SET DEST_SCENE_FOLDER=..\..\..\Cocos2DSceneManagerTest\Resources\creator\Scenes

xcopy %SRC_PLIST_FOLDER% %DEST_PLIST_FOLDER% /s /y
xcopy %SRC_SCENE_FOLDER% %DEST_SCENE_FOLDER% /s /y


pause