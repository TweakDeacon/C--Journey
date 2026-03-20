How to Change FPS Cap , Near Clip, Editor & Game
================================================

**Rate this Article:**

5.00

![](/extensions/VoteNY/images/star_on.gif)![](/extensions/VoteNY/images/star_on.gif)![](/extensions/VoteNY/images/star_on.gif)![](/extensions/VoteNY/images/star_on.gif)![](/extensions/VoteNY/images/star_on.gif) (one vote)

Approved for Versions:4.13

Contents
--------

*   [1 Overview](#Overview)
*   [2 MaxFPS](#MaxFPS)
*   [3 Default Engine Config File](#Default_Engine_Config_File)
*   [4 Editor](#Editor)
*   [5 Game (confirmed in 4.13 as still viable method)](#Game_.28confirmed_in_4.13_as_still_viable_method.29)
*   [6 Commenting Stuff Out](#Commenting_Stuff_Out)
*   [7 Summary](#Summary)

Overview
--------

**Author:** [Rama](/User:Rama "User:Rama") ([talk](/User_talk:Rama "User talk:Rama"))

Dear Community,

Here is how you can set the FPS Cap and the near clip plane for both the editor and your actual game!

Several methods and configuration options now exist.

MaxFPS
------

In ConsoleVariables.ini add:

  \[Startup\]
  t.MaxFPS=30

Additional options and methods below are optional.

Default Engine Config File
--------------------------

Navigate to

 YourProject/Config/DefaultEngine.ini

Editor
------

 \[/Script/UnrealEd.EditorEngine\]
 NearClipPlane=3.0     ;useful when zooming in Persona
 bSmoothFrameRate=true
 MinSmoothedFrameRate=5
 MaxSmoothedFrameRate=120

Game (confirmed in 4.13 as still viable method)
-----------------------------------------------

 \[/Script/Engine.Engine\]
 NearClipPlane=12.0     ;in case you have weapon model clipping when zoomed in close
 bSmoothFrameRate=true
 MinSmoothedFrameRate=5
 MaxSmoothedFrameRate = 90


EDIT: As of 4.2 you instead need to go into the Editor

 Project Settings->Engine->General Settings->Frame Rate

Commenting Stuff Out
--------------------

In config files you can use ; to comment stuff out

 \[/Script/Engine.Engine\]
 ;NearClipPlane=12.0
 bSmoothFrameRate=true
 MinSmoothedFrameRate=5
 MaxSmoothedFrameRate = 90

Summary
-------

Enjoy!

[Rama](/User:Rama "User:Rama") ([talk](/User_talk:Rama "User talk:Rama"))
