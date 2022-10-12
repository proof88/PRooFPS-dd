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
        BULLET_UPDATE
    };

    // server -> self (inject) and clients
    struct MsgUserSetup
    {
        static const ElteFailMsgId id = ElteFailMsgId::USER_SETUP;
        static const uint8_t nUserNameMaxLength = 64;

        static bool initPkt(
            pge_network::PgePacket& pkt,
            const pge_network::PgeNetworkConnectionHandle& connHandleServerSide,
            bool bCurrentClient,
            const std::string& sUserName,
            const std::string& sIpAddress)
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

            return true;
        }

        bool m_bCurrentClient;
        char m_szUserName[nUserNameMaxLength];
        char m_szIpAddress[pge_network::MsgUserConnected::nIpAddressMaxLength];
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
            bool bSwitchToRunning)
        {
            proofps_dd::MsgUserCmdMove& msgUserCmdMove = reinterpret_cast<proofps_dd::MsgUserCmdMove&>(pkt.msg.app.cData);
            msgUserCmdMove.m_bShouldSend = true;
            msgUserCmdMove.m_strafe = strafe;
            msgUserCmdMove.m_bJumpAction = bJump;
            msgUserCmdMove.m_bSendSwitchToRunning = bSwitchToRunning;
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
            TPRREfloat fPlayerAngleY)
        {
            proofps_dd::MsgUserCmdMove& msgUserCmdMove = reinterpret_cast<proofps_dd::MsgUserCmdMove&>(pkt.msg.app.cData);
            msgUserCmdMove.m_bShouldSend = true;
            msgUserCmdMove.m_fPlayerAngleY = fPlayerAngleY;
        }

        static void setWpnAngles(
            pge_network::PgePacket& pkt,
            TPRREfloat fWpnAngleY,
            TPRREfloat fWpnAngleZ)
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
        bool m_bShootAction;
        TPRREfloat m_fPlayerAngleY;
        TPRREfloat m_fWpnAngleY;
        TPRREfloat m_fWpnAngleZ;
    };

    // server -> self (inject) and clients
    struct MsgUserUpdate
    {
        static const ElteFailMsgId id = ElteFailMsgId::USER_UPDATE;

        static bool initPkt(
            pge_network::PgePacket& pkt,
            const pge_network::PgeNetworkConnectionHandle& connHandleServerSide,
            const TPRREfloat x,
            const TPRREfloat y,
            const TPRREfloat z,
            TPRREfloat fPlayerAngleY,
            TPRREfloat fWpnAngleY,
            TPRREfloat fWpnAngleZ,
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
        TPRREfloat m_fPlayerAngleY;
        TPRREfloat m_fWpnAngleY;
        TPRREfloat m_fWpnAngleZ;
        int m_nHealth;
        bool m_bRespawn;
        int m_nFrags;
        int m_nDeaths;
    };

    // server -> clients
    struct MsgBulletUpdate
    {
        static const ElteFailMsgId id = ElteFailMsgId::BULLET_UPDATE;

        static bool initPkt(
            pge_network::PgePacket& pkt,
            const pge_network::PgeNetworkConnectionHandle& connHandleServerSide,
            const Bullet::BulletId bulletId,
            const TPRREfloat px,
            const TPRREfloat py,
            const TPRREfloat pz,
            const TPRREfloat ax,
            const TPRREfloat ay,
            const TPRREfloat az,
            const TPRREfloat sx,
            const TPRREfloat sy,
            const TPRREfloat sz)
        {
            assert(sizeof(MsgBulletUpdate) <= pge_network::MsgApp::nMessageMaxLength);
            memset(&pkt, 0, sizeof(pkt));
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

} // namespace proofps_dd