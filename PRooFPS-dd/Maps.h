#pragma once

/*
    ###################################################################################
    Maps.h
    Map loader for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2022
    ###################################################################################
*/

#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "CConsole.h"
#include "PGE.h" // we use audio also from here so it is easier to just include everything

#include "Mapcycle.h"
#include "MapItem.h"
#include "PRooFPS-dd-packet.h"

namespace proofps_dd
{
    class Maps
    {
    public:

        static constexpr char* szCVarSvMap = "sv_map";

        static constexpr float fMapBlockSizeWidth = 1.0f;
        static constexpr float fMapBlockSizeHeight = 1.0f;
        static constexpr float fMapBlockSizeDepth = 1.0f;

        static const char* getLoggerModuleName();

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const;

        Maps(
            pge_audio::PgeAudio& audio,
            PGEcfgProfiles& cfgProfiles,
            PR00FsUltimateRenderingEngine& gfx);
        ~Maps();

        Maps(const Maps&) = delete;
        Maps& operator=(const Maps&) = delete;
        Maps(Maps&&) = delete;
        Maps&& operator=(Maps&&) = delete;

        bool initialize();                                   /**< Initializes the map handler. */
        bool isInitialized() const;
        void shutdown();                                     /**< Shuts down the map handler. */
        
        const std::string& serverDecideFirstMapAndUpdateNextMapToBeLoaded();
        const std::string& getNextMapToBeLoaded() const;

        /* Current map handling */

        bool loaded() const;
        bool load(
            const char* fname,
            std::function<void(int)>& cbDisplayProgressUpdate);
        void unload();
        unsigned int width() const;
        unsigned int height() const;
        void updateVisibilitiesForRenderer();
        const std::string& getFilename() const;              /**< Retrieves the currently loaded map filename. */
        const std::set<PureVector>& getSpawnpoints() const;  /**< Retrieves the set of spawnpoints of the currently loaded map. */
        const PureVector& getRandomSpawnpoint() const;       /**< Retrieves a randomly selected spawnpoint from the set of spawnpoints of the currently loaded map. */
        const PureVector& getLeftMostSpawnpoint() const;
        const PureVector& getRightMostSpawnpoint() const;
        const PureVector& getBlockPosMin() const;
        const PureVector& getBlockPosMax() const;
        const PureVector& getBlocksVertexPosMin() const;
        const PureVector& getBlocksVertexPosMax() const;
        PureObject3D** getBlocks(); // TODO: not nice access
        PureObject3D** getForegroundBlocks(); // TODO: not nice access
        int getBlockCount() const;
        int getForegroundBlockCount() const;
        const std::map<MapItem::MapItemId, MapItem*>& getItems() const;
        const std::vector<PureObject3D*>& getJumppads() const;
        const std::map<std::string, PGEcfgVariable>& getVars() const;
        size_t getJumppadValidVarsCount();
        const TPURE_XY& getJumppadForceFactors(const size_t& index) const;
        void update(const float& fps);

        void handlePlayerLanded(const float& fFallHeight, bool bDamageTaken, bool bDied);
        void handleJumppadTriggered(const size_t& index);

        bool handleMapItemUpdateFromServer(
            pge_network::PgeNetworkConnectionHandle /*connHandleServerSide*/,
            const MsgMapItemUpdateFromServer& msg);

        /* Mapcycle and Available maps handling */

        Mapcycle& getMapcycle();

    protected:

    private:

        static constexpr float GAME_PLAYERS_POS_Z = -1.2f;
        static constexpr float GAME_ITEMS_POS_Z = GAME_PLAYERS_POS_Z + 0.1f;  // avoid Z-fighting with items the player cannot take
        static constexpr float GAME_DECOR_POS_Z = fMapBlockSizeDepth / -2.f - 0.1f;  // decors are close to the wall surfaces

        const std::set<char> foregroundBlocks = {
            'B', 'D', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'Q', 'T',
            /* the special foreground stuff (e.g. jump pads) are treated as foreground blocks, see special handling in lineHandleLayout(): */
            '^' /* jump pad vertical */,
            '<' /* jump pad up-left */,
            '>' /* jump pad up-right */,
            '\\' /* stairs descending to the right */,
            '/'  /* stairs ascending to the right */
        };

        const std::set<char> backgroundBlocks = {
            'a', 'c', 'e', 'm', 'n', 'p', 'o', 'r', 'u', 'v', 'w', 'x', 'y', 'z',
            /* the special foreground stuff (e.g. items) are treated as background blocks too, see special handling in lineHandleLayout(): */
            '+' /* medkit */,
            '2' /* weapon key 2 */,
            '3' /* weapon key 3 */,
            '4' /* weapon key 4 */,
            'S' /* spawnpoint */
        };

        pge_audio::PgeAudio& m_audio;
        PGEcfgProfiles& m_cfgProfiles;
        PR00FsUltimateRenderingEngine& m_gfx;
        PureTexture* m_texRed;  // TODO: unique_ptr
        PureTexture* m_texDecorJumpPadVertical;  // TODO: unique_ptr
        SoLoud::Wav m_sndPlayerLandSmallFall;
        SoLoud::Wav m_sndPlayerLandBigFall;
        SoLoud::Wav m_sndPlayerLandDamage;
        SoLoud::Wav m_sndJumppad;
        std::string m_sServerMapFilenameToLoad;                      /**< We set this as soon as we get to know which map we should load. */

        /* Current map handling */

        std::map<char, PureObject3D*> m_mapReferenceBlockObject3Ds;

        PureObject3D** m_blocks; // TODO: not nice, in future we switch to cpp container
        int m_blocks_h;

        PureObject3D** m_foregroundBlocks;
        int m_foregroundBlocks_h;

        std::map<std::string, PGEcfgVariable> m_vars;
        std::string m_sRawName;
        std::string m_sFileName;
        std::map<char, std::string> m_Block2Texture;
        std::set<PureVector> m_spawnpoints;
        PureVector m_blocksVertexPosMin, m_blocksVertexPosMax;
        PureVector m_blockPosMin, m_blockPosMax;
        PureVector m_spawnpointLeftMost, m_spawnpointRightMost;
        unsigned int m_width, m_height;
        std::map<MapItem::MapItemId, MapItem*> m_items;
        std::vector<PureObject3D*> m_decorations; // for now this is only for the up sign of jumppads
        std::vector<PureObject3D*> m_jumppads;
        size_t m_nValidJumppadVarsCount;
        std::vector<TPURE_XY> m_fJumppadForceFactors;

        /* Mapcycle and Available maps handling */
        Mapcycle m_mapcycle;

        // ---------------------------------------------------------------------------

        static bool lineShouldBeIgnored(const std::string& sLine);
        static bool lineIsValueAssignment(const std::string& sLine, std::string& sVar, std::string& sValue, bool& bParseError);

        void lineHandleAssignment(std::string& sVar, std::string& sValue);
        bool lineHandleLayout(const std::string& sLine, TPureFloat& y, bool bDryRun);

    }; // class Maps

} // namespace proofps_dd
