/*
    ###################################################################################
    Maps.cpp
    Map loader for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    ###################################################################################
*/

#include "stdafx.h"  // PCH

#include <cassert>

#include "Consts.h"
#include "Maps.h"


// ############################### PUBLIC ################################


proofps_dd::Maps::Maps(
    pge_audio::PgeAudio& audio,
    PGEcfgProfiles& cfgProfiles,
    PR00FsUltimateRenderingEngine& gfx) :
    m_audio(audio),
    m_cfgProfiles(cfgProfiles),
    m_gfx(gfx),
    m_texRed(PGENULL),
    m_texDecorJumpPadVertical(PGENULL),
    m_blocks(NULL),
    m_blocks_h(0),
    m_foregroundBlocks(NULL),
    m_foregroundBlocks_h(0),
    m_bvh(4,0),
    m_width(0),
    m_height(0),
    m_nValidJumppadVarsCount(0)
{
    proofps_dd::MapItem::resetGlobalData();
}

proofps_dd::Maps::~Maps()
{
    shutdown();
}

CConsole& proofps_dd::Maps::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}

const char* proofps_dd::Maps::getLoggerModuleName()
{
    return "Maps";
}

/**
    Initializes the map handler.
    Reads the mapcycle file if it exists.
    You need to invoke this once before trying to load any map.

    @return True on success, false otherwise.
*/
bool proofps_dd::Maps::initialize()
{
    if (isInitialized())
    {
        return true;
    }

    bool bInitialized = false;
    m_texRed = m_gfx.getTextureManager().createFromFile((std::string(proofps_dd::GAME_TEXTURES_DIR) + "red.bmp").c_str());
    m_texDecorJumpPadVertical = m_gfx.getTextureManager().createFromFile((std::string(proofps_dd::GAME_TEXTURES_DIR) + "decor-jump-pad2-from-www.flaticon.com.bmp").c_str());

    if (m_texRed && m_texDecorJumpPadVertical)
    {
        bInitialized = m_mapcycle.initialize() && m_bvh.setMaxDepthLevel(m_cfgProfiles.getVars()[szCVarSvMapCollisionBvhMaxDepth].getAsUInt());
    }

    if (!bInitialized)
    {
        shutdown();
    }
    
    return bInitialized;
}

bool proofps_dd::Maps::isInitialized() const
{
    return m_texRed != PGENULL;
}

/**
    Shuts down the map handler.
    The currently loaded map will be also unloaded.
    After calling this, initialize() can be invoked again.
*/
void proofps_dd::Maps::shutdown()
{
    getConsole().OLnOI("Maps::shutdown() ...");
    if (isInitialized())
    {
        // There is no specific need to delete loaded sounds, and I could not find specific functions to free up loaded sound data.
        // When initialize() is invoked again, and we load sounds again, SoLoud frees up previously allocated sound data for the
        // same wavs, so in that case memory cannot leak.

        /* Current map handling */
        unload();
        m_sServerMapFilenameToLoad.clear();

        /* Mapcycle and Available Maps Handling */
        m_mapcycle.shutdown();

        delete m_texDecorJumpPadVertical;
        m_texDecorJumpPadVertical = PGENULL;

        delete m_texRed;
        m_texRed = PGENULL;
    }
    getConsole().OOOLn("Maps::shutdown() done!");
}

const std::string& proofps_dd::Maps::serverDecideFirstMapAndUpdateNextMapToBeLoaded()
{
    // PGEcfgProfiles allows the value of a CVAR be full of spaces (it is a valid case), which means that here we should trim
    // the SV_MAP value properly since we at this level KNOW that spaces should NOT be present in this specific CVAR.
    {
        // this is ridiculous, PFL still doesnt have std::string-compatible strClr() !!!
        const std::string sOriginalMapName = m_cfgProfiles.getVars()[szCVarSvMap].getAsString();
        char cLine[200]{};
        strncpy_s(cLine, sizeof(cLine), sOriginalMapName.c_str(), sOriginalMapName.length());
        PFL::strClr(cLine);
        m_cfgProfiles.getVars()[szCVarSvMap].Set(cLine);
    }

    if (m_cfgProfiles.getVars()[szCVarSvMap].getAsString().empty())
    {
        m_mapcycle.mapcycleRewindToFirst();
        m_sServerMapFilenameToLoad = m_mapcycle.mapcycleGetCurrent();
        if (m_sServerMapFilenameToLoad.empty())
        {
            return m_sServerMapFilenameToLoad; // error
        }
        else
        {
            getConsole().OLn("First map by mapcycle: %s", m_sServerMapFilenameToLoad.c_str());
        }
    }
    else
    {
        m_sServerMapFilenameToLoad = m_cfgProfiles.getVars()[szCVarSvMap].getAsString();
        getConsole().OLn("First map by config (%s): %s", szCVarSvMap, m_sServerMapFilenameToLoad.c_str());

        // we go to last map in mapcycle so that when server switches from szCVarSvMap to next map, it will be the
        // first map in mapcycle, this is how it will properly start looping mapcycle.
        m_mapcycle.mapcycleForwardToLast();
    }
    return m_sServerMapFilenameToLoad;
}

const std::string& proofps_dd::Maps::getNextMapToBeLoaded() const
{
    return m_sServerMapFilenameToLoad;
}

bool proofps_dd::Maps::loaded() const
{
    return ( m_blocks != NULL );
}

