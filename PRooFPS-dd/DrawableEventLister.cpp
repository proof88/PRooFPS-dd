/*
    ###################################################################################
    DrawableEventLister.cpp
    Event lister class for PRooFPS-dd, with implemented draw function
    Made by PR00F88, West Whiskhyll Entertainment
    2024
    ###################################################################################
*/

#include "stdafx.h"  // PCH
#include "DrawableEventLister.h"

// ############################### PUBLIC ################################

proofps_dd::DrawableEventLister::DrawableEventLister(const unsigned int& nEventTimeLimitSecs, const size_t& nEventCountLimit, const EventLister::Orientation& eOrientation) :
    EventLister(nEventTimeLimitSecs, nEventCountLimit, eOrientation)
{
}

void proofps_dd::DrawableEventLister::draw()
{
    // TODO: cannot be implemented until drawTextHighlighted() is moved from GUI.cpp to separate unit.
}


// ############################## PROTECTED ##############################


// ############################### PRIVATE ###############################
