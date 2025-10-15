#pragma once

/*
    ###################################################################################
    PureObject3dInOutSlider.h
    PureObject3d In/Out Slider for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2025
    ###################################################################################
*/

#include "InOutSlider.h"
#include "PURE/include/external/Object3D/PureObject3DManager.h"

namespace proofps_dd
{

    class PureObject3dInOutSlider : public InOutSlider
    {
    public:

        static const char* getLoggerModuleName()
        {
            return "PureObject3dInOutSlider";
        }

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const
        {
            return CConsole::getConsoleInstance(getLoggerModuleName());
        }

        PureObject3dInOutSlider() = default;

        virtual ~PureObject3dInOutSlider() override
        {
            clear();
        }

        PureObject3dInOutSlider(const PureObject3dInOutSlider&) = delete;
        PureObject3dInOutSlider& operator=(const PureObject3dInOutSlider&) = delete;
        PureObject3dInOutSlider(PureObject3dInOutSlider&&) = delete;
        PureObject3dInOutSlider& operator=(PureObject3dInOutSlider&&) = delete;

        virtual void update(pge_audio::PgeAudio& audio) override
        {
            InOutSlider::update(audio);
            if (m_pObj)
            {
                m_pObj->getPosVec().SetX(getScreenCurrentPos().x);
                m_pObj->getPosVec().SetY(getScreenCurrentPos().y);
            }
        }

        virtual void clear() override
        {
            if (m_pObj)
            {
                delete m_pObj;  // this triggers remove from Object3dManager too
                m_pObj = nullptr;
            }
            InOutSlider::clear();
        }

        const PureObject3D* getPureObject() const
        {
            return m_pObj;
        }

        const PureObject3D* createPureObject(
            PureTextureManager& textureManager,
            PureObject3DManager& objectManager,
            const char* textureFilename
        )
        {
            PureTexture* const tex = textureManager.createFromFile(textureFilename);
            if (!tex || (tex->getWidth() == 0u) || (tex->getHeight() == 0u))
            {
                getConsole().EOLn("PureObject3dInOutSlider::%s(%s): ERROR: tex is null or has zero-size!", __func__, textureFilename);
                return nullptr;
            }

            m_pObj = objectManager.createPlane(
                static_cast<TPureFloat>(tex->getWidth()),
                static_cast<TPureFloat>(tex->getHeight()));
            if (!m_pObj)
            {
                // no need to delete the texture, TextureManager will take care of it whenever it is cleaned up
                getConsole().EOLn("PureObject3dInOutSlider::%s(%s): ERROR: failed to create object3d!", __func__, textureFilename);
                return nullptr;
            }

            m_pObj->SetStickedToScreen(true);
            m_pObj->SetDoubleSided(true);
            m_pObj->SetTestingAgainstZBuffer(false);
            m_pObj->SetLit(false);
            m_pObj->getMaterial().setTexture(tex);
            
            m_pObj->Hide();

            return m_pObj;
        }

        bool setBlendFuncs(TPURE_BLENDFACTOR src, TPURE_BLENDFACTOR dst)
        {
            if (!m_pObj)
            {
                getConsole().EOLn("PureObject3dInOutSlider::%s(): ERROR: null object3d!", __func__);
                return false;
            }

            return m_pObj->getMaterial(false).setBlendFuncs(src, dst);
        }

        void setScaling(TPureFloat scaling)
        {
            if (!m_pObj)
            {
                getConsole().EOLn("PureObject3dInOutSlider::%s(): ERROR: null object3d!", __func__);
                return;
            }

            m_pObj->SetScaling(scaling);
        }

        PureMaterial* getMaterial()
        {
            return m_pObj ? &(m_pObj->getMaterial()) : nullptr;
        }

        const PureMaterial* getMaterial() const
        {
            return m_pObj ? &(m_pObj->getMaterial()) : nullptr;
        }

    protected:
        virtual void stateEntered(const AnimState& /*oldState*/, const AnimState& newState) override
        {
            switch (newState)
            {
            case AnimState::Finished:
                if (m_pObj)
                {
                    m_pObj->Hide();
                }
                break;
            case AnimState::SlidingIn:
                if (m_pObj)
                {
                    m_pObj->Show();
                }
                break;
            case AnimState::WaitingForTimeout:
            case AnimState::SlidingOut:
                // no-op
                break;
            default:
                getConsole().EOLn("PureObject3dInOutSlider::%s(): ERROR: unhandled new state: %d!", __func__, newState);
            }
        };

    private:

        // ---------------------------------------------------------------------------

        PureObject3D* m_pObj = nullptr;

    }; // class PureObject3dInOutSlider

} // namespace proofps_dd