bool proofps_dd::Maps::load(const char* fname, std::function<void(int)>& cbDisplayProgressUpdate)
{
    getConsole().OLnOI("Maps::load(%s) ...", fname);

    if (!isInitialized())
    {
        getConsole().EOLnOO("ERROR: map handler is not initialized!");
        return false;
    }

    if (loaded())
    {
        getConsole().EOLnOO("ERROR: %s is already loaded, should call unload first!", m_sFileName.c_str());
        return false;
    }

    cbDisplayProgressUpdate(0);

    // this wont be needed after we require unload() before consecutive load()
    proofps_dd::MapItem::resetGlobalData();

    const std::string sFilenameWithRelativePath = std::string(Mapcycle::GAME_MAPS_DIR) + fname;
    m_sFileName = PFL::getFilename(sFilenameWithRelativePath.c_str());
    m_sRawName = PFL::changeExtension(m_sFileName.c_str(), "");
    if (m_sRawName.empty())
    {
        getConsole().EOLnOO("ERROR: empty raw name!");
        unload();
        return false;
    }

    std::ifstream f;                                                                   
    f.open(sFilenameWithRelativePath.c_str(), std::ifstream::in);
    if ( !f.good() )
    {
        getConsole().EOLnOO("ERROR: failed to open file %s!", m_sFileName.c_str());
        unload();
        return false;
    }

    m_sServerMapFilenameToLoad = fname;

    const TPURE_ISO_TEX_FILTERING texFilterMinOriginal = m_gfx.getTextureManager().getDefaultMinFilteringMode();
    const TPURE_ISO_TEX_FILTERING texFilterMagOriginal = m_gfx.getTextureManager().getDefaultMagFilteringMode();
    m_gfx.getTextureManager().setDefaultIsoFilteringMode(
        TPURE_ISO_TEX_FILTERING::PURE_ISO_LINEAR_MIPMAP_LINEAR,
        TPURE_ISO_TEX_FILTERING::PURE_ISO_LINEAR);

    bool bParseError = false;
    bool bMapLayoutReached = false;
    constexpr std::streamsize nBuffSize = 1024;
    char cLine[nBuffSize];
    TPureFloat y = 0.f;  // before v0.2.5 it was 4.f but I dont know why, I'm changing to 0 so all blocks have their Y-pos <= 0.f
    std::streampos fStreamPosMapLayoutStart, fStreamPosPrevLine;
    // First the map layout handling will be "dry", meaning that the map won't be actually created but
    // the number of required blocks will be counted. This is useful, because then in the next non-dry run,
    // we preallocate exactly that array size required for that number of blocks, this way no reallocation of array will be needed.
    // Since the maps are typed into text file by humans, we cannot expect them to save the exact number of blocks. :)
    while ( !bParseError && !f.eof() )
    {
        fStreamPosPrevLine = f.tellg();
        f.getline(cLine, nBuffSize);
        // TODO: we should finally have a strClr() version for std::string or FINALLY UPGRADE TO NEWER CPP THAT MAYBE HAS THIS FUNCTIONALITY!!!
        PFL::strClr( cLine );
        const std::string sLine(cLine);
        std::string sVar, sValue;
        if ( lineShouldBeIgnored(sLine) )
        {
            continue;
        }
        else if ( lineIsValueAssignment(sLine, sVar, sValue, bParseError) )
        {
            if ( bMapLayoutReached )
            {
                getConsole().EOLn("ERROR: parse: assignment after map layout block: %s!", sLine.c_str());
                bParseError = true;
            }
            else
            {
                bParseError = !lineHandleAssignment(sVar, sValue);
            }
        }
        else if ( !bParseError )
        {
            if (!bMapLayoutReached)
            {
                getConsole().OLn("Just reached map layout ...");
                bMapLayoutReached = true;
                fStreamPosMapLayoutStart = fStreamPosPrevLine;
            }
            bParseError = !lineHandleLayout(sLine, y, true);
        }
    };

    // non-dry-run starts, this is the slowest part of map loading, thus we invoke cbDisplayProgressUpdate in this block
    if (!bParseError)
    {
        getConsole().OLn(
            "Just finished dry run, now building up the map with width %u, height %u, m_blocks_h %d, m_foregroundBlocks_h %d ...",
            m_width, m_height, m_blocks_h, m_foregroundBlocks_h);
        
        // During dry-run above, m_blocks_h and m_foregroundBlocks_h had been properly set, but they will be reset and counted again now
        // during the non-dry-run, so save m_blocks_h now to calculate loading progress
        const int nTotalBlocks_h = m_blocks_h;
        f.clear();  // clears error states, including eof bit
        f.seekg(fStreamPosMapLayoutStart);
        y = 0.f; // just reset this to same value as it was before the loop
        while (!bParseError && !f.eof())
        {
            f.getline(cLine, nBuffSize);
            // TODO: we should finally have a strClr() version for std::string or FINALLY UPGRADE TO NEWER CPP THAT MAYBE HAS THIS FUNCTIONALITY!!!
            PFL::strClr(cLine);
            const std::string sLine(cLine);
            if (lineShouldBeIgnored(sLine))
            {
                continue;
            }
            bParseError = !lineHandleLayout(sLine, y, false);
            cbDisplayProgressUpdate(nTotalBlocks_h == 0 ? 0 : static_cast<int>((m_blocks_h / static_cast<float>(nTotalBlocks_h)) * 100));
        }
    }
    f.close();

    // variables are loaded during dry-run, but blocks are not, thus this is the earliest time to fail if their count does not match!
    const auto nJumppadVarsCount = getJumppadValidVarsCount();
    if (m_jumppads.size() != nJumppadVarsCount)
    {
        getConsole().EOLn("ERROR: jumppads size (%u) != valid jumppad vars count (%u)", m_jumppads.size(), nJumppadVarsCount);
        bParseError = true;
    }

    bParseError |= !checkAndUpdateSpawnpoints();

    m_gfx.getTextureManager().setDefaultIsoFilteringMode(
        texFilterMinOriginal,
        texFilterMagOriginal);

    if ( bParseError )
    {
        getConsole().EOLnOO("ERROR: failed to parse file!");
        unload();
        return false;
    }

    getConsole().OLn("Just built up the map with m_blocks_h %d, m_foregroundBlocks_h %d ...", m_blocks_h, m_foregroundBlocks_h);

    m_blockPosMin = m_blocks[0]->getPosVec();
    m_blockPosMax = m_blocks[0]->getPosVec();
    for (int i = 0; i < m_blocks_h; i++)
    {
        if ( m_blocks[i] != PGENULL )
        {
            if (m_blocks[i]->getPosVec().getX() < m_blockPosMin.getX())
            {
                m_blockPosMin.SetX(m_blocks[i]->getPosVec().getX());
            }
            else if (m_blocks[i]->getPosVec().getX() > m_blockPosMax.getX())
            {
                m_blockPosMax.SetX(m_blocks[i]->getPosVec().getX());
            }

            if (m_blocks[i]->getPosVec().getY() < m_blockPosMin.getY())
            {
                m_blockPosMin.SetY(m_blocks[i]->getPosVec().getY());
            }
            else if (m_blocks[i]->getPosVec().getY() > m_blockPosMax.getY())
            {
                m_blockPosMax.SetY(m_blocks[i]->getPosVec().getY());
            }

            if (m_blocks[i]->getPosVec().getZ() < m_blockPosMin.getZ())
            {
                m_blockPosMin.SetZ(m_blocks[i]->getPosVec().getZ());
            }
            else if (m_blocks[i]->getPosVec().getZ() > m_blockPosMax.getZ())
            {
                m_blockPosMax.SetZ(m_blocks[i]->getPosVec().getZ());
            }
        }
    }
    m_blocksVertexPosMin.Set(
        m_blockPosMin.getX() - proofps_dd::Maps::fMapBlockSizeWidth / 2.f,
        m_blockPosMin.getY() - proofps_dd::Maps::fMapBlockSizeHeight / 2.f,
        m_blockPosMin.getZ() - proofps_dd::Maps::fMapBlockSizeDepth / 2.f);
    m_blocksVertexPosMax.Set(
        m_blockPosMax.getX() + proofps_dd::Maps::fMapBlockSizeWidth / 2.f,
        m_blockPosMax.getY() + proofps_dd::Maps::fMapBlockSizeHeight / 2.f,
        m_blockPosMax.getZ() + proofps_dd::Maps::fMapBlockSizeDepth / 2.f);

    if (m_cfgProfiles.getVars()[szCVarSvMapCollisionBvhDebugRender].getAsBool())
    {
        m_bvh.updateAndEnableAabbDebugRendering(m_gfx.getObject3DManager());
    }
    //m_bvh.updateAndEnableNodeDebugRendering(m_gfx.getObject3DManager()); // octree nodes
    getConsole().OLn(
        "%s Built BVH: maxdepth: %u, pos: [%f,%f,%f], size: %f, AABB pos: [%f,%f,%f], AABB size: [%f,%f,%f]",
        __func__,
        m_bvh.getMaxDepthLevel(),
        m_bvh.getPos().getX(),
        m_bvh.getPos().getY(),
        m_bvh.getPos().getZ(),
        m_bvh.getSize(),
        m_bvh.getAABB().getPosVec().getX(),
        m_bvh.getAABB().getPosVec().getY(),
        m_bvh.getAABB().getPosVec().getZ(),
        m_bvh.getAABB().getSizeVec().getX(),
        m_bvh.getAABB().getSizeVec().getY(),
        m_bvh.getAABB().getSizeVec().getZ());

    getConsole().SOLnOO("> Map loaded with width %u and height %u!", m_width, m_height);
    return true;
}

void proofps_dd::Maps::unload()
{
    getConsole().OLnOI("Maps::unload() ...");
    m_bvh.reset();
    m_sServerMapFilenameToLoad.clear();
    m_sRawName.clear();
    m_sFileName.clear();
    m_Block2Texture.clear();
    if ( m_blocks )
    {
        for (int i = 0; i < m_blocks_h; i++)
        {
            m_gfx.getObject3DManager().DeleteAttachedInstance( *(m_blocks[i]) );
        }
        free( m_blocks );
        m_blocks = NULL;

        free(m_foregroundBlocks);
        m_foregroundBlocks = NULL;
    }
    m_blocks_h = 0;
    m_foregroundBlocks_h = 0;

    for (auto& pairChar2RefBlockObject3D : m_mapReferenceBlockObject3Ds)
    {
        delete pairChar2RefBlockObject3D.second; // will remove from object3dmanager too
    }
    m_mapReferenceBlockObject3Ds.clear();

    for (auto& itemPair : m_items)
    {
        delete itemPair.second;
    }
    m_items.clear();
    for (auto& pDecalObj : m_decals)
    {
        delete pDecalObj; // will remove from object3dmanager too
    }
    m_decals.clear();
    for (auto& pDecorObj : m_decorations)
    {
        delete pDecorObj; // will remove from object3dmanager too
    }
    m_decorations.clear();
    m_jumppads.clear();
    m_fJumppadForceFactors.clear();
    proofps_dd::MapItem::resetGlobalData();

    m_width = 0;
    m_height = 0;
    m_blockPosMin.SetZero();
    m_blockPosMax.SetZero();
    m_blocksVertexPosMin.SetZero();
    m_blocksVertexPosMax.SetZero();
    m_vars.clear();
    m_nValidJumppadVarsCount = 0;
    m_spawngroup_1.clear();
    m_spawngroup_2.clear();
    m_spawnpoints.clear();

    getConsole().OOOLn("Maps::unload() done!");
}

unsigned int proofps_dd::Maps::width() const
{
    return m_width;
}

unsigned int proofps_dd::Maps::height() const
{
    return m_height;
}

void proofps_dd::Maps::updateVisibilitiesForRenderer()
{
    const PureVector& campos = m_gfx.getCamera().getPosVec();

    for (int i = 0; i < m_blocks_h; i++)
    {
        PureObject3D* const obj = m_blocks[i];
        if ( obj != PGENULL )
        {
            if ( (obj->getPosVec().getX() + obj->getSizeVec().getX()/2.0f) <= campos.getX() - 13.f )
            {
                obj->SetRenderingAllowed(false);
            }
            else
            {
                if ( (obj->getPosVec().getX() - obj->getSizeVec().getX()/2.0f) >= campos.getX() + 13.f )
                {
                    obj->SetRenderingAllowed(false);
                }
                else
                {
                    obj->SetRenderingAllowed(true);
                }
            }
        }
    }
}

/**
    Retrieves the currently loaded map filename.

    @return Filename of the currently loaded map. Empty string if no map is loaded currently.
*/
const std::string& proofps_dd::Maps::getFilename() const
{
    return m_sFileName;
}


/**
    Retrieves the set of spawnpoints of the currently loaded map.
    A spawnpoint is a 3D coordinate where the player can spawn at.

    @return The set of spawnpoints of the currently loaded map.
*/
const std::vector<PureVector>& proofps_dd::Maps::getSpawnpoints() const
{
    return m_spawnpoints;
}


/**
    Retrieves the spawn group of the currently loaded map, for the specified team.
    A spawn group is a set of spawnpoints dedicated to a specific team.

    @return The set of spawn group of the currently loaded map, for the specified team.
*/
const std::set<size_t>& proofps_dd::Maps::getTeamSpawnpoints(const unsigned int& iTeamId) const
{
    if (m_spawnpoints.empty())
    {
        throw std::runtime_error("No spawnpoints!");
    }

    if (iTeamId == 0 || iTeamId > 2)
    {
        throw std::runtime_error("Invalid Team Id!");
    }

    return (iTeamId == 1) ?
        m_spawngroup_1 :
        m_spawngroup_2;
}


/**
    @return True if spawn points are defined for the teams, false otherwise.
*/
bool proofps_dd::Maps::areTeamSpawnpointsDefined() const
{
    return !m_spawngroup_1.empty() && !m_spawngroup_2.empty();
}


/**
    Tells if all conditions meet for selecting a team spawnpoint from a spawn group:
     - current game mode is team based,
     - team id is non-zero,
     - there are spawn groups defined for the current map,
     - server config allows using team spawn groups.
    
    @param bTeamGame True if current GameMode is team-based game, false otherwise.
    @param iTeamId   The team ID if we want to find a spawn point from a spawn group for the specified team.
    
    @return True if team spawnpoints / spawn groups can be used now, false otherwise.
*/
bool proofps_dd::Maps::canUseTeamSpawnpoints(const bool& bTeamGame, const unsigned int& iTeamId) const
{
    return areTeamSpawnpointsDefined() &&
        m_cfgProfiles.getVars()[szCVarSvMapTeamSpawnGroups].getAsBool() &&
        bTeamGame &&
        (iTeamId > 0);
}


