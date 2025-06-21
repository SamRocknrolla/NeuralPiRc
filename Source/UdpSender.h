#pragma once

#include "NeuralPiProto.h"
#include "ElapsedTimer.h"
#include "SimpleOscMsg.h"

#include "../JuceLibraryCode/JuceHeader.h"
#include <functional>

class IUdpListener {
public:

    enum class EState {
          Idle
        , ReqScan
        , Scanning
        , ReqConnect
        , Connecting
        , Connected
        , Disconnecting = 0xfc
        , Error = 0xff
    };

    inline static const std::unordered_map<EState, String> EStateNames = {
          {EState::Idle          , "Idle"}
        , {EState::ReqScan       , "ReqScan"}
        , {EState::Scanning      , "Scanning"}
        , {EState::ReqConnect    , "ReqConnect"}
        , {EState::Connecting    , "Connecting"}
        , {EState::Connected     , "Connected"}
        , {EState::Disconnecting , "Disconnecting"}
        , {EState::Error         , "Error"}
    };     

    virtual void updateKnob(int id, float value) = 0;
    virtual void updateModelIndex(int id, int index) = 0;
    virtual void addModelItem(int id, String itemValue, int itemIndex) = 0;
    virtual void onStateChanged(IUdpListener::EState prevState, IUdpListener::EState state) = 0;
    virtual void onBrReceived(const juce::String addr) = 0;

    ~IUdpListener() = default;
};

class UdpSender : public juce::Thread
{
public:

    explicit UdpSender(int port, const juce::String mcastAddr, IUdpListener& listener)
        : juce::Thread("UdpReceiverThread")
        , m_rxPort(port)
        , m_txPort(port - 1)
        , m_mcastAddr(mcastAddr)
        , m_udpRxSocket(true)
        , m_udpTxSocket(true)
        , m_listener(listener)
        , m_state(IUdpListener::EState::Idle)
        , m_sessionId(NpRpcMsgHelper::NPRPC_INV_SESS_ID)
        , m_sessionTs(NpRpcMsgHelper::NPRPC_INV_SESS_TS)
        , m_hbState(EHbState::Idle)
        , m_hbMissedCount(0)
    {
    }

    ~UdpSender() override
    {
        signalThreadShouldExit();
        stopThread(1000);
        m_udpRxSocket.shutdown();
        //m_udpTxSocket.shutdown();
    }

