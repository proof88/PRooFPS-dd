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

    enum class ElteFailMsgId : pge_network::TPgeMsgAppMsgId  /* underlying type should be same as type of MsgApp::msgId */
    {
        USER_SETUP = 0,
        USER_CMD_MOVE,
        USER_UPDATE,
        BULLET_UPDATE,
        MAP_ITEM_UPDATE,
        WPN_UPDATE,
        WPN_UPDATE_CURRENT,
        LAST_MSG_ID
    };

    struct ElteFailMsgId2ZStringPair
    {
        ElteFailMsgId msgId;
        const char* const zstring;
    };

    // this way of defining std::array makes sure code cannot compile if we forget to align the array after changing ElteFailMsgId
    constexpr std::array<ElteFailMsgId2ZStringPair, static_cast<size_t>(ElteFailMsgId::LAST_MSG_ID)> MapMsgAppId2String
    { {
         {ElteFailMsgId::USER_SETUP,         "MsgUserSetup"},
         {ElteFailMsgId::USER_CMD_MOVE,      "MsgUserCmdMove"},
         {ElteFailMsgId::USER_UPDATE,        "MsgUserUpdate"},
         {ElteFailMsgId::BULLET_UPDATE,      "MsgBulletUpdate"},
         {ElteFailMsgId::MAP_ITEM_UPDATE,    "MsgMapItemUpdate"},
         {ElteFailMsgId::WPN_UPDATE,         "MsgWpnUpdate"},
         {ElteFailMsgId::WPN_UPDATE_CURRENT, "MsgWpnUpdateCurrent"}
    } };

    // server -> self (inject) and clients
    // sent to all clients after the connecting client has been accepted by server
    struct MsgUserSetup
    {
        static const ElteFailMsgId id = ElteFailMsgId::USER_SETUP;
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
            static_assert(sizeof(MsgUserSetup) <= pge_network::MsgApp::nMaxMessageLength, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            memset(&pkt, 0, sizeof(pkt));
            pge_network::PgePacket::getServerSideConnectionHandle(pkt) = connHandleServerSide;
            pge_network::PgePacket::getPacketId(pkt) = pge_network::PgePktId::APP;
            pge_network::PgePacket::getMessageAppArea(pkt).m_nMessageCount = 1; // TODO: increase it instead!
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            pge_network::MsgApp* pMsgApp = reinterpret_cast<pge_network::MsgApp*>(pge_network::PgePacket::getMessageAppArea(pkt).cData);
            pMsgApp->msgId = static_cast<pge_network::TPgeMsgAppMsgId>(id);
            pMsgApp->nMsgSize = sizeof(MsgUserSetup);  // TODO: sizeof(*this)?

            proofps_dd::MsgUserSetup& msgUserSetup = reinterpret_cast<proofps_dd::MsgUserSetup&>(pMsgApp->cMsgData);
            msgUserSetup.m_bCurrentClient = bCurrentClient;
            strncpy_s(msgUserSetup.m_szUserName, nUserNameMaxLength, sUserName.c_str(), sUserName.length());
            strncpy_s(msgUserSetup.m_szIpAddress, sizeof(msgUserSetup.m_szIpAddress), sIpAddress.c_str(), sIpAddress.length());
            strncpy_s(msgUserSetup.m_szMapFilename, sizeof(msgUserSetup.m_szMapFilename), sMapFilename.c_str(), sMapFilename.length());

            return true;
        }

        bool m_bCurrentClient;
        char m_szUserName[nUserNameMaxLength];
        char m_szIpAddress[pge_network::MsgUserConnected::nIpAddressMaxLength];
        char m_szMapFilename[nMapFilenameMaxLength];
    };  // struct MsgUserSetup

    // clients -> server + server self (inject)
    // MsgUserCmdMove messages are sent from clients to server, so server will do sg and then update all the clients with MsgUserUpdate
    struct MsgUserCmdMove
    {
        static const ElteFailMsgId id = ElteFailMsgId::USER_CMD_MOVE;

        static bool initPkt(
            pge_network::PgePacket& pkt)
        {
            static_assert(sizeof(MsgUserCmdMove) <= pge_network::MsgApp::nMaxMessageLength, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            memset(&pkt, 0, sizeof(pkt));
            // m_connHandleServerSide is ignored in this message
            //pge_network::PgePacket::getServerSideConnectionHandle(pkt) = connHandleServerSide;
            pge_network::PgePacket::getPacketId(pkt) = pge_network::PgePktId::APP;
            pge_network::PgePacket::getMessageAppArea(pkt).m_nMessageCount = 1; // TODO: increase it instead!
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            pge_network::MsgApp* pMsgApp = reinterpret_cast<pge_network::MsgApp*>(pge_network::PgePacket::getMessageAppArea(pkt).cData);
            pMsgApp->msgId = static_cast<pge_network::TPgeMsgAppMsgId>(id);
            pMsgApp->nMsgSize = sizeof(MsgUserCmdMove);  // TODO: sizeof(*this)?

            proofps_dd::MsgUserCmdMove& msgUserCmdMove = reinterpret_cast<proofps_dd::MsgUserCmdMove&>(pMsgApp->cMsgData);
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
            proofps_dd::MsgUserCmdMove& msgUserCmdMove = reinterpret_cast<proofps_dd::MsgUserCmdMove&>(pMsgApp->cMsgData);
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
            const proofps_dd::MsgUserCmdMove& msgUserCmdMove = reinterpret_cast<const proofps_dd::MsgUserCmdMove&>(pMsgApp->cMsgData);
            return msgUserCmdMove.m_cWeaponSwitch;
        }

        static void SetWeaponSwitch(pge_network::PgePacket& pkt, unsigned char cTargetWpnKey)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            pge_network::MsgApp* const pMsgApp = reinterpret_cast<pge_network::MsgApp*>(pge_network::PgePacket::getMessageAppArea(pkt).cData);
            proofps_dd::MsgUserCmdMove& msgUserCmdMove = reinterpret_cast<proofps_dd::MsgUserCmdMove&>(pMsgApp->cMsgData);
            msgUserCmdMove.m_bShouldSend = true;
            msgUserCmdMove.m_cWeaponSwitch = cTargetWpnKey;
        }

        static bool getReloadRequest(const pge_network::PgePacket& pkt)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            const pge_network::MsgApp* const pMsgApp = reinterpret_cast<const pge_network::MsgApp*>(pge_network::PgePacket::getMessageAppArea(pkt).cData);
            const proofps_dd::MsgUserCmdMove& msgUserCmdMove = reinterpret_cast<const proofps_dd::MsgUserCmdMove&>(pMsgApp->cMsgData);
            return msgUserCmdMove.m_bRequestReload;
        }

        static void setMouse(
            pge_network::PgePacket& pkt,
            bool bShootAction)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            pge_network::MsgApp* const pMsgApp = reinterpret_cast<pge_network::MsgApp*>(pge_network::PgePacket::getMessageAppArea(pkt).cData);
            proofps_dd::MsgUserCmdMove& msgUserCmdMove = reinterpret_cast<proofps_dd::MsgUserCmdMove&>(pMsgApp->cMsgData);
            msgUserCmdMove.m_bShouldSend = true;
            msgUserCmdMove.m_bShootAction = bShootAction;
        }

        static void setAngleY(
            pge_network::PgePacket& pkt,
            TPureFloat fPlayerAngleY)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            pge_network::MsgApp* const pMsgApp = reinterpret_cast<pge_network::MsgApp*>(pge_network::PgePacket::getMessageAppArea(pkt).cData);
            proofps_dd::MsgUserCmdMove& msgUserCmdMove = reinterpret_cast<proofps_dd::MsgUserCmdMove&>(pMsgApp->cMsgData);
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
            proofps_dd::MsgUserCmdMove& msgUserCmdMove = reinterpret_cast<proofps_dd::MsgUserCmdMove&>(pMsgApp->cMsgData);
            msgUserCmdMove.m_bShouldSend = true;
            msgUserCmdMove.m_fWpnAngleY = fWpnAngleY;
            msgUserCmdMove.m_fWpnAngleZ = fWpnAngleZ;
        }

        static bool shouldSend(
            const pge_network::PgePacket& pkt)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            const pge_network::MsgApp* const pMsgApp = reinterpret_cast<const pge_network::MsgApp*>(pge_network::PgePacket::getMessageAppArea(pkt).cData);
            const proofps_dd::MsgUserCmdMove& msgUserCmdMove = reinterpret_cast<const proofps_dd::MsgUserCmdMove&>(pMsgApp->cMsgData);
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
    };  // struct MsgUserCmdMove

    // server -> self (inject) and clients
    // sent regularly to all clients
    struct MsgUserUpdate
    {
        static const ElteFailMsgId id = ElteFailMsgId::USER_UPDATE;

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
            static_assert(sizeof(MsgUserUpdate) <= pge_network::MsgApp::nMaxMessageLength, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            memset(&pkt, 0, sizeof(pkt));
            pge_network::PgePacket::getServerSideConnectionHandle(pkt) = connHandleServerSide;
            pge_network::PgePacket::getPacketId(pkt) = pge_network::PgePktId::APP;
            pge_network::PgePacket::getMessageAppArea(pkt).m_nMessageCount = 1; // TODO: increase it instead!
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            pge_network::MsgApp* pMsgApp = reinterpret_cast<pge_network::MsgApp*>(pge_network::PgePacket::getMessageAppArea(pkt).cData);
            pMsgApp->msgId = static_cast<pge_network::TPgeMsgAppMsgId>(id);
            pMsgApp->nMsgSize = sizeof(MsgUserUpdate);  // TODO: sizeof(*this)?

            proofps_dd::MsgUserUpdate& msgUserCmdUpdate = reinterpret_cast<proofps_dd::MsgUserUpdate&>(pMsgApp->cMsgData);
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
    };  // struct MsgUserUpdate

    // server -> clients
    // sent to all clients
    struct MsgBulletUpdate
    {
        static const ElteFailMsgId id = ElteFailMsgId::BULLET_UPDATE;

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
            static_assert(sizeof(MsgBulletUpdate) <= pge_network::MsgApp::nMaxMessageLength, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            //memset(&pkt, 0, sizeof(pkt));
            pge_network::PgePacket::getServerSideConnectionHandle(pkt) = connHandleServerSide;
            pge_network::PgePacket::getPacketId(pkt) = pge_network::PgePktId::APP;
            pge_network::PgePacket::getMessageAppArea(pkt).m_nMessageCount = 1; // TODO: increase it instead!
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            pge_network::MsgApp* pMsgApp = reinterpret_cast<pge_network::MsgApp*>(pge_network::PgePacket::getMessageAppArea(pkt).cData);
            pMsgApp->msgId = static_cast<pge_network::TPgeMsgAppMsgId>(id);
            pMsgApp->nMsgSize = sizeof(MsgBulletUpdate);  // TODO: sizeof(*this)?

            proofps_dd::MsgBulletUpdate& msgBulletUpdate = reinterpret_cast<proofps_dd::MsgBulletUpdate&>(pMsgApp->cMsgData);
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
            static_assert(sizeof(MsgBulletUpdate) <= pge_network::MsgApp::nMaxMessageLength, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::getServerSideConnectionHandle(pkt) = connHandleServerSide;
            pge_network::PgePacket::getPacketId(pkt) = pge_network::PgePktId::APP;
            pge_network::PgePacket::getMessageAppArea(pkt).m_nMessageCount = 1; // TODO: increase it instead!
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            pge_network::MsgApp* pMsgApp = reinterpret_cast<pge_network::MsgApp*>(pge_network::PgePacket::getMessageAppArea(pkt).cData);
            pMsgApp->msgId = static_cast<pge_network::TPgeMsgAppMsgId>(id);
            pMsgApp->nMsgSize = sizeof(MsgBulletUpdate);  // TODO: sizeof(*this)?

            proofps_dd::MsgBulletUpdate& msgBulletUpdate = reinterpret_cast<proofps_dd::MsgBulletUpdate&>(pMsgApp->cMsgData);
            msgBulletUpdate.m_bulletId = bulletId;
            msgBulletUpdate.m_bDelete = true;

            return true;
        }

        static bool& getDelete(pge_network::PgePacket& pkt)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            pge_network::MsgApp* const pMsgApp = reinterpret_cast<pge_network::MsgApp*>(pge_network::PgePacket::getMessageAppArea(pkt).cData);
            proofps_dd::MsgBulletUpdate& msgBulletUpdate = reinterpret_cast<proofps_dd::MsgBulletUpdate&>(pMsgApp->cMsgData);
            return msgBulletUpdate.m_bDelete;
        }

        Bullet::BulletId m_bulletId;
        TXYZ m_pos;
        TXYZ m_angle;
        TXYZ m_size;
        bool m_bDelete;
    };  // struct MsgBulletUpdate

    // server -> clients
    // sent to all clients after specific event, e.g. picking up an item
    struct MsgMapItemUpdate
    {
        static const ElteFailMsgId id = ElteFailMsgId::MAP_ITEM_UPDATE;

        static bool initPkt(
            pge_network::PgePacket& pkt,
            const pge_network::PgeNetworkConnectionHandle& connHandleServerSide,
            const MapItem::MapItemId mapItemId,
            const bool bTaken)
        {
            static_assert(sizeof(MsgMapItemUpdate) <= pge_network::MsgApp::nMaxMessageLength, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            memset(&pkt, 0, sizeof(pkt));
            pge_network::PgePacket::getServerSideConnectionHandle(pkt) = connHandleServerSide;
            pge_network::PgePacket::getPacketId(pkt) = pge_network::PgePktId::APP;
            pge_network::PgePacket::getMessageAppArea(pkt).m_nMessageCount = 1; // TODO: increase it instead!
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            pge_network::MsgApp* pMsgApp = reinterpret_cast<pge_network::MsgApp*>(pge_network::PgePacket::getMessageAppArea(pkt).cData);
            pMsgApp->msgId = static_cast<pge_network::TPgeMsgAppMsgId>(id);
            pMsgApp->nMsgSize = sizeof(MsgMapItemUpdate);  // TODO: sizeof(*this)?

            proofps_dd::MsgMapItemUpdate& msgMapItemUpdate = reinterpret_cast<proofps_dd::MsgMapItemUpdate&>(pMsgApp->cMsgData);
            msgMapItemUpdate.m_mapItemId = mapItemId;
            msgMapItemUpdate.m_bTaken = bTaken;

            return true;
        }

        MapItem::MapItemId m_mapItemId;
        bool m_bTaken;
    };  // struct MsgMapItemUpdate

    // server -> clients
    // availability and bullet counts update to a single client after specific events, e.g. shoot
    struct MsgWpnUpdate
    {
        static const ElteFailMsgId id = ElteFailMsgId::WPN_UPDATE;
        static const uint8_t nWpnNameNameMaxLength = 64;

        static bool initPkt(
            pge_network::PgePacket& pkt,
            const pge_network::PgeNetworkConnectionHandle& connHandleServerSide,
            const std::string& sWpnName,
            bool bAvailable,
            unsigned int nMagBulletCount,
            unsigned int nUnmagBulletCount)
        {
            static_assert(sizeof(MsgWpnUpdate) <= pge_network::MsgApp::nMaxMessageLength, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            memset(&pkt, 0, sizeof(pkt));
            pge_network::PgePacket::getServerSideConnectionHandle(pkt) = connHandleServerSide;
            pge_network::PgePacket::getPacketId(pkt) = pge_network::PgePktId::APP;
            pge_network::PgePacket::getMessageAppArea(pkt).m_nMessageCount = 1; // TODO: increase it instead!
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            pge_network::MsgApp* pMsgApp = reinterpret_cast<pge_network::MsgApp*>(pge_network::PgePacket::getMessageAppArea(pkt).cData);
            pMsgApp->msgId = static_cast<pge_network::TPgeMsgAppMsgId>(id);
            pMsgApp->nMsgSize = sizeof(MsgWpnUpdate);  // TODO: sizeof(*this)?

            proofps_dd::MsgWpnUpdate& msgWpnUpdate = reinterpret_cast<proofps_dd::MsgWpnUpdate&>(pMsgApp->cMsgData);
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
            proofps_dd::MsgWpnUpdate& msgWpnUpdate = reinterpret_cast<proofps_dd::MsgWpnUpdate&>(pMsgApp->cMsgData);
            return msgWpnUpdate.m_bAvailable;
        }

        char m_szWpnName[nWpnNameNameMaxLength];
        bool m_bAvailable;
        unsigned int m_nMagBulletCount;
        unsigned int m_nUnmagBulletCount;
    };  // struct MsgWpnUpdate

    // server -> clients
    // current weapon of a specific client, sent to all clients after specific events, e.g. weapon switch
    struct MsgWpnUpdateCurrent
    {
        static const ElteFailMsgId id = ElteFailMsgId::WPN_UPDATE_CURRENT;

        static bool initPkt(
            pge_network::PgePacket& pkt,
            const pge_network::PgeNetworkConnectionHandle& connHandleServerSide,
            const std::string& sWpnCurrentName)
        {
            static_assert(sizeof(MsgWpnUpdateCurrent) <= pge_network::MsgApp::nMaxMessageLength, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            memset(&pkt, 0, sizeof(pkt));
            pge_network::PgePacket::getServerSideConnectionHandle(pkt) = connHandleServerSide;
            pge_network::PgePacket::getPacketId(pkt) = pge_network::PgePktId::APP;
            pge_network::PgePacket::getMessageAppArea(pkt).m_nMessageCount = 1; // TODO: increase it instead!
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            pge_network::MsgApp* pMsgApp = reinterpret_cast<pge_network::MsgApp*>(pge_network::PgePacket::getMessageAppArea(pkt).cData);
            pMsgApp->msgId = static_cast<pge_network::TPgeMsgAppMsgId>(id);
            pMsgApp->nMsgSize = sizeof(MsgWpnUpdateCurrent);  // TODO: sizeof(*this)?

            proofps_dd::MsgWpnUpdateCurrent& msgWpnUpdate = reinterpret_cast<proofps_dd::MsgWpnUpdateCurrent&>(pMsgApp->cMsgData);
            strncpy_s(msgWpnUpdate.m_szWpnCurrentName, MsgWpnUpdate::nWpnNameNameMaxLength, sWpnCurrentName.c_str(), sWpnCurrentName.length());

            return true;
        }

        char m_szWpnCurrentName[MsgWpnUpdate::nWpnNameNameMaxLength];
    };  // struct MsgWpnUpdateCurrent

} // namespace proofps_dd