/**
    Retrieves a randomly selected spawnpoint from the set of spawnpoints of the currently loaded map.
    A spawnpoint is a 3D coordinate where the player can spawn at.

    This function can be used also if we want to get a randomly selected spawnpoint from a spawn group.
    In that case bTeamGame and iTeamId arguments need to be properly set.
    If the current map does not define spawn groups, or the server setting "Team Spawn Groups" is disabled,
    then these 2 arguments are ignored but still a randomly selected spawnpoint is returned.

    @param bTeamGame True if current GameMode is team-based game, false otherwise.
    @param iTeamId   The team ID if we want to find a spawn point from a spawn group for the specified team.
                     Considered only if bTeamGame is true.
                     Must be 1 or 2 to actually utilize spawn groups.
                     If 0, bTeamGame is considered as false.

    @return A randomly selected spawnpoint on the current map.
*/
const PureVector& proofps_dd::Maps::getRandomSpawnpoint(
    const bool& bTeamGame,
    const unsigned int& iTeamId) const
{
    if ( m_spawnpoints.empty() )
    {
        throw std::runtime_error("No spawnpoints!");
    }

    int iElem;
    if (canUseTeamSpawnpoints(bTeamGame, iTeamId))
    {
        // select a random spawnpoint from the team's spawn group
        const auto& spawngroup = getTeamSpawnpoints(iTeamId);
        iElem = PFL::random(0, spawngroup.size() - 1);
        auto it = spawngroup.begin();
        std::advance(it, iElem);
        iElem = *it; // spawngroup contains m_spawnpoints indices so we have selected a random index to m_spawnpoints
    }
    else
    {
        // select a random spawnpoint from the global pool
        iElem = PFL::random(0, m_spawnpoints.size() - 1);
    }

    //getConsole().EOLn("Maps::%s(): %d, count: %u", __func__, iElem, m_spawnpoints.size());
    return m_spawnpoints[iElem];
}


const PureVector& proofps_dd::Maps::getLeftMostSpawnpoint() const
{
    if (m_spawnpoints.empty())
    {
        throw std::runtime_error("No spawnpoints!");
    }

    return m_spawnpointLeftMost;
}

const PureVector& proofps_dd::Maps::getRightMostSpawnpoint() const
{
    if (m_spawnpoints.empty())
    {
        throw std::runtime_error("No spawnpoints!");
    }

    return m_spawnpointRightMost;
}

const PureVector& proofps_dd::Maps::getBlockPosMin() const
{
    return m_blockPosMin;
}

const PureVector& proofps_dd::Maps::getBlockPosMax() const
{
    return m_blockPosMax;
}

const PureVector& proofps_dd::Maps::getBlocksVertexPosMin() const
{
    return m_blocksVertexPosMin;
}

const PureVector& proofps_dd::Maps::getBlocksVertexPosMax() const
{
    return m_blocksVertexPosMax;
}

PureObject3D** proofps_dd::Maps::getBlocks()
{
    return m_blocks;
}

PureObject3D** proofps_dd::Maps::getForegroundBlocks()
{
    return m_foregroundBlocks;
}

int proofps_dd::Maps::getBlockCount() const
{
    return m_blocks_h;
}

int proofps_dd::Maps::getForegroundBlockCount() const
{
    return m_foregroundBlocks_h;
}

const PureBoundingVolumeHierarchy& proofps_dd::Maps::getBVH() const
{
    return m_bvh;
}

const std::map<proofps_dd::MapItem::MapItemId, proofps_dd::MapItem*>& proofps_dd::Maps::getItems() const
{
    return m_items;
}

const std::map<std::string, PGEcfgVariable>& proofps_dd::Maps::getVars() const
{
    return m_vars;
}

size_t proofps_dd::Maps::getJumppadValidVarsCount()
{
    // This is a lazy evaluator, so it will count valid vars only at first call, and in subsequent calls it will just return the
    // previously calculated value.
    // It keeps recalculating when number of previously calculated valid vars are not matching with the number of jump pads.
    // This is to make sure it always returns valid value, just in case someone would call it in the middle of map loading.
    // In that case speed is not an issue, later during actual gameplay it will be very fast due to no recalculating.
    // And obviously 0 == 0 case is also very straightforward: it does not calculate if there is no jumppad in map layout.
    if (m_jumppads.size() == m_nValidJumppadVarsCount)
    {
        return m_nValidJumppadVarsCount;
    }

    if (m_decorations.size() != m_jumppads.size())
    {
        getConsole().EOLn(
            "PRooFPSddPGE::%s(): decorations count %u != jumppads count %u!",
            __func__,
            m_decorations.size(),
            m_jumppads.size());
        return 0;
    }

    m_fJumppadForceFactors.clear();
    m_nValidJumppadVarsCount = 0;
    size_t iJumppad = 0;
    for (const auto& var : m_vars)
    {
        if (var.first.find("jumppad_") != std::string::npos)
        {
            if (var.first != (std::string("jumppad_") + std::to_string(iJumppad)))
            {
                getConsole().EOLn(
                    "PRooFPSddPGE::%s(): index error: variable name is %s but our running index is %u!",
                    __func__,
                    var.first.c_str(),
                    iJumppad);
                continue;
            }

            if (var.second.getAsString().empty())
            {
                getConsole().EOLn("PRooFPSddPGE::%s(): variable %s empty!", __func__, var.first.c_str());
                continue;
            }

            const auto itCommaChar = var.second.getAsString().find(' ');
            if (itCommaChar == std::string::npos)
            {
                const float fForceY = var.second.getAsFloat();
                if (fForceY > 0.f)
                {
                    m_fJumppadForceFactors.push_back({0.f, fForceY});
                    m_nValidJumppadVarsCount++;
                }
            }
            else
            {
                std::stringstream sstr(var.second.getAsString());
                TPURE_XY fForces{};
                sstr >> fForces.y;
                sstr >> fForces.x;
                if (fForces.y > 0.f)
                {
                    m_fJumppadForceFactors.push_back(fForces);
                    m_nValidJumppadVarsCount++;

                    // This trick I'm doing here is without any serious scientific background and not even something I should be proud of, but
                    // this is how it works: since the jumppad mult factors are specified as Y and X force multipliers, which are input to the
                    // Physics engine for calculating the INITIAL jump force, and we here in Maps do not know how Physics is doing the calculations,
                    // we are just making an ESTIMATION about the resulting jump angle the player will experience!
                    // We are ASSUMING that a relatively big X multiplier (compared to the Y multiplier) will result in a more horizontal force,
                    // rather than a more vertical force, and vice versa.
                    // So, by treating the Y and X multipliers as components of a force vector, and by normalizing this vector, we will have a ROUGH
                    // estimation of the horizontal force compared to the vertical force, and, if we treat this horizontal force component
                    // as sine of the Z angle of the force (where -90° <= Z angle <= 90°), we can make a ROUGH ESTIMATION about the Z angle of
                    // the force, and we set that angle for the decoration arrow. Which might be a wrong estimation since we dont have a clue about
                    // how really these multipliers contribute to the final movement, because at this point Physics is a blackbox to Maps.
                    // Still, I find this working more or less ok, so I leave it here.
                    // I'm treating normalized x value as sine, because after normalization, vec will have: -1 <= x <= 1, and 0 means 0°.
                    PureVector vecForces(fForces.x, fForces.y, 0.f);
                    vecForces.Normalize();
                    //const float fForcesXYratio = fForces.x / fForces.y;
                    //getConsole().EOLn("PRooFPSddPGE::%s(): jumppad %u forces ratio: %f!", __func__, iJumppad, fForcesXYratio);
                    //getConsole().EOLn("PRooFPSddPGE::%s(): jumppad %u forces vec normalized: x: %f, y: %f!", __func__, iJumppad, vecForces.getX(), vecForces.getY());

                    assert(m_decorations[iJumppad]);  // no nulls here
                    
                    // I'm flipping the object, otherwise positive Z angle would turn it to the left instead right
                    m_decorations[iJumppad]->getAngleVec().SetY(180.f);
                    m_decorations[iJumppad]->SetDoubleSided(true);

                    m_decorations[iJumppad]->getAngleVec().SetZ(
                        PFL::radToDeg(std::asinf(vecForces.getX()))
                    );

                    // maybe it is also a good idea to displace this decoration horizontally based on the horizontal component:
                    // (would not need this if we could modify the center of the object before rotation i.e. pivot point)
                    m_decorations[iJumppad]->getPosVec().SetX(
                        m_decorations[iJumppad]->getPosVec().getX() + vecForces.getX()
                    );
                }
            }

            iJumppad++;
        }
    }
    return m_nValidJumppadVarsCount;
}

const TPURE_XY& proofps_dd::Maps::getJumppadForceFactors(const size_t& index) const
{
    if (index >= m_nValidJumppadVarsCount)
    {
        throw std::runtime_error("getJumppadForceFactors(): Invalid jumppad index: " + std::to_string(index));
    }

    // the above condition and load() should make sure this assertion can never fail even during unit tests!
    assert(index < m_fJumppadForceFactors.size());
    return m_fJumppadForceFactors[index];
}

const std::vector<PureObject3D*>& proofps_dd::Maps::getDecals() const
{
    return m_decals;
}

const std::vector<PureObject3D*>& proofps_dd::Maps::getJumppads() const
{
    return m_jumppads;
}

