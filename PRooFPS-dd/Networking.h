#pragma once

/*
    ###################################################################################
    Networking.h
    Network handling for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "../../Console/CConsole/src/CConsole.h"

#include "PGE.h"

#include "Durations.h"

namespace proofps_dd
{

    class Networking
    {
    public:

        static const char* getLoggerModuleName();

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const;

        Networking(
            PGE& pge,
            proofps_dd::Durations& durations);

        Networking(const Networking&) = delete;
        Networking& operator=(const Networking&) = delete;
        Networking(Networking&&) = delete;
        Networking&& operator=(Networking&&) = delete;

        bool reinitialize();
        bool isServer() const;

    protected:

        pge_network::PgeNetworkConnectionHandle m_nServerSideConnectionHandle;   /**< Server-side connection handle received from server in PgePktUserConnected
                                                                                      (server instance also receives this from itself).
                                                                                      Server doesn't have a connection to itself, so it uses default 0 (invalid) handle. */

        bool isMyConnection(const pge_network::PgeNetworkConnectionHandle& connHandleServerSide) const;
        void allowListAppMessages();


    private:

        // ---------------------------------------------------------------------------

        PGE& m_pge;
        proofps_dd::Durations& m_durations;


    }; // class Networking

} // namespace proofps_dd