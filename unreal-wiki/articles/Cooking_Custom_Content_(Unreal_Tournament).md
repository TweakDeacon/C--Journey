Cooking Custom Content (Unreal Tournament)
==========================================

Contents
--------

*   [1 Cooking Custom Content](#Cooking_Custom_Content)
    *   [1.1 Requirements](#Requirements)
    *   [1.2 Cooking a single map](#Cooking_a_single_map)
    *   [1.3 Cooking a single package (Blueprint)](#Cooking_a_single_package_.28Blueprint.29)
    *   [1.4 Cooking a plugin (full mod)](#Cooking_a_plugin_.28full_mod.29)

Cooking Custom Content
======================

Requirements
------------

*   Engine version: 4.6 (or the knepleyp github Engine branch, [https://github.com/knepleyp/UnrealEngine/](https://github.com/knepleyp/UnrealEngine/) , until 4.6 is released)
*   A .uprojectdirs file that will find UnrealTournament.uproject as UAT can't currently handle custom automation scripts without it

Cooking a single map
--------------------

*   Open a command line window up to Engine\\Build\\BatchFiles\\
*   The commandline to cook a single map is "RunUAT MakeUTDLC -MapName=MAPNAME -platform=PLAT1\[+PLAT2\] \[-server\] \[-serverplatform=\[PLAT1\]\[+PLAT2\]\]"
*   The map will end up in a pak file that is located here: UnrealTournament\\Saved\\StagedBuilds\\MAPNAME\\MAPNAME-PLATFORMNAME.pak

Cooking a single package (Blueprint)
------------------------------------

*   Open a command line window up to Engine\\Build\\BatchFiles\\
*   The commandline to cook a single package is "RunUAT MakeUTDLC -DLCName=PACKAGENAME -platform=PLAT1\[+PLAT2\] \[-server\] \[-serverplatform=\[PLAT1\]\[+PLAT2\]\]"
*   The map will end up in a pak file that is located here: UnrealTournament\\Saved\\StagedBuilds\\PACKAGENAME\\PACKAGENAME-PLATFORMNAME.pak

Cooking a plugin (full mod)
---------------------------

*   Open a command line window up to Engine\\Build\\BatchFiles\\
*   The commandline to cook a plugin is "RunUAT MakeUTDLC -PluginName=PLUGINNAME \[-build\] -platform=PLAT1\[+PLAT2\] \[-server\] \[-serverplatform=\[PLAT1\]\[+PLAT2\]\]"
*   You can supply the -build switch to ensure that the dynamic libraries get built if your plugin has native code
*   The plugin will end up in UnrealTournament\\Saved\\StagedBuilds\\PLUGINNAME\\PLATFORM\\UnrealTournament\\Plugins\\PLUGINNAME
*   The uplugin file is needed for the game to know to load your binaries and/or content directory
