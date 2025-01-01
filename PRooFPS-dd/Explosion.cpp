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

static constexpr float SndExplosionDistMin = 6.f;
static constexpr float SndExplosionDistMax = 14.f;


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

bool proofps_dd::Explosion::updateReferenceExplosions(PGE& pge, const std::string& filenameWithRelPath, const std::string& soundFilenameWithRelPath)
{
    const std::string sFilenameOnly = PFL::getFilename(filenameWithRelPath.c_str());
    const ExplosionObjRefId refId = PFL::calcHash(sFilenameOnly);
    if (m_explosionRefObjects.find(refId) != m_explosionRefObjects.end())
    {
        return true;
    }

    PureObject3D* const pReferenceObjExplosion = pge.getPure().getObject3DManager().createFromFile((std::string(GAME_MODELS_DIR) + sFilenameOnly).c_str());
    if (!pReferenceObjExplosion)
    {
        CConsole::getConsoleInstance("Explosion").EOLn("Failed to create reference object!");
        return false;
    }

    pReferenceObjExplosion->SetDoubleSided(true);
    pReferenceObjExplosion->SetLit(false);
    // We want the animation to start with 0.1 unit size so we set scaling with this formula.
    // For example, diameter of rocketl_xpl.obj is 16 units, so to have a diameter of 1 unit, its scaling should be 1/16,
    // we use getSizeVec().getX() as diameter, considering the model object is always a sphere.
    pReferenceObjExplosion->SetScaling(1 / pReferenceObjExplosion->getSizeVec().getX() / 10.f);
    pReferenceObjExplosion->getMaterial(false).setBlendFuncs(PURE_SRC_ALPHA, PURE_ONE);
    pReferenceObjExplosion->Hide();

    SoLoud::Wav* const pWav = new SoLoud::Wav();
    pge.getAudio().loadSound(*pWav, std::string(proofps_dd::GAME_AUDIO_DIR) + "/weapons/" + PFL::getFilename(soundFilenameWithRelPath.c_str()));

    m_explosionRefObjects.insert({ refId, {pReferenceObjExplosion, pWav} });

    CConsole::getConsoleInstance("Explosion").OLn("A new reference object is created from %s!", sFilenameOnly.c_str());

    return true;
}

void proofps_dd::Explosion::destroyReferenceExplosions()
{
    for (const auto& refExplosion : m_explosionRefObjects)
    {
        assert(refExplosion.second.m_pRefObj);
        delete refExplosion.second.m_pRefObj;

        assert(refExplosion.second.m_pSndExplosion);
        refExplosion.second.m_pSndExplosion->stop();
        delete refExplosion.second.m_pSndExplosion;
    }
    m_explosionRefObjects.clear();
}

/**
    Ctor to be used by PGE server instance: explosion id will be assigned within the ctor.
*/
proofps_dd::Explosion::Explosion(
    PGE& pge,
    const pge_network::PgeNetworkConnectionHandle& connHandle,
    const ExplosionObjRefId& refId,
    const PureVector& pos,
    const TPureFloat& fDamageAreaSize) :
    m_id(m_globalExplosionId++),
    m_pge(pge),
    m_connHandle(connHandle),
    m_refId(refId),
    m_fDamageAreaSize(fDamageAreaSize),
    m_objPrimary(nullptr),
    m_objSecondary(nullptr),
    m_bCreateSentToClients(false)
{
    const auto itRef = m_explosionRefObjects.find(refId);
    if (itRef == m_explosionRefObjects.end())
    {
        getConsole().EOLn("Explosion ctor (server): no ref explosion found with id: %u", static_cast<uint32_t>(refId));
        throw std::runtime_error("Explosion ctor (server): no ref explosion found with id: " + std::to_string(refId));
    }

    const auto explRefData = itRef->second;
    assert(explRefData.m_pRefObj);

    m_fScalingPrimary = explRefData.m_pRefObj->getScaling().getX();
    m_fScalingSecondary = explRefData.m_pRefObj->getScaling().getX();

    m_objPrimary = m_pge.getPure().getObject3DManager().createCloned(*explRefData.m_pRefObj);
    m_objSecondary = m_pge.getPure().getObject3DManager().createCloned(*explRefData.m_pRefObj);

    if (!m_objPrimary || !m_objSecondary)
    {
        CConsole::getConsoleInstance("Explosion").EOLn("Explosion ctor (server): Failed to create a cloned object!");
        throw std::runtime_error("Explosion ctor (server): Failed to create a cloned object!");
    }

    m_objPrimary->Show();
    m_objPrimary->getPosVec() = pos;

    m_objSecondary->Hide();
    m_objSecondary->getPosVec() = pos;

    // NOT playing sound here but in the copy ctor! Reason: copy ctor is invoked anyway when we store the explosion in the container!
    //m_sndHandle = m_pge.getAudio().play3dSound(*explRefData.m_pSndExplosion, pos);
    //m_pge.getAudio().getAudioEngineCore().set3dSourceMinMaxDistance(m_sndHandle, SndExplosionDistMin, SndExplosionDistMax);
    //m_pge.getAudio().getAudioEngineCore().set3dSourceAttenuation(m_sndHandle, SoLoud::AudioSource::ATTENUATION_MODELS::LINEAR_DISTANCE, 1.f);
}