void proofps_dd::Maps::update(const float& fps, const PureObject3D& objCurrentPlayer)
{
    // invoked by both server and client

    static constexpr float MAPITEM_VERTICAL_ANIM_UPDATE_SPEED = 480.0f;
    
    for (auto& itemPair : getItems())
    {
        if (!itemPair.second)
        {
            continue;
        }

        proofps_dd::MapItem& mapItem = *(itemPair.second);
        if (mapItem.isTaken())
        {
            continue;
        }

        mapItem.update(MAPITEM_VERTICAL_ANIM_UPDATE_SPEED / fps);
    }

    static constexpr float DECOR_ALPHA_CHANGE_SPEED = 180.0f;
    static constexpr float fDecorAlphaMin = 100.f;
    static constexpr float fDecorAlphaMax = 200.f;
    static_assert(fDecorAlphaMin >= 0, "will be casted to TPureUByte");
    static_assert(fDecorAlphaMin <= 255, "will be casted to TPureUByte");
    static float fDecorAlpha = 200.f;
    static bool bDecorAlphaUp = false;

    if (bDecorAlphaUp)
    {
        fDecorAlpha += DECOR_ALPHA_CHANGE_SPEED / fps;
        if (fDecorAlpha > fDecorAlphaMax)
        {
            fDecorAlpha = fDecorAlphaMax;
            bDecorAlphaUp = false;
        }
    }
    else
    {
        fDecorAlpha -= DECOR_ALPHA_CHANGE_SPEED / fps;
        if (fDecorAlpha < fDecorAlphaMin)
        {
            fDecorAlpha = fDecorAlphaMin;
            bDecorAlphaUp = true;
        }
    }

    for (auto& pDecor : m_decorations)
    {
        assert(pDecor);  // we dont store nulls here
        pDecor->getMaterial(false).getTextureEnvColor().SetAlpha(
            static_cast<TPureUByte>(std::llroundl(fDecorAlpha))
        );
    }

    if (m_cfgProfiles.getVars()[szCVarSvMapCollisionBvhDebugRender].getAsBool())
    {
        auto pTightestPlayerFittingBvhNode = m_bvh.findOneLowestLevelFittingNode(objCurrentPlayer, BvhSearchDirection::DownFromRootNode);
        m_bvh.markAabbDebugRendering(pTightestPlayerFittingBvhNode);
    }
}

bool proofps_dd::Maps::handleMapItemUpdateFromServer(
    pge_network::PgeNetworkConnectionHandle /*connHandleServerSide*/,
    const proofps_dd::MsgMapItemUpdateFromServer& msg)
{
    if (!loaded())
    {
        // we not yet loaded any map, but already received update from server, which is normal because server already registered us as connected client
        return true;
    }

    const auto it = getItems().find(msg.m_mapItemId);
    if (it == getItems().end())
    {
        getConsole().EOLn("PRooFPSddPGE::%s(): unknown map item id %u, CANNOT HAPPEN!", __func__, msg.m_mapItemId);
        return false;
    }

    MapItem* const pMapItem = it->second;

    if (msg.m_bTaken)
    {
        pMapItem->take();
    }
    else
    {
        pMapItem->unTake();
    }

    return true;
}  // handleMapItemUpdateFromServer()

proofps_dd::Mapcycle& proofps_dd::Maps::getMapcycle()
{
    return m_mapcycle;
}


// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################


bool proofps_dd::Maps::lineShouldBeIgnored(const std::string& sLine)
{
    return sLine.empty() || (sLine[0] == '#');
}

bool proofps_dd::Maps::lineIsValueAssignment(const std::string& sLine, std::string& sVar, std::string& sValue, bool& bParseError)
{
    const std::string::size_type nAssignmentPos = sLine.find('=');
    if ( nAssignmentPos == std::string::npos )
    {
        return false;
    }

    if ( (nAssignmentPos == (sLine.length() - 1)) || (nAssignmentPos == 0 ) )
    {
        CConsole::getConsoleInstance("Maps").EOLn("ERROR: erroneous assignment: %s!", sLine.c_str());
        bParseError = true;
        return false;
    }

    // sLine is already trimmed: neither leading nor trailing spaces

    // get rid of trailing spaces from the variable name itself, standing before the '=' char
    // TODO: should rather use std::string compatible PFL::strClr()
    std::string::size_type nSpPos = sLine.find(' ');
    if ( nSpPos != std::string::npos )
    {
        if ( nSpPos < nAssignmentPos )
        {
            sVar = sLine.substr(0, nSpPos);
            if ( sVar.find(' ') != std::string::npos )
            {
                // we should not have more space before '=' char
                CConsole::getConsoleInstance("Maps").EOLn("ERROR: erroneous assignment, failed to parse variable in line: %s!", sLine.c_str());
                bParseError = true;
                return false;
            }
        }
        else
        {
            // should never reach this point based on above 2 conditions
            CConsole::getConsoleInstance("Maps").EOLn("ERROR: erroneous assignment: %s!", sLine.c_str());
            bParseError = true;
            return false;
        }
    }
    else
    {
        sVar = sLine.substr(0, nAssignmentPos);
    }

    // get rid of leading spaces from the value itself, standing after the '=' char
    std::string::size_type i = nAssignmentPos+1;
    while ( (i < sLine.length()) && sLine[i] == ' ' )
    {
        i++;
    }

    if ( i < sLine.length() )
    {
        sValue = sLine.substr(i);
    }
    else
    {
        CConsole::getConsoleInstance("Maps").EOLn("ERROR: erroneous assignment, failed to parse value in line: %s!", sLine.c_str());
        bParseError = true;
        return false;
    }

    return true;
}

bool proofps_dd::Maps::lineHandleDecalAssignment(const std::string& sValue)
{
    std::stringstream sstr(sValue);
    
    std::string sFilename;
    sstr >> sFilename;
    if (sstr.fail() || sstr.bad())
    {
        getConsole().EOLn("%s ERROR: failed to read filename from: %s!", __func__, sValue.c_str());
        return false;
    }

    float px{}, py{};
    sstr >> px;
    sstr >> py;
    float sx{}, sy{};
    sstr >> sx;
    sstr >> sy;
    if (sstr.fail() || sstr.bad())
    {
        getConsole().EOLn("%s ERROR: failed to read pos or size from: %s!", __func__, sValue.c_str());
        return false;
    }
    if ((sx <= 0.f) || (sy <= 0.f))
    {
        getConsole().EOLn("%s ERROR: size values must be positive in: %s!", __func__, sValue.c_str());
        return false;
    }

    const std::string sTexName = proofps_dd::GAME_TEXTURES_DIR + m_sRawName + "\\" + sFilename;
    PureTexture* const tex = m_gfx.getTextureManager().createFromFile(sTexName.c_str());

    PureObject3D* const pDecalObj = m_gfx.getObject3DManager().createPlane(
        sx * proofps_dd::Maps::fMapBlockSizeWidth,
        sy * proofps_dd::Maps::fMapBlockSizeHeight);
    pDecalObj->getPosVec().Set(
        px * proofps_dd::Maps::fMapBlockSizeWidth + proofps_dd::Maps::fMapBlockSizeWidth / 2.f,
        -py * proofps_dd::Maps::fMapBlockSizeHeight + proofps_dd::Maps::fMapBlockSizeHeight / 2.f,
        GAME_DECAL_POS_Z);
    pDecalObj->getMaterial().setTexture(tex);
    pDecalObj->getMaterial(false).setDecalOffset(true);
    //pDecalObj->getMaterial(false).setBlendFuncs(PURE_SRC_ALPHA, PURE_ONE_MINUS_SRC_ALPHA);
    //pDecalObj->getMaterial(false).getTextureEnvColor().SetAlpha(200u);
    m_decals.push_back(pDecalObj);

    return true;
}

bool proofps_dd::Maps::lineHandleAssignment(const std::string& sVar, const std::string& sValue)
{
    assert(sVar.length());  // lineIsValueAssignment() takes care of this

    if ( sVar.length() == 1 )
    {
        // dont store these variables, they just for block texture assignment

        const size_t iSpace = sValue.find(' ');
        if (iSpace == std::string::npos)
        {
            // sValue is already trimmed, so absence of space char means no UV-coords are specified, we use default values
            m_Block2Texture[sVar[0]].m_sTexFilename = sValue;
        }
        else
        {
            // space char indicates presence of UV-coords in this line after tex filename
            m_Block2Texture[sVar[0]].m_sTexFilename = sValue.substr(0, iSpace);

            std::stringstream sstr(sValue.substr(iSpace));
            sstr >> m_Block2Texture[sVar[0]].m_fU0;
            sstr >> m_Block2Texture[sVar[0]].m_fV0;
            sstr >> m_Block2Texture[sVar[0]].m_fU1;
            sstr >> m_Block2Texture[sVar[0]].m_fV1;
            if (sstr.fail() || sstr.bad())
            {
                getConsole().EOLn("%s ERROR: failed to parse UV-coords in variable: %s = %s", __func__, sVar.c_str(), sValue.c_str());
                return false;
            }
        }
        
        getConsole().OLn("%s Block %s has texture %s", __func__, sVar.c_str(), sValue.c_str());
        return true;
    }

    if (sVar == "decal")
    {
        // not to be an actual variable, we call it "decal assignment", treated as anonymous var, just to define decals
        return lineHandleDecalAssignment(sValue);
    }

    // only vars with length > 1 are to be stored as actual variables
    getConsole().OLn("%s Var \"%s\" = \"%s\"", __func__, sVar.c_str(), sValue.c_str());
    m_vars[sVar] = sValue.c_str();

    return true;
} // lineHandleAssignment()

