#pragma once

#include <JuceHeader.h>

#include "UdpSender.h"

using namespace juce;

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public Component
                     , public IUdpRcClientListener
                     //, private juce::AsyncUpdater
                     , private Button::Listener
                     , private Slider::Listener
                     , private ComboBox::Listener
                     //, private Value::Listener
                     , private Timer
{
public:
    //==============================================================================
    static const inline float constDefSliderVal { 0.0f };

    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    String ampName{ "NeuralPiRc" };

    const Identifier gainName{ "gain" };
    const Identifier masterName{ "master" };
    const Identifier bassName{ "bass" };
    const Identifier midName{ "mid" };
    const Identifier trebleName{ "treble" };
    const Identifier presenceName{ "presence" };
    const Identifier delayName{ "delay" };
    const Identifier reverbName{ "reverb" };

    const Identifier modelName{ "model" };
    const Identifier irName{ "ir" };


//    std::function<void(int sliderIndex, float sliderValue)> getUpdateCallback();

    // This function is called when data is received from the UDP thread.
    //void postDataToUI()
    //{
    //    triggerAsyncUpdate();
    //}

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    //NeuralPiAudioProcessor& processor;
    //juce::Atomic<float> knobValues[EKnobId_MAX];  // Atomic floats for thread-safe updates

    Image background = ImageCache::getFromMemory(BinaryData::npi_background_jpg, BinaryData::npi_background_jpgSize);
    juce::LookAndFeel_V4 blueLookAndFeel;
    juce::LookAndFeel_V4 redLookAndFeel;

    // UDP connector
    std::unique_ptr<UdpSender> m_conn;
     
    ComboBox modelSelect;
    ComboBox irSelect;

    IdPtrMap<NpRpcProto::EComboBoxId, juce::ComboBox*, static_cast<int>(NpRpcProto::EComboBoxId::MAX)> m_cboxMap;

    //ComboBox* modelCbox[static_cast<size_t>(EModelId::MAX)] = { &modelSelect, &irSelect};

    Slider ampGainKnob;
    Slider ampMasterKnob;
    Slider ampBassKnob;
    Slider ampMidKnob;
    Slider ampTrebleKnob;
    Slider ampPresenceKnob;
    Slider ampDelayKnob;
    Slider ampReverbKnob;

    IdPtrMap<NpRpcProto::ESliderId, juce::Slider*, static_cast<int>(NpRpcProto::ESliderId::MAX)> m_sliderMap;

    void initKnobSlider(juce::Slider& slider, float value, SliderListener* listener);

    Label GainLabel;
    Label LevelLabel;
    Label BassLabel;
    Label MidLabel;
    Label TrebleLabel;
    Label PresenceLabel;
    Label DelayLabel;
    Label ReverbLabel;
    Label toneDropDownLabel;
    Label irDropDownLabel;
    Label versionLabel;


    TextButton loadButton;
    TextButton loadIR;
    ToggleButton irButton;
    ToggleButton lstmButton;

    void updateToggleState(juce::Button* button, juce::String name);

    virtual void buttonClicked(Button* button) override;
    virtual void sliderValueChanged(Slider* slider) override;
    virtual void comboBoxChanged(ComboBox* cbox) override;

    Label ampNameLabel{ {}, "Amp Name (no spaces): " };
    Label ampNameField{ {}, "NeuralPi" };

    Label gainLabel{ {}, "Gain" };
    Label masterLabel{ {}, "Master" };

    Label modelLabel{ {}, "Model" };

    TextButton connectButton;
    TextButton scanButton;
    ComboBox ipCbox;

    void onConnectClicker();
    void onAbortClicker();
    void onScanClicked();



    Label& getOutConnectedLabel();
    Label& getInConnectedLabel();

    void timerCallback() override;

    void setParamKnobColor(int params);

    // IUdpClientListener impl
    virtual void updateKnob(int id, float value) override;
    virtual void updateModelIndex(int id, int index) override;
    virtual void addModelItem(int id, String itemValue, int itemIndex) override;
    virtual void onStateChanged(IUdpRcListener::EState prevState, IUdpRcListener::EState state) override;
    virtual void onBrReceived(const juce::String addr) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
