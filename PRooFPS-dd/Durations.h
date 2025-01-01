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
        std::chrono::microseconds::rep m_nGravityCollisionDurationUSecs;
        std::chrono::microseconds::rep m_nActiveWindowStuffDurationUSecs;
        std::chrono::microseconds::rep m_nUpdateWeaponsDurationUSecs;
        std::chrono::microseconds::rep m_nUpdateBulletsDurationUSecs;
        std::chrono::microseconds::rep m_nUpdateRespawnTimersDurationUSecs;
        std::chrono::microseconds::rep m_nPickupAndRespawnItemsDurationUSecs;
        std::chrono::microseconds::rep m_nUpdateGameModeDurationUSecs;
        std::chrono::microseconds::rep m_nCameraMovementDurationUSecs;
        std::chrono::microseconds::rep m_nSendUserUpdatesDurationUSecs;
        std::chrono::microseconds::rep m_nFullOnGameRunningDurationUSecs;
        std::chrono::microseconds::rep m_nHandleUserCmdMoveDurationUSecs;
        std::chrono::microseconds::rep m_nFullOnPacketReceivedDurationUSecs;
        std::chrono::microseconds::rep m_nFullRoundtripDurationUSecs;
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