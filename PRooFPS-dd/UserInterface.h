#pragma once

/*
    ###################################################################################
    UserInterface.h
    User interface handling for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "../../../CConsole/CConsole/src/CConsole.h"

#include "../../../PGE/PGE/PGE.h"

#include "Durations.h"

namespace proofps_dd
{

    class UserInterface
    {
    public:

        static const char* getLoggerModuleName();

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const;

        UserInterface(PGE& pge);

        UserInterface(const UserInterface&) = delete;
        UserInterface& operator=(const UserInterface&) = delete;
        UserInterface(UserInterface&&) = delete;
        UserInterface&& operator=(UserInterface&&) = delete;

    protected:

        void Text(const std::string& s, int x, int y) const;
        void AddText(const std::string& s, int x, int y) const;

    private:

        // ---------------------------------------------------------------------------

        PGE& m_pge;

    }; // class UserInterface

} // namespace proofps_dd