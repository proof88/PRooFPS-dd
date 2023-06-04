/*
    ###################################################################################
    Strafe.cpp
    Strafe struct for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include "stdafx.h"  // PCH

#include "Strafe.h"

std::ostream& operator<< (std::ostream& os, const proofps_dd::Strafe& obj)
{
    os << static_cast<uint32_t>(obj);
    return os;
}
