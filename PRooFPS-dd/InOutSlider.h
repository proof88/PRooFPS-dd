#pragma once

/*
    ###################################################################################
    InOutSlider.h
    In/Out Slider GUI Element for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2025
    ###################################################################################
*/

#include "CConsole.h"
#include "PURE/include/external/PureTypes.h"
#include "Audio/PgeAudio.h"

namespace proofps_dd
{

    /**
    * In/Out Slider GUI Element for PRooFPS-dd.
    * 
    * This class has the slide animation control logic only, does not know about the actual GUI element.
    * The idea is that this can be used as a base for a more specific class that encapsulates the
    * actual GUI element.
    * Therefore, this class can be used for sliding not only PureObject3D instances but also
    * Dear ImGui elements or anything else that can have 2D-coordinates.
    * 
    * See the animation logic described at enum AnimState.
    */
    class InOutSlider
    {
    public:

        /**
        * Initially, the animation is sitting in Finished state.
        * The animation can be triggered at the beginning using show().
        *
        * The animation is transitioning through states by periodic calls to update().
        *
        * The animation can be instructed to go back to Finished state using hide(), but
        * it will follow the state machine flow to do that gradually.
        *
        * If a TimeOut is set, then between the SlidingIn and SlidingOut states, the
        * animation will stay in WaitingForTimeout state for the TimeOut duration, otherwise
        * the animation will stay in WaitingForTimeout until an explicit call to hide().
        *
        * Summary:
        * Therefore, the recommended way of controlling the animation is to always do periodical
        * calls to update(), no need to explicitly check for states. The only additional explicit
        * call that is needed only once is show(). If a TimeOut is set, no need to explicitly call hide().
        *
        *                                                  hide()
        *                              >--------------------->------------------------------>
        *                              |                                                     ¡
        *                              |          update():              update(): timeout,  |         update():
        *                   show()     ^      FinishPos reached                hide()        ¡    FinishPos reached
        *     >-> Finished -------> SlidingIn ------------> WaitingForTimeout -------> SlidingOut --------------->
        *     ^   ^      ¡          ^      ¡ ^                ^      ¡                 ^      ¡ ¡                 ¡
        *     |   |hide()|          |show()| |                |show()|                 |hide()| |                 |
        *     |   ^------<          ^------< |                ^------<                 ^------< |                 |
        *     |                              |                           show()                 |                 |
        *     |                              ^--------------<-----------------<------------------<                 |
        *     |                                                                                                   |
        *     ^----------<---------------<---------------<-----------------<--------------------<-----------------<
        */
        enum class AnimState
        {
            Finished,
            SlidingIn,
            WaitingForTimeout,
            SlidingOut
        };

        static const char* getLoggerModuleName()
        {
            return "InOutSlider";
        }

        // ---------------------------------------------------------------------------

        CConsole& getConsole() const
        {
            return CConsole::getConsoleInstance(getLoggerModuleName());
        }

        InOutSlider() = default;

        virtual ~InOutSlider()
        {
            if (m_sndPlayedInSlidingInState)
            {
                m_sndPlayedInSlidingInState->stop();
                delete m_sndPlayedInSlidingInState;
            }
        };

        InOutSlider(const InOutSlider&) = default;
        InOutSlider& operator=(const InOutSlider&) = default;
        InOutSlider(InOutSlider&&) = default;
        InOutSlider& operator=(InOutSlider&&) = default;

        /**
        * The user shall use this function to set the initial position of the slide-in animation.
        * This Start position is the initial position in Finished and SlidingIn states.
        * This Start position is the ending position of the slide-out animation.
        *
        * @return The initial position of the GUI element, from where it will slide in towards the Finish position.
        */
        TXY& getScreenStartPos()
        {
            return m_posScreenStart;
        }

        /**
        * @return The initial position of the GUI element, from where it will slide in towards the Finish position.
        */
        const TXY& getScreenStartPos() const
        {
            return m_posScreenStart;
        }

        /**
        * The user shall use this function to set the ending position of the slide-in animation.
        * This Finish position is the initial position in WaitingForTimeout and SlidingOut states.
        * This Finish position is the initial position of the slide-out animation.
        *
        * @return The ending position of the GUI element, to where it will slide in from the Start position.
        */
        TXY& getScreenFinishPos()
        {
            return m_posScreenFinish;
        }

        /**
        * @return The ending position of the GUI element, to where it will slide in from the Start position.
        */
        const TXY& getScreenFinishPos() const
        {
            return m_posScreenFinish;
        }

        /**
        * @return The current position of the GUI element, controlled by update(), somewhere between the Start and Finish positions.
        */
        const TXY& getScreenCurrentPos() const
        {
            return m_posScreenCurrent;
        }

        /**
        * @return The current animation state, controlled by show(), hide() and update().
        */
        const AnimState& getState() const
        {
            return m_state;
        }

        /**
        * @return The duration of waiting in milliseconds in WaitingForTimeout state before transitioning to SlidingOut state.
        */
        const std::chrono::milliseconds::rep& getTimeoutInWaitingState() const
        {
            return m_durationTimeoutMillisecsInWaitingState;
        }

