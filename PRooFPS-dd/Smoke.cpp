/*
    ###################################################################################
    Smoke.cpp
    Simple smoke object for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    ###################################################################################
*/

#include "stdafx.h"  // PCH

#include <cassert>

#include "Consts.h"
#include "Smoke.h"


// ############################### PUBLIC ################################


proofps_dd::Smoke::SmokeConfigAmount proofps_dd::Smoke::enumFromSmokeAmountString(const char* zstring)
{
    const std::string_view sSmoke(zstring);
    if (sSmoke == "none")
    {
        return Smoke::SmokeConfigAmount::None;
    }
    else if (sSmoke == "moderate")
    {
        return Smoke::SmokeConfigAmount::Moderate;
    }
    else if (sSmoke == "normal")
    {
        return Smoke::SmokeConfigAmount::Normal;
    }
    return Smoke::SmokeConfigAmount::Extreme;
}

bool proofps_dd::Smoke::isValidSmokeAmountString(const std::string& str)
{
    return std::find(
        validSmokeConfigAmountStringValues.begin(),
        validSmokeConfigAmountStringValues.end(),
        str) != validSmokeConfigAmountStringValues.end();
}

/**
* Config::validate() invokes this (config load/change), so we always know about the config here.
* Cannot use Config in Smoke, hence I need this dirty behavior.
*/
void proofps_dd::Smoke::updateSmokeConfigAmount(const SmokeConfigAmount& eSmokeConfigAmount)
{
    m_eSmokeConfigAmount = eSmokeConfigAmount;
}

const char* proofps_dd::Smoke::getLoggerModuleName()
{
    return "Smoke";
}

void proofps_dd::Smoke::destroyReferenceObject()
{
    // we would not need explicit call if Smoke implemented reference counting
    if (m_pSmokeRefObject)
    {
        delete m_pSmokeRefObject;
        m_pSmokeRefObject = nullptr;
    }
}

proofps_dd::Smoke::Smoke(
    PgeObjectPoolBase& parentPool,
    PR00FsUltimateRenderingEngine& gfx) :
    PgePooledObject(parentPool),
    m_gfx(gfx),
    m_obj(nullptr),
    m_fScaling(1.f),
    m_bGoingLeft(false),
    m_fInitialClrRedAsFloat(1.f),
    m_fInitialClrGreenAsFloat(1.f),
    m_fInitialClrBlueAsFloat(1.f)
{
    build3dObject();
}

proofps_dd::Smoke::~Smoke()
{
    if (m_obj)
    {
        m_gfx.getObject3DManager().DeleteAttachedInstance(*m_obj);
    }
}

void proofps_dd::Smoke::init(
    const PurePosUpTarget& put,
    bool bGoingLeft,
    TPureFloat fClrRedAsFloat,
    TPureFloat fClrGreenAsFloat,
    TPureFloat fClrBlueAsFloat)
{
    assert(m_obj);
    // probably looks more interesting if positions are a bit displaced randomly from the requested position ...
    m_obj->getPosVec().Set(
        put.getPosVec().getX() + ((PFL::random(0, 10) - 5) / 50.f),
        put.getPosVec().getY() + ((PFL::random(0, 10) - 5) / 50.f),
        put.getPosVec().getZ()
    );
    m_put = put;
    m_put.getPosVec() = m_obj->getPosVec();
    // even initial scaling is randomized a bit for less easily visible repeating smoke pattern
    m_fScaling = 1.f + ((PFL::random(0, 10) - 5) / 10.f);
    m_obj->SetScaling(PureVector(m_fScaling, m_fScaling, m_obj->getScaling().getZ()));
    m_bGoingLeft = bGoingLeft;
    m_obj->getMaterial(false).getTextureEnvColor().SetAsFloats(fClrRedAsFloat, fClrGreenAsFloat, fClrBlueAsFloat, 1.f);
    // storing the readback values because SetAsFloats() does PFL::constrain() for us too
    m_fInitialClrRedAsFloat = m_obj->getMaterial(false).getTextureEnvColor().getRedAsFloat();
    m_fInitialClrGreenAsFloat = m_obj->getMaterial(false).getTextureEnvColor().getGreenAsFloat();
    m_fInitialClrBlueAsFloat = m_obj->getMaterial(false).getTextureEnvColor().getBlueAsFloat();
}

