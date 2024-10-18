#pragma once

/*
    ###################################################################################
    Smoke.h
    Simple smoke object for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    ###################################################################################
*/

#include "CConsole.h"

#include "PGE.h"

namespace proofps_dd
{

    class Smoke : public PgePooledObject
    {

    public:
        static const char* getLoggerModuleName();          /**< Returns the logger module name of this class. */

        static void destroyReferenceObject();

        // ---------------------------------------------------------------------------

        Smoke(
            PgeObjectPoolBase& parentPool,
            PR00FsUltimateRenderingEngine& gfx);

        virtual ~Smoke();

        Smoke(const Smoke&) = delete;
        Smoke& operator=(const Smoke&) = delete;
        Smoke(Smoke&&) = delete;
        Smoke& operator=(Smoke&&) = delete;

        void init(
            const PurePosUpTarget& put,
            bool bGoingLeft);

        virtual void onSetUsed() override;

        CConsole& getConsole() const;                    /**< Returns access to console preset with logger module name as this class. */

        void update(const unsigned int& nFactor);

        PureObject3D& getObject3D();
        const PureObject3D& getObject3D() const;

    protected:

    private:

        static PureObject3D* m_pSmokeRefObject;

        PR00FsUltimateRenderingEngine& m_gfx;

        PureObject3D* m_obj;                     /**< Associated Pure object to be rendered. Used by PGE server and client instances. TODO: shared ptr. */
        TPureFloat m_fScaling;                   /**< To be increased during animation. Used by both PGE client and server instances. */
        PurePosUpTarget m_put;                   /**< PUT to calculate next position. Used by both PGE client and server instances. */
        bool m_bGoingLeft;                       /**< True if bullet and smoke going to left, false otherwise. Used by both PGE client and server instances. */

        // ---------------------------------------------------------------------------

        void build3dObject();

    }; // class Smoke

} // namespace proofps_dd
