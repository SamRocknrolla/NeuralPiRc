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
                     , private juce::Timer
                     , public IUdpRcClientListener
                     , private Button::Listener
                     , private Slider::Listener
                     , private ComboBox::Listener
{
public:
    //==============================================================================
    static const inline float constDefSliderVal { 0.0f };

    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void timerCallback() override;
    //void paint (Graphics&) override;
    void resized() override;
    void setupIU();

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

private:
#if JUCE_ANDROID
    juce::OpenGLContext openGLContext;
#endif

    Image background = ImageCache::getFromMemory(BinaryData::npi_bg2_jpg, BinaryData::npi_bg2_jpgSize);
    juce::LookAndFeel_V4 blueLookAndFeel;
    juce::LookAndFeel_V4 redLookAndFeel;

    // UDP connector
    std::unique_ptr<UdpSender> m_conn;
     
    ComboBox modelSelect;
    ComboBox irSelect;

    IdPtrMap<NpRpcProto::EComboBoxId, juce::ComboBox*, static_cast<int>(NpRpcProto::EComboBoxId::MAX)> m_cboxMap;

    Slider ampGainKnob;
    Slider ampMasterKnob;
    Slider ampBassKnob;
    Slider ampMidKnob;
    Slider ampTrebleKnob;
    Slider ampPresenceKnob;
    Slider ampDelayKnob;
    Slider ampReverbKnob;

    IdPtrMap<NpRpcProto::ESliderId, juce::Slider*, static_cast<int>(NpRpcProto::ESliderId::MAX)> m_sliderMap;

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


    TextButton nextModelButton;
    TextButton prevModelButton;
    TextButton nextIrButton;
    TextButton prevIrButton;
    ToggleButton irButton;
    ToggleButton lstmButton;

    Label ampNameLabel{ {}, "Amp Name (no spaces): " };
    Label ampNameField{ {}, "NeuralPi" };

    Label gainLabel{ {}, "Gain" };
    Label masterLabel{ {}, "Master" };

    Label modelLabel{ {}, "Model" };

    TextButton connectButton;
    TextButton scanButton;
    ComboBox ipCbox;

private:
    void paint(Graphics& g) override;
    //====================================================================
    // UI initialization
    //====================================================================
    void initKnobSlider(juce::Slider& slider, float value, SliderListener* listener);

    //====================================================================
    // UI events
    //====================================================================
    void onConnectClicker();
    void onAbortClicker();
    void onScanClicked();

    void setNextComboBoxItem(ComboBox& cbox);
    void setPrevComboBoxItem(ComboBox& cbox);

    void setParamKnobColor(int params);
    void updateToggleState(juce::Button* button, juce::String name);

    //====================================================================
    // Listeners
    //====================================================================
    void buttonClicked(Button* button) override;
    void sliderValueChanged(Slider* slider) override;
    void comboBoxChanged(ComboBox* cbox) override;

    //====================================================================
    // IUdpClientListener implementation
    //====================================================================
    void updateKnob(int id, float value) override;
    void updateModelIndex(int id, int index) override;
    void addModelItem(int id, String itemValue, int itemIndex) override;
    void onStateChanged(IUdpRcListener::EState prevState, IUdpRcListener::EState state) override;
    void onBrReceived(const juce::String addr) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