/**
* Creates a stairstep.
* The given texture UV-coordinates are for the front and back faces only, the other faces will have different UV-coordinates so
* that the texture will look properly aligned on all surfaces.
*
* @param bDryRun         Caller must pass its own such variable.
*                        To be on the same page, this function shall never run in dry run!
* @param fStairstepPosX  World-space position X where to place this new stairstep.
* @param fStairstepPosY  World-space position Y where to place this new stairstep.
* @param fStairstepSizeX Horizontal size of this new stairstep.
* @param fStairstepSizeY Vertical size of this new stairstep.
* @param pTexture        Texture to be set.
*                        Expected to be nullptr for ascending stairsteps since the next regular foreground block will decide that.
* @param bDescending     True if we are creating stairstep as part of a descending stairs block, false if for ascending.
* @param fU0             Texture U coordinate of the 2 LEFT-side vertices of the front and back faces of the stairstep box.
* @param fU1             Texture U coordinate of the 2 RIGHT-side vertices of the front and back faces of the stairstep box.
* @param fV0             Texture V coordinate of the 2 BOTTOM vertices of the front and back faces of the stairstep box.
* @param fV1             Texture V coordinate of the 2 TOP vertices of the front and back faces of the stairstep box.
*/
bool proofps_dd::Maps::createSingleSmallStairStep(
    const bool& bDryRun,
    const float& fStairstepPosX,
    const float& fStairstepPosY,
    const float& fStairstepSizeX,
    const float& fStairstepSizeY,
    PureTexture* pTexture,
    const bool& bDescending,
    const float& fU0,
    const float& fV0,
    const float& fU1,
    const float& fV1
)
{
    (void)bDescending;
    assert(!bDryRun);

    // in dry run, these counters are incremented by caller, but in non-dry run here we increment for each stairstep
    m_blocks_h++;
    m_foregroundBlocks_h++;  // same for m_foregroundBlocks_h

    PureObject3D* const pStairstep = m_gfx.getObject3DManager().createBox(
        fStairstepSizeX, fStairstepSizeY, proofps_dd::Maps::fMapBlockSizeWidth,
        PURE_VMOD_DYNAMIC /* based on createBox() API doc, argument bForceUseClientMemory is considered only if modifying habit is dynamic */,
        PURE_VREF_DIRECT,
        true /* force-use client memory because we override UV coords first, and then upload geometry to server memory */);

    // here we have valid texture only for descending stairsteps, but for ascending it is nullptr!
    // For ascending, we set the texture in lineHandleLayout(), after the next normal foreground block is created!!!
    pStairstep->getMaterial().setTexture(pTexture);

    // update UVW
    assert(pStairstep->getCount() == 1);  // box always has exactly 1 subobj
    PureObject3D* const pSubObj = dynamic_cast<PureObject3D*>(pStairstep->getAttachedAt(0));
    if (!pSubObj)
    {
        getConsole().EOLn("%s pSubObj cast failure!", __func__);
        return false;
    }
    
    if (pSubObj->getMaterial().getTexcoordsCount() != 24)
    {
        getConsole().EOLn("%s pSubObj unexpected texcoords count: %u!", __func__, pSubObj->getMaterial().getTexcoordsCount());
        return false;
    }
    
    // order of box faces:
    // front, back, left, right, top, bottom.

    // first set only front and back to exactly same as specified in parameters:
    for (TPureUInt iTexcoord = 0; iTexcoord < 8; iTexcoord += 4)
    {
        // left bottom vertex
        pSubObj->getMaterial().getTexcoords()[iTexcoord].u = fU0;
        pSubObj->getMaterial().getTexcoords()[iTexcoord].v = fV0;
        // right bottom vertex
        pSubObj->getMaterial().getTexcoords()[iTexcoord + 1].u = fU1;
        pSubObj->getMaterial().getTexcoords()[iTexcoord + 1].v = fV0;
        // right top vertex
        pSubObj->getMaterial().getTexcoords()[iTexcoord + 2].u = fU1;
        pSubObj->getMaterial().getTexcoords()[iTexcoord + 2].v = fV1;
        // left top vertex
        pSubObj->getMaterial().getTexcoords()[iTexcoord + 3].u = fU0;
        pSubObj->getMaterial().getTexcoords()[iTexcoord + 3].v = fV1;
    }

    // then left and right faces:
    for (TPureUInt iTexcoord = 8; iTexcoord < 16; iTexcoord += 4)
    {
        // left bottom vertex
        pSubObj->getMaterial().getTexcoords()[iTexcoord].u = 0.f;
        pSubObj->getMaterial().getTexcoords()[iTexcoord].v = fV0;
        // right bottom vertex
        pSubObj->getMaterial().getTexcoords()[iTexcoord + 1].u = 1.f;
        pSubObj->getMaterial().getTexcoords()[iTexcoord + 1].v = fV0;
        // right top vertex
        pSubObj->getMaterial().getTexcoords()[iTexcoord + 2].u = 1.f;
        pSubObj->getMaterial().getTexcoords()[iTexcoord + 2].v = fV1;
        // left top vertex
        pSubObj->getMaterial().getTexcoords()[iTexcoord + 3].u = 0.f;
        pSubObj->getMaterial().getTexcoords()[iTexcoord + 3].v = fV1;
    }

    // then top and bottom faces:
    for (TPureUInt iTexcoord = 16; iTexcoord < 24; iTexcoord += 4)
    {
        // left bottom vertex
        pSubObj->getMaterial().getTexcoords()[iTexcoord].u = fU0;
        pSubObj->getMaterial().getTexcoords()[iTexcoord].v = 0.f;
        // right bottom vertex
        pSubObj->getMaterial().getTexcoords()[iTexcoord + 1].u = fU1;
        pSubObj->getMaterial().getTexcoords()[iTexcoord + 1].v = 0.f;
        // right top vertex
        pSubObj->getMaterial().getTexcoords()[iTexcoord + 2].u = fU1;
        pSubObj->getMaterial().getTexcoords()[iTexcoord + 2].v = 1.f;
        // left top vertex
        pSubObj->getMaterial().getTexcoords()[iTexcoord + 3].u = fU0;
        pSubObj->getMaterial().getTexcoords()[iTexcoord + 3].v = 1.f;
    }

    // finally upload with new UV-coords to server memory
    const TPURE_VERTEX_TRANSFER_MODE vtransmode = PureVertexTransfer::selectVertexTransferMode(
        PURE_VMOD_STATIC,
        PURE_VREF_DIRECT /* in the future we may change this to indexed probably */,
        false /* no force-use client mem */
    );
    if (!pStairstep->setVertexTransferMode(vtransmode))
    {
        // do not terminate, but will render slow
        getConsole().EOLn("%s setVertexTransferMode(%u) failed!", __func__, vtransmode);
    }
    if (!PureVertexTransfer::isVideoMemoryUsed(vtransmode))
    {
        getConsole().EOLn("%s WARNING selectVertexTransferMode(): %u NOT using VRAM!", __func__, vtransmode);
    }
    getConsole().OLn("%s selectVertexTransferMode(): %u", __func__, vtransmode);

    m_blocks[m_blocks_h - 1] = pStairstep;
    m_blocks[m_blocks_h - 1]->SetLit(true);

    pStairstep->getPosVec().Set(fStairstepPosX, fStairstepPosY, -proofps_dd::Maps::fMapBlockSizeDepth);

    m_foregroundBlocks[m_foregroundBlocks_h - 1] = pStairstep;

    if (!m_bvh.insertObject(*pStairstep))
    {
        getConsole().EOLn(
            "%s Failed to insert block into BVH at [x,y,z]: [%f,%f,%f], BVH is at: [%f,%f,%f], BVH size: %f, maxdepth: %u !",
            __func__,
            pStairstep->getPosVec().getX(),
            pStairstep->getPosVec().getY(),
            pStairstep->getPosVec().getZ(),
            m_bvh.getPos().getX(),
            m_bvh.getPos().getY(),
            m_bvh.getPos().getZ(),
            m_bvh.getSize(),
            m_bvh.getMaxDepthLevel());
        return false;
    }

    return true;
} // createSingleSmallStairStep()