        /**
        * Sets the duration for waiting in WaitingForTimeout state before transitioning to SlidingOut state.
        * 
        * Setting 0 means waiting in WaitingForTimeout state without time limit, until an explicit call to hide().
        * 
        * @param millisecs The duration expressed in milliseconds for waiting in WaitingForTimeout state.
        */
        void setTimeoutInWaitingState(std::chrono::milliseconds::rep millisecs)
        {
            if (millisecs < 0)
            {
                getConsole().EOLn("InOutSlider::%s(): ERROR: negative input!", __func__);
                return;
            }

            m_durationTimeoutMillisecsInWaitingState = millisecs;
        }

        void loadSoundForSlidingIn(pge_audio::PgeAudio& audio, const char* szFilename)
        {
            if (m_sndPlayedInSlidingInState)
            {
                m_sndPlayedInSlidingInState->stop();
                delete m_sndPlayedInSlidingInState;
                m_sndPlayedInSlidingInState = nullptr;
            }

            m_sndPlayedInSlidingInState = new SoLoud::Wav();
            // new would had thrown in case of failure, no need to check
            audio.loadSound(*m_sndPlayedInSlidingInState, szFilename);
            if (m_sndPlayedInSlidingInState->getLength() == 0.f)
            {
                getConsole().EOLn("InOutSlider::%s(): ERROR: failed to load sound: %s!", __func__, szFilename);
            }
        }

        /**
        * Starts or resumes the slide-in animation from the Current position towards the Finish position.
        * No visible effect if the animation is already in SlidingIn or WaitingForTimeout state.
        *
        * A periodical call to update() is required to eventually reach the Finish position.
        * It is advised to call show() only when you want to start the slide-in animation.
        */
        void show(pge_audio::PgeAudio& audio)
        {
            switch (m_state)
            {
            case AnimState::Finished:
            case AnimState::SlidingIn:
            case AnimState::SlidingOut:
                stateEnter(audio, AnimState::SlidingIn, false /* forceStopAudio */);
                break;
            case AnimState::WaitingForTimeout:
                stateEnter(audio, AnimState::WaitingForTimeout, false /* forceStopAudio */);
                break;
            default:
                getConsole().EOLn("InOutSlider::%s(): ERROR: unhandled current state: %d!", __func__, m_state);
            }
        }

        /**
        * Starts or resumes the slide-out animation from the Current position towards the Start position.
        * No visible effect if the animation is already in SlidingOut or Finished state.
        *
        * A periodical call to update() is required to eventually reach the Start position.
        * It is advised to call hide() only when you want to start the slide-out animation.
        * 
        * If current state is WaitingForTimeout, we are transitioning to SlidingOut even if a non-zero
        * timeout duration is not yet elapsed.
        * 
        * @param forceStopAudio Set it to true to immediately stop the optionally playing audio that was previously
        *                       loaded by loadSoundForSlidingIn().
        */
        void hide(pge_audio::PgeAudio& audio, bool forceStopAudio)
        {
            switch (m_state)
            {
            case AnimState::WaitingForTimeout:
            case AnimState::SlidingIn:
            case AnimState::SlidingOut:
                stateEnter(audio, AnimState::SlidingOut, forceStopAudio);
                break;
            case AnimState::Finished:
                stateEnter(audio, AnimState::Finished, false);
                break;
            default:
                getConsole().EOLn("InOutSlider::%s(): ERROR: unhandled current state: %d!", __func__, m_state);
            }
        }

        /**
        * Steps the slide animation between the Start and Finish positions.
        * Direction and speed depends on the Current position and the SlidingIn state.
        * Derived class may extend this function by using the updated Current position.
        */
        virtual void update(pge_audio::PgeAudio& audio)
        {
            stateUpdate(audio);
        }

        /**
        * Frees up any loaded resource, like the sound that can be optionally loaded by loadSoundForSlidingIn().
        * Derived classes must extend this method by freeing up their loaded resources as well.
        */
        virtual void clear()
        {
            if (m_sndPlayedInSlidingInState)
            {
                m_sndPlayedInSlidingInState->stop();
                delete m_sndPlayedInSlidingInState;
                m_sndPlayedInSlidingInState = nullptr;
            }
        }

    protected:
        /**
        * Invoked by stateEnter() after a successful state transition.
        */
        virtual void stateEntered(const AnimState& /*oldState*/, const AnimState& /*newState*/)
        {};

    private:

        TXY m_posScreenStart{};
        TXY m_posScreenFinish{};
        TXY m_posScreenTarget{};
        TXY m_posScreenCurrent{};
        AnimState m_state{ AnimState::Finished };
        std::chrono::milliseconds::rep m_durationTimeoutMillisecsInWaitingState{ 0 };
        std::chrono::time_point<std::chrono::steady_clock> m_timeEnteredWaitingState;
        SoLoud::Wav* m_sndPlayedInSlidingInState{ nullptr };

        // ---------------------------------------------------------------------------

