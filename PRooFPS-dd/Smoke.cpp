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
    m_bGoingLeft(false)
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
    bool bGoingLeft)
{
    assert(m_obj);
    m_obj->getPosVec() = put.getPosVec();
    m_put = put;
    m_fScaling = 1.f;
    m_bGoingLeft = bGoingLeft;
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

    m_fScaling += 2.f / static_cast<TPureFloat>(nFactor);

    constexpr float fTargetScaling = 3.f;
    if (m_fScaling >= fTargetScaling)
    {
        remove(); // becomes free again in the pool
    }
    else
    {
        m_obj->SetScaling(PureVector(m_fScaling, m_fScaling, m_obj->getScaling().getZ()));
        m_put.Move(0.1f / nFactor);
        m_put.Elevate(0.5f / nFactor);
        m_obj->getPosVec() = m_put.getPosVec();
        m_obj->getAngleVec().SetZ(
            m_obj->getAngleVec().getZ() + ((m_bGoingLeft ? 1 : -1) * (60.f / nFactor))
        );

        const float fAnimationProgress = m_fScaling / fTargetScaling;
        const float fTargetTransparency = 1 - fAnimationProgress;
        m_obj->getMaterial(false).getTextureEnvColor().SetAsFloats(fTargetTransparency, fTargetTransparency, fTargetTransparency, 1.f);
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
