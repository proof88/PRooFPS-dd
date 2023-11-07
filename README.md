# PRooFPS-dd

<p align="center">
  <a href="http://www.youtube.com/watch?feature=player_embedded&v=XPMMzPYjR98" target="_blank"><img src="PR00FPS-dd-logo.png" alt="Click to see the video!"/></a>
</p>
Clicking on the image above brings you to a short **YouTube teaser video**:

## Summary

This game is the successor of the original PR00FPS:
https://github.com/proof88/PR00FPS

This game is still **under development**.

**[Known Bugs](https://github.com/proof88/PRooFPS-dd/labels/bug)**  
**[Known Network Performance Issues](https://github.com/proof88/PRooFPS-dd/issues?q=is%3Aopen+is%3Aissue+milestone%3A%22Networking+%28Public+Beta%29%22+label%3Aoptimization+sort%3Aupdated-desc)**  
**[Other Performance Issues](https://github.com/proof88/PRooFPS-dd/issues?q=is%3Aopen+is%3Aissue+-milestone%3A%22Networking+%28Public+Beta%29%22+label%3Aoptimization+sort%3Aupdated-desc+)**  
**[Full Backlog](https://github.com/proof88/PRooFPS-dd/milestones?direction=asc&sort=title&state=open)**

You can follow the project on my **facebook page**:
https://www.facebook.com/whiskhyll

## Try the Game

The latest available download version [v0.1.4 is here](https://drive.google.com/file/d/14iu3raQSt0GcuE3IlXKdd6FQdUTGr7W-/view?usp=share_link).  
If the game cannot start due to missing DLL files, [this Visual C++ Redistributable Package](https://drive.google.com/file/d/1B61VzifHvK-wTNGUai4HaEeik2cXLRHH/view?usp=share_link) should be installed.

### Run as Server

To start as server, simply run PRooFPS-dd.exe.

### Run as Client

To start as client, simply run the convenient PRooFPS-dd-as-client.bat (Windows) or PRooFPS-dd-as-client.sh (Linux).  
To specify IP-address of the server for the client, change cl_server_ip CVAR in gamedata\profiles\proof88\proof88.cfg.  

Alternatively, you can use command line arguments as well, e.g.:  
./PRooFPS-dd.exe --net_server=false --cl_server_ip=192.168.1.100

### Keyboard, Mouse

 - **A/D, LEFT/RIGHT:** move the player;
 - **SHIFT:** toggle move speed: walking or running;
 - **R**: reload current weapon;
 - **2,3 / Mouse Wheel**: change to pistol or machine gun;
 - **Mouse Click**: attack;
 - **TAB:** show frag table;
 - **BACKSPACE:** show GUI demo;
 - **ENTER/RETURN:** if "testing=true" CVAR is defined, it makes a dump of testing data into "RegTestDumpServer.txt" or "RegTestDumpClient.txt";
 - **T**: if instance is server, it respawns the player to a random spawnpoint and dumps debug data to console window.

### Debugging

TBD

## Build

You must have the Visual Studio solution file including other relevant projects as well in [PGE-misc](https://github.com/proof88/PGE-misc) repo.  
**Follow the build instructions** in [PGE-WoW.txt](https://github.com/proof88/PGE-misc/blob/master/src/PGE-WoW.txt).  
As described in that file, you need to build other projects before PRooFPS-dd can be built.

## History

### v0.1.5 Private Beta (TBD)

TBD

### v0.1.4 Private Beta (Oct 13, 2023)

A compressed build for Windows is available for [download here](https://drive.google.com/file/d/14iu3raQSt0GcuE3IlXKdd6FQdUTGr7W-/view?usp=share_link).

The major improvements in this version are the **networking improvements** that solved [all the networking-related problems experienced with v0.1.0](https://github.com/proof88/PRooFPS-dd/issues/184):
 - **client side rate limit**: sending packets are limited so a single client is not be able to flood the server with too many packets;
 - **server side rate limit 1**: as a protection, server also ignores packets arriving from a client with higher rate than a predefined rate limit;
 - **server side rate limit 2**: server sends out updates to clients also in rate limited fashion, to avoid too much load on the network, and causing unnecessary big load on clients;
 - **don't send unnecessary info**: bullet travel updates are no longer sent by server to clients as from now clients also simulate bullet travel.
 - **variable packet size**: packets had unreasonably fixed big sizes in previous versions, from now their sizes are much smaller and variable as actually required.

**Comparison** of packet rates and used bandwidth between previous and this version, considering **8 players @ 60 FPS in a worst-case scenario** [described on the PGE Networking wiki page](https://github.com/proof88/PGE/blob/master/PGE/docpages/menu-04-Networking.md) (every player is shooting and picking up items and many bullets are travelling at the same time):
 - **in server -> client direction:**
   - **v0.1.0**: 30 240 PKT/s with 8 104 320 Byte/s used bandwidth;
   - **v0.4.0**:
     - **20 Hz tickrate**: 3 626 PKT/s resulting **88% decrease** with 189 574 Byte/s used bandwidth resulting **98% decrease**;
     - **60 Hz tickrate**: 10 626 PKT/s resulting **65% decrease** with 548 814 Byte/s used bandwidth resulting **93% decrease**.
 - **in client -> server direction:**
   - **v0.1.0**: summarized 480 PKT/s from all clients with 128 640 Byte/s data rate;
   - **v0.4.0**: summarized 128 PKT/s from all clients resulting **73% decrease** with 3 968 Byte/s data rate resulting **97% decrease**.

This version was tested with 9 players with 20 and 60 Hz tickrate configs and didn't have any problem with either of them.  
Contains NO changes to gameplay compared to the previous version except fixing [bug of removed players after the first round](https://github.com/proof88/PRooFPS-dd/issues/221).

### v0.1.3 Private Beta (Aug 20, 2023)

This version contains no changes to gameplay compared to the previous version except that the movement of the player might have a slightly different feeling.  
This version introduces **a lot of changes to overcome network- and physics related issues** experienced with v0.1 Private Beta during the LAN Party event last December: lost packets between server and clients and occasional huge jumps of players.

Under-the-hood changes:
 - **Hardening Physics Calculations**: results are now calculated in fixed timesteps, have become rate-independent i.e. updating physics with lower rate still generate same results;
 - **Introduce Tickrate**: physics calculations and updating clients are now tied to a new rate called tickrate that is independent of framerate, that is usually lower than framerate;
 - **CVAR "tickrate"** added for configuring tickrate: currently values between 20 and 60 Hz are supported. Value of 20 Hz reduces packet rate in server->client direction to 1/3 of original tickrate 60 Hz;
 - **Regression Test Update**: now regression test runs with 2 different tickrate configs: 60 and 20 Hz and expect the same result;
 - FPS-independent camera movement.

Expected changes in upcoming v0.1.4 Private Beta:
 - decrease packet rate in client->server direction by aligning input processing to tickrate;
 - further decrease packet rate in server->client direction by reducing bullet travel updates by doing bullet movement simulation in clients (in the form of prediction, still server remains the authoritive).

### v0.1.2 Private Beta (May 13, 2023)

This version contains no changes to gameplay compared to the previous version, only massive code refactoring.

### v0.1.1 Private Beta (March 10, 2023)

This version contains no changes to gameplay compared to the previous version, only changes that make testing for regression easier:
 - **FTR: user profile config support**: each user can have their own config files. Currently the config file of the first found user profile is loaded automatically. With this change, there is no more need for "gyorsan.txt" and "server.txt".
The following cvars (config variables) are supported so far: **cl_name, cl_server_ip, gfx_windowed, gfx_vsync, net_server, sv_map**.
 - **FTR: command line support**: cvars (config variables) can be also defined now in command line arguments. These override values loaded from user profile config file (except cl_name).
Example command line to start up a client: **"PRooFPS-dd.exe --net_server=false --cl_server_ip=127.0.0.1"**
 - **FTR: regression test**: a simple [regression test](https://github.com/proof88/PRooFPS-dd/blob/main/PRooFPS-dd/Tests/RegTestBasicServerClient2Players.h) is added that starts up 2 instances of the game: 1 server and 1 client. User input (running, jumping, shooting) is simulated for both instances, and then some values (player data, frag table data, packet statistics) are checked to decide the result of the test.
 - **FTR: testing mode**: to support regression test, a testing mode has been introduced that can be enabled by setting cvar "testing" to "true". In such case, the game saves data to be evaluated by regression test to either "RegTestDumpServer.txt" or "RegTestDumpClient.txt".
 - **RFR: OPT**: for creating map blocks and items, the PureObject3DManager::createCloned() of the graphics engine is now utilized to clone same-looking objects instead of creating them one by one from scratch. This way the memory usage and loading time decreases in both debug and release modes the following way, using map_warhouse.txt: **system memory usage decreases by 30% (-16 MByte), video memory usage decreases by 25% (-2 MByte), map loading time decreases by 40% (-1-2 secs)**.
 - **RFR: OPT**: when loading a map and creating blocks, instead of reallocating array every time we are creating a new block, now we count the number of blocks and allocate array with the proper size only once, before actually creating blocks. This also supposed to speed up the loading.

### v0.1 Private Beta (Dec 16, 2022)

On Dec 16, 2022, the game reached **v0.1 private beta version**, and a compressed build for Windows is available for **[download here](https://drive.google.com/file/d/1K_BQpJHMxsSwKw0s62dnDM7hJs5gK4RP/view?usp=share_link)**.

The Visual C++ Redistributable Package installer is also included in the zip, it might be needed to be installed before running the game.
