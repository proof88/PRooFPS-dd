#pragma once

/*
    ###################################################################################
    Maps.h
    Map loader for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include <map>
#include <set>
#include <string>

#include "../../../CConsole/CConsole/src/CConsole.h"
#include "../../../PGE/PGE/PGEcfgVariable.h"
#include "../../../PGE/PGE/PRRE/include/external/PR00FsReducedRenderingEngine.h"

#include "MapItem.h"

class Maps
{
public:

    static const char* getLoggerModuleName();
    
    // ---------------------------------------------------------------------------

    CConsole& getConsole() const;
   
    Maps(PR00FsReducedRenderingEngine& gfx);
    virtual ~Maps();

    bool initialize();
    bool loaded() const;
    bool load(const char* fname);
    void unload();
    void shutdown();
    unsigned int width() const;
    unsigned int height() const;
    void UpdateVisibilitiesForRenderer();
    const std::string& getFilename() const;
    const std::set<PRREVector>& getSpawnpoints() const;
    const PRREVector& getRandomSpawnpoint() const;
    const PRREVector& getBlockPosMin() const;
    const PRREVector& getBlockPosMax() const;
    const PRREVector& getBlocksVertexPosMin() const;
    const PRREVector& getBlocksVertexPosMax() const;
    PRREObject3D** getBlocks(); // TODO: not nice access
    int getBlockCount() const;
    const std::map<MapItem::MapItemId, MapItem*>& getItems() const;
    const std::map<std::string, PGEcfgVariable>& getVars() const;
    void Update();

protected:

    Maps(const Maps&) :
        m_gfx(m_gfx)
    {}

    Maps& operator=(const Maps&)
    {
        return *this;
    }

private:
    PRREObject3D** m_blocks; // TODO: not nice, in future we switch to cpp container
    int m_blocks_h;

    std::map<std::string, PGEcfgVariable> m_vars;
    PR00FsReducedRenderingEngine& m_gfx;
    std::string m_sRawName;
    std::string m_sFileName;
    std::map<char, std::string> m_Block2Texture;
    PRRETexture* m_texRed;
    std::set<PRREVector> m_spawnpoints;
    PRREVector m_blocksVertexPosMin, m_blocksVertexPosMax;
    PRREVector m_blockPosMin, m_blockPosMax;
    unsigned int m_width, m_height;
    std::map<MapItem::MapItemId, MapItem*> m_items;

    // ---------------------------------------------------------------------------

    static bool lineShouldBeIgnored(const std::string& sLine);
    static bool lineIsValueAssignment(const std::string& sLine, std::string& sVar, std::string& sValue, bool& bParseError);

    void lineHandleAssignment(std::string& sVar, std::string& sValue);
    bool lineHandleLayout(const std::string& sLine, TPRREfloat& y);

}; // class Maps