    void run() override
    {
        DBG("UdpReceiverThread =>");
        if (!m_udpRxSocket.bindToPort(m_rxPort)) {
            DBG("Failed to bind UDP socket on port: " << m_rxPort);
            setState(IUdpListener::EState::Error);
            return;
        }
        if (!m_udpRxSocket.joinMulticast(m_mcastAddr)) {
            DBG("Listening for UDP packets on port: " << m_rxPort);
            setState(IUdpListener::EState::Error);
            return;
        }

        setState(IUdpListener::EState::Idle);
        while (!threadShouldExit())
        {
            juce::MemoryBlock buffer;
            //juce::IPAddress senderIp;
            int resPort;
            juce::String resIpStr;
            buffer.setSize(1024, true);

            // Wait until the socket is ready to receive data (blocking with a timeout)
            if (m_udpRxSocket.waitUntilReady(true, 10))  // Wait for data with a timeout of 10ms
            {
                // Receive a packet (non-blocking)
                int bytesRead = m_udpRxSocket.read(buffer.getData(), static_cast<int>(buffer.getSize()), false, resIpStr, resPort);

                if (bytesRead > 0)
                {
                    // Parse the UDP packet
                    SimpleOscMsg msg;
                    if (msg.DeserializeFrom(buffer, bytesRead)) {
                        // Check protocol ID and protocol version
                        if (msg.size() >= 3 &&
                            msg[0].isInt32() && msg[0].getInt32() == NpRpcMsgHelper::NPRPC_VER &&
                            msg[2].isInt32() && msg[2].getInt32() >= (uint32_t)NpRpcMsgHelper::EPacketType::ConnectReq &&
                            msg[2].isInt32() && msg[2].getInt32() <= (uint32_t)NpRpcMsgHelper::EPacketType::BroadcastReq) {

                            // process broadcast response
                            if (msg.getAddress() == NpRpcMsgHelper::NRPC_BCAST_CH &&
                                msg[1].isInt32() && msg[1].getInt32() == NpRpcMsgHelper::NPRPC_INV_SESS_ID &&
                                msg[2].getInt32() == (uint32_t)NpRpcMsgHelper::EPacketType::BroadcastRes) {
                                juce::MessageManager::callAsync([&updater = m_listener, resIpStr]() {
                                    updater.onBrReceived(resIpStr);
                                    });
                            }
                            // process OCR /NpRps/connect
                            else if (msg.getAddress() == NpRpcMsgHelper::NRPC_CONNECT_CH &&
                                     msg[1].isInt32() && msg[1].getInt32() != NpRpcMsgHelper::NPRPC_INV_SESS_ID) {

                                switch (static_cast<NpRpcMsgHelper::EPacketType>(msg[2].getInt32())) {
                                case NpRpcMsgHelper::EPacketType::AddModelMsg:
                                    if (msg[3].isInt32() && msg[4].isInt32() && msg[5].isString()) {
                                        // Thread-safe UI update
                                        juce::MessageManager::callAsync([&updater = m_listener, msg]() {
                                            updater.addModelItem(msg[3].getInt32(), msg[5].getString(), msg[4].getInt32());
                                            });
                                        break;
                                    }
                                case NpRpcMsgHelper::EPacketType::ConnectRes:
                                    m_sessionId = msg[1].getInt32();
                                    m_sessionTs = Time::getMillisecondCounter();

                                    setState(IUdpListener::EState::Connected);
                                    m_hbMissedCount = 0;
                                    setHbState(EHbState::Ready);
                                    break;
                                case NpRpcMsgHelper::EPacketType::HeartbeatRes:
                                    if (m_state.get() == IUdpListener::EState::Connected) {
                                        m_hbMissedCount = 0;
                                        setHbState(EHbState::Ready);
                                    }
                                    break;
                                default:
                                    break;
                                }
                            }
                            // process OCR /NpRps/knob
                            else if (msg.getAddress() == NpRpcMsgHelper::NRPC_KNOB_CH &&
                                msg[1].isInt32() && msg[1].getInt32() != NpRpcMsgHelper::NPRPC_INV_SESS_ID &&
                                (m_sessionId == NpRpcMsgHelper::NPRPC_INV_SESS_ID || msg[1].getInt32() == m_sessionId) &&
                                msg[2].getInt32() == (uint32_t)NpRpcMsgHelper::EPacketType::UpdateKnobMsg) {

                                juce::MessageManager::callAsync([&updater = m_listener, msg]() {
                                    updater.updateKnob(msg[3].getInt32(), msg[4].getFloat32());
                                    });
                            }
                            // process OCR /NpRps/model
                            else if (msg.getAddress() == NpRpcMsgHelper::NRPC_MODEL_CH &&
                                msg[1].isInt32() && msg[1].getInt32() != NpRpcMsgHelper::NPRPC_INV_SESS_ID &&
                                (m_sessionId == NpRpcMsgHelper::NPRPC_INV_SESS_ID || msg[1].getInt32() == m_sessionId) &&
                                msg[2].getInt32() == (uint32_t)NpRpcMsgHelper::EPacketType::SelectModelMsg ) {

                                juce::MessageManager::callAsync([&updater = m_listener, msg]() {
                                    updater.updateModelIndex(msg[3].getInt32(), msg[4].getInt32());
                                    });
                            }
                        }
                    }
                }
            }
            stateStep();
            sendTxQueue();       
//            sendMcastTxQueue();
        }
        DBG("UdpReceiverThread <=");
    }

    bool scan() {
        bool ret = m_state.compareAndSetBool(IUdpListener::EState::ReqScan, IUdpListener::EState::Idle);
        return ret;
    }