/**
    Ctor to be used by PGE client instance: explosion id as received from server.
*/
proofps_dd::Explosion::Explosion(
    PGE& pge,
    const Explosion::ExplosionId& id,
    const pge_network::PgeNetworkConnectionHandle& connHandle,
    const ExplosionObjRefId& refId,
    const PureVector& pos,
    const TPureFloat& fDamageAreaSize) :
    m_id(id),
    m_pge(pge),
    m_connHandle(connHandle),
    m_refId(refId),
    m_fDamageAreaSize(fDamageAreaSize),
    m_objPrimary(nullptr),
    m_objSecondary(nullptr),
    m_bCreateSentToClients(true) /* irrelevant for this client-side ctor but we are client so yes it is sent :) */
{
    const auto itRef = m_explosionRefObjects.find(refId);
    if (itRef == m_explosionRefObjects.end())
    {
        getConsole().EOLn("Explosion ctor (client): no ref explosion found with id: %u", static_cast<uint32_t>(refId));
        throw std::runtime_error("Explosion ctor (client): no ref explosion found with id: " + std::to_string(refId));
    }

    const auto explRefData = itRef->second;
    assert(explRefData.m_pRefObj);

    m_fScalingPrimary = explRefData.m_pRefObj->getScaling().getX();
    m_fScalingSecondary = explRefData.m_pRefObj->getScaling().getX();

    m_objPrimary = m_pge.getPure().getObject3DManager().createCloned(*explRefData.m_pRefObj);
    m_objSecondary = m_pge.getPure().getObject3DManager().createCloned(*explRefData.m_pRefObj);
    if (!m_objPrimary || !m_objSecondary)
    {
        CConsole::getConsoleInstance("Explosion").EOLn("Explosion ctor (client): Failed to create a cloned object!");
        throw std::runtime_error("Explosion ctor (client): Failed to create a cloned object!");
    }

    m_objPrimary->Show();
    m_objPrimary->getPosVec() = pos;
    
    m_objSecondary->Hide();
    m_objSecondary->getPosVec() = pos;

    // NOT playing sound here but in the copy ctor! Reason: copy ctor is invoked anyway when we store the explosion in the container!
    //m_sndHandle = m_pge.getAudio().play3dSound(*explRefData.m_pSndExplosion, pos);
    //m_pge.getAudio().getAudioEngineCore().set3dSourceMinMaxDistance(m_sndHandle, SndExplosionDistMin, SndExplosionDistMax);
    //m_pge.getAudio().getAudioEngineCore().set3dSourceAttenuation(m_sndHandle, SoLoud::AudioSource::ATTENUATION_MODELS::LINEAR_DISTANCE, 1.f);
}

proofps_dd::Explosion::Explosion(const proofps_dd::Explosion& other) :
    m_id(other.m_id),
    m_pge(other.m_pge),
    m_connHandle(other.m_connHandle),
    m_refId(other.m_refId),
    m_fDamageAreaSize(other.m_fDamageAreaSize),
    m_fScalingPrimary(other.m_fScalingPrimary),
    m_fScalingSecondary(other.m_fScalingSecondary),
    m_bCreateSentToClients(other.m_bCreateSentToClients)
{
    const auto itRef = m_explosionRefObjects.find(m_refId);
    if (itRef == m_explosionRefObjects.end())
    {
        getConsole().EOLn("Explosion copy ctor: no ref explosion found with id: %u", static_cast<uint32_t>(m_refId));
        throw std::runtime_error("Explosion copy ctor: no ref explosion found with id: " + std::to_string(m_refId));
    }

    const auto explRefData = itRef->second;
    assert(explRefData.m_pRefObj);

    PureVector pos;

    if (other.m_objPrimary)
    {
        m_objPrimary = m_pge.getPure().getObject3DManager().createCloned(*explRefData.m_pRefObj);
        m_objPrimary->SetRenderingAllowed(other.m_objPrimary->isRenderingAllowed());
        m_objPrimary->getPosVec() = other.m_objPrimary->getPosVec();
        m_objPrimary->SetScaling(other.m_objPrimary->getScaling());
        pos = m_objPrimary->getPosVec();
        
        if (!m_objPrimary)
        {
            CConsole::getConsoleInstance("Explosion").EOLn("Explosion copy ctor: Failed to create a cloned object!");
            throw std::runtime_error("Explosion copy ctor: Failed to create a cloned object!");
        }
    }
    else
    {
        m_objPrimary = nullptr;
    }

    if (other.m_objSecondary)
    {
        m_objSecondary = m_pge.getPure().getObject3DManager().createCloned(*explRefData.m_pRefObj);
        m_objSecondary->SetRenderingAllowed(other.m_objSecondary->isRenderingAllowed());
        m_objSecondary->getPosVec() = other.m_objSecondary->getPosVec();
        m_objSecondary->SetScaling(other.m_objSecondary->getScaling());
        pos = m_objSecondary->getPosVec();

        if (!m_objSecondary)
        {
            CConsole::getConsoleInstance("Explosion").EOLn("Explosion copy ctor: Failed to create a cloned object!");
            throw std::runtime_error("Explosion copy ctor: Failed to create a cloned object!");
        }
    }
    else
    {
        m_objSecondary = nullptr;
    }

    m_sndHandle = m_pge.getAudio().play3dSound(*explRefData.m_pSndExplosion, pos);
    m_pge.getAudio().getAudioEngineCore().set3dSourceMinMaxDistance(m_sndHandle, SndExplosionDistMin, SndExplosionDistMax);
    m_pge.getAudio().getAudioEngineCore().set3dSourceAttenuation(m_sndHandle, SoLoud::AudioSource::ATTENUATION_MODELS::LINEAR_DISTANCE, 1.f);
}

