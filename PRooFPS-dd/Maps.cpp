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
        bInitialized = m_mapcycle.initialize();
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

    getConsole().SOLnOO("> Map loaded with width %u and height %u!", m_width, m_height);
    return true;
}

void proofps_dd::Maps::unload()
{
    getConsole().OLnOI("Maps::unload() ...");
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
const std::set<PureVector>& proofps_dd::Maps::getSpawnpoints() const
{
    return m_spawnpoints;
}


/**
    Retrieves a randomly selected spawnpoint from the set of spawnpoints of the currently loaded map.
    A spawnpoint is a 3D coordinate where the player can spawn at.

    @return A randomly selected spawnpoint on the current map.
*/
const PureVector& proofps_dd::Maps::getRandomSpawnpoint() const
{
    if ( m_spawnpoints.empty() )
    {
        throw std::runtime_error("No spawnpoints!");
    }
    const int iElem = PFL::random(0, m_spawnpoints.size()-1);
    //getConsole().EOLn("Maps::%s(): %d, count: %u", __func__, iElem, m_spawnpoints.size());
    auto it = m_spawnpoints.begin();
    std::advance(it, iElem);

    return *it;
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

void proofps_dd::Maps::update(const float& fps)
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
}

bool proofps_dd::Maps::handleMapItemUpdateFromServer(
    pge_network::PgeNetworkConnectionHandle /*connHandleServerSide*/,
    const proofps_dd::MsgMapItemUpdateFromServer& msg)
{
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
}

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
            // first line in non-dry run, now we can allocate memory for all blocks
            // TODO: handle memory allocation errors
            m_blocks = (PureObject3D**)malloc(m_blocks_h * sizeof(PureObject3D*));
            m_blocks_h = 0; // we are incrementing it again during non-dry run
            m_foregroundBlocks = (PureObject3D**)malloc(m_foregroundBlocks_h * sizeof(PureObject3D*));
            m_foregroundBlocks_h = 0; // we are incrementing it again during non-dry run
        }
    }

    TPureFloat x = 0.0f;
    TPureFloat maxx = x;
    std::string::size_type iLinePos = 0;
    
    // Item character specifies the item type, but not the background behind the item.
    // So the idea is to copy the previous _neighbor_ background block to be used behind the item, but
    // only if there is a neighbor block created previously, otherwise we should not put any
    // background block behind the item.
    // So iObjectBgToBeCopied is > -1 only if there is neighbor background block created previously.
    int iObjectBgToBeCopied = -1;

    // The idea with special foreground block copying the previous neighbor foreground block is similar as
    // described above with special background blocks.
    int iObjectFgToBeCopied = -1;

    while ( iLinePos != sLine.length() )
    {
        const char c = sLine[iLinePos];
        iLinePos++;
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
                m_spawnpoints.insert(vecSpawnPointPos);
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
        if (!bSpecialBgBlock || (bSpecialBgBlock && bCopyPreviousBgBlock))
        {
            m_blocks_h++;
            if (!bSpecialBgBlock && bBackground)
            {
                iObjectBgToBeCopied = m_blocks_h - 1;
            } else if (!bSpecialFgBlock && bForeground)
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
                        getConsole().EOLn("%s selectVertexTransferMode(): %u", __func__, vtransmode);
                    }
                    pNewBlockObj = m_gfx.getObject3DManager().createCloned(*(m_mapReferenceBlockObject3Ds[c]));
                }
                // dont need to show, can stay hidden, since main game loop invokes UpdateVisibilitiesForRenderer() anyway which shows what is needed
                //pNewBlockObj->Show();
                m_blocks[m_blocks_h - 1] = pNewBlockObj;
                m_blocks[m_blocks_h - 1]->SetLit(true);
            }

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
        }

        if (!pNewBlockObj)
        {
            continue;
        }

        pNewBlockObj->getPosVec().Set(x, y, bBackground ? 0.0f : -proofps_dd::Maps::fMapBlockSizeDepth);
    }  // while
    y = y - proofps_dd::Maps::fMapBlockSizeHeight;
    return true;
}  // lineHandleLayout()
