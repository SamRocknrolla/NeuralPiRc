#pragma once

#include <JuceHeader.h>

#include "SimpleOscMsg.h"

using namespace juce;


class NpRpcProto {
public:
    static const juce::int32 NPRPC_VER{ 0x77770001 };
    static const juce::int32 NPRPC_INV_SESS_ID{ -1 };
    static const juce::int32 NPRPC_INV_SESS_TS{ -1 };

    enum class EPacketType
    {
        Unknown = 0x0
        , ConnectReq     = 0x01
        , ConnectRes     = 0x02
        , AbortReq       = 0x03
        , HeartbeatReq   = 0x04
        , HeartbeatRes   = 0x05
        , AddModelMsg    = 0x20
        , UpdateKnobMsg  = 0x21
        , SelectModelMsg = 0x22
        , BroadcastReq   = 0xFF
        , BroadcastRes   = 0xFE
    };

    enum EHeader
    {
          EHeader_Version = 0
        , EHeader_SessionId = 1
        , EHeader_Type = 2
    };

    enum HeartbeatReq
    {
        HeartbeatReq_SessTimeMs = 3
    };

    enum EHeartbeatRes
    {
        EHeartbeatRes_Counter = 3
    };

    enum EAddModelMsg
    {
        EAddModelMsg_ModelId = 3
        , EAddModelMsg_ItemIndex = 4
        , EAddModelMsg_ItemText = 5
    };

    enum EUpdateKnobMsg {
        EUpdateKnobMsg_KnobId = 3
        , EUpdateKnobMsg_KnobValue = 4
    };

    enum ESelectModel {
        ESelectModel_ModelId = 3
        , ESelectModel_ItemIndex = 4
    };

    inline static const juce::String NRPC_BCAST_CH   = "/NpRpc/bcast";
    inline static const juce::String NRPC_CONNECT_CH = "/NpRpc/connect";
    inline static const juce::String NRPC_KNOB_CH    = "/NpRpc/knob";
    inline static const juce::String NRPC_MODEL_CH   = "/NpRpc/model";

private:
        inline static const std::unordered_map<EPacketType, juce::String> oscAddrMap = {
          { EPacketType::ConnectReq     , NRPC_CONNECT_CH }
        , { EPacketType::ConnectRes     , NRPC_CONNECT_CH }
        , { EPacketType::AbortReq       , NRPC_CONNECT_CH }
        , { EPacketType::HeartbeatReq   , NRPC_CONNECT_CH }
        , { EPacketType::HeartbeatRes   , NRPC_CONNECT_CH }
        , { EPacketType::AddModelMsg    , NRPC_CONNECT_CH }
        , { EPacketType::UpdateKnobMsg  , NRPC_KNOB_CH    }
        , { EPacketType::SelectModelMsg , NRPC_MODEL_CH   }
        , { EPacketType::BroadcastReq   , NRPC_BCAST_CH   }
        , { EPacketType::BroadcastRes   , NRPC_BCAST_CH   }
    };

public:
    static SimpleOscMsg genConnectReq() {
        return genHeader(EPacketType::ConnectReq, NPRPC_INV_SESS_ID);
    }

    static SimpleOscMsg genConnectRes(int32 sessionId) {
        return genHeader(EPacketType::ConnectRes, sessionId);
    }

    static SimpleOscMsg genAbortReq(int32 sessionId) {
        return genHeader(EPacketType::AbortReq, sessionId);
    }

    static SimpleOscMsg genHeartbeatReq(int32 sessionId, int32 sessionTimeMs) {
        SimpleOscMsg msg = genHeader(EPacketType::HeartbeatReq, sessionId);
        msg.AddInt32(sessionTimeMs);
        return msg;
    }

    static SimpleOscMsg genHeartbeatRes(int32 sessionId, int32 heartbeatCounter) {
        SimpleOscMsg msg = genHeader(EPacketType::HeartbeatRes, sessionId);
        msg.AddInt32(heartbeatCounter);
        return msg;
    }

    static SimpleOscMsg genAddModelMsg(int32 sessionId, int32 id, String itemText, int32 itemIndex) {
        SimpleOscMsg msg = genHeader(EPacketType::AddModelMsg, sessionId);
        msg.AddInt32(id);
        msg.AddInt32(itemIndex);
        msg.AddString(itemText.toStdString());
        return msg;
    }
    
    static SimpleOscMsg genUpdateKnobMsg(int32 sessionId, int32 id, float val) {
        SimpleOscMsg msg = genHeader(EPacketType::UpdateKnobMsg, sessionId);
        msg.AddInt32(id);
        msg.AddFloat32(val);
        return msg;
    }

    static SimpleOscMsg genSelectModelMsg(int32 sessionId, int32 id, int32 itemIndex) {
        SimpleOscMsg msg = genHeader(EPacketType::SelectModelMsg, sessionId);
        msg.AddInt32(id);
        msg.AddInt32(itemIndex);
        return msg;
    }

    static SimpleOscMsg genBroadcastReq() {
        return genHeader(EPacketType::BroadcastReq, NPRPC_INV_SESS_ID);
    }

    static SimpleOscMsg genBroadcastRes() {
        return genHeader(EPacketType::BroadcastRes, NPRPC_INV_SESS_ID);
    }

private:
    static SimpleOscMsg genHeader(EPacketType type, int32 sessionId) {
        SimpleOscMsg msg;

        msg.setAddress(oscAddrMap.find(type)->second.toStdString());
        msg.AddInt32(NPRPC_VER);
        msg.AddInt32(sessionId);
        msg.AddInt32(static_cast<int32_t>(type));

        return msg;
    }
};

