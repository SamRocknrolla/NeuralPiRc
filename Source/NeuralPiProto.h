#pragma once

#include <JuceHeader.h>

#include "SimpleOscMsg.h"

using namespace juce;


class NpRpcMsgHelper {
public:
    static const juce::int32 NPRPC_VER{ 0x77770001 };
    static const juce::int32 NPRPC_INV_SESS_ID{ -1 };
    static const juce::int32 NPRPC_INV_SESS_TS{ -1 };

    enum class EPacketType
    {
        Unknown = 0x0
        , ConnectReq    = 0x01
        , ConnectRes    = 0x02
        , Abort         = 0x03
        , HeartbeatReq  = 0x04
        , HeartbeatRes  = 0x05
        , ModelAdd      = 0x20
        , SliderUpdate  = 0x21
        , SelectModel   = 0x22
        , bcastReq      = 0xFF
        , bcastRes      = 0xFE
    };

    inline static const juce::String NRPC_BCAST_CH   = "/NpRpc/bcast";
    inline static const juce::String NRPC_CONNECT_CH = "/NpRpc/connect";
    inline static const juce::String NRPC_KNOB_CH    = "/NpRpc/knob";
    inline static const juce::String NRPC_MODEL_CH   = "/NpRpc/model";

private:
        inline static const std::unordered_map<EPacketType, juce::String> oscAddrMap = {
          { EPacketType::ConnectReq   , NRPC_CONNECT_CH }
        , { EPacketType::ConnectRes   , NRPC_CONNECT_CH }
        , { EPacketType::Abort        , NRPC_CONNECT_CH }
        , { EPacketType::HeartbeatReq , NRPC_CONNECT_CH }
        , { EPacketType::HeartbeatRes , NRPC_CONNECT_CH }
        , { EPacketType::SliderUpdate , NRPC_KNOB_CH }
        , { EPacketType::SelectModel  , NRPC_MODEL_CH }
        , { EPacketType::bcastReq     , NRPC_BCAST_CH }
        , { EPacketType::bcastRes     , NRPC_BCAST_CH }
    };

public: 
    static void genHeader(EPacketType type, int32 sessionId, SimpleOscMsg& msg) {
        msg.clear();
        msg.setAddress(oscAddrMap.find(type)->second.toStdString());
        msg.AddInt32(NPRPC_VER);
        msg.AddInt32(sessionId);
        msg.AddInt32(static_cast<int32_t>(type));
    }
};