//proofps_dd::Explosion& proofps_dd::Explosion::operator=(const proofps_dd::Explosion& other)
//{
//    m_id = other.m_id;
//    m_pge = other.m_pge;
//    m_connHandle = other.m_connHandle;
//    m_refId = other.m_refId;
//    m_fDamageAreaSize = other.m_fDamageAreaSize;
//    m_fScalingPrimary = other.m_fScalingPrimary;
//    m_fScalingSecondary = other.m_fScalingSecondary;
//    m_bCreateSentToClients = other.m_bCreateSentToClients;
//
//    const auto itRef = m_explosionRefObjects.find(m_refId);
//    if (itRef == m_explosionRefObjects.end())
//    {
//        getConsole().EOLn("Explosion copy assignment: no ref explosion found with id: %u", static_cast<uint32_t>(m_refId));
//        throw std::runtime_error("Explosion copy assignment: no ref explosion found with id: " + std::to_string(m_refId));
//    }
//
//    const auto explRefData = itRef->second;
//    assert(explRefData.m_pRefObj);
//
//    if (other.m_objPrimary)
//    {
//        m_objPrimary = m_gfx.getObject3DManager().createCloned(*explRefData.m_pRefObj);
//        m_objPrimary->SetRenderingAllowed(other.m_objPrimary->isRenderingAllowed());
//        m_objPrimary->getPosVec() = other.m_objPrimary->getPosVec();
//        m_objPrimary->SetScaling(other.m_objPrimary->getScaling());
//        if (!m_objPrimary)
//        {
//            CConsole::getConsoleInstance("Explosion").EOLn("Explosion copy ctor: Failed to create a cloned object!");
//            throw std::runtime_error("Explosion copy ctor: Failed to create a cloned object!");
//        }
//    }
//    else
//    {
//        m_objPrimary = nullptr;
//    }
//
//    if (other.m_objSecondary)
//    {
//        m_objSecondary = m_gfx.getObject3DManager().createCloned(*explRefData.m_pRefObj);
//        m_objSecondary->SetRenderingAllowed(other.m_objSecondary->isRenderingAllowed());
//        m_objSecondary->getPosVec() = other.m_objSecondary->getPosVec();
//        m_objSecondary->SetScaling(other.m_objSecondary->getScaling());
//        if (!m_objSecondary)
//        {
//            CConsole::getConsoleInstance("Explosion").EOLn("Explosion copy ctor: Failed to create a cloned object!");
//            throw std::runtime_error("Explosion copy ctor: Failed to create a cloned object!");
//        }
//    }
//    else
//    {
//        m_objSecondary = nullptr;
//    }
//
//    return *this;
//}

proofps_dd::Explosion::~Explosion()
{
    if (m_objPrimary)
    {
        m_pge.getPure().getObject3DManager().DeleteAttachedInstance(*m_objPrimary);
    }
    if (m_objSecondary)
    {
        m_pge.getPure().getObject3DManager().DeleteAttachedInstance(*m_objSecondary);
    }
    
    // no need to stop instance, keep playing! The Wav resource itself is in the ExplosionRef anyway.
    //m_pge.getAudio().stopSoundInstance(m_sndHandle);
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
    const Bullet::DamageAreaEffect& eDamageAreaEffect,
    const int& nDamage) const
{
    // fDistance is distance between explosion center and other entity's center,
    // m_fDamageAreaSize is radius
    assert(fDistance >= 0.f);
    assert(m_fDamageAreaSize > 0.f);

    if (eDamageAreaEffect == Bullet::DamageAreaEffect::Constant)
    {
        return fDistance <= m_fDamageAreaSize ? nDamage : 0.f;
    }

    return nDamage * std::max(0.f, (1 - (fDistance / m_fDamageAreaSize)));
}

bool proofps_dd::Explosion::shouldBeDeleted() const
{
    return !m_objPrimary && !m_objSecondary;
}


// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################


proofps_dd::Explosion::ExplosionId proofps_dd::Explosion::m_globalExplosionId = 0;
std::map<proofps_dd::ExplosionObjRefId, proofps_dd::Explosion::ExplosionRefData> proofps_dd::Explosion::m_explosionRefObjects;
