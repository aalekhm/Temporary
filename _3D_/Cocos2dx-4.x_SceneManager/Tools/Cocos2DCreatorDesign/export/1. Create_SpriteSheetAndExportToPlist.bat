@echo off

SET SCALE=1.0
SET PLIST_FILE_NAME=./plist/Common.plist
SET SPRITESHEET_FILE_NAME=./plist/Common.png
SET FOLDER_TO_PROCESS=./Resources/creator/Common
TexturePacker.exe --opt RGBA5555 --multipack --scale %SCALE% --force-squared --algorithm MaxRects --max-size 2048 --size-constraints POT --trim-mode None --extrude 1 --shape-padding 3  --border-padding 3 --maxrects-heuristics Best --pack-mode Best --prepend-folder-name --data %PLIST_FILE_NAME% --format cocos2d --sheet %SPRITESHEET_FILE_NAME% %FOLDER_TO_PROCESS%

pause