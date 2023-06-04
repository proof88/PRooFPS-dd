#pragma once

/*
    ###################################################################################
    Strafe.h
    Strafe struct for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include <cstdint>
#include <ostream>

namespace proofps_dd
{

    enum class Strafe : std::uint8_t
    {
        NONE = 0,
        LEFT,
        RIGHT
    };

} // namespace proofps_dd

std::ostream& operator<< (std::ostream& os, const proofps_dd::Strafe& obj);
