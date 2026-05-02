\page page_game_logic Game Logic

Some basic knowledge about game logic implementation.

[TOC]

\section gameplay_basics Basics

There is no separate binary for server and client instance.  
There is only a single executable that can behave either as a server or a client, depending on if we create or join a server from the main menu.

There is some shared code between server and clients, some examples:
 - keyboard and mouse inputs are detected and sent to the server (see [section: Player Input Handling](#gameplay_player_input_handling)),
 - GUI updates (proofps_dd::GUI class),
 - Camera update: proofps_dd::CameraHandling::cameraUpdatePosAndAngle().

Server instance is responsible for:
 - processing connecting and disconnecting players (elaborated in separate sections [Player Bootup](#gameplay_player_bootup) and [Player Disconnect](#gameplay_player_disconnect)),
 - processing inputs from all players, including the server player and the client players (see [section: Player Input Handling](#gameplay_player_input_handling)),
 - physics calculations (see [section: Physics Handling](#gameplay_physics_handling)),
 - map item pickup and respawn: proofps_dd::PRooFPSddPGE::serverPickupAndRespawnItems(),
 - sending weapon updates to the clients (see [section: Weapon Handling](#gameplay_weapon_handling)),
 - sending player updates to the clients (see [section: Physics Handling](#gameplay_physics_handling)),
 - defining game objectives and checking if those are fulfilled (see [section: Game Objectives](#gameplay_game_objectives)).
 
Client instances accept all calculations from the server instance, and responsible for displaying server game state to client players.  
Cheating is not possible by altering the behavior of a client instance, because **server is the single source of truth**.

\section gameplay_tick_rate Tickrate and Other Rates

TODO explain ticks, link to Networking page.

\section gameplay_spectating Spectator Mode and Forced Spectating

**Spectator Mode** is the initial state for all joined players in all game modes, where they can observe the game without affecting it.  
They can actually start playing the game by explicitly exiting Spectator Mode:
 - in team-based games this is done by selecting a team in the Team Selection menu,
 - in non-team-based games this is done by pressing the JOIN GAME button in the Welcome menu.

Entering Spectator Mode again is always available during gameplay by pressing 'M' to open the Team Selection / Welcome menu and pressing the SPECTATE button.

**Forced Spectating**, on the other hand, is not the player's choice: some game modes such as Team Round Game does not allow immediate respawn after dieing,
dead players need to wait for the next round to respawn. Until that happens, they are automatically put into Forced Spectating state, where they can observe the game
without affecting it, similarly to Spectator Mode. Server takes care of automatic exiting from Forced Spectating state too.

What is common in Spectator Mode and Forced Spectating state:
 - both offer "free camera spectating" and "player spectating camera" modes, the latter allows switching between different players and automatically moving the camera,
 - both is maintained per player.

What is different between Spectator Mode and Forced Spectating state:
 - Spectator Mode is the player's choice, entering and exiting it is manual action,
 - Forced Spectating state is controlled by the server automatically.

A player can be in Spectator Mode and Forced Spectating state at the same time. A typical example is after connecting to a server running Team Round Game mode:  
by default the player is dead, so it is in Forced Spectating state, and by default all new players are in Spectator Mode too.  
Exiting Spectator Mode by selecting a team may not automatically exit Forced Spectating state, the player stays in that state until a new round starts.

DOC: create flowchart about handlePlayerTeamIdChangedOrToggledSpectatorMode(), to understand when serverRespawnPlayer() is invoked, how it has effect on isForcedSpectating().

\section gameplay_player_bootup Player Bootup

A player is considered as "booted up" when its initiated connection to the server is accepted and synchronization of the full game state has been fully finished:
 - player has an accepted, unique name on this server,
 - player object and assigned weapon objects are created,
 - player is aware of all other players on this server, including their position, current weapon and other states,
 - other players are also aware of this new player on the server, including position, current weapon and other states,
 - player is aware of the states of map items on this server,
 - player is aware of the game objectives and their state on this server,
 - proofps_dd::PlayerHandling::hasPlayerBootedUp() returns true for the new player,
 - proofps_dd::Player::hasBootedUp() returns true for the new player,
 - server has logged "Player BOOTED UP" for the new player (visible only if PlayerHandling logs are enabled).

Only after finished bootup, the player is allowed to exit Spectator Mode.

To boot up a connecting player, 3 kind of messages need to be handled by the server and the other client instances:
 - pge_network::MsgUserConnectedServerSelf,
 - proofps_dd::MsgUserSetupFromServer,
 - proofps_dd::MsgUserNameChangeAndBootupDone.
 
In PRooFPS-dd-packet.h, the processing of these 3 messages are explained in more details.

The connecting client obviously receives more than 3 messages, since the whole game state needs to be replicated to it.

PGE takes care of checking client application version, and only in case of matching with server application version, a pge_network::MsgUserConnectedServerSelf will be propagated to the application layer.  
Therefore, the application (PRooFPS) does not get notified about clients that try to connect with mismatching application version.

\section gameplay_player_disconnect Player Disconnect

When a player gets disconnected from the server (either by themselves or forcefully by the server), a pge_network::MsgUserDisconnectedFromServer message needs to be processed.  
This is received by ALL instances, including the server, other clients and the disconnected player too.

\section gameplayer_map_change Map Change

Server sends out proofps_dd::MsgMapChangeFromServer to all instances (including self) with the name of the target map in case of map change.  
Server disconnects all clients during map change, in such case the clients are required to automatically try reconnecting to the server after they have loaded the target map.

\section gameplay_game_objectives Game Objectives

Only the server instance defines and checks the fulfillment of game objectives.  
Clients just accept updates about this from the server.

Game objectives handling is implemented by the proofps_dd::GameMode parent class and its derived classes.  
Each derived class represents a game mode:
 - proofps_dd::DeathMatchMode implements Deathmatch,
 - proofps_dd::TeamDeathMatchMode implements Team Deathmatch, and
 - proofps_dd::TeamRoundGameMode implements Team Round Game.
 
Game objectives are primarily checked by proofps_dd::GameMode::serverCheckAndUpdateWinningConditions(), however as these can be different in each
game mode, it is overridden by derived classes.  
Check is being done by server in every frame or tick.

Keeping an ordered list of players based on their frags and deaths is also done by proofps_dd::GameMode parent class, so rendering the Frag Table in the proofps_dd::GUI class depends on this.

There are 2 methods in proofps_dd::GameMode class that are used to check if players are allowed to play in any given moment.  
The difference between them is that one decides based on configurational data, the other decides based on operational data:
 - proofps_dd::GameMode::isPlayerAllowedForGameplay():  
   this is specific to a player instance and depends on player configuration.  
   For example if the given player is in Spectator Mode, then cannot play. Or: in team-based games, if the player is not assigned to any team, then cannot play.
 - proofps_dd::GameMode::isPlayerMovementAllowed():
   this applies to all players equally, and depends on GameMode operational data.  
   For example, in round-based game modes this is true only if RoundState is not in Prepare state.
   
Therefore, the main high-level logic of proofps_dd::PRooFPSddPGE class needs to check for these 2 functions to decide what actions are actually available for the players.  
When deriving a new class from proofps_dd::GameMode, optionally overriding these 2 methods shall be done with this in mind.

\section gameplay_player_input_handling Player Input Handling

Mouse and keyboard actions are put into a proofps_dd::MsgUserCmdFromClient message and sent to the server (server instance also does it for server player, and injects to its network message queue).  
This is being done in proofps_dd::InputHandling::clientHandleInputWhenConnectedAndSendUserCmdMoveToServer().  
Then server processes this message in proofps_dd::InputHandling::serverHandleUserCmdMoveFromClient().

Neither the server nor the client players see themselves moving right after pressing the relevant key:  
the result of any player input will be visible earliest after the next server tick, after physics calculations are also done and player position updates are sent in the form of proofps_dd::MsgUserUpdateFromServer.  
This message is sent out by proofps_dd::PlayerHandling::serverSendUserUpdates().  
This also applies to how the server player itself sees these updates, because for the server player itself proofps_dd::MsgUserUpdateFromServer is also sent by the server to self (injects into its network message queue).

**So, all players see themselves moving only after server has processed their input**, therefore cheating is not possible by a hacked client instance.  
Basically client instances just send player input to server and replicate what server thinks about the position of each player.  
This also applies to weapon and inventory item use as well, so a client can use a weapon or item only if server also thinks the player has that weapon or inventory item.

Player movement cannot be hacked either by increasing the frequency of sending proofps_dd::MsgUserCmdFromClient to it.

\section gameplay_physics_handling Physics Handling

Physics are NOT calculated by client instances because that would open the way for cheating.  
Since all player inputs are processed by the server instance, only server instance calculates physics, and based on the results it updates player positions and other data, and sends these to the clients.

The only exception is if network traffic can be cut, for example bullet movement is also simulated on client-side so server does not need to send bullet movement updates to clients.  
Note that bullet movement is also calculated on server side, and bullet collision is calculated on server side ONLY.  
Clients cannot decide on such, and must always accept server's calculations.  

Physics calculations consist of the following:
 - for players:
   - proofps_dd::Physics::serverGravity(),
   - proofps_dd::Physics::serverPlayerCollisionWithWalls(),
 - for bullets:
   - proofps_dd::WeaponHandling::serverUpdateBulletsAndHandleHittingWallsAndPlayers(),
   - proofps_dd::WeaponHandling::serverHandleBulletsVsBullets().

\section gameplay_weapon_handling Weapon Handling

Player weapon handling is implemented in proofps_dd::WeaponHandling class.  
Similar to physics, weapon actions are executed on server side for all players, therefore cheating is not possible by a client by locally modifying weapon files or trying to switch to an unavailable weapon, since
even the weapon switch action is executed on server side, only after basic checks such as weapon availability for the given player pass.
