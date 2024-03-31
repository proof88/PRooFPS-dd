#pragma once

/*
    ###################################################################################
    Explosion.h
    Simple explosion object for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    ###################################################################################
*/

#include "CConsole.h"

#include "PGE.h"

namespace proofps_dd
{

    class Explosion
    {

    public:
        // uint32_t will be fine for explosion id ... since it was also good for bullet id.
        typedef uint32_t ExplosionId;

        static const char* getLoggerModuleName();          /**< Returns the logger module name of this class. */

        static ExplosionId getGlobalExplosionId();
        static void resetGlobalExplosionId();

        static bool initExplosionsReference(PGE& pge);
        static void destroyExplosionsReference();

        // ---------------------------------------------------------------------------

        /** Ctor to be used by PGE server instance: bullet id will be assigned within the ctor. */
        Explosion(
            PR00FsUltimateRenderingEngine& gfx,
            const pge_network::PgeNetworkConnectionHandle& connHandle,
            const PureVector& pos,
            const TPureFloat& fDamageAreaSize);

        /** Ctor to be used by PGE client instance: bullet id as received from server. */
        Explosion(
            PR00FsUltimateRenderingEngine& gfx,
            const ExplosionId& id,
            const pge_network::PgeNetworkConnectionHandle& connHandle,
            const PureVector& pos,
            const TPureFloat& fDamageAreaSize);

        Explosion(const Explosion& other);            // TODO check if we really cannot live with just compiler generated copy ctor?

        Explosion& operator=(const Explosion& other); // TODO check if we really cannot live with just compiler generated operator=?

        virtual ~Explosion();

        CConsole& getConsole() const;                    /**< Returns access to console preset with logger module name as this class. */

        ExplosionId getId() const;

        pge_network::PgeNetworkConnectionHandle getOwner() const;

        bool& isCreateSentToClients();

        void update(const unsigned int& nFactor);

        PureObject3D& getPrimaryObject3D();
        const PureObject3D& getPrimaryObject3D() const;
        PureObject3D& getSecondaryObject3D();
        const PureObject3D& getSecondaryObject3D() const;

        const float& getDamageAreaSize() const;
        float getDamageAtDistance(
            const float& fDistance,
            const int& nDamageHp) const;

        bool shouldBeDeleted() const;

    protected:

    private:

        static ExplosionId m_globalExplosionId;                /**< Next unique explosion id for identifying. Used by PGE server instance only. */
        static PureObject3D* m_pReferenceObjExplosion;

        ExplosionId m_id;                                      /**< Unique explosion id for identifying. Used by PGE server and client instances. */
        PR00FsUltimateRenderingEngine& m_gfx;
        pge_network::PgeNetworkConnectionHandle m_connHandle;  /**< Owner (caused by) of this explosion. Used by PGE server instance only. */
        TPureFloat m_fDamageAreaSize;                          /**< Originating bullet's fDamageAreaSize. Used by PGE server and client instances. */

        PureObject3D* m_objPrimary;                            /**< Associated Primary Pure object to be rendered. Used by PGE server and client instances. TODO: shared ptr. */
        PureObject3D* m_objSecondary;                          /**< Associated Secondary Pure object to be rendered. Used by PGE server and client instances. TODO: shared ptr. */
        TPureFloat m_fScalingPrimary;                          /**< To be increased during animation. */
        TPureFloat m_fScalingSecondary;                        /**< To be increased during animation. */
        bool m_bCreateSentToClients;                           /**< Server should send update to clients about creation of new explosions. By default false. */

        // ---------------------------------------------------------------------------

    }; // class Explosion

} // namespace proofps_dd
