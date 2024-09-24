\page baddesign Bad Design

This page is about what I don't like in the code of PRooFPS-dd, and should be changed in 2025.  
Stuff here will be turned into Issues on Github at some point, but currently these are only thoughts.

[TOC]

\section add_new_weapon Adding New Weapon

Adding a new weapon to the game is currently possible only by extending the code.  
This definitely need to be changed so anyone easily can add a new weapon.  
What is even worse, extending the code currently involves too much modification here and there.  
[This is an example commit of adding a new weapon.](https://github.com/proof88/PRooFPS-dd/commit/de64125063baf6c3723ec3b6f93de2bad65dcf5c)  
Even the keyboard-weapon pairs should NOT be hardcoded but come from the weapon definition files somehow.  
Also, map item types are also hardcoded, which is used in MsgWpnUpdateFromServer for identifying the weapon instead of using the filename char array.

\section add_new_bullet_property Adding New Bullet Property

I hate that anytime I'm adding a new property to Bullet, I need to extend MsgBulletUpdateFromServer.  
Clients are able to find out those weapon-specific bullet properties anyway, we should just send the weapon ID in MsgBulletUpdateFromServer.  
We have already introduced m_eMapItemType in MsgWpnUpdateFromServer, either that should be used here as well, or something more weapon-specific ID.

\section sending_variable Sending Data over Network

TODO