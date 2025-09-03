#pragma once

#include <JuceHeader.h>

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

    inline static const std::unordered_map<EState, juce::String> EStateNames = {
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
    virtual void addModelItem(int id, juce::String itemValue, int itemIndex) = 0;
    virtual void onStateChanged(IUdpListener::EState prevState, IUdpListener::EState state) = 0;
    virtual void onBrReceived(const juce::String addr) = 0;

    ~IUdpListener() = default;
};

