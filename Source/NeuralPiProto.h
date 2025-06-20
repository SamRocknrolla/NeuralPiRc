#pragma once

#include <JuceHeader.h>

using namespace juce;

struct UdpPacketHdr {
    static const juce::uint16 NPRPC_PROTO_ID    { 0x7777 };
    static const juce::uint16 NPRPC_VER         { 0x0001 };
    static const juce::uint32 NPRPC_INV_SESS_ID { 0xFFFFFFFF };

    /* HACK for support bcast of legacy receiver */
    inline static const std::string NPRPC_BCAST_REQ = std::string("NeuralPiRcBroadcast") + '\x0';
    inline static const std::string NPRPC_BCAST_RES = std::string("NeuralPiRcBcoadcastAck") + '\x0';

    enum class EPacketType
    {
          Unknown        = 0x0
        , ConnectReq     = 0x01
        , ConnectRes     = 0x02
        , Abort          = 0x03
        , HeartbeatReq   = 0x04
        , HeartbeatRes   = 0x05
        , SliderUpdate   = 0x21
        , StrListUpdate  = 0x22
    };

    uint16_t protoId;
    uint16_t ver;
    uint32_t sessionId;
    EPacketType packetType;
    uint8_t packetData[];

    UdpPacketHdr()
        : protoId(NPRPC_PROTO_ID)
        , ver(NPRPC_VER)
        , sessionId(NPRPC_INV_SESS_ID)
        , packetType(UdpPacketHdr::EPacketType::Unknown)
    {

    }

    void toMemoryBlock(juce::MemoryBlock& block) const
    {
        block.append(&protoId, sizeof(protoId));
        block.append(&packetType, sizeof(packetType));
    }
};

// Structure to hold the UDP packet data (type, index, value)
struct UdpSliderUpdate
{
    int index;                // Index of the slider or combo box
    float value;              // Value of the slider or combo box index

    static const int INVALID_ID = -1;

    UdpSliderUpdate()
        : index(INVALID_ID)
        , value(1.0f) {}

    // Convert the struct to a MemoryBlock for sending over the network
    void toMemoryBlock(juce::MemoryBlock& block) const
    {
        block.append(&index, sizeof(index));
        block.append(&value, sizeof(value));
    }

    // Parse from a MemoryBlock received over the network
    static UdpSliderUpdate fromMemoryBlock(const juce::MemoryBlock& block)
    {
        UdpSliderUpdate data;
        if (block.getSize() >= sizeof(UdpSliderUpdate))
        {
            std::memcpy(&data, block.getData(), sizeof(UdpSliderUpdate));
        }
        return data;
    }
};

struct UdpStrUpdate
{
    static const size_t CBOX_VAL_MAX = 1024;
    int index;
    uint16 size;
    char data[];

    static const int INVALID_ID = -1;

    UdpStrUpdate()
        : index(INVALID_ID)
        , size(0) {}

    // Convert the struct to a MemoryBlock for sending over the network
    void toMemoryBlock(juce::MemoryBlock& block) const
    {
        block.append(&index, sizeof(index));
        block.append(&size, sizeof(size));
        block.append(data, size);
    }

    // Parse from a MemoryBlock received over the network
    static UdpStrUpdate fromMemoryBlock(const juce::MemoryBlock& block)
    {
        UdpStrUpdate data;
        if (block.getSize() >= sizeof(UdpStrUpdate))
        {
            std::memcpy(&data, block.getData(), (block.getSize() > CBOX_VAL_MAX)? CBOX_VAL_MAX : block.getSize());
        }
        return data;
    }
};

