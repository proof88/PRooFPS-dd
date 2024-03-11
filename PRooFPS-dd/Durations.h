#pragma once

/*
    ###################################################################################
    Durations.h
    Durations for PRooFPS-dd
    Made by PR00F88, West Whiskhyll Entertainment
    2023
    ###################################################################################
*/

#include <chrono>  // requires cpp11

namespace proofps_dd
{

    struct Durations
    {
        unsigned int m_nFramesElapsedSinceLastDurationsReset;
        long long m_nGravityCollisionDurationUSecs;
        long long m_nActiveWindowStuffDurationUSecs;
        long long m_nUpdateWeaponsDurationUSecs;
        long long m_nUpdateBulletsDurationUSecs;
        long long m_nUpdateRespawnTimersDurationUSecs;
        long long m_nPickupAndRespawnItemsDurationUSecs;
        long long m_nUpdateGameModeDurationUSecs;
        long long m_nCameraMovementDurationUSecs;
        long long m_nSendUserUpdatesDurationUSecs;
        long long m_nFullOnGameRunningDurationUSecs;
        long long m_nHandleUserCmdMoveDurationUSecs;
        long long m_nFullOnPacketReceivedDurationUSecs;
        long long m_nFullRoundtripDurationUSecs;
        std::chrono::time_point<std::chrono::steady_clock> m_timeFullRoundtripStart;

        Durations()
        {
            reset();
        }

        void reset()
        {
            m_nFramesElapsedSinceLastDurationsReset = 0;
            m_nGravityCollisionDurationUSecs = 0;
            m_nActiveWindowStuffDurationUSecs = 0;
            m_nUpdateWeaponsDurationUSecs = 0;
            m_nUpdateBulletsDurationUSecs = 0;
            m_nUpdateRespawnTimersDurationUSecs = 0;
            m_nPickupAndRespawnItemsDurationUSecs = 0;
            m_nUpdateGameModeDurationUSecs = 0;
            m_nCameraMovementDurationUSecs = 0;
            m_nSendUserUpdatesDurationUSecs = 0;
            m_nFullOnGameRunningDurationUSecs = 0;
            m_nHandleUserCmdMoveDurationUSecs = 0;
            m_nFullOnPacketReceivedDurationUSecs = 0;
            m_nFullRoundtripDurationUSecs = 0;

            // m_timeFullRoundtripStart does not need to be reset, it is always updated properly by game logic
        }
    };

} // namespace proofps_dd