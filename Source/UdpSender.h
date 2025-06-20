#pragma once

#include "NeuralPiProto.h"
#include "ElapsedTimer.h"

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
        , Error = 0xff
    };

    enum class EHbState {
        Idle
        , Ready
        , Wait
        , Error = 0xff
    };

    virtual void updateKnob(int id, float value) = 0;
    virtual void updateStrList(int id, String value) = 0;
    virtual void onStateChanged(IUdpListener::EState prevState, IUdpListener::EState state) = 0;
    virtual void onBrReceived(const juce::String addr) = 0;

    ~IUdpListener() = default;
};

class UdpSender : public juce::Thread
{
public:

    //using DataCallback  = std::function<void(int id, float value)>;
    //using StateCallback = std::function<void(UdpSender::EState state)>;
    //using ScanCallback  = std::function<void(std::vector<juce::String> addrList)>;

    explicit UdpSender(int port, const juce::String mcastAddr, IUdpListener& listener)
        : juce::Thread("UdpReceiverThread")
        , m_rxPort(port)
        , m_txPort(port - 1)
        , m_mcastAddr(mcastAddr)
        , m_udpRxSocket(true)
        , m_udpTxSocket(true)
        , m_listener(listener)
        , m_state(IUdpListener::EState::Idle)
        , m_hbState(IUdpListener::EHbState::Idle)
        , m_hbCount(0)
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
                    if (bytesRead == UdpPacketHdr::NPRPC_BCAST_RES.size() && 
                        memcmp(buffer.getData(), UdpPacketHdr::NPRPC_BCAST_RES.data(), UdpPacketHdr::NPRPC_BCAST_RES.size()) == 0)
                    {
                        // Thread-safe UI update
                        juce::MessageManager::callAsync([&updater = m_listener, resIpStr]() {
                            updater.onBrReceived(resIpStr);
                        });
                    }
                    else if (bytesRead > sizeof(UdpPacketHdr)) {
                        // Parse the UDP header
                        UdpPacketHdr* hdr = reinterpret_cast<UdpPacketHdr*>(buffer.getData());
                        if (hdr->protoId == UdpPacketHdr::NPRPC_PROTO_ID && hdr->ver == UdpPacketHdr::NPRPC_VER) {
                            switch (hdr->packetType) {
                            case UdpPacketHdr::EPacketType::ConnectRes:
                                setState(IUdpListener::EState::Connected);
                                break;
                            case UdpPacketHdr::EPacketType::HeartbeatRes:
                                setState(IUdpListener::EState::Connected);
                                m_hbCount = 0;
                                setHbState(IUdpListener::EHbState::Ready);
                                break;
                            case UdpPacketHdr::EPacketType::SliderUpdate:
                                if (bytesRead >= sizeof(UdpPacketHdr) + sizeof(UdpSliderUpdate)) {
                                    UdpSliderUpdate* sliderUpd = reinterpret_cast<UdpSliderUpdate*>(hdr->packetData);
                                    // Thread-safe UI update
                                    juce::MessageManager::callAsync([&updater = m_listener, sliderUpd]() {
                                        updater.updateKnob(sliderUpd->index, sliderUpd->value);
                                        });
                                }
                                break;
                            case UdpPacketHdr::EPacketType::StrListUpdate:
                                if (bytesRead >= sizeof(UdpPacketHdr) + sizeof(UdpStrUpdate)) {
                                    UdpStrUpdate* strUpd = reinterpret_cast<UdpStrUpdate*>(hdr->packetData);
                                    // Thread-safe UI update
                                    juce::MessageManager::callAsync([&updater = m_listener, strUpd]() {
                                        updater.updateStrList(strUpd->index, String::fromUTF8(strUpd->data, strUpd->size));
                                     });
                                }
                                break;
                            default:
                                break;
                            }
                            // Parse the UDP Slider Update
                        }
                    }
                    // Notify the UI thread to update
                    //uiComponent->postDataToUI();
                }
            }
            stateStep();
            //else
            //{
            //    DBG("UDP socket timed out while waiting for data.");
            //}
        }
        DBG("UdpReceiverThread <=");
    }

    bool scan() {
        bool ret = m_state.compareAndSetBool(IUdpListener::EState::ReqScan, IUdpListener::EState::Idle);
        return ret;
    }

    bool connect(String addr) {
        m_addr = addr;
        bool ret = m_state.compareAndSetBool(IUdpListener::EState::ReqConnect, IUdpListener::EState::Idle);
        if (ret) {
            m_reqTimer.start(CFG_TIMEOUT_MS);
        }
        return ret;
    }

private:
    const int SCAN_TIMEOUT_MS{ 1000 };
    const int CFG_TIMEOUT_MS{ 2000 };
    const int HEARTBEAT_PERIOD_MS{ 1000 };
    const int HEARTBEAT_MAX{ 3 };

