#pragma once

#include "cmn/IUdpRcListener.h"
#include "cmn/NeuralPiProto.h"
#include "cmn/ElapsedTimer.h"
#include "cmn/SimpleOscMsg.h"

#include <JuceHeader.h>
#include <functional>

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
        , m_sessionId(NpRpcProto::NPRPC_INV_SESS_ID)
        , m_sessionTs(NpRpcProto::NPRPC_INV_SESS_TS)
        , m_hbState(EHbState::Idle)
        , m_hbMissedCount(0)
    {
    }

    ~UdpSender() override
    {
        signalThreadShouldExit();
        stopThread(1000);
        m_udpRxSocket.shutdown();
        m_udpTxSocket.shutdown();
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
                    // Parse the UDP packet and check header
                    SimpleOscMsg msg;
                    if (msg.DeserializeFrom(buffer, bytesRead) && msg.size() >= 3 && 
                        msg[NpRpcProto::EHeader_Version].isInt32() &&
                        msg[NpRpcProto::EHeader_SessionId].isInt32() &&
                        msg[NpRpcProto::EHeader_Type].isInt32()) {
                        // Check protocol Version and Type
                        int32 _version = msg[NpRpcProto::EHeader_Version].getInt32();
                        int32 _sessionId = msg[NpRpcProto::EHeader_SessionId].getInt32();
                        NpRpcProto::EPacketType _type = static_cast<NpRpcProto::EPacketType>(msg[NpRpcProto::EHeader_Type].getInt32());

                        if (_version == NpRpcProto::NPRPC_VER &&
                            _type >= NpRpcProto::EPacketType::ConnectReq &&
                            _type <= NpRpcProto::EPacketType::BroadcastReq) {

                            // process broadcast response
                            if (msg.getAddress() == NpRpcProto::NRPC_BCAST_CH &&
                                _sessionId == NpRpcProto::NPRPC_INV_SESS_ID &&
                                _type == NpRpcProto::EPacketType::BroadcastRes) {

                                juce::MessageManager::callAsync([&updater = m_listener, resIpStr]() {
                                    updater.onBrReceived(resIpStr);
                                    });
                            }
                            // process OCR /NpRps/connect
                            else if (msg.getAddress() == NpRpcProto::NRPC_CONNECT_CH &&
                                     _sessionId != NpRpcProto::NPRPC_INV_SESS_ID) {

                                switch (_type) {
                                case NpRpcProto::EPacketType::AddModelMsg:
                                    if (msg[NpRpcProto::EAddModelMsg_ModelId].isInt32() &&
                                        msg[NpRpcProto::EAddModelMsg_ItemIndex].isInt32() && 
                                        msg[NpRpcProto::EAddModelMsg_ItemText].isString()) {
                                        // Thread-safe UI update
                                        juce::MessageManager::callAsync([&updater = m_listener, msg]() {
                                            updater.addModelItem(msg[NpRpcProto::EAddModelMsg_ModelId].getInt32(),
                                                                 msg[NpRpcProto::EAddModelMsg_ItemText].getString(), 
                                                                 msg[NpRpcProto::EAddModelMsg_ItemIndex].getInt32());
                                            });
                                        break;
                                    }
                                case NpRpcProto::EPacketType::ConnectRes:
                                    m_sessionId = _sessionId;
                                    m_sessionTs = Time::getMillisecondCounter();

                                    setState(IUdpListener::EState::Connected);
                                    m_hbMissedCount = 0;
                                    setHbState(EHbState::Ready);
                                    break;
                                case NpRpcProto::EPacketType::HeartbeatRes:
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
                            else if (msg.getAddress() == NpRpcProto::NRPC_KNOB_CH &&
                                _sessionId != NpRpcProto::NPRPC_INV_SESS_ID &&
                                (m_sessionId == NpRpcProto::NPRPC_INV_SESS_ID || _sessionId == m_sessionId) &&
                                _type == NpRpcProto::EPacketType::UpdateKnobMsg) {

                                DBG("UpdateKnobMsg: id: " << msg[NpRpcProto::EUpdateKnobMsg_KnobId].getInt32() << " value: " << msg[NpRpcProto::EUpdateKnobMsg_KnobValue].getFloat32());
                                juce::MessageManager::callAsync([&updater = m_listener, msg]() {
                                    updater.updateKnob(msg[NpRpcProto::EUpdateKnobMsg_KnobId].getInt32(), 
                                                       msg[NpRpcProto::EUpdateKnobMsg_KnobValue].getFloat32());
                                    });
                            }
                            // process OCR /NpRps/model
                            else if (msg.getAddress() == NpRpcProto::NRPC_MODEL_CH &&
                                _sessionId != NpRpcProto::NPRPC_INV_SESS_ID &&
                                (m_sessionId == NpRpcProto::NPRPC_INV_SESS_ID || _sessionId == m_sessionId) &&
                                _type == NpRpcProto::EPacketType::SelectModelMsg ) {

                                DBG("SelectModel: id: " << msg[NpRpcProto::ESelectModel_ModelId].getInt32() << " index: " << msg[NpRpcProto::ESelectModel_ItemIndex].getInt32());
                                juce::MessageManager::callAsync([&updater = m_listener, msg]() {
                                    updater.updateModelIndex(msg[NpRpcProto::ESelectModel_ModelId].getInt32(),
                                                             msg[NpRpcProto::ESelectModel_ItemIndex].getInt32());
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
        sendUdp(NpRpcProto::genUpdateKnobMsg(m_sessionId, id, val));
    }

    void selectModel(int32_t id, int32_t itemId) {
        sendUdp(NpRpcProto::genSelectModelMsg(m_sessionId, id, itemId));
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
            ret += (m_udpTxSocket.write(m_txAddr, m_txPort, buf.getData(), static_cast<int>(buf.getSize())) < 0)? -1 : 0;
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
            NpRpcProto::genBroadcastReq().SerializeTo(buf);
            m_udpTxSocket.write(m_mcastAddr, m_txPort, buf.getData(), static_cast<int>(buf.getSize()));

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
                sendUdp(NpRpcProto::genConnectReq());
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
                NpRpcProto::genAbortReq(m_sessionId).SerializeTo(buf);
                m_udpTxSocket.write(m_mcastAddr, m_txPort, buf.getData(), static_cast<int>(buf.getSize()));
            }
        case IUdpListener::EState::Error:
            m_sessionId = NpRpcProto::NPRPC_INV_SESS_ID;
            m_sessionTs = NpRpcProto::NPRPC_INV_SESS_TS;
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
                sendUdp(NpRpcProto::genHeartbeatReq(m_sessionId, Time::getMillisecondCounter() - m_sessionTs));
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

    ElapsedTimer m_reqTimer;
    ElapsedTimer m_hbTimer;
};
