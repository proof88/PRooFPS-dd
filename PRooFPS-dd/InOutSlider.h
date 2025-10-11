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

namespace proofps_dd
{

    /**
    * In/Out Slider GUI Element for PRooFPS-dd.
    * 
    * This class has the slide control logic only, does not know about the actual GUI element.
    * The idea is that this can be used as a base for a more specific class that encapsulates the
    * actual GUI element.
    * Therefore, this class can be used for sliding not only PureObject3D instances but also
    * Dear ImGui elements or anything else that can have 2D-coordinates.
    */
    class InOutSlider
    {
    public:

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
        virtual ~InOutSlider() {};

        InOutSlider(const InOutSlider&) = default;
        InOutSlider& operator=(const InOutSlider&) = default;
        InOutSlider(InOutSlider&&) = default;
        InOutSlider& operator=(InOutSlider&&) = default;

        /**
        * The user shall use this function to set the initial position of the slide-in animation.
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
            return m_posScreenFinish;
        }

        /**
        * @return True if we are sliding in, false if we are sliding out.
        *         Controlled by show() and hide() functions.
        */
        bool isSlidingIn() const
        {
            return m_bSlidingIn;
        }

        /**
        * Starts the slide-in animation from the Current position towards the Finish position.
        * A periodical call to update() is required to reach the Finish position.
        */
        void show()
        {
            m_posScreenTarget = m_posScreenFinish;
            m_bSlidingIn = true;
        }

        /**
        * Starts the slide-out animation from the Current position towards the Start position.
        * A periodical call to update() is required to reach the Start position.
        */
        void hide()
        {
            m_posScreenTarget = m_posScreenStart;
            m_bSlidingIn = false;
        }

        /**
        * Steps the slide animation between the Start and Finish positions.
        * Direction and speed depends on the Current position and the SlidingIn state.
        */
        void update()
        {
            // TODO
        }

    protected:

    private:

        TXY m_posScreenStart{};
        TXY m_posScreenFinish{};
        TXY m_posScreenTarget{};
        TXY m_posScreenCurrent{};
        bool m_bSlidingIn = true;

        // ---------------------------------------------------------------------------

    }; // class InOutSlider

} // namespace proofps_dd
