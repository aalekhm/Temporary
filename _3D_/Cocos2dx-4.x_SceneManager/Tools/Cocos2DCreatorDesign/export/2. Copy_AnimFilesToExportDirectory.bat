@echo off

SET SRC_ANIM_FOLDER=..\assets\Scenes\*.anim
SET DEST_ANIM_FOLDER=Resources\creator\Scenes

xcopy %SRC_ANIM_FOLDER% %DEST_ANIM_FOLDER% /s /y


pause