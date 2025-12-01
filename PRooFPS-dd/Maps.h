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
#include "PURE/include/external/SpatialStructures/PureBoundingVolumeHierarchy.h"

#include "Mapcycle.h"
#include "MapItem.h"
#include "PRooFPS-dd-packet.h"

namespace proofps_dd
{
    class Maps
    {
    public:

        static constexpr char* szCVarSvMap = "sv_map";
        static constexpr char* szCVarSvMapTeamSpawnGroups = "sv_map_team_spawn_groups";

        static constexpr char* szCVarSvMapCollisionMode = "sv_map_collision_mode";
        static constexpr char* szCVarSvMapCollisionBvhDebugRender = "sv_map_collision_bvh_debug_render";
        static constexpr char* szCVarSvMapCollisionBvhMaxDepth = "sv_map_collision_bvh_max_depth";

        static constexpr float fMapBlockSizeWidth = 1.0f;
        static constexpr float fMapBlockSizeHeight = 1.0f;
        static constexpr float fMapBlockSizeDepth = 1.0f;

        static constexpr size_t nStairstepsCount = 4;

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
        const std::vector<PureVector>& getSpawnpoints() const;  /**< Retrieves the set of spawnpoints of the currently loaded map. */
        const std::set<size_t>& getTeamSpawnpoints(
            const unsigned int& iTeamId) const;              /**< Retrieves the spawn group of the currently loaded map, for the specified team. */
        bool areTeamSpawnpointsDefined() const;
        bool canUseTeamSpawnpoints(
            const bool& bTeamGame,
            const unsigned int& iTeamId) const;
        const PureVector& getRandomSpawnpoint(
            const bool& bTeamGame,
            const unsigned int& iTeamId = 0) const;          /**< Retrieves a randomly selected spawnpoint from the set of spawnpoints of the currently loaded map. */
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
        const PureBoundingVolumeHierarchy& getBVH() const;
        const std::map<MapItem::MapItemId, MapItem*>& getItems() const;
        const std::vector<PureObject3D*>& getDecals() const;
        const std::vector<PureObject3D*>& getJumppads() const;
        const std::map<std::string, PGEcfgVariable>& getVars() const;
        size_t getJumppadValidVarsCount();
        const TPURE_XY& getJumppadForceFactors(const size_t& index) const;
        void update(const float& fps, const PureObject3D& objCurrentPlayer);

        bool handleMapItemUpdateFromServer(
            pge_network::PgeNetworkConnectionHandle /*connHandleServerSide*/,
            const MsgMapItemUpdateFromServer& msg);

        /* Mapcycle and Available maps handling */

        Mapcycle& getMapcycle();

    protected:

    private:

        static constexpr float GAME_PLAYERS_POS_Z = -1.2f;
        static constexpr float GAME_ITEMS_POS_Z = GAME_PLAYERS_POS_Z + 0.1f;  // avoid Z-fighting with items the player cannot take
        static constexpr float GAME_DECAL_POS_Z = fMapBlockSizeDepth / -2.f;
        static constexpr float GAME_DECOR_POS_Z = fMapBlockSizeDepth / -2.f - 0.1f;  // decors are close to the wall surfaces TODO: rename because this is just for jumppads only

        struct BlockTexture
        {
            std::string m_sTexFilename;
            float m_fU0{0.f}, m_fV0{0.f};  /* vertex 1 UV (bottom left) */
            float m_fU1{1.f}, m_fV1{1.f};  /* vertex 3 UV (top right) */
        };

        const std::set<char> foregroundBlocks = {
            'B', 'D', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'Q', 'T',
            /* the special foreground stuff (e.g. jump pads) are treated as foreground blocks, see special handling in lineHandleLayout(): */
            '^' /* jump pad vertical */,
            '\\' /* stairs descending to the right */,
            '/'  /* stairs ascending to the right */
        };

