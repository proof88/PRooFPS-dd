\page baddesign Bad Design

This page is about what I don't like in the code of PRooFPS-dd, and should be changed in 2026.  
Stuff here will be turned into Issues on Github at some point, but currently these are only thoughts.

[TOC]

\section baddesign_development Development Perspective

\subsection add_new_weapon_dev Adding New Weapon is Difficult

Adding a new weapon requires too much modification here and there in the code.  
[This is an example commit of adding a new weapon.](https://github.com/proof88/PRooFPS-dd/commit/de64125063baf6c3723ec3b6f93de2bad65dcf5c)  
Even the keyboard-weapon pairs should NOT be hardcoded but come from the weapon definition files somehow.  
Also, map item types are also hardcoded, which is used in MsgWpnUpdateFromServer for identifying the weapon instead of using the filename char array.

\subsection sending_variable Sending Data over Network

TODO

\subsection memory_handling Memory Handling

https://github.com/gperftools/gperftools  
https://jemalloc.net/  
https://github.com/electronicarts/EASTL  
https://github.com/cacay/MemoryPool/tree/master  
https://github.com/lenonk/memorypool  
https://github.com/danielkrupinski/StringPool?tab=readme-ov-file

\subsection add_new_bullet_property Adding New Bullet Property

I hate that anytime I'm adding a new property to Bullet, I need to extend MsgBulletUpdateFromServer.  
Clients are able to find out those weapon-specific bullet properties anyway, we should just send the weapon ID in MsgBulletUpdateFromServer.  
We have already introduced m_eMapItemType in MsgWpnUpdateFromServer, either that should be used here as well, or something more weapon-specific ID.

\section baddesign_community Community/Modding Perspective

\subsection add_new_weapon_community Adding New Weapon Requires Code Rebuild

Adding a new weapon to the game is currently possible only by extending the code.  
This definitely needs to be changed so anyone can easily add a new weapon.  
First the development perspective shall be simplified as explained above, and then we can think about weapons from modding perspective.

\subsection add_new_map_community Adding New Map is Difficult

There is no map editor.  
Even though map file format is simple ASCII text-based format, it can be a bit difficult to master map creation using a text editor.

\subsection add_new_logic_community Adding New Logic is Impossible

Rebuild of the game code is required if someone wants to add new logic into the game.  
To solve this, the base part shall be separated, and it shall be able to invoke extra code using external mod library.  
For this the base shall be able to provide run-time game data to the mod libraries, such as list of players and their status.  
For example, the Minimap could be also an external lib instead of an internal class.
