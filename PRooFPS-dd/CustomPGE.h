#pragma once

/*
    ###################################################################################
    CustomPGE.h
    Customized PGE for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "../../../PGE/PGE/PGE.h"
#include "../../../PGE/PGE/PRRE/include/external/Object3D/PRREObject3DManager.h"

#include "Consts.h"
#include "Maps.h"

/**
    The customized game engine class. This handles the game logic. Singleton.
*/
class CustomPGE :
    public PGE
{

public:

    static CustomPGE* createAndGetCustomPGEinstance();
    static const char* getLoggerModuleName();

    // ---------------------------------------------------------------------------

    CConsole& getConsole() const;
    
   
protected:

    CustomPGE() :
        maps(getPRRE())
    {}

    CustomPGE(const CustomPGE&) :
        maps(getPRRE())
    {}

    CustomPGE& operator=(const CustomPGE&)
    {
        return *this;
    }

    explicit CustomPGE(const char* gametitle);  /**< This is the only usable ctor, this is used by the static createAndGet(). */
    virtual ~CustomPGE();

    virtual void onGameInitializing(); /**< Must-have minimal stuff before loading anything. */
    virtual void onGameInitialized();  /**< Loading game content here. */
    virtual void onGameRunning();      /**< Game logic here. */
    virtual void onGameDestroying();   /**< Freeing up game content here. */


private:

    Maps maps;

    // ---------------------------------------------------------------------------


}; // class CustomPGE