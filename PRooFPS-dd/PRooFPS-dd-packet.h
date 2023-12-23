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

    enum class PRooFPSappMsgId : pge_network::MsgApp::TMsgId  /* underlying type should be same as type of pge_network::MsgApp::msgId */
    {
        MapChangeFromServer = 0,
        UserSetupFromServer,
        UserNameChange,
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

    const auto MapMsgAppId2String = PFL::std_array_of<PRooFPSappMsgId2ZStringPair>
    (
        PRooFPSappMsgId2ZStringPair{ PRooFPSappMsgId::MapChangeFromServer,         "MapChangeFromServer" },
        PRooFPSappMsgId2ZStringPair{ PRooFPSappMsgId::UserSetupFromServer,         "MsgUserSetupFromServer" },
        PRooFPSappMsgId2ZStringPair{ PRooFPSappMsgId::UserNameChange,              "MsgUserNameChange" },
        PRooFPSappMsgId2ZStringPair{ PRooFPSappMsgId::UserCmdFromClient,           "MsgUserCmdFromClient" },
        PRooFPSappMsgId2ZStringPair{ PRooFPSappMsgId::UserUpdateFromServer,        "MsgUserUpdateFromServer" },
        PRooFPSappMsgId2ZStringPair{ PRooFPSappMsgId::BulletUpdateFromServer,      "MsgBulletUpdateFromServer" },
        PRooFPSappMsgId2ZStringPair{ PRooFPSappMsgId::MapItemUpdateFromServer,     "MsgMapItemUpdateFromServer" },
        PRooFPSappMsgId2ZStringPair{ PRooFPSappMsgId::WpnUpdateFromServer,         "MsgWpnUpdateFromServer" },
        PRooFPSappMsgId2ZStringPair{ PRooFPSappMsgId::CurrentWpnUpdateFromServer,  "MsgCurrentWpnUpdateFromServer" }
    );

    // this way nobody will forget updating both the enum and the array
    static_assert(static_cast<size_t>(PRooFPSappMsgId::LastMsgId) == MapMsgAppId2String.size());

    // server -> self (inject) and clients
    // sent to all clients when map is changing
    // So currently this is NOT used at bootup.
    struct MsgMapChangeFromServer
    {
        static const PRooFPSappMsgId id = PRooFPSappMsgId::MapChangeFromServer;
        static const uint8_t nMapFilenameMaxLength = 64;

        static bool initPkt(
            pge_network::PgePacket& pkt,
            const std::string& sMapFilename)
        {
            // although preparePktMsgAppFill() does runtime check, we should fail already at compile-time if msg is too big!
            static_assert(sizeof(MsgMapChangeFromServer) <= pge_network::MsgApp::nMaxMessageLengthBytes, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::initPktMsgApp(pkt, 0u /*m_connHandleServerSide is ignored in this message*/);

            pge_network::TByte* const pMsgAppData = pge_network::PgePacket::preparePktMsgAppFill(
                pkt, static_cast<pge_network::MsgApp::TMsgId>(id), sizeof(MsgMapChangeFromServer));
            if (!pMsgAppData)
            {
                return false;
            }

            proofps_dd::MsgMapChangeFromServer& msgUserSetup = reinterpret_cast<proofps_dd::MsgMapChangeFromServer&>(*pMsgAppData);
            strncpy_s(msgUserSetup.m_szMapFilename, sizeof(msgUserSetup.m_szMapFilename), sMapFilename.c_str(), sMapFilename.length());

            return true;
        }

        char m_szMapFilename[nMapFilenameMaxLength];
    };  // struct MsgMapChangeFromServer
    static_assert(std::is_trivial_v<MsgMapChangeFromServer>);
    static_assert(std::is_trivially_copyable_v<MsgMapChangeFromServer>);
    static_assert(std::is_standard_layout_v<MsgMapChangeFromServer>);

    // server -> self (inject) and clients
    // sent to all clients after the connecting client has been accepted by server
    // This message is also used to tell the client which map to load at bootup.
    // However, in case of map change, MsgMapChangeFromServer contains this info, but this message also contains it again.
    // A unique pre-generated user name is also in the message, which must be accepted by clients, however clients are
    // also encouraged to request user name change using MsgUserNameChange message.
    struct MsgUserSetupFromServer
    {
        static const PRooFPSappMsgId id = PRooFPSappMsgId::UserSetupFromServer;

        static bool initPkt(
            pge_network::PgePacket& pkt,
            const pge_network::PgeNetworkConnectionHandle& connHandleServerSide,
            bool bCurrentClient,
            const std::string& sIpAddress,
            const std::string& sMapFilename)
        {
            // although preparePktMsgAppFill() does runtime check, we should fail already at compile-time if msg is too big!
            static_assert(sizeof(MsgUserSetupFromServer) <= pge_network::MsgApp::nMaxMessageLengthBytes, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::initPktMsgApp(pkt, connHandleServerSide);

            pge_network::TByte* const pMsgAppData = pge_network::PgePacket::preparePktMsgAppFill(
                pkt, static_cast<pge_network::MsgApp::TMsgId>(id), sizeof(MsgUserSetupFromServer));
            if (!pMsgAppData)
            {
                return false;
            }

            proofps_dd::MsgUserSetupFromServer& msgUserSetup = reinterpret_cast<proofps_dd::MsgUserSetupFromServer&>(*pMsgAppData);
            msgUserSetup.m_bCurrentClient = bCurrentClient;
            strncpy_s(msgUserSetup.m_szIpAddress, sizeof(msgUserSetup.m_szIpAddress), sIpAddress.c_str(), sIpAddress.length());
            strncpy_s(msgUserSetup.m_szMapFilename, sizeof(msgUserSetup.m_szMapFilename), sMapFilename.c_str(), sMapFilename.length());

            return true;
        }

        bool m_bCurrentClient;
        char m_szIpAddress[pge_network::MsgUserConnectedServerSelf::nIpAddressMaxLength];
        char m_szMapFilename[MsgMapChangeFromServer::nMapFilenameMaxLength];
    };  // struct MsgUserSetupFromServer
    static_assert(std::is_trivial_v<MsgUserSetupFromServer>);
    static_assert(std::is_trivially_copyable_v<MsgUserSetupFromServer>);
    static_assert(std::is_standard_layout_v<MsgUserSetupFromServer>);

    // client -> server -> clients
    // MsgUserNameChange messages are first sent by client to server to request user name change, for which the server will reply back the
    // same kind of message. Since server is in charge for ensuring unique user names for all players, it might generate a unique user name
    // for the user in case of name collision. A response message is always sent back to the clients, containing either the accepted or the
    // newly generated user name, which must be accepted by the clients.
    // Server also injects this message to itself, names are handled in the same code path.
    struct MsgUserNameChange
    {
        static const PRooFPSappMsgId id = PRooFPSappMsgId::UserNameChange;
        static const uint8_t nUserNameBufferLength = 11;  // for now very short, to keep the frag table look nice

        static bool initPkt(
            pge_network::PgePacket& pkt,
            const pge_network::PgeNetworkConnectionHandle& connHandleServerSide,
            bool bCurrentClient,
            const std::string& sUserName)
        {
            // although preparePktMsgAppFill() does runtime check, we should fail already at compile-time if msg is too big!
            static_assert(sizeof(MsgUserNameChange) <= pge_network::MsgApp::nMaxMessageLengthBytes, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::initPktMsgApp(pkt, connHandleServerSide);

            pge_network::TByte* const pMsgAppData = pge_network::PgePacket::preparePktMsgAppFill(
                pkt, static_cast<pge_network::MsgApp::TMsgId>(id), sizeof(MsgUserNameChange));
            if (!pMsgAppData)
            {
                return false;
            }

            proofps_dd::MsgUserNameChange& msgUserNameChange = reinterpret_cast<proofps_dd::MsgUserNameChange&>(*pMsgAppData);
            msgUserNameChange.m_bCurrentClient = bCurrentClient;
            strncpy_s(msgUserNameChange.m_szUserName, nUserNameBufferLength, sUserName.c_str(), sUserName.length());

            return true;
        }  // initPkt()

        bool m_bCurrentClient; // TODO: this could be removed once every instance saves its server-side connection handle
        char m_szUserName[nUserNameBufferLength];
    };  // MsgUserNameChange
    static_assert(std::is_trivial_v<MsgUserNameChange>);
    static_assert(std::is_trivially_copyable_v<MsgUserNameChange>);
    static_assert(std::is_standard_layout_v<MsgUserNameChange>);

    // clients -> server + server self (inject)
    // MsgUserCmdFromClient messages are sent from clients to server, so server will do sg and then update all the clients with MsgUserUpdateFromServer
    struct MsgUserCmdFromClient
    {
        static const PRooFPSappMsgId id = PRooFPSappMsgId::UserCmdFromClient;

        static bool initPkt(
            pge_network::PgePacket& pkt,
            proofps_dd::Strafe strafe,
            bool bAttack,
            bool bCrouch,
            TPureFloat fPlayerAngleY,
            TPureFloat fWeaponAngleZ)
        {
            // although preparePktMsgAppFill() does runtime check, we should fail already at compile-time if msg is too big!
            static_assert(sizeof(MsgUserCmdFromClient) <= pge_network::MsgApp::nMaxMessageLengthBytes, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::initPktMsgApp(pkt, 0u /*m_connHandleServerSide is ignored in this message*/);

            pge_network::TByte* const pMsgAppData = pge_network::PgePacket::preparePktMsgAppFill(
                pkt, static_cast<pge_network::MsgApp::TMsgId>(id), sizeof(MsgUserCmdFromClient));
            if (!pMsgAppData)
            {
                return false;
            }

            proofps_dd::MsgUserCmdFromClient& msgUserCmdMove = reinterpret_cast<proofps_dd::MsgUserCmdFromClient&>(*pMsgAppData);
            msgUserCmdMove.m_bShouldSend = false;
            msgUserCmdMove.m_strafe = strafe;
            msgUserCmdMove.m_bShootAction = bAttack;
            msgUserCmdMove.m_bCrouch = bCrouch;
            msgUserCmdMove.m_fPlayerAngleY = fPlayerAngleY;
            msgUserCmdMove.m_fWpnAngleZ = fWeaponAngleZ;

            return true;
        }

        static void setKeybd(
            pge_network::PgePacket& pkt,
            const Strafe& strafe,
            bool bJump,
            bool bSwitchToRunning,
            bool bCrouch,
            bool bRequestReload,
            unsigned char cWeaponSwitch)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            proofps_dd::MsgUserCmdFromClient& msgUserCmdMove = pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserCmdFromClient>(pkt);
            msgUserCmdMove.m_bShouldSend = true;
            msgUserCmdMove.m_strafe = strafe;
            msgUserCmdMove.m_bJumpAction = bJump;
            msgUserCmdMove.m_bSendSwitchToRunning = bSwitchToRunning;
            msgUserCmdMove.m_bCrouch = bCrouch;
            msgUserCmdMove.m_bRequestReload = bRequestReload;
            msgUserCmdMove.m_cWeaponSwitch = cWeaponSwitch;
        }

        static unsigned char getWeaponSwitch(const pge_network::PgePacket& pkt)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            const proofps_dd::MsgUserCmdFromClient& msgUserCmdMove = pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserCmdFromClient>(pkt);
            return msgUserCmdMove.m_cWeaponSwitch;
        }

        static void SetWeaponSwitch(pge_network::PgePacket& pkt, unsigned char cTargetWpnKey)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            proofps_dd::MsgUserCmdFromClient& msgUserCmdMove = pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserCmdFromClient>(pkt);
            msgUserCmdMove.m_bShouldSend = true;
            msgUserCmdMove.m_cWeaponSwitch = cTargetWpnKey;
        }

        static bool getReloadRequest(const pge_network::PgePacket& pkt)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            const proofps_dd::MsgUserCmdFromClient& msgUserCmdMove = pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserCmdFromClient>(pkt);
            return msgUserCmdMove.m_bRequestReload;
        }

        static void setMouse(
            pge_network::PgePacket& pkt,
            bool bShootAction)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            proofps_dd::MsgUserCmdFromClient& msgUserCmdMove = pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserCmdFromClient>(pkt);
            msgUserCmdMove.m_bShouldSend = true;
            msgUserCmdMove.m_bShootAction = bShootAction;
        }

        static void setAngleY(
            pge_network::PgePacket& pkt,
            TPureFloat fPlayerAngleY)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            proofps_dd::MsgUserCmdFromClient& msgUserCmdMove = pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserCmdFromClient>(pkt);
            msgUserCmdMove.m_bShouldSend = true;
            msgUserCmdMove.m_fPlayerAngleY = fPlayerAngleY;
        }

        static void setWpnAngles(
            pge_network::PgePacket& pkt,
            TPureFloat fWpnAngleZ)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            proofps_dd::MsgUserCmdFromClient& msgUserCmdMove = pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserCmdFromClient>(pkt);
            msgUserCmdMove.m_bShouldSend = true;
            msgUserCmdMove.m_fWpnAngleZ = fWpnAngleZ;
        }

        static bool shouldSend(
            const pge_network::PgePacket& pkt)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            const proofps_dd::MsgUserCmdFromClient& msgUserCmdMove = pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgUserCmdFromClient>(pkt);
            return msgUserCmdMove.m_bShouldSend;
        }

        bool m_bShouldSend;
        Strafe m_strafe;                 // continuous op
        bool m_bJumpAction;
        bool m_bSendSwitchToRunning;
        bool m_bRequestReload;
        unsigned char m_cWeaponSwitch;
        bool m_bShootAction;             // continuous op
        bool m_bCrouch;                  // continuous op
        TPureFloat m_fPlayerAngleY;
        TPureFloat m_fWpnAngleZ;
    };  // struct MsgUserCmdFromClient
    static_assert(std::is_trivial_v<MsgUserCmdFromClient>);
    static_assert(std::is_trivially_copyable_v<MsgUserCmdFromClient>);
    static_assert(std::is_standard_layout_v<MsgUserCmdFromClient>);

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
            TPureFloat fWpnAngleZ,
            bool bCrouch,
            int nHealth,
            bool bRespawn,
            int nFrags,
            int nDeaths)
        {
            // although preparePktMsgAppFill() does runtime check, we should fail already at compile-time if msg is too big!
            static_assert(sizeof(MsgUserUpdateFromServer) <= pge_network::MsgApp::nMaxMessageLengthBytes, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::initPktMsgApp(pkt, connHandleServerSide);

            pge_network::TByte* const pMsgAppData = pge_network::PgePacket::preparePktMsgAppFill(
                pkt, static_cast<pge_network::MsgApp::TMsgId>(id), sizeof(MsgUserUpdateFromServer));
            if (!pMsgAppData)
            {
                return false;
            }

            proofps_dd::MsgUserUpdateFromServer& msgUserCmdUpdate = reinterpret_cast<proofps_dd::MsgUserUpdateFromServer&>(*pMsgAppData);
            msgUserCmdUpdate.m_pos.x = x;
            msgUserCmdUpdate.m_pos.y = y;
            msgUserCmdUpdate.m_pos.z = z;
            msgUserCmdUpdate.m_fPlayerAngleY = fPlayerAngleY;
            msgUserCmdUpdate.m_fWpnAngleZ = fWpnAngleZ;
            msgUserCmdUpdate.m_bCrouch = bCrouch;
            msgUserCmdUpdate.m_nHealth = nHealth;
            msgUserCmdUpdate.m_bRespawn = bRespawn;
            msgUserCmdUpdate.m_nFrags = nFrags;
            msgUserCmdUpdate.m_nDeaths = nDeaths;

            return true;
        }

        // important: the data members here should be kept in sync with the PgeOldNewValue data members of Player class!
        // basically what we have here should be the data evaluated by Player.isDirty() and handled in handleUserUpdateFromServer().
        TXYZ m_pos;
        TPureFloat m_fPlayerAngleY;
        TPureFloat m_fWpnAngleZ;
        bool m_bCrouch;
        int m_nHealth;
        bool m_bRespawn;
        int m_nFrags;
        int m_nDeaths;
    };  // struct MsgUserUpdateFromServer
    static_assert(std::is_trivial_v<MsgUserUpdateFromServer>);
    static_assert(std::is_trivially_copyable_v<MsgUserUpdateFromServer>);
    static_assert(std::is_standard_layout_v<MsgUserUpdateFromServer>);

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
            const TPureFloat sz,
            const TPureFloat speed,
            const TPureFloat gravity,
            const TPureFloat drag)
        {
            // although preparePktMsgAppFill() does runtime check, we should fail already at compile-time if msg is too big!
            static_assert(sizeof(MsgBulletUpdateFromServer) <= pge_network::MsgApp::nMaxMessageLengthBytes, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::initPktMsgApp(pkt, connHandleServerSide, pge_network::PgePacket::AutoFill::NONE);

            pge_network::TByte* const pMsgAppData = pge_network::PgePacket::preparePktMsgAppFill(
                pkt, static_cast<pge_network::MsgApp::TMsgId>(id), sizeof(MsgBulletUpdateFromServer));
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
            msgBulletUpdate.m_fSpeed = speed;
            msgBulletUpdate.m_fGravity = gravity;
            msgBulletUpdate.m_fDrag = drag;
            msgBulletUpdate.m_bDelete = false;

            return true;
        }

        // this version doesnt call memset, so other properties left as garbage values
        static bool initPktForDeleting_WithGarbageValues(
            pge_network::PgePacket& pkt,
            const pge_network::PgeNetworkConnectionHandle& connHandleServerSide,
            const Bullet::BulletId bulletId)
        {
            // although preparePktMsgAppFill() does runtime check, we should fail already at compile-time if msg is too big!
            static_assert(sizeof(MsgBulletUpdateFromServer) <= pge_network::MsgApp::nMaxMessageLengthBytes, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::initPktMsgApp(pkt, connHandleServerSide, pge_network::PgePacket::AutoFill::NONE);

            pge_network::TByte* const pMsgAppData = pge_network::PgePacket::preparePktMsgAppFill(
                pkt, static_cast<pge_network::MsgApp::TMsgId>(id), sizeof(MsgBulletUpdateFromServer));
            if (!pMsgAppData)
            {
                return false;
            }

            proofps_dd::MsgBulletUpdateFromServer& msgBulletUpdate = reinterpret_cast<proofps_dd::MsgBulletUpdateFromServer&>(*pMsgAppData);
            msgBulletUpdate.m_bulletId = bulletId;
            msgBulletUpdate.m_bDelete = true;

            return true;
        }

        static bool& getDelete(pge_network::PgePacket& pkt)
        {
            // TODO: later we should offset pMsgApp because other messages might be already inside this pkt!
            proofps_dd::MsgBulletUpdateFromServer& msgBulletUpdate = pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgBulletUpdateFromServer>(pkt);
            return msgBulletUpdate.m_bDelete;
        }

        Bullet::BulletId m_bulletId;
        TXYZ m_pos;
        TXYZ m_angle;
        TXYZ m_size;
        TPureFloat m_fSpeed;
        TPureFloat m_fGravity;
        TPureFloat m_fDrag;
        bool m_bDelete;
    };  // struct MsgBulletUpdateFromServer
    static_assert(std::is_trivial_v<MsgBulletUpdateFromServer>);
    static_assert(std::is_trivially_copyable_v<MsgBulletUpdateFromServer>);
    static_assert(std::is_standard_layout_v<MsgBulletUpdateFromServer>);

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
            static_assert(sizeof(MsgMapItemUpdateFromServer) <= pge_network::MsgApp::nMaxMessageLengthBytes, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::initPktMsgApp(pkt, connHandleServerSide);

            pge_network::TByte* const pMsgAppData = pge_network::PgePacket::preparePktMsgAppFill(
                pkt, static_cast<pge_network::MsgApp::TMsgId>(id), sizeof(MsgMapItemUpdateFromServer));
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
    static_assert(std::is_trivial_v<MsgMapItemUpdateFromServer>);
    static_assert(std::is_trivially_copyable_v<MsgMapItemUpdateFromServer>);
    static_assert(std::is_standard_layout_v<MsgMapItemUpdateFromServer>);

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
            static_assert(sizeof(MsgWpnUpdateFromServer) <= pge_network::MsgApp::nMaxMessageLengthBytes, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::initPktMsgApp(pkt, connHandleServerSide);

            pge_network::TByte* const pMsgAppData = pge_network::PgePacket::preparePktMsgAppFill(
                pkt, static_cast<pge_network::MsgApp::TMsgId>(id), sizeof(MsgWpnUpdateFromServer));
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
            proofps_dd::MsgWpnUpdateFromServer& msgWpnUpdate = pge_network::PgePacket::getMsgAppDataFromPkt<proofps_dd::MsgWpnUpdateFromServer>(pkt);
            return msgWpnUpdate.m_bAvailable;
        }

        char m_szWpnName[nWpnNameNameMaxLength];
        bool m_bAvailable;
        unsigned int m_nMagBulletCount;
        unsigned int m_nUnmagBulletCount;
    };  // struct MsgWpnUpdateFromServer
    static_assert(std::is_trivial_v<MsgWpnUpdateFromServer>);
    static_assert(std::is_trivially_copyable_v<MsgWpnUpdateFromServer>);
    static_assert(std::is_standard_layout_v<MsgWpnUpdateFromServer>);

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
            static_assert(sizeof(MsgCurrentWpnUpdateFromServer) <= pge_network::MsgApp::nMaxMessageLengthBytes, "msg size");

            // TODO: initPkt to be invoked only once by app, in future it might already contain some message we shouldnt zero out!
            pge_network::PgePacket::initPktMsgApp(pkt, connHandleServerSide);

            pge_network::TByte* const pMsgAppData = pge_network::PgePacket::preparePktMsgAppFill(
                pkt, static_cast<pge_network::MsgApp::TMsgId>(id), sizeof(MsgCurrentWpnUpdateFromServer));
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
    static_assert(std::is_trivial_v<MsgCurrentWpnUpdateFromServer>);
    static_assert(std::is_trivially_copyable_v<MsgCurrentWpnUpdateFromServer>);
    static_assert(std::is_standard_layout_v<MsgCurrentWpnUpdateFromServer>);

} // namespace proofps_dd