/**
* Invoked when a stairs block character is encountered.
* 
* @param bDryRun              Caller must pass their same variable. Need to have same logic based on this variable as caller has.
* @param iLinePos             The current horizontal position index in the current line.
* @param bCopyPreviousFgBlock True if we are handling a descending stairs block i.e. the previous block's texture needs to be copied.
*                             False if we are handling an ascending stairs block.
* @param iObjectFgToBeCopied  Index of the foreground block of which texture needs to be copied.
*                             Valid only if bCopyPreviousFgBlock is true i.e. when handling a descending stairs block.
* @param bCopyPreviousBgBlock True if we can use iObjectBgToBeCopied to make a copy of another background block behind this new stairs block.
*                             False if we don't create such background block behind this new stairs block.
* @param iObjectBgToBeCopied  Index of the background block of we are going to copy.
*                             Valid only if bCopyPreviousBgBlock is true.
* @param fBlockPosX           The horizontal world-position of the stairs block we are handling now.
* @param fBlockPosY           The vertical world-position of the stairs block we are handling now.
*/
bool proofps_dd::Maps::createSmallStairStepsForSingleBigStairsBlock(
    const bool& bDryRun,
    const size_t& iLinePos,
    const size_t& nLineLength,
    const bool& bCopyPreviousFgBlock,
    const int& iObjectFgToBeCopied,
    const bool& bCopyPreviousBgBlock,
    const int& iObjectBgToBeCopied,
    const float& fBlockPosX,
    const float& fBlockPosY)
{
    if ((iLinePos == 0) || (iLinePos >= (nLineLength-1)))
    {
        // no line can start or end with any stairs block, this restriction is due to how m_blocksVertexPosMin and m_blocksVertexPosMax are
        // calculated after loading a map, they are using const width and height values and I'm not changing that for now.
        getConsole().EOLn("%s: A line is starting or ending with a stairs block, which is not permitted!", __func__);
        return false;
    }

    if (bDryRun)
    {
        // since m_blocks_h is incremented in dry run, then reset, and incremented again in non-dry run, we have to keep
        // the same logic as caller does for non-stairs blocks.
        // However, for more clarity, during dry run here we simply increase by the number of stairsteps that are created in non-dry run,
        // and return quickly, but in non-dry run we increase these numbers in the loops below.
        m_blocks_h += nStairstepsCount;
        m_foregroundBlocks_h += nStairstepsCount;  // same for m_foregroundBlocks_h

        if (bCopyPreviousBgBlock)
        {
            // because we will also make 1 extra bg block behind!
            m_blocks_h++;
        }

        // thats all for now
        return true;
    }

    const float fStairstepHeight = proofps_dd::Maps::fMapBlockSizeHeight / static_cast<float>(nStairstepsCount);
    if (bCopyPreviousFgBlock)
    {
        // create descending stair blocks
        if (iObjectFgToBeCopied == -1)
        {
            getConsole().EOLn("%s: bCopyPreviousFgBlock is set but iObjectFgToBeCopied is -1!", __func__);
            return false;
        }

        // since all regular foreground blocks are clones, we need the referred object where texture is actually set!
        assert(m_blocks[iObjectFgToBeCopied]->getReferredObject());

        // building stairsteps from top to bottom, left to right
        bool bRet = true;
        const float fStairsBlockLeftEdge = fBlockPosX - proofps_dd::Maps::fMapBlockSizeWidth / 2.f;
        const float fStairsBlockTopEdge = fBlockPosY + proofps_dd::Maps::fMapBlockSizeHeight / 2.f;
        for (auto i = 0; bRet && (i < nStairstepsCount); i++)
        {
            const float fStairstepWidth = (i + 1) * proofps_dd::Maps::fMapBlockSizeWidth / static_cast<float>(nStairstepsCount);
            bRet &= createSingleSmallStairStep(
                bDryRun,
                fStairsBlockLeftEdge + fStairstepWidth / 2.f,
                fStairsBlockTopEdge - (i * fStairstepHeight) - fStairstepHeight / 2.f,
                fStairstepWidth,
                fStairstepHeight,
                m_blocks[iObjectFgToBeCopied]->getReferredObject()->getMaterial().getTexture(),
                bCopyPreviousFgBlock,
                0.f,
                (nStairstepsCount - i - 1) / static_cast<float>(nStairstepsCount),
                (i + 1) / static_cast<float>(nStairstepsCount),
                (nStairstepsCount - i) / static_cast<float>(nStairstepsCount));
        }
        
        if (!bRet)
        {
            getConsole().EOLn("%s: error during creating small stairstep!", __func__);
            return false;
        }
    }
    else
    {
        // create ascending stair blocks

        // building stairsteps from bottom to top, left to right
        bool bRet = true;
        const float fStairsBlockRightEdge = fBlockPosX + proofps_dd::Maps::fMapBlockSizeWidth / 2.f;
        const float fStairsBlockBottomEdge = fBlockPosY - proofps_dd::Maps::fMapBlockSizeHeight / 2.f;
        for (auto i = 0; bRet && (i < nStairstepsCount); i++)
        {
            const float fStairstepWidth = (nStairstepsCount-i) * proofps_dd::Maps::fMapBlockSizeWidth / static_cast<float>(nStairstepsCount);
            bRet &= createSingleSmallStairStep(
                bDryRun,
                fStairsBlockRightEdge - fStairstepWidth / 2.f,
                fStairsBlockBottomEdge + (i * fStairstepHeight) + fStairstepHeight / 2.f,
                fStairstepWidth,
                fStairstepHeight,
                nullptr,
                bCopyPreviousFgBlock,
                (i) / static_cast<float>(nStairstepsCount),
                (i) / static_cast<float>(nStairstepsCount),
                1.f,
                (i+1) / static_cast<float>(nStairstepsCount));
        }

        if (!bRet)
        {
            getConsole().EOLn("%s: error during creating small stairstep!", __func__);
            return false;
        }
    }

    if (bCopyPreviousBgBlock)
    {
        if (iObjectBgToBeCopied == -1)
        {
            getConsole().EOLn("%s: bCopyPreviousBgBlock is set but iObjectBgToBeCopied is -1!", __func__);
            return false;
        }
        PureObject3D* const pNewBgBlockObj = m_gfx.getObject3DManager().createCloned(*(m_blocks[iObjectBgToBeCopied]->getReferredObject()));
        pNewBgBlockObj->getPosVec().Set(fBlockPosX, fBlockPosY, 0.0f);
        m_blocks_h++;
        m_blocks[m_blocks_h - 1] = pNewBgBlockObj;
        m_blocks[m_blocks_h - 1]->SetLit(true);
    }

    return true;
} // createSmallStairStepsForSingleBigStairsBlock()

/**
 * This function to be invoked for every single line of the map layout definition.
 * Map layout definition is the last part of a map file, containing the blocks building up the map (walls, floor, etc.).
 * 
 * @param sLine   The current line of the map layout definition we want to process.
 * @param y       The current height we are currently placing newly created blocks for this line of the map definition layout.
 * @param bDryRun If true, blocks are not allocated thus the map is not actually created, however the following
 *                variables are actually updated: m_width, m_height, m_blocks_h, m_foregroundBlocks_h, m_items, y.
 *                If we finish processing all lines with bDryRun as true, we will know the actual size of the map and number of
 *                blocks, thus in the next non-dry run we will have to allocate memory only once for the blocks.
 *                And yes, dry run actually creates all the items in m_items.
 */
