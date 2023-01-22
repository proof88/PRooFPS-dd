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

#include <cassert>

#include "../../../PGE/PGE/Network/PgePacket.h"
#include "../../../PGE/PGE/Weapons/WeaponManager.h" // for BulletId

namespace proofps_dd
{

    enum class ElteFailMsgId : pge_network::TPgeMsgAppMsgId  /* underlying type should be same as type of MsgApp::msgId */
    {
        USER_SETUP = 0,
        USER_CMD_MOVE,
        USER_CMD_TARGET,
        USER_UPDATE,
        USER_UPDATE_2,
        BULLET_UPDATE,
        MAP_ITEM_UPDATE,
        WPN_UPDATE,
        WPN_UPDATE_CURRENT
    };

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
            assert(sizeof(MsgUserSetup) <= pge_network::MsgApp::nMessageMaxLength);
            memset(&pkt, 0, sizeof(pkt));
            pkt.m_connHandleServerSide = connHandleServerSide;
            pkt.pktId = pge_network::PgePktId::APP;
            pkt.msg.app.msgId = static_cast<pge_network::TPgeMsgAppMsgId>(proofps_dd::MsgUserSetup::id);

            proofps_dd::MsgUserSetup& msgUserSetup = reinterpret_cast<proofps_dd::MsgUserSetup&>(pkt.msg.app.cData);
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
    };

    enum class Strafe : std::uint8_t
    {
        NONE = 0,
        LEFT,
        RIGHT
    };

    // clients -> server + server self (inject)
    // MsgUserCmdMove messages are sent from clients to server, so server will do sg and then update all the clients with MsgUserUpdate
    struct MsgUserCmdMove
    {
        static const ElteFailMsgId id = ElteFailMsgId::USER_CMD_MOVE;

        static bool initPkt(
            pge_network::PgePacket& pkt)
        {
            assert(sizeof(MsgUserCmdMove) <= pge_network::MsgApp::nMessageMaxLength);
            memset(&pkt, 0, sizeof(pkt));
            // m_connHandleServerSide is ignored in this message
            //pkt.m_connHandleServerSide = connHandleServerSide;
            pkt.pktId = pge_network::PgePktId::APP;
            pkt.msg.app.msgId = static_cast<pge_network::TPgeMsgAppMsgId>(proofps_dd::MsgUserCmdMove::id);

            proofps_dd::MsgUserCmdMove& msgUserCmdMove = reinterpret_cast<proofps_dd::MsgUserCmdMove&>(pkt.msg.app.cData);
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
            proofps_dd::MsgUserCmdMove& msgUserCmdMove = reinterpret_cast<proofps_dd::MsgUserCmdMove&>(pkt.msg.app.cData);
            msgUserCmdMove.m_bShouldSend = true;
            msgUserCmdMove.m_strafe = strafe;
            msgUserCmdMove.m_bJumpAction = bJump;
            msgUserCmdMove.m_bSendSwitchToRunning = bSwitchToRunning;
            msgUserCmdMove.m_bRequestReload = bRequestReload;
            msgUserCmdMove.m_cWeaponSwitch = cWeaponSwitch;
        }

        static unsigned char getWeaponSwitch(const pge_network::PgePacket& pkt)
        {
            const proofps_dd::MsgUserCmdMove& msgUserCmdMove = reinterpret_cast<const proofps_dd::MsgUserCmdMove&>(pkt.msg.app.cData);
            return msgUserCmdMove.m_cWeaponSwitch;
        }

        static void SetWeaponSwitch(pge_network::PgePacket& pkt, unsigned char cTargetWpnKey)
        {
            proofps_dd::MsgUserCmdMove& msgUserCmdMove = reinterpret_cast<proofps_dd::MsgUserCmdMove&>(pkt.msg.app.cData);
            msgUserCmdMove.m_bShouldSend = true;
            msgUserCmdMove.m_cWeaponSwitch = cTargetWpnKey;
        }

        static bool getReloadRequest(const pge_network::PgePacket& pkt)
        {
            const proofps_dd::MsgUserCmdMove& msgUserCmdMove = reinterpret_cast<const proofps_dd::MsgUserCmdMove&>(pkt.msg.app.cData);
            return msgUserCmdMove.m_bRequestReload;
        }

        static void setMouse(
            pge_network::PgePacket& pkt,
            bool bShootAction)
        {
            proofps_dd::MsgUserCmdMove& msgUserCmdMove = reinterpret_cast<proofps_dd::MsgUserCmdMove&>(pkt.msg.app.cData);
            msgUserCmdMove.m_bShouldSend = true;
            msgUserCmdMove.m_bShootAction = bShootAction;
        }

        static void setAngleY(
            pge_network::PgePacket& pkt,
            TPurefloat fPlayerAngleY)
        {
            proofps_dd::MsgUserCmdMove& msgUserCmdMove = reinterpret_cast<proofps_dd::MsgUserCmdMove&>(pkt.msg.app.cData);
            msgUserCmdMove.m_bShouldSend = true;
            msgUserCmdMove.m_fPlayerAngleY = fPlayerAngleY;
        }

        static void setWpnAngles(
            pge_network::PgePacket& pkt,
            TPurefloat fWpnAngleY,
            TPurefloat fWpnAngleZ)
        {
            proofps_dd::MsgUserCmdMove& msgUserCmdMove = reinterpret_cast<proofps_dd::MsgUserCmdMove&>(pkt.msg.app.cData);
            msgUserCmdMove.m_bShouldSend = true;
            msgUserCmdMove.m_fWpnAngleY = fWpnAngleY;
            msgUserCmdMove.m_fWpnAngleZ = fWpnAngleZ;
        }

        static bool shouldSend(
            const pge_network::PgePacket& pkt)
        {
            const proofps_dd::MsgUserCmdMove& msgUserCmdMove = reinterpret_cast<const proofps_dd::MsgUserCmdMove&>(pkt.msg.app.cData);
            return msgUserCmdMove.m_bShouldSend;
        }

        bool m_bShouldSend;
        Strafe m_strafe;
        bool m_bJumpAction;
        bool m_bSendSwitchToRunning;
        bool m_bRequestReload;
        unsigned char m_cWeaponSwitch;
        bool m_bShootAction;
        TPurefloat m_fPlayerAngleY;
        TPurefloat m_fWpnAngleY;
        TPurefloat m_fWpnAngleZ;
    };

    // server -> self (inject) and clients
    // sent regularly to all clients
    struct MsgUserUpdate
    {
        static const ElteFailMsgId id = ElteFailMsgId::USER_UPDATE;

        static bool initPkt(
            pge_network::PgePacket& pkt,
            const pge_network::PgeNetworkConnectionHandle& connHandleServerSide,
            const TPurefloat x,
            const TPurefloat y,
            const TPurefloat z,
            TPurefloat fPlayerAngleY,
            TPurefloat fWpnAngleY,
            TPurefloat fWpnAngleZ,
            int nHealth,
            bool bRespawn,
            int nFrags,
            int nDeaths)
        {
            assert(sizeof(MsgUserUpdate) <= pge_network::MsgApp::nMessageMaxLength);
            memset(&pkt, 0, sizeof(pkt));
            pkt.m_connHandleServerSide = connHandleServerSide;
            pkt.pktId = pge_network::PgePktId::APP;
            pkt.msg.app.msgId = static_cast<pge_network::TPgeMsgAppMsgId>(proofps_dd::MsgUserUpdate::id);

            proofps_dd::MsgUserUpdate& msgUserCmdUpdate = reinterpret_cast<proofps_dd::MsgUserUpdate&>(pkt.msg.app.cData);
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
        TPurefloat m_fPlayerAngleY;
        TPurefloat m_fWpnAngleY;
        TPurefloat m_fWpnAngleZ;
        int m_nHealth;
        bool m_bRespawn;
        int m_nFrags;
        int m_nDeaths;
    };

    // server -> clients
    // sent to all clients
    struct MsgBulletUpdate
    {
        static const ElteFailMsgId id = ElteFailMsgId::BULLET_UPDATE;

        static bool initPkt(
            pge_network::PgePacket& pkt,
            const pge_network::PgeNetworkConnectionHandle& connHandleServerSide,
            const Bullet::BulletId bulletId,
            const TPurefloat px,
            const TPurefloat py,
            const TPurefloat pz,
            const TPurefloat ax,
            const TPurefloat ay,
            const TPurefloat az,
            const TPurefloat sx,
            const TPurefloat sy,
            const TPurefloat sz)
        {
            assert(sizeof(MsgBulletUpdate) <= pge_network::MsgApp::nMessageMaxLength);
            //memset(&pkt, 0, sizeof(pkt));  // TODO: what is the use of memset anyway if we initialize all the members properly?!
            pkt.m_connHandleServerSide = connHandleServerSide;
            pkt.pktId = pge_network::PgePktId::APP;
            pkt.msg.app.msgId = static_cast<pge_network::TPgeMsgAppMsgId>(proofps_dd::MsgBulletUpdate::id);

            proofps_dd::MsgBulletUpdate& msgBulletUpdate = reinterpret_cast<proofps_dd::MsgBulletUpdate&>(pkt.msg.app.cData);
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
            assert(sizeof(MsgBulletUpdate) <= pge_network::MsgApp::nMessageMaxLength);
            pkt.m_connHandleServerSide = connHandleServerSide;
            pkt.pktId = pge_network::PgePktId::APP;
            pkt.msg.app.msgId = static_cast<pge_network::TPgeMsgAppMsgId>(proofps_dd::MsgBulletUpdate::id);

            proofps_dd::MsgBulletUpdate& msgBulletUpdate = reinterpret_cast<proofps_dd::MsgBulletUpdate&>(pkt.msg.app.cData);
            msgBulletUpdate.m_bulletId = bulletId;
            msgBulletUpdate.m_bDelete = true;

            return true;
        }

        static bool& getDelete(pge_network::PgePacket& pkt)
        {
            proofps_dd::MsgBulletUpdate& msgBulletUpdate = reinterpret_cast<proofps_dd::MsgBulletUpdate&>(pkt.msg.app.cData);
            return msgBulletUpdate.m_bDelete;
        }

        Bullet::BulletId m_bulletId;
        TXYZ m_pos;
        TXYZ m_angle;
        TXYZ m_size;
        bool m_bDelete;
    };

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
            assert(sizeof(MsgMapItemUpdate) <= pge_network::MsgApp::nMessageMaxLength);
            memset(&pkt, 0, sizeof(pkt));
            pkt.m_connHandleServerSide = connHandleServerSide;
            pkt.pktId = pge_network::PgePktId::APP;
            pkt.msg.app.msgId = static_cast<pge_network::TPgeMsgAppMsgId>(proofps_dd::MsgMapItemUpdate::id);

            proofps_dd::MsgMapItemUpdate& msgMapItemUpdate = reinterpret_cast<proofps_dd::MsgMapItemUpdate&>(pkt.msg.app.cData);
            msgMapItemUpdate.m_mapItemId = mapItemId;
            msgMapItemUpdate.m_bTaken = bTaken;

            return true;
        }

        MapItem::MapItemId m_mapItemId;
        bool m_bTaken;
    };

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
            assert(sizeof(MsgWpnUpdate) <= pge_network::MsgApp::nMessageMaxLength);
            memset(&pkt, 0, sizeof(pkt));
            pkt.m_connHandleServerSide = connHandleServerSide;
            pkt.pktId = pge_network::PgePktId::APP;
            pkt.msg.app.msgId = static_cast<pge_network::TPgeMsgAppMsgId>(proofps_dd::MsgWpnUpdate::id);

            proofps_dd::MsgWpnUpdate& msgWpnUpdate = reinterpret_cast<proofps_dd::MsgWpnUpdate&>(pkt.msg.app.cData);
            strncpy_s(msgWpnUpdate.m_szWpnName, nWpnNameNameMaxLength, sWpnName.c_str(), sWpnName.length());
            msgWpnUpdate.m_bAvailable = bAvailable;
            msgWpnUpdate.m_nMagBulletCount = nMagBulletCount;
            msgWpnUpdate.m_nUnmagBulletCount = nUnmagBulletCount;

            return true;
        }

        static bool& getAvailable(pge_network::PgePacket& pkt)
        {
            proofps_dd::MsgWpnUpdate& msgWpnUpdate = reinterpret_cast<proofps_dd::MsgWpnUpdate&>(pkt.msg.app.cData);
            return msgWpnUpdate.m_bAvailable;
        }

        char m_szWpnName[nWpnNameNameMaxLength];
        bool m_bAvailable;
        unsigned int m_nMagBulletCount;
        unsigned int m_nUnmagBulletCount;
    };

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
            assert(sizeof(MsgWpnUpdateCurrent) <= pge_network::MsgApp::nMessageMaxLength);
            memset(&pkt, 0, sizeof(pkt));
            pkt.m_connHandleServerSide = connHandleServerSide;
            pkt.pktId = pge_network::PgePktId::APP;
            pkt.msg.app.msgId = static_cast<pge_network::TPgeMsgAppMsgId>(proofps_dd::MsgWpnUpdateCurrent::id);

            proofps_dd::MsgWpnUpdateCurrent& msgWpnUpdate = reinterpret_cast<proofps_dd::MsgWpnUpdateCurrent&>(pkt.msg.app.cData);
            strncpy_s(msgWpnUpdate.m_szWpnCurrentName, MsgWpnUpdate::nWpnNameNameMaxLength, sWpnCurrentName.c_str(), sWpnCurrentName.length());

            return true;
        }

        char m_szWpnCurrentName[MsgWpnUpdate::nWpnNameNameMaxLength];
    };


} // namespace proofps_dd