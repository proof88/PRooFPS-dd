/*
    ###################################################################################
    CustomPGE.cpp
    Customized PGE for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "stdafx.h"  // PCH

#include "CustomPGE.h"

#include "../../../PGE/PGE/PRRE/include/external/PRREuiManager.h"
#include "../../../PGE/PGE/PRRE/include/external/Display/PRREWindow.h"
#include "../../../PGE/PGE/PRRE/include/external/PRRECamera.h"
#include "../../../CConsole/CConsole/src/CConsole.h"


static const std::string GAME_NAME    = "PRooFPS-dd";
static const std::string GAME_VERSION = "0.1.0.0 Alpha";


// ############################### PUBLIC ################################


CustomPGE* CustomPGE::createAndGetCustomPGEinstance()
{
    static CustomPGE pgeInstance((GAME_NAME + " " + GAME_VERSION).c_str());
    return &pgeInstance;
}


// ############################## PROTECTED ##############################


/**
    This is the only usable ctor, this is used by the static createAndGet().
*/
CustomPGE::CustomPGE(const char* gameTitle) :
    PGE(gameTitle),
    maps(getPRRE())
{

}

CustomPGE::~CustomPGE()
{

}

CConsole& CustomPGE::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}


const char* CustomPGE::getLoggerModuleName()
{
    return "CustomPGE";
}

/**
    Must-have minimal stuff before loading anything.
    Game engine calls this before even finishing its own initialization.
*/
void CustomPGE::onGameInitializing()
{
    // Earliest we can enable our own logging
    getConsole().Initialize((GAME_NAME + " " + GAME_VERSION + " log").c_str(), true);
    getConsole().SetLoggingState(getLoggerModuleName(), true);
    getConsole().SetFGColor( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, "999999" );
    getConsole().SetIntsColor( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00" );
    getConsole().SetStringsColor( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY, "FFFFFF" );
    getConsole().SetFloatsColor( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "FFFF00" );
    getConsole().SetBoolsColor( FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "00FFFF" );

    // Turn everything on for development only
    //getConsole().SetLoggingState("4LLM0DUL3S", true);

    CConsole::getConsoleInstance().SetLoggingState(Maps::getLoggerModuleName(), true);
}

/** 
    Loading game content here.
*/
void CustomPGE::onGameInitialized()
{
    getConsole().OLnOI("CustomPGE::onGameInitialized()");

    getPRRE().getCamera().SetNearPlane(0.1f);
    getPRRE().getCamera().SetFarPlane(100.0f);
    getPRRE().getCamera().getPosVec().Set( 0, 0, GAME_CAM_Z );
    getPRRE().getCamera().getTargetVec().Set( 0, 0, -GAME_BLOCK_SIZE_Z );

    maps.initialize();
    maps.load("gamedata/maps/map_test_good.txt");

    getPRRE().WriteList();

}

/** 
    Game logic here.
    Game engine invokes this in every frame.
    DO NOT make any unnecessary operations here, as this function must always complete below 16 msecs to keep stable 60 fps!
*/
void CustomPGE::onGameRunning()
{
    const PRREWindow& window = getPRRE().getWindow();
    const PGEInputHandler& input = PGEInputHandler::createAndGet();

    static bool bCameraLocked = false;

    /*const int oldmx = input.getMouse().getCursorPosX();
    const int oldmy = input.getMouse().getCursorPosY();*/

    input.getMouse().SetCursorPos(window.getX() + window.getWidth()/2,
                                  window.getY() + window.getHeight()/2);

    if ( input.getKeyboard().isKeyPressed( VK_LEFT ) )
    {
        getPRRE().getCamera().Strafe(-0.1f);
    }
    if ( input.getKeyboard().isKeyPressed( VK_RIGHT ) )
    {
        getPRRE().getCamera().Strafe(0.1f);
    }

    std::stringstream str;
    //str << "MX1: " << changeX << "   MY1: " << changeY;
    //str << "key: " << input.getKeyboard().isKeyPressed(VK_RETURN);

    //window.SetCaption(str.str());
}

/** 
    Freeing up game content here.
    Free up everything that has been allocated in onGameInitialized() and onGameRunning().
*/
void CustomPGE::onGameDestroying()
{
    getPRRE().getObject3DManager().DeleteAll();

    getConsole().Deinitialize();
}


// ############################### PRIVATE ###############################