//    void sendUdp(const std::string& buf)
//    {
//        const juce::ScopedLock lock(m_udpTxLock);
//        m_udpTxQueue.add(juce::MemoryBlock(buf.data(), buf.size()));
//        notify();
//    }
//
//    void sendUdp(const juce::MemoryBlock& buf)
//    {
//        const juce::ScopedLock lock(m_udpTxLock);
//        m_udpTxQueue.add(buf);
//        notify();
//    }

    void setState(IUdpListener::EState newState) {
        IUdpListener::EState curState = m_state.get();
        if (newState != curState) {
            DBG("State: " << static_cast<int>(curState) << " => " << static_cast<int>(newState));
            m_state = newState;
            // Thread-safe UI update
            juce::MessageManager::callAsync([&updater = m_listener, curState, newState]() {
                updater.onStateChanged(curState, newState);
            });
        }
        else {
            DBG("Already in state: " << static_cast<int>(newState));
        }
    }

    void stateStep() {
        switch (m_state.get()) {
        case IUdpListener::EState::ReqScan:
            {
                if (m_udpTxSocket.write(m_mcastAddr, m_txPort,
                    UdpPacketHdr::NPRPC_BCAST_REQ.data(), UdpPacketHdr::NPRPC_BCAST_REQ.size()) < 0) {
                    setState(IUdpListener::EState::Error);
                }
                else {
                    setState(IUdpListener::EState::Scanning);
                    m_scanTimer.start(SCAN_TIMEOUT_MS);
                }
            }
            break;
        case IUdpListener::EState::Scanning:
            {
                if (!m_scanTimer.isValid() || m_scanTimer.IsElapsed()) {
                    setState(IUdpListener::EState::Idle);
                }
            }
            break;
        case IUdpListener::EState::ReqConnect:
            {
                juce::MemoryBlock req;
                UdpPacketHdr hdr;
                int ret;
                hdr.sessionId = 1;
                hdr.packetType = UdpPacketHdr::EPacketType::ConnectReq;
                hdr.toMemoryBlock(req);
                if (m_udpTxSocket.write(m_addr, m_txPort, req.getData(), req.getSize()) < 0) {
                    setState(IUdpListener::EState::Error);
                }
                else {
                    m_reqTimer.start(CFG_TIMEOUT_MS);
                    setState(IUdpListener::EState::Connecting);
                }
            }
            break;
        case IUdpListener::EState::Connecting:
            if (!m_reqTimer.isValid() || m_reqTimer.IsElapsed()) {
                setState(IUdpListener::EState::Idle);
            }
            break;
        case IUdpListener::EState::Connected:
            stepHbState();
            break;
        default:
            break;
        }
    }

    void setHbState(IUdpListener::EHbState newState) {
        IUdpListener::EHbState curState = m_hbState.get();
        if (newState != curState) {
            DBG("State: " << static_cast<int>(curState) << " => " << static_cast<int>(newState));
            m_hbState = newState;
        }
        else {
            DBG("Already in state: " << static_cast<int>(newState));
        }
    }

    void stepHbState() {
        switch (m_hbState.get()) {
        case IUdpListener::EHbState::Ready:
            {
                juce::MemoryBlock req;
                UdpPacketHdr hdr;
                int ret;
                hdr.sessionId = 1;
                hdr.packetType = UdpPacketHdr::EPacketType::HeartbeatReq;
                hdr.toMemoryBlock(req);
                if (m_udpTxSocket.write(m_addr, m_txPort, req.getData(), req.getSize()) < 0) {
                    setState(IUdpListener::EState::Error);
                }
                else {
                    m_hbTimer.start(HEARTBEAT_PERIOD_MS);
                    setHbState(IUdpListener::EHbState::Wait);
                }
            }
            break;
        case IUdpListener::EHbState::Wait:
            if (!m_hbTimer.isValid() || m_hbTimer.IsElapsed()) {
                m_hbCount++;
                if (m_hbCount < HEARTBEAT_MAX) {
                    setHbState(IUdpListener::EHbState::Ready);
                }
                else {
                    m_hbCount = 0;
                    setHbState(IUdpListener::EHbState::Idle);
                    setState(IUdpListener::EState::Idle);
                }
            }
            break;
        case IUdpListener::EHbState::Idle:
        default:
            break;
        }
    }


private:
    int m_rxPort;
    int m_txPort;
    juce::String m_addr;
    juce::String m_mcastAddr;
    juce::DatagramSocket m_udpRxSocket;
    juce::DatagramSocket m_udpTxSocket;

//    juce::CriticalSection m_udpTxLock;
//    juce::Array<juce::MemoryBlock> m_udpTxQueue;

    IUdpListener& m_listener;

    juce::Atomic<IUdpListener::EState> m_state;
    juce::Atomic<IUdpListener::EHbState> m_hbState;
    int m_hbCount;

    ElapsedTimer m_scanTimer;
    ElapsedTimer m_reqTimer;
    ElapsedTimer m_hbTimer;
};
