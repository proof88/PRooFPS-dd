/*
    ###################################################################################
    Explosion.cpp
    Simple explosion object for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    ###################################################################################
*/

#include "stdafx.h"  // PCH

#include <cassert>

#include "Consts.h"
#include "Explosion.h"


// ############################### PUBLIC ################################


const char* proofps_dd::Explosion::getLoggerModuleName()
{
    return "Explosion";
}

proofps_dd::Explosion::ExplosionId proofps_dd::Explosion::getGlobalExplosionId()
{
    return m_globalExplosionId;
}

void proofps_dd::Explosion::resetGlobalExplosionId()
{
    m_globalExplosionId = 0;
}

bool proofps_dd::Explosion::initExplosionsReference(PGE& pge)
{
    if (m_pReferenceObjExplosion)
    {
        return true;
    }

    m_pReferenceObjExplosion = pge.getPure().getObject3DManager().createFromFile((std::string(GAME_MODELS_DIR) + "rocketl_xpl.obj").c_str());
    if (!m_pReferenceObjExplosion)
    {
        return false;
    }

    m_pReferenceObjExplosion->SetDoubleSided(true);
    m_pReferenceObjExplosion->SetLit(false);
    // We want the animation to start with 0.1 unit size so we set scaling with this formula.
    // For example, diameter of rocketl_xpl.obj is 16 units, so to have a diameter of 1 unit, its scaling should be 1/16,
    // we use getSizeVec().getX() as diameter, considering the model object is always a sphere.
    m_pReferenceObjExplosion->SetScaling(1 / m_pReferenceObjExplosion->getSizeVec().getX() / 10.f);
    m_pReferenceObjExplosion->getMaterial(false).setBlendFuncs(PURE_SRC_ALPHA, PURE_ONE);
    m_pReferenceObjExplosion->Hide();

    return true;
}

void proofps_dd::Explosion::destroyExplosionsReference()
{
    if (m_pReferenceObjExplosion)
    {
        delete m_pReferenceObjExplosion; // will detach from manager
        m_pReferenceObjExplosion = nullptr;
    }
}

/**
    Ctor to be used by PGE server instance: bullet id will be assigned within the ctor.
*/
proofps_dd::Explosion::Explosion(
    PR00FsUltimateRenderingEngine& gfx,
    const pge_network::PgeNetworkConnectionHandle& connHandle,
    const PureVector& pos,
    const TPureFloat& fDamageAreaSize) :
    m_id(m_globalExplosionId++),
    m_gfx(gfx),
    m_connHandle(connHandle),
    m_fDamageAreaSize(fDamageAreaSize),
    m_objPrimary(nullptr),
    m_objSecondary(nullptr),
    m_fScalingPrimary(m_pReferenceObjExplosion->getScaling().getX()),
    m_fScalingSecondary(m_pReferenceObjExplosion->getScaling().getX()),
    m_bCreateSentToClients(false)
{
    // TODO throw exception if cant create!
    m_objPrimary = gfx.getObject3DManager().createCloned(*m_pReferenceObjExplosion);
    m_objPrimary->Show();
    m_objPrimary->getPosVec() = pos;

    m_objSecondary = gfx.getObject3DManager().createCloned(*m_pReferenceObjExplosion);
    m_objSecondary->Hide();
    m_objSecondary->getPosVec() = pos;
}

/**
    Ctor to be used by PGE client instance: bullet id as received from server.
*/
proofps_dd::Explosion::Explosion(
    PR00FsUltimateRenderingEngine& gfx,
    const Explosion::ExplosionId& id,
    const pge_network::PgeNetworkConnectionHandle& connHandle,
    const PureVector& pos,
    const TPureFloat& fDamageAreaSize) :
    m_id(id),
    m_gfx(gfx),
    m_connHandle(connHandle),
    m_fDamageAreaSize(fDamageAreaSize),
    m_objPrimary(nullptr),
    m_objSecondary(nullptr),
    m_fScalingPrimary(m_pReferenceObjExplosion->getScaling().getX()),
    m_fScalingSecondary(m_pReferenceObjExplosion->getScaling().getX()),
    m_bCreateSentToClients(true) /* irrelevant for this client-side ctor but we are client so yes it is sent :) */
{
    // TODO throw exception if cant create!
    m_objPrimary = gfx.getObject3DManager().createCloned(*m_pReferenceObjExplosion);
    m_objPrimary->Show();
    m_objPrimary->getPosVec() = pos;

    m_objSecondary = gfx.getObject3DManager().createCloned(*m_pReferenceObjExplosion);
    m_objSecondary->Hide();
    m_objSecondary->getPosVec() = pos;
}