bool proofps_dd::Maps::lineHandleLayout(const std::string& sLine, TPureFloat& y, bool bDryRun)
{
    if (bDryRun)
    {
        m_height++;
        if (m_width < sLine.length())
        {
            m_width = sLine.length();
        }
    }
    else
    {
        if (m_blocks == nullptr)
        {
            // first line in non-dry run

            // Octree root node size needs to be set before inserting any objects into it, also reposition it so
            // the whole map spatially fits inside the root node!
            if (!m_bvh.setPos(
                PureVector(
                    m_width * proofps_dd::Maps::fMapBlockSizeWidth / 2.f,
                    m_height * proofps_dd::Maps::fMapBlockSizeHeight / -2.f /* minus because vertically elements start from 0 and going down towards negative Y */,
                    0.f)))
            {
                getConsole().EOLn("%s Failed to set BVH pos!", __func__);
                assert(false);
                return false;
            }

            const float fBvhSize = std::max(
                m_width * proofps_dd::Maps::fMapBlockSizeWidth,
                m_height * proofps_dd::Maps::fMapBlockSizeHeight);
            if (!m_bvh.setSize(fBvhSize))
            {
                getConsole().EOLn("%s Failed to set BVH size: %f!", __func__, fBvhSize);
                assert(false);
                return false;
            }

            // now we can allocate memory for all blocks
            // TODO: handle memory allocation errors
            m_blocks = (PureObject3D**)malloc(m_blocks_h * sizeof(PureObject3D*));
            m_blocks_h = 0; // we are incrementing it again during non-dry run
            m_foregroundBlocks = (PureObject3D**)malloc(m_foregroundBlocks_h * sizeof(PureObject3D*));
            m_foregroundBlocks_h = 0; // we are incrementing it again during non-dry run
        }
    }

    TPureFloat x = 0.0f;
    TPureFloat maxx = x;
    int iLinePos = -1;
    
    // Item character specifies the item type, but not the background behind the item.
    // So the idea is to copy the previous _neighbor_ background block to be used behind the item, but
    // only if there is a neighbor block created previously, otherwise we should not put any
    // background block behind the item.
    // So iObjectBgToBeCopied is > -1 only if there is neighbor background block created previously.
    int iObjectBgToBeCopied = -1;

    // The idea with special foreground block copying the previous neighbor foreground block is similar as
    // described above with special background blocks.
    int iObjectFgToBeCopied = -1;

    while ( iLinePos != static_cast<int>(sLine.length()) )
    {
        ++iLinePos;
        assert(iLinePos >= 0);
        const char c = sLine[iLinePos];
        const bool bForeground = foregroundBlocks.find(c) != foregroundBlocks.end();
        const bool bBackground = backgroundBlocks.find(c) != backgroundBlocks.end();

        x = x + proofps_dd::Maps::fMapBlockSizeWidth;
        if ( x > maxx )
        {
            maxx = x;
        }
        
        if ( !bForeground && !bBackground )
        {
            iObjectBgToBeCopied = -1;
            iObjectFgToBeCopied = -1;
            continue;
        }

        if (bForeground && bBackground)
        {
            const std::string sc(1, c); // WA for CConsole lack support of %c
            getConsole().EOLn("%s Block defined as both foreground and background: %s!", __func__, sc.c_str());
            assert(false);
            return false;
        }

        // special background block handling
        bool bCopyPreviousBgBlock = false;
        bool bSpecialBgBlock = false;
        switch (c)
        {
        case ',':
        {
            if (bDryRun)
            {
                proofps_dd::MapItem* pMapItem = new proofps_dd::MapItem(m_gfx, MapItemType::ITEM_ARMOR, PureVector(x, y, GAME_ITEMS_POS_Z));
                m_items.insert({ pMapItem->getId(), pMapItem });
            }
            bSpecialBgBlock = true;
            bCopyPreviousBgBlock = iObjectBgToBeCopied > -1;
            break;
        }
        case '+':
        {
            if (bDryRun)
            {
                proofps_dd::MapItem* pMapItem = new proofps_dd::MapItem(m_gfx, MapItemType::ITEM_HEALTH, PureVector(x, y, GAME_ITEMS_POS_Z));
                m_items.insert({ pMapItem->getId(), pMapItem });
            }
            bSpecialBgBlock = true;
            bCopyPreviousBgBlock = iObjectBgToBeCopied > -1;
            break;
        }
        case '2':
        {
            if (bDryRun)
            {
                proofps_dd::MapItem* pMapItem = new proofps_dd::MapItem(m_gfx, MapItemType::ITEM_WPN_PISTOL, PureVector(x, y, GAME_ITEMS_POS_Z));
                m_items.insert({ pMapItem->getId(), pMapItem });
            }
            bSpecialBgBlock = true;
            bCopyPreviousBgBlock = iObjectBgToBeCopied > -1;
            break;
        }
        case '3':
        {
            if (bDryRun)
            {
                proofps_dd::MapItem* pMapItem = new proofps_dd::MapItem(m_gfx, MapItemType::ITEM_WPN_MACHINEGUN, PureVector(x, y, GAME_ITEMS_POS_Z));
                m_items.insert({ pMapItem->getId(), pMapItem });
            }
            bSpecialBgBlock = true;
            bCopyPreviousBgBlock = iObjectBgToBeCopied > -1;
            break;
        }
        case '4':
        {
            if (bDryRun)
            {
                proofps_dd::MapItem* pMapItem = new proofps_dd::MapItem(m_gfx, MapItemType::ITEM_WPN_BAZOOKA, PureVector(x, y, GAME_ITEMS_POS_Z));
                m_items.insert({ pMapItem->getId(), pMapItem });
            }
            bSpecialBgBlock = true;
            bCopyPreviousBgBlock = iObjectBgToBeCopied > -1;
            break;
        }
        case '5':
        {
            if (bDryRun)
            {
                proofps_dd::MapItem* pMapItem = new proofps_dd::MapItem(m_gfx, MapItemType::ITEM_WPN_PUSHA, PureVector(x, y, GAME_ITEMS_POS_Z));
                m_items.insert({ pMapItem->getId(), pMapItem });
            }
            bSpecialBgBlock = true;
            bCopyPreviousBgBlock = iObjectBgToBeCopied > -1;
            break;
        }
        case '6':
        {
            if (bDryRun)
            {
                proofps_dd::MapItem* pMapItem = new proofps_dd::MapItem(m_gfx, MapItemType::ITEM_WPN_MACHINEPISTOL, PureVector(x, y, GAME_ITEMS_POS_Z));
                m_items.insert({ pMapItem->getId(), pMapItem });
            }
            bSpecialBgBlock = true;
            bCopyPreviousBgBlock = iObjectBgToBeCopied > -1;
            break;
        }
        case '7':
        {
            if (bDryRun)
            {
                proofps_dd::MapItem* pMapItem = new proofps_dd::MapItem(m_gfx, MapItemType::ITEM_WPN_SHOTGUN, PureVector(x, y, GAME_ITEMS_POS_Z));
                m_items.insert({ pMapItem->getId(), pMapItem });
            }
            bSpecialBgBlock = true;
            bCopyPreviousBgBlock = iObjectBgToBeCopied > -1;
            break;
        }
        case '8':
        {
            if (bDryRun)
            {
                proofps_dd::MapItem* pMapItem = new proofps_dd::MapItem(m_gfx, MapItemType::ITEM_WPN_GRENADELAUNCHER, PureVector(x, y, GAME_ITEMS_POS_Z));
                m_items.insert({ pMapItem->getId(), pMapItem });
            }
            bSpecialBgBlock = true;
            bCopyPreviousBgBlock = iObjectBgToBeCopied > -1;
            break;
        }
        case 'S':
        {
            if (!bDryRun)
            {
                // spawnpoint is background block by default
                const PureVector vecSpawnPointPos(x, y, GAME_PLAYERS_POS_Z);
                if (m_spawnpoints.empty())
                {
                    m_spawnpointLeftMost = vecSpawnPointPos;
                    m_spawnpointRightMost = vecSpawnPointPos;
                }
                else
                {
                    if (x < m_spawnpointLeftMost.getX())
                    {
                        m_spawnpointLeftMost = vecSpawnPointPos;
                    }
                    if (x > m_spawnpointRightMost.getX())
                    {
                        m_spawnpointRightMost = vecSpawnPointPos;
                    }
                }
                m_spawnpoints.push_back(vecSpawnPointPos);
            }
            bSpecialBgBlock = true;
            bCopyPreviousBgBlock = iObjectBgToBeCopied > -1;
            break;
        }
        default: /* NOP */;
            break;
        }

        // special foreground block handling
        bool bCopyPreviousFgBlock = false;
        bool bSpecialFgBlock = false;
        bool bJumppad = false;
        bool bStairs = false;
        switch (c)
        {
        case '^':
        {
            if (bDryRun)
            {
                PureObject3D* const pDecorObj = m_gfx.getObject3DManager().createPlane(1.f, 1.2f);
                pDecorObj->getPosVec().Set(
                    x,
                    y + proofps_dd::Maps::fMapBlockSizeHeight + pDecorObj->getSizeVec().getY() / 2.f,
                    GAME_DECOR_POS_Z);
                pDecorObj->getMaterial().setTexture(m_texDecorJumpPadVertical);
                pDecorObj->getMaterial(false).setBlendFuncs(PURE_SRC_ALPHA, PURE_ONE_MINUS_SRC_ALPHA);
                pDecorObj->getMaterial(false).getTextureEnvColor().SetAlpha(200u);
                m_decorations.push_back(pDecorObj);
            }
            bJumppad = true;
            bSpecialFgBlock = true;
            bCopyPreviousFgBlock = iObjectFgToBeCopied > -1;
            break;
        }
        case '/':
            bStairs = true;
            bCopyPreviousBgBlock = iObjectBgToBeCopied > -1; // otherwise there will be "hole" behind the stairs block!
            // in case of ascending stairs, next neighbor regular fg object's texture shall be applied when handling that fg object
            break;
        case '\\':
            bStairs = true;
            bCopyPreviousBgBlock = iObjectBgToBeCopied > -1; // otherwise there will be "hole" behind the stairs block!
            // in case of descending stairs, previous regular fg object's texture shall be applied
            bCopyPreviousFgBlock = iObjectFgToBeCopied > -1;
            break;
        default:
            break;
        }

        if (bSpecialFgBlock && bSpecialBgBlock)
        {
            const std::string sc(1, c); // WA for CConsole lack support of %c
            getConsole().EOLn("%s Block defined both as special foreground and special background: %s!", __func__, sc.c_str());
            assert(false);
            return false;
        }

        PureObject3D* pNewBlockObj = nullptr;
        if (bStairs)
        {
            // in case of a single "stairs" block, multiple smaller-sized blocks are created.
            // I'm handling it in this separated code because currently we are not utilizing object cloning for stairs, 
            if (!createSmallStairStepsForSingleBigStairsBlock(
                    bDryRun, iLinePos, sLine.length(), bCopyPreviousFgBlock, iObjectFgToBeCopied, bCopyPreviousBgBlock, iObjectBgToBeCopied, x, y))
            {
                getConsole().EOLn("%s: Stairs handling problem in line: %s!", __func__, sLine.c_str());
                return false;
            }
        }
        else if (!bSpecialBgBlock || (bSpecialBgBlock && bCopyPreviousBgBlock))
        {
            m_blocks_h++;
            if (!bSpecialBgBlock && bBackground)
            {
                iObjectBgToBeCopied = m_blocks_h - 1;
            } else if (!bSpecialFgBlock && bForeground && !bStairs)
            {
                iObjectFgToBeCopied = m_blocks_h - 1;
            }
            if (!bDryRun)
            {
                if (bSpecialBgBlock && bCopyPreviousBgBlock)
                {
                    pNewBlockObj = m_gfx.getObject3DManager().createCloned(*(m_blocks[iObjectBgToBeCopied]->getReferredObject()));
                }
                else if (bSpecialFgBlock && bCopyPreviousFgBlock)
                {
                    pNewBlockObj = m_gfx.getObject3DManager().createCloned(*(m_blocks[iObjectFgToBeCopied]->getReferredObject()));
                }
                else
                {
                    const auto it = m_mapReferenceBlockObject3Ds.find(c);
                    if (it == m_mapReferenceBlockObject3Ds.end())
                    {
                        m_mapReferenceBlockObject3Ds[c] = m_gfx.getObject3DManager().createBox(
                            proofps_dd::Maps::fMapBlockSizeWidth, proofps_dd::Maps::fMapBlockSizeWidth, proofps_dd::Maps::fMapBlockSizeWidth,
                            PURE_VMOD_DYNAMIC /* based on createBox() API doc, argument bForceUseClientMemory is considered only if modifying habit is dynamic */,
                            PURE_VREF_DIRECT,
                            true /* force-use client memory because we override UV coords first, and then upload geometry to server memory */);
                        if (!m_mapReferenceBlockObject3Ds[c])
                        {
                            getConsole().EOLn("%s createBox() failed!", __func__);
                            return false;
                        }

                        m_mapReferenceBlockObject3Ds[c]->Hide();
                        PureTexture* tex = PGENULL;
                        if (m_Block2Texture.find(c) == m_Block2Texture.end())
                        {
                            const std::string sc(1, c); // WA for CConsole lack support of %c
                            getConsole().EOLn("%s No texture defined for block %s!", __func__, sc.c_str());
                            tex = m_texRed;
                        }
                        else
                        {
                            const std::string sTexName = proofps_dd::GAME_TEXTURES_DIR + m_sRawName + "\\" + m_Block2Texture[c].m_sTexFilename;
                            tex = m_gfx.getTextureManager().createFromFile(sTexName.c_str());
                            if (tex)
                            {
                                //tex->setTextureWrappingMode(
                                //    TPURE_TEX_WRAPPING::PURE_TW_CLAMP_TO_EDGE, TPURE_TEX_WRAPPING::PURE_TW_CLAMP_TO_EDGE);

                                assert(m_mapReferenceBlockObject3Ds[c]->getCount() == 1);  // box always has exactly 1 subobj
                                PureObject3D* const pSubObj = dynamic_cast<PureObject3D*>(m_mapReferenceBlockObject3Ds[c]->getAttachedAt(0));
                                if (!pSubObj)
                                {
                                    getConsole().EOLn("%s pSubObj cast failure!", __func__);
                                    return false;
                                }

                                if (pSubObj->getMaterial().getTexcoordsCount() != 24)
                                {
                                    getConsole().EOLn("%s pSubObj unexpected texcoords count: %u!", __func__, pSubObj->getMaterial().getTexcoordsCount());
                                    return false;
                                }

                                // overriding UV-coords does not take much time since we do this only for unique boxes, 90+% will be a clone anyway
                                for (TPureUInt iTexcoord = 0; iTexcoord < pSubObj->getMaterial().getTexcoordsCount(); iTexcoord += 4)
                                {
                                    // left bottom vertex
                                    pSubObj->getMaterial().getTexcoords()[iTexcoord].u = m_Block2Texture[c].m_fU0;
                                    pSubObj->getMaterial().getTexcoords()[iTexcoord].v = m_Block2Texture[c].m_fV0;
                                    // right bottom vertex
                                    pSubObj->getMaterial().getTexcoords()[iTexcoord + 1].u = m_Block2Texture[c].m_fU1;
                                    pSubObj->getMaterial().getTexcoords()[iTexcoord + 1].v = m_Block2Texture[c].m_fV0;
                                    // right top vertex
                                    pSubObj->getMaterial().getTexcoords()[iTexcoord + 2].u = m_Block2Texture[c].m_fU1;
                                    pSubObj->getMaterial().getTexcoords()[iTexcoord + 2].v = m_Block2Texture[c].m_fV1;
                                    // left top vertex
                                    pSubObj->getMaterial().getTexcoords()[iTexcoord + 3].u = m_Block2Texture[c].m_fU0;
                                    pSubObj->getMaterial().getTexcoords()[iTexcoord + 3].v = m_Block2Texture[c].m_fV1;
                                }
                            }
                            else
                            {
                                getConsole().EOLn("%s Could not load texture %s!", __func__, sTexName.c_str());
                                tex = m_texRed;
                            }
                        }
                        if (!tex)
                        {
                            // should happen only if default red texture could not be loaded, but that should had been detected in initialize() tho
                            const std::string sc(1, c); // WA for CConsole lack support of %c
                            getConsole().EOLn("%s Not assigning any texture for block %s!", __func__, sc.c_str());
                        }
                        m_mapReferenceBlockObject3Ds[c]->getMaterial().setTexture(tex);

                        // finally upload with new UV-coords to server memory
                        const TPURE_VERTEX_TRANSFER_MODE vtransmode = PureVertexTransfer::selectVertexTransferMode(
                            PURE_VMOD_STATIC,
                            PURE_VREF_DIRECT /* in the future we may change this to indexed probably */,
                            false /* no force-use client mem */
                        );
                        if (!m_mapReferenceBlockObject3Ds[c]->setVertexTransferMode(vtransmode))
                        {
                            // do not terminate, but will render slow
                            getConsole().EOLn("%s setVertexTransferMode(%u) failed for a block!", __func__, vtransmode);
                        }
                        if (!PureVertexTransfer::isVideoMemoryUsed(vtransmode))
                        {
                            getConsole().EOLn("%s WARNING selectVertexTransferMode(): %u NOT using VRAM!", __func__, vtransmode);
                        }
                        getConsole().OLn("%s selectVertexTransferMode(): %u", __func__, vtransmode);
                    }
                    pNewBlockObj = m_gfx.getObject3DManager().createCloned(*(m_mapReferenceBlockObject3Ds[c]));
                }
                // dont need to show, can stay hidden, since main game loop invokes UpdateVisibilitiesForRenderer() anyway which shows what is needed
                //pNewBlockObj->Show();
                m_blocks[m_blocks_h - 1] = pNewBlockObj;
                m_blocks[m_blocks_h - 1]->SetLit(true);
            } // endif !dryRun

            if (bForeground)
            {
                m_foregroundBlocks_h++;
                if (!bDryRun)
                {
                    m_foregroundBlocks[m_foregroundBlocks_h - 1] = pNewBlockObj;

                    if (bJumppad)
                    {
                        m_jumppads.push_back(pNewBlockObj);
                    }
                }
            }
        } // endif !bStairs && ...

        if (!pNewBlockObj)
        {
            // it is null for stairs, therefore createSmallStairStepsForSingleBigStairsBlock() shall set stair blocks position and insert into BVH!
            continue;
        }

        pNewBlockObj->getPosVec().Set(x, y, bBackground ? 0.0f : -proofps_dd::Maps::fMapBlockSizeDepth);

        if (bForeground)
        {
            // only here we can insert into BVH because block position has just been set
            if (!m_bvh.insertObject(*pNewBlockObj))
            {
                getConsole().EOLn(
                    "%s Failed to insert block into BVH at [x,y,z]: [%f,%f,%f], BVH is at: [%f,%f,%f], BVH size: %f, maxdepth: %u !",
                    __func__,
                    pNewBlockObj->getPosVec().getX(),
                    pNewBlockObj->getPosVec().getY(),
                    pNewBlockObj->getPosVec().getZ(),
                    m_bvh.getPos().getX(),
                    m_bvh.getPos().getY(), 
                    m_bvh.getPos().getZ(),
                    m_bvh.getSize(),
                    m_bvh.getMaxDepthLevel());
                return false;
            }

            if ((iLinePos > 0) && (sLine[iLinePos-1] == '/'))
            {
                // Only now we can set the texture for the previous ascending stairsteps,
                // as createSmallStairStepsForSingleBigStairsBlock() sets proper texture only for descending stairsteps,
                // even tho createSmallStairStepsForSingleBigStairsBlock() creates all kind of stairsteps.
                assert(m_blocks_h > nStairstepsCount);
                assert(m_foregroundBlocks_h > nStairstepsCount);

                // since all regular foreground blocks are clones, we need the referred object where texture is actually set!
                assert(pNewBlockObj->getReferredObject());
                PureTexture* const pTexFgBlock = pNewBlockObj->getReferredObject()->getMaterial().getTexture();
                for (auto iStairstep = 0; iStairstep < nStairstepsCount; iStairstep++)
                {
                    // we are indexing in m_foregroundBlocks instead of m_blocks, because in m_blocks there MIGHT BE also a
                    // background block that was created behind the stairs block. In m_foregroundBlocks we can be sure that only
                    // the stairsteps are right before pNewBlockObj.
                    const size_t iStairstepInBlocksArray = m_foregroundBlocks_h - 1 - nStairstepsCount + iStairstep;
                    m_foregroundBlocks[iStairstepInBlocksArray]->getMaterial().setTexture(pTexFgBlock);
                    // as of 2025 March, after setTexture() we dont need to reupload geometry to VRAM because PURE does not
                    // put any texture binding call into display lists or anywhere else during setVertexTransferMode(), therefore
                    // setting texture can be always done.
                }
            }
        }
    }  // while
    y = y - proofps_dd::Maps::fMapBlockSizeHeight;
    return true;
}  // lineHandleLayout()