void proofps_dd::Smoke::onSetUsed()
{
    assert(m_obj);
    m_obj->SetRenderingAllowed(used());
}

CConsole& proofps_dd::Smoke::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}

void proofps_dd::Smoke::update(const unsigned int& nFactor)
{
    if (!m_obj)
    {
        return;
    }

    assert(
        static_cast<int>(m_eSmokeConfigAmount) < static_cast<int>(smokeEmitOperValues.size())
    );
    const float& fScalingSpeed = smokeEmitOperValues[static_cast<int>(m_eSmokeConfigAmount)].m_fScalingSpeed;
    m_fScaling += fScalingSpeed / static_cast<TPureFloat>(nFactor);

    constexpr float fTargetScaling = 3.f;
    if (m_fScaling >= fTargetScaling)
    {
        remove(); // becomes free again in the pool
    }
    else
    {
        m_obj->SetScaling(PureVector(m_fScaling, m_fScaling, m_obj->getScaling().getZ()));
        m_put.Move(0.1f / nFactor); // should be relative to bullet speed, but for know this constliteral is ok
        m_put.Elevate(0.5f / nFactor); // this should be also relative to something but for now it is ok
        m_obj->getPosVec() = m_put.getPosVec();
        m_obj->getAngleVec().SetZ(
            m_obj->getAngleVec().getZ() + ((m_bGoingLeft ? 1 : -1) * (60.f / nFactor))
        );

        // TODO: now transparency is linear 1 to 0 per scaling, however in the future I could imagine 1 more config value, which
        // could control from what fAnimationProgress we should start fading the smoke away, so for example it could start
        // fading away only when fAnimationProgress has reached 0.3f, so that recently emitted smokes "saturate" a bit more.
        const float fAnimationProgress = m_fScaling / fTargetScaling;
        const float fTargetTransparency = 1 - fAnimationProgress;
        m_obj->getMaterial(false).getTextureEnvColor().SetAsFloats(
            m_fInitialClrRedAsFloat * fTargetTransparency,
            m_fInitialClrGreenAsFloat * fTargetTransparency,
            m_fInitialClrBlueAsFloat * fTargetTransparency, 1.f);
    }
}

PureObject3D& proofps_dd::Smoke::getObject3D()
{
    return *m_obj;
}

const PureObject3D& proofps_dd::Smoke::getObject3D() const
{
    return *m_obj;
}

void proofps_dd::Smoke::build3dObject()
{
    if (m_pSmokeRefObject == nullptr)
    {
        // this is the initial size, and then we gradually scaling it up in update()
        m_pSmokeRefObject = m_gfx.getObject3DManager().createPlane(0.5f, 0.5f);
        // TODO throw exception if cant create!
        m_pSmokeRefObject->SetDoubleSided(true);
        m_pSmokeRefObject->Hide();
        PureTexture* const pTexSmoke = m_gfx.getTextureManager().createFromFile(
            (std::string(proofps_dd::GAME_TEXTURES_DIR) + "smoke.bmp").c_str());
        m_pSmokeRefObject->getMaterial().setTexture(pTexSmoke);
        m_pSmokeRefObject->getMaterial(false).setBlendFuncs(PURE_ONE, PURE_ONE_MINUS_SRC_COLOR);
    }

    // TODO throw exception if cant create!
    m_obj = m_gfx.getObject3DManager().createCloned(*m_pSmokeRefObject);
    m_obj->getAngleVec().SetZ( static_cast<float>(PFL::random(0, 359)) );
}


// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################


PureObject3D* proofps_dd::Smoke::m_pSmokeRefObject = nullptr;
proofps_dd::Smoke::SmokeConfigAmount proofps_dd::Smoke::m_eSmokeConfigAmount = Smoke::SmokeConfigAmount::Normal;
