# PRooFPS-dd


## Summary

This game is the successor of the original PR00FPS:
https://github.com/proof88/PR00FPS

This game is still **under development**.

**Milestones**: https://github.com/proof88/PRooFPS-dd/milestones?direction=asc&sort=due_date&state=open

You can follow the project on my **facebook page**:
https://www.facebook.com/whiskhyll

## History

### v0.1.1 Private Beta (March 10, 2023)

This version contains no changes to gameplay compared to the previous version, only changes that make testing for regression easier:
 - **FTR: user profile config support**: each user can have their own config files. Currently the config file of the first found user profile is loaded automatically. With this change, there is no more need for "gyorsan.txt" and "server.txt".
The following cvars (config variables) are supported so far: **cl_name, cl_server_ip, gfx_windowed, gfx_vsync, net_server, sv_map**.
 - **FTR: command line support**: cvars (config variables) can be also defined now in command line arguments. These override values loaded from user profile config file (except cl_name).
Example command line to start up a client: **"PRooFPS-dd.exe --net_server=false --cl_server_ip=127.0.0.1"**
 - **FTR: regression test**: a simple [regression test](https://github.com/proof88/PRooFPS-dd/blob/main/PRooFPS-dd/Tests/RegTestBasicServerClient2Players.h) is added that starts up 2 instances of the game: 1 server and 1 client. User input (running, jumping, shooting) is simulated for both instances, and then some values (player data, frag table data, packet statistics) are checked to decide the result of the test.
 - **FTR: testing mode**: to support regression test, a testing mode has been introduced that can be enabled by setting cvar "testing" to "true". In such case, the game saves data to be evaluated by regression test to either "RegTestDumpServer.txt" or "RegTestDumpClient.txt".
 - **RFR: OPT**: for creating map blocks and items, the PureObject3DManager::createCloned() of the graphics engine is now utilized to clone same-looking objects instead of creating them one by one from scratch. This way the memory usage and loading time decreases in both debug and release modes the following way, using map_warhouse.txt: **system memory usage decreases by 30% (-16 MByte), video memory usage decreases by 25% (-2 MByte), map loading time decreases by 40% (-1-2 secs)**.

### v0.1 Private Beta (Dec 16, 2022)

On Dec 16, 2022, the game reached **v0.1 private beta version**, and a compressed build for Windows is available from **download here**: https://drive.google.com/file/d/1K_BQpJHMxsSwKw0s62dnDM7hJs5gK4RP/view?usp=share_link

The Visual C++ Redistributable Package installer is also included in the zip, it might be needed to be installed before running the game.

Clicking on the image below brings you to a short **YouTube teaser video**:
<p align="center">
  <a href="http://www.youtube.com/watch?feature=player_embedded&v=XPMMzPYjR98" target="_blank"><img src="PR00FPS-dd-logo.png" alt="Click to see the video!"/></a>
</p>