    bool connect(String addr) {
        m_txAddr = addr;
        bool ret = m_state.compareAndSetBool(IUdpListener::EState::ReqConnect, IUdpListener::EState::Idle);
        if (ret) {
            m_reqTimer.start(CFG_TIMEOUT_MS);
        }
        return ret;
    }

    bool disconnect() {
        bool ret = m_state.compareAndSetBool(IUdpListener::EState::Disconnecting, IUdpListener::EState::Connected);
        if (!ret) {
            if (m_state.get() == IUdpListener::EState::Idle) {
                ret = true;
            }
            else {
                DBG("Disconnect request ignored by state: [" << IUdpListener::EStateNames.at(m_state.get()));
            }
        }
        return ret;
    }

    void updateKnob(int32_t id, float val) {
        sendUdp(NpRpcMsgHelper::genUpdateKnobMsg(m_sessionId, id, val));
    }

    void selectModel(int32_t id, int32_t itemId) {
        sendUdp(NpRpcMsgHelper::genSelectModelMsg(m_sessionId, id, itemId));
    }

private:
    const int SCAN_TIMEOUT_MS{ 1000 };
    const int CFG_TIMEOUT_MS{ 2000 };
    const int HEARTBEAT_PERIOD_MS{ 1000 };
    const int HEARTBEAT_MAX{ 3 };

    enum class EHbState {
        Idle
        , Ready
        , Wait
        , Timeout = 0xfe
    };

    inline static const std::unordered_map<EHbState, String> EHbStateNames = {
          {EHbState::Idle    , "Idle"}
        , {EHbState::Ready   , "Ready"}
        , {EHbState::Wait    , "Wait"}
        , {EHbState::Timeout , "Timeout"}
    };

    void sendUdp(const SimpleOscMsg& msg)
    {
        juce::MemoryBlock buf;
        msg.SerializeTo(buf);

        const juce::ScopedLock lock(m_udpTxLock);
        m_udpTxQueue.add(buf);
        notify();
    }

//    void sendUdpMcast(const SimpleOscMsg& msg)
//    {
//        juce::MemoryBlock buf;
//        msg.SerializeTo(buf);
//
//        const juce::ScopedLock lock(m_mcastTxLock);
//        m_mcastTxQueue.add(buf);
//        notify();
//    }

    int sendTxQueue() {
        const juce::ScopedLock lock(m_udpTxLock);
        int ret = 0;
        while (m_udpTxQueue.size() > 0) {
            juce::MemoryBlock buf = m_udpTxQueue.removeAndReturn(0);
            ret += (m_udpTxSocket.write(m_txAddr, m_txPort, buf.getData(), buf.getSize()) < 0)? -1 : 0;
        }
        return ret;
    }

//    int sendMcastTxQueue() {
//        const juce::ScopedLock lock(m_mcastTxLock);
//        int ret = 0;
//        while (m_mcastTxQueue.size() > 0) {
//            juce::MemoryBlock buf = m_mcastTxQueue.removeAndReturn(0);
//            ret += (m_udpTxSocket.write(m_mcastAddr, m_txPort, buf.getData(), buf.getSize()) < 0) ? -1 : 0;
//        }
//        return ret;
//    }

    void setState(IUdpListener::EState newState) {
        IUdpListener::EState curState = m_state.get();
        if (newState != curState) {
            DBG("State: " << IUdpListener::EStateNames.at(curState) << " => " << IUdpListener::EStateNames.at(newState));
            m_state = newState;
            // Thread-safe UI update
            juce::MessageManager::callAsync([&updater = m_listener, curState, newState]() {
                updater.onStateChanged(curState, newState);
            });
        }
        else {
            DBG("Already in state: " << IUdpListener::EStateNames.at(newState));
        }
    }

