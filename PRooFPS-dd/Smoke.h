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
        enum class SmokeConfigAmount
        {
            None = 0,
            Moderate,
            Normal,
            Extreme
        };

        static constexpr auto validSmokeConfigAmountStringValues = PFL::std_array_of<const char*>(
            "none",
            "moderate",
            "normal",
            "extreme"
        );

        static_assert(
            (static_cast<int>(SmokeConfigAmount::Extreme) + 1) == validSmokeConfigAmountStringValues.size(),
            "SmokeConfigAmount enum labels count should match validSmokeConfigAmountStringValues");

        struct SmokeEmitOperValues
        {
            float m_fScalingSpeed;
            int m_nEmitInEveryNPhysicsIteration;
        };

        /**
        * Based on user configuration SmokeConfigAmount, this table shows operational values used for smoke generation.
        * Due to current design, the max number of smokes at a time per bullet cannot be explicitly set, however with these
        * values, it can be tuned:
        *  - higher scaling speed leads to less smoke objects at a time,
        *  - higher EmitInEveryNPhysicsIteration leads to less smoke objects at a time.
        * The resulting max smoke objects per bullet with 60 Hz physics rate is mentioned near each setting.
        */
        static constexpr auto smokeEmitOperValues = PFL::std_array_of<SmokeEmitOperValues>(
            SmokeEmitOperValues{ /* m_fScalingSpeed */ 2.f, /* m_nEmitInEveryNPhysicsIteration */ 6 }, /* SmokeConfigAmount::None, max smoke/bullet: 0, in this case these are just dummies */
            SmokeEmitOperValues{ /* m_fScalingSpeed */ 2.f, /* m_nEmitInEveryNPhysicsIteration */ 6 }, /* SmokeConfigAmount::Moderate, max smoke/bullet: 10, */
            SmokeEmitOperValues{ /* m_fScalingSpeed */ 2.f, /* m_nEmitInEveryNPhysicsIteration */ 3 }, /* SmokeConfigAmount::Normal, max smoke/bullet: 20, */
            SmokeEmitOperValues{ /* m_fScalingSpeed */ 2.f, /* m_nEmitInEveryNPhysicsIteration */ 2 }  /* SmokeConfigAmount::Extreme, max smoke/bullet: 30, */
        );

        static_assert(
            (static_cast<int>(SmokeConfigAmount::Extreme) + 1) == smokeEmitOperValues.size(),
            "SmokeConfigAmount enum labels count should match smokeEmitOperValues");

        static constexpr char* szCVarGfxSmokeAmount = "gfx_smoke_amount";

        static SmokeConfigAmount enumFromSmokeAmountString(const char* zstring);
        static bool isValidSmokeAmountString(const std::string& str);
        static void updateSmokeConfigAmount(const SmokeConfigAmount& eSmokeConfigAmount);

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
            bool bGoingLeft,
            TPureFloat fClrRedAsFloat = 1.f,
            TPureFloat fClrGreenAsFloat = 1.f,
            TPureFloat fClrBlueAsFloat = 1.f);

        virtual void onSetUsed() override;

        CConsole& getConsole() const;                    /**< Returns access to console preset with logger module name as this class. */

        void update(const unsigned int& nFactor);

        PureObject3D& getObject3D();
        const PureObject3D& getObject3D() const;

    protected:

    private:

        static PureObject3D* m_pSmokeRefObject;
        static SmokeConfigAmount m_eSmokeConfigAmount; /**< Updated by Config when smoke config changes. Smoke does not access Config, hence we need this. */

        PR00FsUltimateRenderingEngine& m_gfx;

        PureObject3D* m_obj;                     /**< Associated Pure object to be rendered. Used by PGE server and client instances. TODO: shared ptr. */
        TPureFloat m_fScaling;                   /**< To be increased during animation. Used by both PGE client and server instances. */
        PurePosUpTarget m_put;                   /**< PUT to calculate next position. Used by both PGE client and server instances. */
        bool m_bGoingLeft;                       /**< True if bullet and smoke going to left, false otherwise. Used by both PGE client and server instances. */
        TPureFloat m_fInitialClrRedAsFloat;
        TPureFloat m_fInitialClrGreenAsFloat;
        TPureFloat m_fInitialClrBlueAsFloat;

        // ---------------------------------------------------------------------------

        void build3dObject();

    }; // class Smoke

} // namespace proofps_dd