proofps_dd::Explosion::Explosion(const proofps_dd::Explosion& other) :
    m_id(other.m_id),
    m_gfx(other.m_gfx),
    m_connHandle(other.m_connHandle),
    m_fDamageAreaSize(other.m_fDamageAreaSize),
    m_fScalingPrimary(other.m_fScalingPrimary),
    m_fScalingSecondary(other.m_fScalingSecondary),
    m_bCreateSentToClients(other.m_bCreateSentToClients)
{
    // TODO throw exception if cant create!
    if (other.m_objPrimary)
    {
        m_objPrimary = m_gfx.getObject3DManager().createCloned(*m_pReferenceObjExplosion);
        m_objPrimary->SetRenderingAllowed(other.m_objPrimary->isRenderingAllowed());
        m_objPrimary->getPosVec() = other.m_objPrimary->getPosVec();
        m_objPrimary->SetScaling(other.m_objPrimary->getScaling());
    }
    else
    {
        m_objPrimary = nullptr;
    }

    if (other.m_objSecondary)
    {
        m_objSecondary = m_gfx.getObject3DManager().createCloned(*m_pReferenceObjExplosion);
        m_objSecondary->SetRenderingAllowed(other.m_objSecondary->isRenderingAllowed());
        m_objSecondary->getPosVec() = other.m_objSecondary->getPosVec();
        m_objSecondary->SetScaling(other.m_objSecondary->getScaling());
    }
    else
    {
        m_objSecondary = nullptr;
    }
}

proofps_dd::Explosion& proofps_dd::Explosion::operator=(const proofps_dd::Explosion& other)
{
    m_id = other.m_id;
    m_gfx = other.m_gfx;
    m_connHandle = other.m_connHandle;
    m_fDamageAreaSize = other.m_fDamageAreaSize;
    m_fScalingPrimary = other.m_fScalingPrimary;
    m_fScalingSecondary = other.m_fScalingSecondary;
    m_bCreateSentToClients = other.m_bCreateSentToClients;

    // TODO throw exception if cant create!
    if (other.m_objPrimary)
    {
        m_objPrimary = m_gfx.getObject3DManager().createCloned(*m_pReferenceObjExplosion);
        m_objPrimary->SetRenderingAllowed(other.m_objPrimary->isRenderingAllowed());
        m_objPrimary->getPosVec() = other.m_objPrimary->getPosVec();
        m_objPrimary->SetScaling(other.m_objPrimary->getScaling());
    }
    else
    {
        m_objPrimary = nullptr;
    }

    if (other.m_objSecondary)
    {
        m_objSecondary = m_gfx.getObject3DManager().createCloned(*m_pReferenceObjExplosion);
        m_objSecondary->SetRenderingAllowed(other.m_objSecondary->isRenderingAllowed());
        m_objSecondary->getPosVec() = other.m_objSecondary->getPosVec();
        m_objSecondary->SetScaling(other.m_objSecondary->getScaling());
    }
    else
    {
        m_objSecondary = nullptr;
    }

    return *this;
}

proofps_dd::Explosion::~Explosion()
{
    if (m_objPrimary)
    {
        m_gfx.getObject3DManager().DeleteAttachedInstance(*m_objPrimary);
    }
    if (m_objSecondary)
    {
        m_gfx.getObject3DManager().DeleteAttachedInstance(*m_objSecondary);
    }
}

CConsole& proofps_dd::Explosion::getConsole() const
{
    return CConsole::getConsoleInstance(getLoggerModuleName());
}

proofps_dd::Explosion::ExplosionId proofps_dd::Explosion::getId() const
{
    return m_id;
}

pge_network::PgeNetworkConnectionHandle proofps_dd::Explosion::getOwner() const
{
    return m_connHandle;
}

bool& proofps_dd::Explosion::isCreateSentToClients()
{
    return m_bCreateSentToClients;
}