        const std::set<char> backgroundBlocks = {
            'a', 'c', 'e', 'm', 'n', 'p', 'o', 'r', 'u', 'v', 'w', 'x', 'y', 'z',
            /* the special foreground stuff (e.g. items) are treated as background blocks too, see special handling in lineHandleLayout(): */
            ',' /* armor */,
            '+' /* medkit */,
            '.' /* jetlax */,
            '2' /* weapon key 2 */,
            '3' /* weapon key 3 */,
            '4' /* weapon key 4 */,
            '5' /* weapon key 5 */,
            '6' /* weapon key 6 */,
            '7' /* weapon key 7 */,
            '8' /* weapon key 8 */,
            'S' /* spawnpoint */
        };

        pge_audio::PgeAudio& m_audio;
        PGEcfgProfiles& m_cfgProfiles;
        PR00FsUltimateRenderingEngine& m_gfx;
        PureTexture* m_texRed;  // TODO: unique_ptr
        PureTexture* m_texDecorJumpPadVertical;  // TODO: unique_ptr
        std::string m_sServerMapFilenameToLoad;                      /**< We set this as soon as we get to know which map we should load. */

        /* Current map handling */

        std::map<char, PureObject3D*> m_mapReferenceBlockObject3Ds;

        PureObject3D** m_blocks; // TODO: not nice, in future we switch to cpp container
        int m_blocks_h;

        PureObject3D** m_foregroundBlocks; // TODO: as BVH has been introduced, this might be removed in the future, BVH is also holding almost same objects!
        int m_foregroundBlocks_h;

        PureBoundingVolumeHierarchyRoot m_bvh; // for now, same as m_foregroundBlocks

        std::map<std::string, PGEcfgVariable> m_vars;
        std::string m_sRawName;     /**< Raw map name, basically filename without extension. */
        std::string m_sFileName;
        std::map<char, BlockTexture> m_Block2Texture;
        std::vector<PureVector> m_spawnpoints;  // before v0.5 it was std::set, but I want to have them in file parsing order!
        std::set<size_t> m_spawngroup_1;
        std::set<size_t> m_spawngroup_2;
        PureVector m_blocksVertexPosMin, m_blocksVertexPosMax;
        PureVector m_blockPosMin, m_blockPosMax;
        PureVector m_spawnpointLeftMost, m_spawnpointRightMost;
        unsigned int m_width, m_height;
        std::map<MapItem::MapItemId, MapItem*> m_items;
        std::vector<PureObject3D*> m_decals;      // these are the decal planes introduced in v0.4.2
        std::vector<PureObject3D*> m_decorations; // TODO: for now this is only for the up sign of jumppads, should rename, these are up signs
        std::vector<PureObject3D*> m_jumppads;    // TODO: should rename this too because these are blocks
        size_t m_nValidJumppadVarsCount;
        std::vector<TPURE_XY> m_fJumppadForceFactors;

        /* Mapcycle and Available maps handling */
        Mapcycle m_mapcycle;

        // ---------------------------------------------------------------------------

        static bool lineShouldBeIgnored(const std::string& sLine);
        static bool lineIsValueAssignment(const std::string& sLine, std::string& sVar, std::string& sValue, bool& bParseError);

        bool lineHandleDecalAssignment(const std::string& sValue);
        bool lineHandleAssignment(const std::string& sVar, const std::string& sValue);
        bool createSingleSmallStairStep(
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
        );
        bool createSmallStairStepsForSingleBigStairsBlock(
            const bool& bDryRun,
            const size_t& iLinePos,
            const size_t& nLineLength,
            const bool& bCopyPreviousFgBlock,
            const int& iObjectFgToBeCopied,
            const bool& bCopyPreviousBgBlock,
            const int& iObjectBgToBeCopied,
            const float& fBlockPosX,
            const float& fBlockPosY);
        bool lineHandleLayout(const std::string& sLine, TPureFloat& y, bool bDryRun);
        bool parseTeamSpawnpointsFromString(
            const std::string& sVarValue, std::set<size_t>& targetSet);
        bool parseTeamSpawnpoints();
        bool checkAndUpdateSpawnpoints();

    }; // class Maps

} // namespace proofps_dd