    void stateStep() {
        switch (m_state.get()) {
        case IUdpListener::EState::ReqScan:
        {
            // Send broadcast message without TX queue
            juce::MemoryBlock buf;
            NpRpcMsgHelper::genBroadcastReq().SerializeTo(buf);
            m_udpTxSocket.write(m_mcastAddr, m_txPort, buf.getData(), buf.getSize());

            setState(IUdpListener::EState::Scanning);
            m_reqTimer.start(SCAN_TIMEOUT_MS);

            break;
        }
        case IUdpListener::EState::Scanning:
        {
            if (!m_reqTimer.isValid() || m_reqTimer.IsElapsed()) {
                setState(IUdpListener::EState::Disconnecting);
            }
            break;
        }
        case IUdpListener::EState::ReqConnect:
            {
                sendUdp(NpRpcMsgHelper::genConnectReq());
                m_reqTimer.start(CFG_TIMEOUT_MS);
                setState(IUdpListener::EState::Connecting);
            }
            break;
        case IUdpListener::EState::Connecting:
            if (!m_reqTimer.isValid() || m_reqTimer.IsElapsed()) {
                setState(IUdpListener::EState::Disconnecting);
            }
            break;
        case IUdpListener::EState::Connected:
            stepHbState();
            break;
        case IUdpListener::EState::Disconnecting:
            {
                juce::MemoryBlock buf;
                NpRpcMsgHelper::genAbortReq(m_sessionId).SerializeTo(buf);
                m_udpTxSocket.write(m_mcastAddr, m_txPort, buf.getData(), buf.getSize());
            }
        case IUdpListener::EState::Error:
            m_sessionId = NpRpcMsgHelper::NPRPC_INV_SESS_ID;
            m_sessionTs = NpRpcMsgHelper::NPRPC_INV_SESS_TS;
//            {
//                const juce::ScopedLock lock(m_mcastTxLock);
//                m_mcastTxQueue.clear();
//            }
            {
                const juce::ScopedLock lock(m_udpTxLock);
                m_udpTxQueue.clear();
            }

            m_reqTimer.stop();
            m_hbTimer.stop();
            setHbState(EHbState::Idle);
            setState(IUdpListener::EState::Idle);
            m_hbMissedCount = 0;
            break;
        default:
            break;
        }
    }

    void setHbState(EHbState newState) {
        EHbState curState = m_hbState.get();
        if (newState != curState) {
            //DBG("Heartbeat: " << EHbStateNames.at(curState) << " => " << EHbStateNames.at(newState));
            m_hbState = newState;
        }
        else {
            DBG("Heartbeat: Already in state: " << EHbStateNames.at(newState));
        }
    }

    void stepHbState() {
        switch (m_hbState.get()) {
        case EHbState::Ready:
            {
                sendUdp(NpRpcMsgHelper::genHeartbeatReq(m_sessionId, Time::getMillisecondCounter() - m_sessionTs));
                m_hbTimer.start(HEARTBEAT_PERIOD_MS);
                setHbState(EHbState::Wait);
            }
            break;
        case EHbState::Wait:
            if (!m_hbTimer.isValid() || m_hbTimer.IsElapsed()) {
                m_hbMissedCount++;
                if (m_hbMissedCount < HEARTBEAT_MAX) {
                    setHbState(EHbState::Ready);
                }
                else {
                    m_hbMissedCount = 0;
                    setState(IUdpListener::EState::Disconnecting);
                }
            }
            break;
        case EHbState::Idle:
        default:
            break;
        }
    }


private:
    int m_rxPort;
    int m_txPort;
    juce::String m_txAddr;
    juce::String m_mcastAddr;
    juce::DatagramSocket m_udpRxSocket;
    juce::DatagramSocket m_udpTxSocket;

    juce::CriticalSection m_udpTxLock;
    juce::Array<juce::MemoryBlock> m_udpTxQueue;

//    juce::CriticalSection m_mcastTxLock;
//    juce::Array<juce::MemoryBlock> m_mcastTxQueue;


    IUdpListener& m_listener;

    int32_t m_sessionId;
    int32_t m_sessionTs;
    juce::Atomic<IUdpListener::EState> m_state;
    juce::Atomic<EHbState> m_hbState;
    int m_hbSessionCount;
    int m_hbMissedCount;

    //ElapsedTimer m_scanTimer;
    ElapsedTimer m_reqTimer;
    ElapsedTimer m_hbTimer;
};
