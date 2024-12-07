# PRooFPS-dd

<p align="center">
  <a href="http://www.youtube.com/watch?feature=player_embedded&v=XPMMzPYjR98" target="_blank"><img src="PR00FPS-dd-logo.png" alt="Click to see the video!"/></a>
</p>

- Table of Contents [(generated by markdown-toc)](http://ecotrust-canada.github.io/markdown-toc/)
  * [Summary](#summary)
  * [Try the Game](#try-the-game)
  * [FAQ](#faq)
  * [Build](#build)

## Summary

This game is the successor of the [original PR00FPS made in 2007](https://github.com/proof88/PR00FPS).  
It uses my open-source [game engine](https://github.com/proof88/PGE).

This game is still **under development**.

**[Version History](HISTORY.md)**  
**[Kanban Board View for 2024 to Reach Public Beta 1](https://github.com/users/proof88/projects/9/views/1)**  
**[Roadmap View for 2024 to Reach Public Beta 1](https://github.com/users/proof88/projects/9/views/4)**  
**[Online Dev Doc](https://proof88.github.io/proofps-dd-doc/index.html)**  
**[Known Bugs](https://github.com/proof88/PRooFPS-dd/labels/bug)**  
**[Known Network Performance Issues](https://github.com/proof88/PRooFPS-dd/issues?q=is%3Aopen+is%3Aissue+milestone%3A%22Networking+%28Public+Beta%29%22+label%3Aoptimization+sort%3Aupdated-desc)**  
**[Other Performance Issues](https://github.com/proof88/PRooFPS-dd/issues?q=is%3Aopen+is%3Aissue+-milestone%3A%22Networking+%28Public+Beta%29%22+label%3Aoptimization+sort%3Aupdated-desc+)**  
**[Full Backlog](https://github.com/proof88/PRooFPS-dd/milestones?direction=asc&sort=title&state=open)**

You can follow the project on my **[facebook page](https://www.facebook.com/whiskhyll)**.

## Try the Game

The latest available download version [v0.4.3 Private Beta is here](https://drive.google.com/file/d/1T5d9ksZYaB6SNq0FGmOuKkerI0h_QFiY/view?usp=drive_link).  
The latest ALP (Approofed für Lan Party) version is [v0.2.1 Private Beta](https://drive.google.com/file/d/17b1RFXXoP8qCyIGxTNDUV2xEEtLSQLa1/view?usp=sharing).  
If the game cannot start due to missing DLL files, [this Visual C++ Redistributable Package](https://drive.google.com/file/d/1B61VzifHvK-wTNGUai4HaEeik2cXLRHH/view?usp=share_link) should be installed.

[Version history is here.](HISTORY.md)

### Run the Game

Simply run `PRooFPS-dd.exe`.  
From the main menu, you can either start a new server or join to an existing one.

### Keyboard, Mouse

**Movement**

 - **A/D, LEFT/RIGHT:** strafe: move the player in either left or right direction;
 - **LSHIFT:** toggle move speed: walking or running;
 - **SPACE:** jump;
 - **LCTRL:** crouch/duck.

On-the-ground somersault: hold crouch/duck key down, then press the appropriate strafe key twice (depending on in which direction you want to perform the somersault).

In-air salto/somersault: jump-crouch-jump or jump-jump combo, depending on server configuration of CVAR `sv_somersault_mid_air_auto_crouch`.

**Weapons**

 - **1,2,3,4,5 / Mouse Wheel**: change weapon (knife, pistol, machine gun, bazooka, pusha);
 - **R**: reload current weapon;
 - **Mouse Click**: attack (fire weapon).

**Misc**

 - **TAB:** show frag table, server config, client live network data;
 - **M:** switch to next map based on mapcycle.txt (server only);
 - **ENTER/RETURN:** if `testing=true` CVAR is defined, it makes a dump of testing data into `RegTestDumpServer.txt` or `RegTestDumpClient.txt`;
 - **T**: if instance is server, it respawns the player to a random spawnpoint and dumps debug data to console window.

### Debugging

If you have any problem with the game, it is always recommended to save the latest generated log file(s) from the game's main directory.  
Their file name format is: `log_<hostname>_<date>_<time>.html` .  
Since they are saved in HTML format, it is easy to open and read them in your favourite web browser, but what is better is if you send them to the developer.

If you have performance issues and you are running the server instance of the game, pressing 'T' key anytime during the game saves extra information into the HTML log file.  

If you have network related issue, you can check real-time network statistics by displaying the frag table by pressing the 'TAB' key.  
Note that some network statistics are also saved into the HTML log file whenever a client- or server instance disconnects.

## FAQ

### Mouse Cursor is Not Visible in Fullscreen Menu when OBS Studio is Running, How to Fix It?

This seems to be an issue which does not happen on all computers but when it happens then **most probably you have a "Display Capture" set as Source** in OBS Studio.  
Sometimes simply **switching to another application and then switching back to the game (ALT+TAB, then ALT+TAB again)** resolves the issue, so this should be the first thing you try.  
As a next idea to resolve this issue, go to the Properties of your "Display Capture" Source and try to **change the Capture Method to "DXGI Desktop Duplication"**, also make sure **"Capture Mouse" is checked**.  
If this does not help, **remove "Display Capture"** from Sources, start the game, then switch to OBS Studio (ALT+TAB), add a **"Window Capture" Source** and select the game window. You might also need to change the Capture Method too.  
If the issue still persists, try to upgrade OBS Studio, your video card driver, or just simply turn Fullscreen mode off in game settings when you are using OBS Studio in parallel.  
This is NOT considered as a game bug.

### My Display Resolution is Small, I Cannot See the Bottom of the Window, How to Fix It?

The game starts up in windowed mode, in a 1024x768 pixels client-size window. Thus, 720p screens won't show the bottom of the window.  
After starting the game, go to the Settings menu, and enable Fullscreen. This will restart the game in a window with client size matching the display resolution thus the entire game content will be visible.  
If you cannot see the entire Settings menu in fullscreen mode, don't worry, you can use the mouse cursor to scroll vertically in the menu.  
This is NOT considered as a game bug, however in the future the windowed mode will default to 800x600, and the window will be resizable.

### Somehow the Main Menu is NOT Centered Horizontally, the Crosshair also Disappears Towards the Right and Bottom Edges of the Screen, How to Fix It?

Most probably this is because your screen DPI scaling setting in the operating system is not set to 100% but to a larger value. The game currently does not support different DPI scalings.  
Only workaround is to set DPI scaling to 100%.  
This is considered as a game bug, the issue is tracked (https://github.com/proof88/PRooFPS-dd/issues/183) and will be resolved in the future.

### Why the Compressed Release is So Big?

At this point of development, I'm not really concerned about the disk space usage of my game. We are talking about a ZIP file less than 10 MiBytes.  
I'm also fully aware of the existence of a big UNUSED texture file in map_warhouse directory, and I also know that textures could be downscaled a bit.  
Believe it or not but there is also a build configuration issue that results in unnecessary big executable size.  
Look at the version number and you will understand why disk space usage is really not my number 1 priority at this point.

## Build

You must have the Visual Studio solution file including other relevant projects as well in [PGE-misc](https://github.com/proof88/PGE-misc) repo.  
**Follow the build instructions** in [PGE-WoW.txt](https://github.com/proof88/PGE-misc/blob/master/src/PGE-WoW.txt).  
As described in that file, you need to build other projects before PRooFPS-dd can be built.
