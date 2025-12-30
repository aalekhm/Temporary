
Steps to Build:
	1. Open "Cocos2dx-4.x_SceneManager.sln" in VS 2015.
	2. Make sure that the "Solution Platforms" is set to "x86".
	3. Build "Cocos2D".
	4. Build "Cocos2DSceneManager".
	5. Run "CopyLibsAndIncludes.bat". This copies the "Cocos2D_d.lib" & "Cocos2DSceneManager_d.lib" & the headers to the external-deps.
	6. Build "Cocos2DSceneManagerTest".
	
Steps to Create Scenes in CocosCreatr(1.9.3) or CocosStudio(2.3.4)
	1. This project can take both "*.creator" & "*.csb" as Scenes.
	2. Project to design scenes using CocosStudio reside in "/tools/Cocos2DCreatorDesign".
	3. Project to design scenes using CocosCreator reside in "/tools/Cocos2DDesign".
	4. In CocosCreator, "Project->LuaCpp Support->Setup Target Project", set the "Project Path" to ".\Tools\Cocos2DCreatorDesign\export" (absolute path).
	5. Under ".\Tools\Cocos2DCreatorDesign\export", Run the 3 batch scripts in the same order as named.
	6. These batch files create a Texture Packed using TexturePacker & copies all the .anim & .creator files to the Cocos2DSceneManagerTest's Resource folder.