bool proofps_dd::Maps::parseTeamSpawnpointsFromString(
    const std::string& sVarValue, std::set<size_t>& targetSet)
{
    try
    {
        std::stringstream sstr(sVarValue);
        while (!sstr.eof() && !sstr.fail())
        {
            int iSp;
            sstr >> iSp;
            if ((iSp < 0) || (static_cast<size_t>(iSp) >= m_spawnpoints.size()))
            {
                getConsole().EOLn("PRooFPSddPGE::%s(): index error: spawngroup definition contains invalid spawn point index: %d", __func__, iSp);
                return false;
            }
            if (targetSet.find(iSp) != targetSet.end())
            {
                getConsole().EOLn("PRooFPSddPGE::%s(): index error: spawngroup definition contains the same spawn point index multiple times: %d", __func__, iSp);
                return false;
            }

            targetSet.insert(iSp);
        }
    }
    catch (const std::exception& e)
    {
        getConsole().EOLn(
            "PRooFPSddPGE::%s(): index error: spawngroup definition contains something bad, exception: %s, definition: %s",
            __func__,
            e.what(),
            sVarValue.c_str());
        return false;
    }

    return true;
}

bool proofps_dd::Maps::parseTeamSpawnpoints()
{
    assert(!m_spawnpoints.empty());  // to be called from checkAndUpdateSpawnpoints()

    const auto itVarSp1 = m_vars.find("spawngroup_1");
    if ((itVarSp1 == m_vars.end()) || itVarSp1->second.getAsString().empty())
    {
        getConsole().OLn("PRooFPSddPGE::%s(): spawngroup_1 not defined or empty, not considering any spawn groups!", __func__);
        return true;
    }

    if (!parseTeamSpawnpointsFromString(itVarSp1->second.getAsString(), m_spawngroup_1))
    {
        return false;
    }

    if (m_spawngroup_1.size() == m_spawnpoints.size())
    {
        getConsole().OLn("PRooFPSddPGE::%s(): spawngroup_1 contains ALL spawn point indices, which is non-sense!", __func__);
        return false;
    }

    const auto itVarSp2 = m_vars.find("spawngroup_2");
    if ((itVarSp2 == m_vars.end()) || itVarSp2->second.getAsString().empty())
    {
        // need to fill this group automatically with the rest of spawn points
        for (size_t iSp = 0; iSp < m_spawnpoints.size(); iSp++)
        {
            if (m_spawngroup_1.find(iSp) == m_spawngroup_1.end())
            {
                m_spawngroup_2.insert(iSp);
            }
        }
    }
    else
    {
        if (!parseTeamSpawnpointsFromString(itVarSp2->second.getAsString(), m_spawngroup_2))
        {
            return false;
        }

        // since both spawn groups are defined explicitly by map designer, need to do some further checks below

        int nUnassignedSpCounter = 0;
        std::string sUnassignedLog;
        for (size_t iSp = 0; iSp < m_spawnpoints.size(); iSp++)
        {
            const auto itSpGroup_1 = m_spawngroup_1.find(iSp);
            const auto itSpGroup_2 = m_spawngroup_2.find(iSp);
            
            // same spawn point being in both groups is definitely map error!
            if ((itSpGroup_1 != m_spawngroup_1.end()) &&
                (itSpGroup_2 != m_spawngroup_2.end()))
            {
                getConsole().EOLn("PRooFPSddPGE::%s(): ERROR: spawn point index %d is present in both spawn groups!", __func__, iSp);
                return false;
            }
            // check if there is any spawn point remained unassigned, if so make a warning log only (non-critical, probably intentional)!
            else if ((itSpGroup_1 == m_spawngroup_1.end()) &&
                (itSpGroup_2 == m_spawngroup_2.end()))
            {
                ++nUnassignedSpCounter;
                sUnassignedLog += std::to_string(iSp) + " ";
            }
        }

        if (nUnassignedSpCounter != 0)
        {
            getConsole().EOLn("PRooFPSddPGE::%s(): WARNING: %d unassigned spawn point(s): %s!", __func__, nUnassignedSpCounter, sUnassignedLog.c_str());
        }
    }

    getConsole().OLn("PRooFPSddPGE::%s(): spawngroup_1 has %u, spawngroup_2 has %u spawn points.", __func__, m_spawngroup_1.size(), m_spawngroup_2.size());

    return true;
} // parseTeamSpawnpoints()

bool proofps_dd::Maps::checkAndUpdateSpawnpoints()
{
    if (m_spawnpoints.empty())
    {
        getConsole().EOLn("%s ERROR: no spawn points found in the map!", __func__);
        return false;
    }

    return parseTeamSpawnpoints();
}
