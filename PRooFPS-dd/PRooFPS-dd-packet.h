#pragma once

/*
    ###################################################################################
    PRooFPS-dd-packet.h
    Network packets defined for PRooFPS-dd.
    Made by PR00F88
    2022
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/

#include <array>
#include <string>
#include <unordered_map>

#include "../../PGE/PGE/Network/PgePacket.h"
#include "../../PGE/PGE/Weapons/WeaponManager.h" // for BulletId

#include "Strafe.h"

namespace proofps_dd
{

    enum class PRooFPSappMsgId : pge_network::TPgeMsgAppMsgId  /* underlying type should be same as type of pge_network::MsgApp::msgId */
    {
        UserSetupFromServer = 0,
        UserCmdFromClient,
        UserUpdateFromServer,
        BulletUpdateFromServer,
        MapItemUpdateFromServer,
        WpnUpdateFromServer,
        CurrentWpnUpdateFromServer,
        LastMsgId
    };

    struct PRooFPSappMsgId2ZStringPair
    {
        PRooFPSappMsgId msgId;
        const char* const zstring;
    };

    // this way of defining std::array makes sure code cannot compile if we forget to align the array after changing PRooFPSappMsgId
    constexpr std::array<PRooFPSappMsgId2ZStringPair, static_cast<size_t>(PRooFPSappMsgId::LastMsgId)> MapMsgAppId2String
    { {
         {PRooFPSappMsgId::UserSetupFromServer,         "MsgUserSetupFromServer"},
         {PRooFPSappMsgId::UserCmdFromClient,           "MsgUserCmdFromClient"},
         {PRooFPSappMsgId::UserUpdateFromServer,        "MsgUserUpdateFromServer"},
         {PRooFPSappMsgId::BulletUpdateFromServer,      "MsgBulletUpdateFromServer"},
         {PRooFPSappMsgId::MapItemUpdateFromServer,     "MsgMapItemUpdateFromServer"},
         {PRooFPSappMsgId::WpnUpdateFromServer,         "MsgWpnUpdateFromServer"},
         {PRooFPSappMsgId::CurrentWpnUpdateFromServer,  "MsgCurrentWpnUpdateFromServer"}
    } };

    // server -> self (inject) and clients
    // sent to all clients after the connecting client has been accepted by server
    struct MsgUserSetupFromServer
    {
        static const PRooFPSappMsgId id = PRooFPSappMsgId::UserSetupFromServer;
        static const uint8_t nUserNameMaxLength = 64;
        static const uint8_t nMapFilenameMaxLength = 64;

        static bool initPkt(
            pge_network::PgePacket& pkt,
            const pge_network::PgeNetworkConnectionHandle& connHandleServerSide,
            bool bCurrentClient,
            const std::string& sUserName,
            const std::string& sIpAddress,
            const std::string& sMapFilename)
        {
            // although preparePktMsgAppFill() does runtime check, we should fail already at compile-time if msg is too big!
            static_assert(sizeof(MsgUserSetupFromServer) <= pge_network::MsgApp::nMaxMessageLength, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::initPktMsgApp(pkt, connHandleServerSide);

            uint8_t* const pMsgAppData = pge_network::PgePacket::preparePktMsgAppFill(
                pkt, static_cast<pge_network::TPgeMsgAppMsgId>(id), sizeof(MsgUserSetupFromServer));
            if (!pMsgAppData)
            {
                return false;
            }

            proofps_dd::MsgUserSetupFromServer& msgUserSetup = reinterpret_cast<proofps_dd::MsgUserSetupFromServer&>(*pMsgAppData);
            msgUserSetup.m_bCurrentClient = bCurrentClient;
            strncpy_s(msgUserSetup.m_szUserName, nUserNameMaxLength, sUserName.c_str(), sUserName.length());
            strncpy_s(msgUserSetup.m_szIpAddress, sizeof(msgUserSetup.m_szIpAddress), sIpAddress.c_str(), sIpAddress.length());
            strncpy_s(msgUserSetup.m_szMapFilename, sizeof(msgUserSetup.m_szMapFilename), sMapFilename.c_str(), sMapFilename.length());

            return true;
        }

        bool m_bCurrentClient;
        char m_szUserName[nUserNameMaxLength];
        char m_szIpAddress[pge_network::MsgUserConnectedServerSelf::nIpAddressMaxLength];
        char m_szMapFilename[nMapFilenameMaxLength];
    };  // struct MsgUserSetupFromServer

    // clients -> server + server self (inject)
    // MsgUserCmdFromClient messages are sent from clients to server, so server will do sg and then update all the clients with MsgUserUpdateFromServer
    struct MsgUserCmdFromClient
    {
        static const PRooFPSappMsgId id = PRooFPSappMsgId::UserCmdFromClient;

        static bool initPkt(
            pge_network::PgePacket& pkt)
        {
            // although preparePktMsgAppFill() does runtime check, we should fail already at compile-time if msg is too big!
            static_assert(sizeof(MsgUserCmdFromClient) <= pge_network::MsgApp::nMaxMessageLength, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::initPktMsgApp(pkt, 0u /*m_connHandleServerSide is ignored in this message*/);

            uint8_t* const pMsgAppData = pge_network::PgePacket::preparePktMsgAppFill(
                pkt, static_cast<pge_network::TPgeMsgAppMsgId>(id), sizeof(MsgUserCmdFromClient));
            if (!pMsgAppData)
            {
                return false;
            }

            proofps_dd::MsgUserCmdFromClient& msgUserCmdMove = reinterpret_cast<proofps_dd::MsgUserCmdFromClient&>(*pMsgAppData);
            msgUserCmdMove.m_bShouldSend = false;
            msgUserCmdMove.m_fPlayerAngleY = -1.f;

            return true;
        }

        static void setKeybd(
            pge_network::PgePacket& pkt,
            const Strafe& strafe,
            bool bJump,
            bool bSwitchToRunning,
            bool bRequestReload,
            unsigned char cWeaponSwitch)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            pge_network::MsgApp* const pMsgApp = reinterpret_cast<pge_network::MsgApp*>(pge_network::PgePacket::getMessageAppArea(pkt).cData);
            proofps_dd::MsgUserCmdFromClient& msgUserCmdMove = reinterpret_cast<proofps_dd::MsgUserCmdFromClient&>(pMsgApp->cMsgData);
            msgUserCmdMove.m_bShouldSend = true;
            msgUserCmdMove.m_strafe = strafe;
            msgUserCmdMove.m_bJumpAction = bJump;
            msgUserCmdMove.m_bSendSwitchToRunning = bSwitchToRunning;
            msgUserCmdMove.m_bRequestReload = bRequestReload;
            msgUserCmdMove.m_cWeaponSwitch = cWeaponSwitch;
        }

        static unsigned char getWeaponSwitch(const pge_network::PgePacket& pkt)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            const pge_network::MsgApp* const pMsgApp = reinterpret_cast<const pge_network::MsgApp*>(pge_network::PgePacket::getMessageAppArea(pkt).cData);
            const proofps_dd::MsgUserCmdFromClient& msgUserCmdMove = reinterpret_cast<const proofps_dd::MsgUserCmdFromClient&>(pMsgApp->cMsgData);
            return msgUserCmdMove.m_cWeaponSwitch;
        }

        static void SetWeaponSwitch(pge_network::PgePacket& pkt, unsigned char cTargetWpnKey)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            pge_network::MsgApp* const pMsgApp = reinterpret_cast<pge_network::MsgApp*>(pge_network::PgePacket::getMessageAppArea(pkt).cData);
            proofps_dd::MsgUserCmdFromClient& msgUserCmdMove = reinterpret_cast<proofps_dd::MsgUserCmdFromClient&>(pMsgApp->cMsgData);
            msgUserCmdMove.m_bShouldSend = true;
            msgUserCmdMove.m_cWeaponSwitch = cTargetWpnKey;
        }

        static bool getReloadRequest(const pge_network::PgePacket& pkt)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            const pge_network::MsgApp* const pMsgApp = reinterpret_cast<const pge_network::MsgApp*>(pge_network::PgePacket::getMessageAppArea(pkt).cData);
            const proofps_dd::MsgUserCmdFromClient& msgUserCmdMove = reinterpret_cast<const proofps_dd::MsgUserCmdFromClient&>(pMsgApp->cMsgData);
            return msgUserCmdMove.m_bRequestReload;
        }

        static void setMouse(
            pge_network::PgePacket& pkt,
            bool bShootAction)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            pge_network::MsgApp* const pMsgApp = reinterpret_cast<pge_network::MsgApp*>(pge_network::PgePacket::getMessageAppArea(pkt).cData);
            proofps_dd::MsgUserCmdFromClient& msgUserCmdMove = reinterpret_cast<proofps_dd::MsgUserCmdFromClient&>(pMsgApp->cMsgData);
            msgUserCmdMove.m_bShouldSend = true;
            msgUserCmdMove.m_bShootAction = bShootAction;
        }

        static void setAngleY(
            pge_network::PgePacket& pkt,
            TPureFloat fPlayerAngleY)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            pge_network::MsgApp* const pMsgApp = reinterpret_cast<pge_network::MsgApp*>(pge_network::PgePacket::getMessageAppArea(pkt).cData);
            proofps_dd::MsgUserCmdFromClient& msgUserCmdMove = reinterpret_cast<proofps_dd::MsgUserCmdFromClient&>(pMsgApp->cMsgData);
            msgUserCmdMove.m_bShouldSend = true;
            msgUserCmdMove.m_fPlayerAngleY = fPlayerAngleY;
        }

        static void setWpnAngles(
            pge_network::PgePacket& pkt,
            TPureFloat fWpnAngleY,
            TPureFloat fWpnAngleZ)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            pge_network::MsgApp* const pMsgApp = reinterpret_cast<pge_network::MsgApp*>(pge_network::PgePacket::getMessageAppArea(pkt).cData);
            proofps_dd::MsgUserCmdFromClient& msgUserCmdMove = reinterpret_cast<proofps_dd::MsgUserCmdFromClient&>(pMsgApp->cMsgData);
            msgUserCmdMove.m_bShouldSend = true;
            msgUserCmdMove.m_fWpnAngleY = fWpnAngleY;
            msgUserCmdMove.m_fWpnAngleZ = fWpnAngleZ;
        }

        static bool shouldSend(
            const pge_network::PgePacket& pkt)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            const pge_network::MsgApp* const pMsgApp = reinterpret_cast<const pge_network::MsgApp*>(pge_network::PgePacket::getMessageAppArea(pkt).cData);
            const proofps_dd::MsgUserCmdFromClient& msgUserCmdMove = reinterpret_cast<const proofps_dd::MsgUserCmdFromClient&>(pMsgApp->cMsgData);
            return msgUserCmdMove.m_bShouldSend;
        }

        bool m_bShouldSend;
        Strafe m_strafe;
        bool m_bJumpAction;
        bool m_bSendSwitchToRunning;
        bool m_bRequestReload;
        unsigned char m_cWeaponSwitch;
        bool m_bShootAction;
        TPureFloat m_fPlayerAngleY;
        TPureFloat m_fWpnAngleY;
        TPureFloat m_fWpnAngleZ;
    };  // struct MsgUserCmdFromClient

    // server -> self (inject) and clients
    // sent regularly to all clients
    struct MsgUserUpdateFromServer
    {
        static const PRooFPSappMsgId id = PRooFPSappMsgId::UserUpdateFromServer;

        static bool initPkt(
            pge_network::PgePacket& pkt,
            const pge_network::PgeNetworkConnectionHandle& connHandleServerSide,
            const TPureFloat x,
            const TPureFloat y,
            const TPureFloat z,
            TPureFloat fPlayerAngleY,
            TPureFloat fWpnAngleY,
            TPureFloat fWpnAngleZ,
            int nHealth,
            bool bRespawn,
            int nFrags,
            int nDeaths)
        {
            // although preparePktMsgAppFill() does runtime check, we should fail already at compile-time if msg is too big!
            static_assert(sizeof(MsgUserUpdateFromServer) <= pge_network::MsgApp::nMaxMessageLength, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::initPktMsgApp(pkt, connHandleServerSide);

            uint8_t* const pMsgAppData = pge_network::PgePacket::preparePktMsgAppFill(
                pkt, static_cast<pge_network::TPgeMsgAppMsgId>(id), sizeof(MsgUserUpdateFromServer));
            if (!pMsgAppData)
            {
                return false;
            }

            proofps_dd::MsgUserUpdateFromServer& msgUserCmdUpdate = reinterpret_cast<proofps_dd::MsgUserUpdateFromServer&>(*pMsgAppData);
            msgUserCmdUpdate.m_pos.x = x;
            msgUserCmdUpdate.m_pos.y = y;
            msgUserCmdUpdate.m_pos.z = z;
            msgUserCmdUpdate.m_fPlayerAngleY = fPlayerAngleY;
            msgUserCmdUpdate.m_fWpnAngleY = fWpnAngleY;
            msgUserCmdUpdate.m_fWpnAngleZ = fWpnAngleZ;
            msgUserCmdUpdate.m_nHealth = nHealth;
            msgUserCmdUpdate.m_bRespawn = bRespawn;
            msgUserCmdUpdate.m_nFrags = nFrags;
            msgUserCmdUpdate.m_nDeaths = nDeaths;

            return true;
        }

        TXYZ m_pos;
        TPureFloat m_fPlayerAngleY;
        TPureFloat m_fWpnAngleY;
        TPureFloat m_fWpnAngleZ;
        int m_nHealth;
        bool m_bRespawn;
        int m_nFrags;
        int m_nDeaths;
    };  // struct MsgUserUpdateFromServer

    // server -> clients
    // sent to all clients
    struct MsgBulletUpdateFromServer
    {
        static const PRooFPSappMsgId id = PRooFPSappMsgId::BulletUpdateFromServer;

        static bool initPkt(
            pge_network::PgePacket& pkt,
            const pge_network::PgeNetworkConnectionHandle& connHandleServerSide,
            const Bullet::BulletId bulletId,
            const TPureFloat px,
            const TPureFloat py,
            const TPureFloat pz,
            const TPureFloat ax,
            const TPureFloat ay,
            const TPureFloat az,
            const TPureFloat sx,
            const TPureFloat sy,
            const TPureFloat sz)
        {
            // although preparePktMsgAppFill() does runtime check, we should fail already at compile-time if msg is too big!
            static_assert(sizeof(MsgBulletUpdateFromServer) <= pge_network::MsgApp::nMaxMessageLength, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::initPktMsgApp(pkt, connHandleServerSide, pge_network::PgePacket::AutoFill::NONE);

            uint8_t* const pMsgAppData = pge_network::PgePacket::preparePktMsgAppFill(
                pkt, static_cast<pge_network::TPgeMsgAppMsgId>(id), sizeof(MsgBulletUpdateFromServer));
            if (!pMsgAppData)
            {
                return false;
            }

            proofps_dd::MsgBulletUpdateFromServer& msgBulletUpdate = reinterpret_cast<proofps_dd::MsgBulletUpdateFromServer&>(*pMsgAppData);
            msgBulletUpdate.m_bulletId = bulletId;
            msgBulletUpdate.m_pos.x = px;
            msgBulletUpdate.m_pos.y = py;
            msgBulletUpdate.m_pos.z = pz;
            msgBulletUpdate.m_angle.x = ax;
            msgBulletUpdate.m_angle.y = ay;
            msgBulletUpdate.m_angle.z = az;
            msgBulletUpdate.m_size.x = sx;
            msgBulletUpdate.m_size.y = sy;
            msgBulletUpdate.m_size.z = sz;
            msgBulletUpdate.m_bDelete = false;

            return true;
        }

        // this version doesnt call memset, so other properties left as garbage values
        static bool initPktForDeleting_WithGarbageValues(
            pge_network::PgePacket& pkt,
            const pge_network::PgeNetworkConnectionHandle& connHandleServerSide,
            const Bullet::BulletId bulletId)
        {
            static_assert(sizeof(MsgBulletUpdateFromServer) <= pge_network::MsgApp::nMaxMessageLength, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::getServerSideConnectionHandle(pkt) = connHandleServerSide;
            pge_network::PgePacket::getPacketId(pkt) = pge_network::PgePktId::Application;
            pge_network::PgePacket::getMessageAppArea(pkt).m_nMessageCount = 1; // TODO: increase it instead!
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            pge_network::MsgApp* pMsgApp = reinterpret_cast<pge_network::MsgApp*>(pge_network::PgePacket::getMessageAppArea(pkt).cData);
            pMsgApp->msgId = static_cast<pge_network::TPgeMsgAppMsgId>(id);
            pMsgApp->nMsgSize = sizeof(MsgBulletUpdateFromServer);  // TODO: sizeof(*this)?

            proofps_dd::MsgBulletUpdateFromServer& msgBulletUpdate = reinterpret_cast<proofps_dd::MsgBulletUpdateFromServer&>(pMsgApp->cMsgData);
            msgBulletUpdate.m_bulletId = bulletId;
            msgBulletUpdate.m_bDelete = true;

            return true;
        }

        static bool& getDelete(pge_network::PgePacket& pkt)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            pge_network::MsgApp* const pMsgApp = reinterpret_cast<pge_network::MsgApp*>(pge_network::PgePacket::getMessageAppArea(pkt).cData);
            proofps_dd::MsgBulletUpdateFromServer& msgBulletUpdate = reinterpret_cast<proofps_dd::MsgBulletUpdateFromServer&>(pMsgApp->cMsgData);
            return msgBulletUpdate.m_bDelete;
        }

        Bullet::BulletId m_bulletId;
        TXYZ m_pos;
        TXYZ m_angle;
        TXYZ m_size;
        bool m_bDelete;
    };  // struct MsgBulletUpdateFromServer

    // server -> clients
    // sent to all clients after specific event, e.g. picking up an item
    struct MsgMapItemUpdateFromServer
    {
        static const PRooFPSappMsgId id = PRooFPSappMsgId::MapItemUpdateFromServer;

        static bool initPkt(
            pge_network::PgePacket& pkt,
            const pge_network::PgeNetworkConnectionHandle& connHandleServerSide,
            const MapItem::MapItemId mapItemId,
            const bool bTaken)
        {
            // although preparePktMsgAppFill() does runtime check, we should fail already at compile-time if msg is too big!
            static_assert(sizeof(MsgMapItemUpdateFromServer) <= pge_network::MsgApp::nMaxMessageLength, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::initPktMsgApp(pkt, connHandleServerSide);

            uint8_t* const pMsgAppData = pge_network::PgePacket::preparePktMsgAppFill(
                pkt, static_cast<pge_network::TPgeMsgAppMsgId>(id), sizeof(MsgMapItemUpdateFromServer));
            if (!pMsgAppData)
            {
                return false;
            }

            proofps_dd::MsgMapItemUpdateFromServer& msgMapItemUpdate = reinterpret_cast<proofps_dd::MsgMapItemUpdateFromServer&>(*pMsgAppData);
            msgMapItemUpdate.m_mapItemId = mapItemId;
            msgMapItemUpdate.m_bTaken = bTaken;

            return true;
        }

        MapItem::MapItemId m_mapItemId;
        bool m_bTaken;
    };  // struct MsgMapItemUpdateFromServer

    // server -> clients
    // availability and bullet counts update to a single client after specific events, e.g. shoot
    struct MsgWpnUpdateFromServer
    {
        static const PRooFPSappMsgId id = PRooFPSappMsgId::WpnUpdateFromServer;
        static const uint8_t nWpnNameNameMaxLength = 64;

        static bool initPkt(
            pge_network::PgePacket& pkt,
            const pge_network::PgeNetworkConnectionHandle& connHandleServerSide,
            const std::string& sWpnName,
            bool bAvailable,
            unsigned int nMagBulletCount,
            unsigned int nUnmagBulletCount)
        {
            // although preparePktMsgAppFill() does runtime check, we should fail already at compile-time if msg is too big!
            static_assert(sizeof(MsgWpnUpdateFromServer) <= pge_network::MsgApp::nMaxMessageLength, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::initPktMsgApp(pkt, connHandleServerSide);

            uint8_t* const pMsgAppData = pge_network::PgePacket::preparePktMsgAppFill(
                pkt, static_cast<pge_network::TPgeMsgAppMsgId>(id), sizeof(MsgWpnUpdateFromServer));
            if (!pMsgAppData)
            {
                return false;
            }

            proofps_dd::MsgWpnUpdateFromServer& msgWpnUpdate = reinterpret_cast<proofps_dd::MsgWpnUpdateFromServer&>(*pMsgAppData);
            strncpy_s(msgWpnUpdate.m_szWpnName, nWpnNameNameMaxLength, sWpnName.c_str(), sWpnName.length());
            msgWpnUpdate.m_bAvailable = bAvailable;
            msgWpnUpdate.m_nMagBulletCount = nMagBulletCount;
            msgWpnUpdate.m_nUnmagBulletCount = nUnmagBulletCount;

            return true;
        }

        static bool& getAvailable(pge_network::PgePacket& pkt)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            pge_network::MsgApp* const pMsgApp = reinterpret_cast<pge_network::MsgApp*>(pge_network::PgePacket::getMessageAppArea(pkt).cData);
            proofps_dd::MsgWpnUpdateFromServer& msgWpnUpdate = reinterpret_cast<proofps_dd::MsgWpnUpdateFromServer&>(pMsgApp->cMsgData);
            return msgWpnUpdate.m_bAvailable;
        }

        char m_szWpnName[nWpnNameNameMaxLength];
        bool m_bAvailable;
        unsigned int m_nMagBulletCount;
        unsigned int m_nUnmagBulletCount;
    };  // struct MsgWpnUpdateFromServer

    // server -> clients
    // current weapon of a specific client, sent to all clients after specific events, e.g. weapon switch
    struct MsgCurrentWpnUpdateFromServer
    {
        static const PRooFPSappMsgId id = PRooFPSappMsgId::CurrentWpnUpdateFromServer;

        static bool initPkt(
            pge_network::PgePacket& pkt,
            const pge_network::PgeNetworkConnectionHandle& connHandleServerSide,
            const std::string& sWpnCurrentName)
        {
            // although preparePktMsgAppFill() does runtime check, we should fail already at compile-time if msg is too big!
            static_assert(sizeof(MsgCurrentWpnUpdateFromServer) <= pge_network::MsgApp::nMaxMessageLength, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::initPktMsgApp(pkt, connHandleServerSide);

            uint8_t* const pMsgAppData = pge_network::PgePacket::preparePktMsgAppFill(
                pkt, static_cast<pge_network::TPgeMsgAppMsgId>(id), sizeof(MsgCurrentWpnUpdateFromServer));
            if (!pMsgAppData)
            {
                return false;
            }

            proofps_dd::MsgCurrentWpnUpdateFromServer& msgWpnUpdate = reinterpret_cast<proofps_dd::MsgCurrentWpnUpdateFromServer&>(*pMsgAppData);
            strncpy_s(msgWpnUpdate.m_szWpnCurrentName, MsgWpnUpdateFromServer::nWpnNameNameMaxLength, sWpnCurrentName.c_str(), sWpnCurrentName.length());

            return true;
        }

        char m_szWpnCurrentName[MsgWpnUpdateFromServer::nWpnNameNameMaxLength];
    };  // struct MsgCurrentWpnUpdateFromServer

} // namespace proofps_dd
