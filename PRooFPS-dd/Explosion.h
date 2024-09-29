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

    typedef PFL::StringHash ExplosionObjRefId;

    class Explosion
    {

    public:
        // uint32_t will be fine for explosion id ... since it was also good for bullet id.
        typedef uint32_t ExplosionId;

        struct ExplosionRefData
        {
            PureObject3D* m_pRefObj;
            SoLoud::Wav* m_pSndExplosion;
        };

        static const char* getLoggerModuleName();          /**< Returns the logger module name of this class. */

        static ExplosionId getGlobalExplosionId();
        static void resetGlobalExplosionId();

        static bool updateReferenceExplosions(PGE& pge, const std::string& filenameWithRelPath, const std::string& soundFilenameWithRelPath);
        static void destroyReferenceExplosions();

        // ---------------------------------------------------------------------------

        /** Ctor to be used by PGE server instance: explosion id will be assigned within the ctor. */
        Explosion(
            PGE& pge,
            const pge_network::PgeNetworkConnectionHandle& connHandle,
            const ExplosionObjRefId& refId,
            const PureVector& pos,
            const TPureFloat& fDamageAreaSize);

        /** Ctor to be used by PGE client instance: explosion id as received from server. */
        Explosion(
            PGE& pge,
            const ExplosionId& id,
            const pge_network::PgeNetworkConnectionHandle& connHandle,
            const ExplosionObjRefId& refId,
            const PureVector& pos,
            const TPureFloat& fDamageAreaSize);

        Explosion(const Explosion& other);            // TODO check if we really cannot live with just compiler generated copy ctor?

        Explosion& operator=(const Explosion& other) = delete; // if needed, uncomment and update in cpp as well

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
            const Bullet::DamageAreaEffect& eDamageAreaEffect,
            const int& nDamage) const;

        bool shouldBeDeleted() const;

    protected:

    private:

        static ExplosionId m_globalExplosionId;                /**< Next unique explosion id for identifying. Used by PGE server instance only. */

        static std::map<ExplosionObjRefId, ExplosionRefData> m_explosionRefObjects;

        ExplosionId m_id;                                      /**< Unique explosion id for identifying. Used by PGE server and client instances. */
        PGE& m_pge;
        pge_network::PgeNetworkConnectionHandle m_connHandle;  /**< Owner (caused by) of this explosion. Used by PGE server instance only. */
        ExplosionObjRefId m_refId;                             /**< Explosion object reference id, used to search in m_explosionRefObjects. Used by PGE server and client instances. */
        TPureFloat m_fDamageAreaSize;                          /**< Originating bullet's fDamageAreaSize. Used by PGE server and client instances. */

        PureObject3D* m_objPrimary;                            /**< Associated Primary Pure object to be rendered. Used by PGE server and client instances. TODO: shared ptr. */
        PureObject3D* m_objSecondary;                          /**< Associated Secondary Pure object to be rendered. Used by PGE server and client instances. TODO: shared ptr. */
        TPureFloat m_fScalingPrimary;                          /**< To be increased during animation. */
        TPureFloat m_fScalingSecondary;                        /**< To be increased during animation. */
        bool m_bCreateSentToClients;                           /**< Server should send update to clients about creation of new explosions. By default false. */
        SoLoud::handle m_sndHandle{};

        // ---------------------------------------------------------------------------

    }; // class Explosion

} // namespace proofps_dd