void proofps_dd::Explosion::update(const unsigned int& nFactor)
{
    if (m_objPrimary)
    {
        m_fScalingPrimary += 1.5f / static_cast<TPureFloat>(nFactor);
        // either we are scaling in all dimensions or just in XY directions, not sure which looks better!
        //m_objPrimary->SetScaling(m_fScalingPrimary);
        m_objPrimary->SetScaling(PureVector(m_fScalingPrimary, m_fScalingPrimary, m_objPrimary->getScaling().getZ()));

        // the scaling animation should end when we reached the damage area size, which is the radius of damage,
        // thus we should stop at m_fDamageAreaSize * 2, however I use *3 because the transparency is also increasing:
        const float fTargetDiameter = m_fDamageAreaSize * 3;
        const float fCurrentDiameter = m_objPrimary->getSizeVec().getX() * m_fScalingPrimary;

        if (fCurrentDiameter > fTargetDiameter)
        {
            delete m_objPrimary;
            m_objPrimary = nullptr;
        }
        else
        {
            const float fAnimationProgress = fCurrentDiameter / fTargetDiameter;
            if (m_objSecondary && !m_objSecondary->isRenderingAllowed())
            {
                if (fAnimationProgress >= 0.4f)
                {
                    m_objSecondary->Show();
                }
            }
            const float fTargetTransparency = 1 - fAnimationProgress;
            m_objPrimary->getMaterial(false).getTextureEnvColor().SetAsFloats(fTargetTransparency, fTargetTransparency, fTargetTransparency, 1.f);
        }
    }

    // copy-paste code, not nice ...
    if (m_objSecondary && m_objSecondary->isRenderingAllowed())
    {
        m_fScalingSecondary += 1.5f / static_cast<TPureFloat>(nFactor);
        // either we are scaling in all dimensions or just in XY directions, not sure which looks better!
        //m_objSecondary->SetScaling(m_fScalingSecondary);
        m_objSecondary->SetScaling(PureVector(m_fScalingSecondary, m_fScalingSecondary, m_objSecondary->getScaling().getZ()));

        // the scaling animation should end when we reached the damage area size, which is the radius of damage,
        // thus we should stop at m_fDamageAreaSize * 2, however I use *3 because the transparency is also increasing:
        const float fTargetDiameter = m_fDamageAreaSize * 3;
        const float fCurrentDiameter = m_objSecondary->getSizeVec().getX() * m_fScalingSecondary;

        if (fCurrentDiameter > fTargetDiameter)
        {
            delete m_objSecondary;
            m_objSecondary = nullptr;
        }
        else
        {
            const float fTargetTransparency = 1 - (fCurrentDiameter / fTargetDiameter);
            m_objSecondary->getMaterial(false).getTextureEnvColor().SetAsFloats(fTargetTransparency, fTargetTransparency, fTargetTransparency, 1.f);
        }
    }
}

PureObject3D& proofps_dd::Explosion::getPrimaryObject3D()
{
    return *m_objPrimary;
}

const PureObject3D& proofps_dd::Explosion::getPrimaryObject3D() const
{
    return *m_objPrimary;
}

PureObject3D& proofps_dd::Explosion::getSecondaryObject3D()
{
    return *m_objSecondary;
}

const PureObject3D& proofps_dd::Explosion::getSecondaryObject3D() const
{
    return *m_objSecondary;
}

const float& proofps_dd::Explosion::getDamageAreaSize() const
{
    return m_fDamageAreaSize;
}

float proofps_dd::Explosion::getDamageAtDistance(
    const float& fDistance,
    const int& nDamageHp) const
{
    // fDistance is distance between explosion center and other entity's center,
    // m_fDamageAreaSize is radius
    assert(fDistance >= 0.f);
    assert(m_fDamageAreaSize > 0.f);

    return nDamageHp * std::max(0.f, (1 - (fDistance / m_fDamageAreaSize)));
}

bool proofps_dd::Explosion::shouldBeDeleted() const
{
    return !m_objPrimary && !m_objSecondary;
}


// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################


proofps_dd::Explosion::ExplosionId proofps_dd::Explosion::m_globalExplosionId = 0;
PureObject3D* proofps_dd::Explosion::m_pReferenceObjExplosion = nullptr;
