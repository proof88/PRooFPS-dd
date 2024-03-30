#pragma once

/*
    ###################################################################################
    Mapcycle.h
    Mapcycle handling for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    ###################################################################################
*/

#include <set>
#include <string>
#include <vector>

#include "CConsole.h"

namespace proofps_dd
{
    class Mapcycle
    {
    public:

        static constexpr char* GAME_MAPS_DIR = "gamedata/maps/";

        static const char* getLoggerModuleName();

        static bool isValidMapFilename(const std::string& sFilename);

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const;

        Mapcycle();
        ~Mapcycle();

        Mapcycle(const Mapcycle&) = delete;
        Mapcycle& operator=(const Mapcycle&) = delete;
        Mapcycle(Mapcycle&&) = delete;
        Mapcycle&& operator=(Mapcycle&&) = delete;

        bool initialize();                                   /**< Updates list of available maps and mapcycle content. */
        bool isInitialized() const;
        void shutdown();                                     /**< Releases any allocated resources related to available maps list and mapcycle. */

        /* Available maps handling */
        /* Logic is restricted to available maps, not impacting mapcycle. */

        const std::set<std::string>& availableMapsGet() const;
        const std::string& availableMapsGetElem(const size_t& index) const;
        const char** availableMapsGetAsCharPtrArray() const;
        const std::set<std::string>& availableMapsNoChangingGet() const;
        const std::string& availableMapsNoChangingGetElem(const size_t& index) const;

        /* Mapcycle handling */
        /* Logic is restricted to mapcycle, not impacting available maps. */

        const std::vector<std::string>& mapcycleGet() const;
        const char** mapcycleGetAsCharPtrArray() const;
        std::string mapcycleGetCurrent() const;
        bool mapcycleIsCurrentLast() const;
        std::string mapcycleNext();
        std::string mapcycleRewindToFirst();
        std::string mapcycleForwardToLast();
        bool mapcycleSaveToFile();

        /* Available maps and Mapcycle handling together */
        /* Here we tie both together, these complex functions are recommended to be used by GUI. */

        void mapcycle_availableMaps_Synchronize();
        bool mapcycleAdd_availableMapsRemove(const std::string& sMapFilename);
        bool mapcycleAdd_availableMapsRemove(const std::vector<std::string>& vMapFilenames);
        bool mapcycleAdd_availableMapsRemove();
        bool mapcycleRemove_availableMapsAdd(const std::string& sMapFilename);
        bool mapcycleRemove_availableMapsAdd(const size_t& indexToMapcycle);
        bool mapcycleRemove_availableMapsAdd(const std::vector<std::string>& vMapFilenames);
        bool mapcycleRemove_availableMapsAdd();

    protected:

        /* Available maps handling */
        /* Logic is restricted to available maps, not impacting mapcycle. */

        void availableMapsRefresh();
        bool availableMapsAdd(const std::string& sMapFilename);
        bool availableMapsAdd(const std::vector<std::string>& vMapFilenames);
        bool availableMapsRemove(const std::string& sMapFilename);
        bool availableMapsRemove(const size_t& index);
        bool availableMapsRemove(const std::vector<std::string>& vMapFilenames);

        /* Mapcycle handling */
        /* Logic is restricted to mapcycle, not impacting available maps. */

        bool mapcycleReload();
        bool mapcycleAdd(const std::string& sMapFilename);
        bool mapcycleAdd(const std::vector<std::string>& vMapFilenames);
        bool mapcycleAdd(const std::set<std::string>& vMapFilenames);
        bool mapcycleRemove(const std::string& sMapFilename);
        bool mapcycleRemove(const size_t& index);
        bool mapcycleRemove(const std::vector<std::string>& vMapFilenames);
        void mapcycleClear();
        size_t mapcycleRemoveNonExisting();

    private:

        bool m_bInitialized = false; // none of the other members can hold a clear value reflecting initialized state so I need this flag

        /* Available maps handling */

        std::set<std::string> m_availableMaps;
        const char** m_vszAvailableMaps = nullptr;

        std::set<std::string> m_availableMapsNoChanging;
        std::string m_sEmptyStringToReturn;

        /* Mapcycle handling */

        // I want unique elements in mapcycle. Yes, std::set would be trivial, but:
        // - I don't want elements to be sorted;
        // - I'm not sure if uniqueness will be forever, maybe I want to allow repeating maps!
        // Yes I know about std::unordered_multiset, but for some reason I wanted to stick to vector for now.
        std::vector<std::string> m_mapcycle;
        std::vector<std::string>::iterator m_mapcycleItCurrent = m_mapcycle.end();
        const char** m_vszMapcycle = nullptr;

        // ---------------------------------------------------------------------------

        static bool lineShouldBeIgnored(const std::string& sLine);

        bool mapFilenameAddToVector_NoDuplicatesAllowed(
            const std::string& sMapFilename,
            std::vector<std::string>& vec);
        bool mapFilenameAddToSet_NoDuplicatesAllowed(
            const std::string& sMapFilename,
            std::set<std::string>& settt);
        bool mapFilenameRemoveFromVector(
            const std::string& sMapFilename,
            std::vector<std::string>& vec);
        bool mapFilenameRemoveFromSet(
            const std::string& sMapFilename,
            std::set<std::string>& settt);
        void availableMapsRefreshCharPtrArray();
        void mapcycleRefreshCharPtrArray();

    }; // class Mapcycle

} // namespace proofps_dd