        /**
        * Expected to be invoked only for events, such as explicit call to show(), hide(), or when update() detects an event.
        */
        bool stateEnter(pge_audio::PgeAudio& audio, const AnimState& newState, bool forceStopAudio)
        {
            //getConsole().EOLn("InOutSlider::%s(): %d -> %d", __func__, m_state, newState);

            if (forceStopAudio && m_sndPlayedInSlidingInState)
            {
                m_sndPlayedInSlidingInState->stop();
            }
            
            switch (newState)
            {
            case AnimState::Finished:
                if ((m_state != newState /* transition to same state is always allowed, e.g.: show(), hide() */) &&
                    (m_state != AnimState::SlidingOut))
                {
                    getConsole().EOLn("InOutSlider::%s(): ERROR: invalid state transition: %d -> %d!", __func__, m_state, newState);
                    return false;
                }
                m_posScreenTarget = m_posScreenFinish;
                break;
            case AnimState::SlidingIn:
                if ((m_state != newState /* transition to same state is always allowed, e.g.: show(), hide() */) &&
                    (m_state != AnimState::Finished) &&
                    (m_state != AnimState::SlidingOut))
                {
                    getConsole().EOLn("InOutSlider::%s(): ERROR: invalid state transition: %d -> %d!", __func__, m_state, newState);
                    return false;
                }
                m_posScreenTarget = m_posScreenFinish;
                if (m_state == AnimState::Finished)
                {
                    m_posScreenCurrent = m_posScreenStart;
                    if (m_sndPlayedInSlidingInState)
                    {
                        audio.playSound(*m_sndPlayedInSlidingInState);
                    }
                }
                break;
            case AnimState::WaitingForTimeout:
                if ((m_state != newState /* transition to same state is always allowed, e.g.: show(), hide() */) &&
                    (m_state != AnimState::SlidingIn))
                {
                    getConsole().EOLn("InOutSlider::%s(): ERROR: invalid state transition: %d -> %d!", __func__, m_state, newState);
                    return false;
                }
                m_posScreenTarget = m_posScreenStart;
                m_timeEnteredWaitingState = std::chrono::steady_clock::now();
                break;
            case AnimState::SlidingOut:
                if ((m_state != newState /* transition to same state is always allowed, e.g.: show(), hide() */) &&
                    (m_state != AnimState::SlidingIn) &&
                    (m_state != AnimState::WaitingForTimeout))
                {
                    getConsole().EOLn("InOutSlider::%s(): ERROR: invalid state transition: %d -> %d!", __func__, m_state, newState);
                    return false;
                }
                m_posScreenTarget = m_posScreenStart;
                break;
            default:
                getConsole().EOLn("InOutSlider::%s(): ERROR: unhandled new state: %d!", __func__, newState);
                return false;
            }

            const AnimState oldState = m_state;
            m_state = newState;

            stateEntered(oldState, newState);

            return true;
        }
        
        /**
        * Expected to be invoked periodically.
        */
        void stateUpdate(pge_audio::PgeAudio& audio)
        {
            constexpr float fSmoothSpeed = 10.f;
            constexpr float fEpsilon = 0.1f;
            switch (m_state)
            {
            case AnimState::Finished:
                break;
            case AnimState::SlidingIn:
                m_posScreenCurrent.x = PFL::smooth(m_posScreenCurrent.x, m_posScreenTarget.x, fSmoothSpeed, fEpsilon);
                m_posScreenCurrent.y = PFL::smooth(m_posScreenCurrent.y, m_posScreenTarget.y, fSmoothSpeed, fEpsilon);
                if ((m_posScreenCurrent.x == m_posScreenTarget.x) &&
                    (m_posScreenCurrent.y == m_posScreenTarget.y))
                {
                    stateEnter(audio, AnimState::WaitingForTimeout, false /* forceStopAudio */);
                }
                break;
            case AnimState::WaitingForTimeout:
                if (m_durationTimeoutMillisecsInWaitingState != 0)
                {
                    const auto nTimeElapsedSinceEnteredWaitingStateMillisecs =
                        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_timeEnteredWaitingState).count();
                    if (nTimeElapsedSinceEnteredWaitingStateMillisecs >= m_durationTimeoutMillisecsInWaitingState)
                    {
                        stateEnter(audio, AnimState::SlidingOut, false /* forceStopAudio */);
                    }
                }
                break;
            case AnimState::SlidingOut:
                m_posScreenCurrent.x = PFL::smooth(m_posScreenCurrent.x, m_posScreenTarget.x, fSmoothSpeed, fEpsilon);
                m_posScreenCurrent.y = PFL::smooth(m_posScreenCurrent.y, m_posScreenTarget.y, fSmoothSpeed, fEpsilon);
                if ((m_posScreenCurrent.x == m_posScreenTarget.x) &&
                    (m_posScreenCurrent.y == m_posScreenTarget.y))
                {
                    stateEnter(audio, AnimState::Finished, false /* forceStopAudio */);
                }
                break;
            default:
                getConsole().EOLn("InOutSlider::%s(): ERROR: unhandled current state: %d!", __func__, m_state);
            }
        }

    }; // class InOutSlider

} // namespace proofps_dd
