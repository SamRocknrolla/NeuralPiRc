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
                     , public IUdpListener
                     //, private juce::AsyncUpdater
                     , private Button::Listener
                     , private Slider::Listener
                     , private Value::Listener
                     , private Label::Listener
                     , private Timer
{
public:
    //==============================================================================
    enum EKnobId {
        EKnobId_Gain
        , EKnobId_Master
        , EKnobId_Delay
        , EKnobId_Reverb
        , EKnobId_Bass
        , EKnobId_Mid
        , EKnobId_Treble
        , EKnobId_Presence

        , EKnobId_MAX
    };

    inline static const String constDefBcAddr = "227.0.0.1";
    static const juce::int32 constDefLocPort{ 24025 };
    static const juce::int32 constDefRemPort{ 24024 };

    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    //AmpOSCReceiver oscReceiver;
    //OSCSender oscSender;

    String outgoingIP{ "127.0.0.1" };
    int outgoingPort{ 25024 };
    int incomingPort{ 24024 };

    String ampName{ "NeuralPiRpc" };
    String gainAddressPattern{ "/parameter/NeuralPi/Gain" };
    String masterAddressPattern{ "/parameter/NeuralPi/Master" };
    String modelAddressPattern{ "/parameter/NeuralPi/Model" };
    String irAddressPattern{ "/parameter/NeuralPi/Ir" };
    String bassAddressPattern{ "/parameter/NeuralPi/Bass" };
    String midAddressPattern{ "/parameter/NeuralPi/Mid" };
    String trebleAddressPattern{ "/parameter/NeuralPi/Treble" };
    String presenceAddressPattern{ "/parameter/NeuralPi/Presence" };
    String delayAddressPattern{ "/parameter/NeuralPi/Delay" };
    String reverbAddressPattern{ "/parameter/NeuralPi/Reverb" };

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

    // Param data storage
    const Identifier paramStorageName{ "NeuralPiRc" };
    ValueTree paramStorage;
    void initParamStorage();

    // UDP connector
    std::unique_ptr<UdpSender> m_conn;
     
    // Amp Widgets
    Slider ampGainKnob;
    Slider ampMasterKnob;
    Slider modelKnob;
    Slider irKnob;
    //ImageButton ampOnButton;
    //ImageButton ampLED;
    ComboBox modelSelect;
    ComboBox irSelect;
    Slider ampBassKnob;
    Slider ampMidKnob;
    Slider ampTrebleKnob;
    Slider ampPresenceKnob;
    Slider ampDelayKnob;
    Slider ampReverbKnob;

    Slider* knobSliders[EKnobId_MAX] = {   &ampGainKnob, &ampMasterKnob, &ampDelayKnob, &ampReverbKnob 
                                         , &ampBassKnob, &ampMidKnob, &ampTrebleKnob, & ampPresenceKnob};

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

    void modelSelectChanged();
    void updateToggleState(juce::Button* button, juce::String name);
    void irSelectChanged();

    virtual void buttonClicked(Button* button) override;
    virtual void sliderValueChanged(Slider* slider) override;

    Label ampNameLabel{ {}, "Amp Name (no spaces): " };
    Label ampNameField{ {}, "NeuralPi" };

    //Label ipLabel{ {}, "Target IP Address: " };
    //Label ipField{ {}, "127.0.0.1" };

    //Label outPortNumberLabel{ {}, "Outgoing OSC Port: " };
    //Label outPortNumberField{ {}, "25024" };

    //Label inPortNumberLabel{ {}, "Incoming OSC Port: " };
    //Label inPortNumberField{ {}, "24024" };

    Label gainLabel{ {}, "Gain" };
    Label masterLabel{ {}, "Master" };

    Label modelLabel{ {}, "Model" };

    Label inConnectedLabel{ "(connected)" };
    Label outConnectedLabel{ "(connected)" };

    TextButton connectButton;
    TextButton scanButton;
    ComboBox ipCbox;

    void onConnectClicker();
    void onAbortClicker();
    void onScanClicked();


    // OSC Messages
    Slider& getGainSlider();
    Slider& getMasterSlider();
    Slider& getModelSlider();
    Slider& getIrSlider();
    Slider& getBassSlider();
    Slider& getMidSlider();
    Slider& getTrebleSlider();
    Slider& getPresenceSlider();
    Slider& getDelaySlider();
    Slider& getReverbSlider();

    Label& getOutPortNumberField();
    Label& getInPortNumberField();
    Label& getIPField();
    Label& getAmpNameField();
    Label& getOutConnectedLabel();
    Label& getInConnectedLabel();

    void buildAddressPatterns();
    void connectSender();
    void updateOutgoingIP(String ip);
    void updateOutgoingPort(int port);
    void labelTextChanged(Label* labelThatHasChanged) override;
    void updateInConnectedLabel();
    void updateOutConnectedLabel(bool connected);
    // This callback is invoked if an OSC message has been received setting either value.
    void valueChanged(Value& value) override;

    void timerCallback() override;

    void setParamKnobColor(int params);

    float getParameterValue(const Identifier& paramId) {
        return paramStorage.getPropertyAsValue(paramId, nullptr).getValue();
    }

    void setParameterValue(const Identifier& paramId, float value) {
        paramStorage.setProperty(paramId, value, nullptr);
    }
    // IUdpListener impl
    virtual void updateKnob(int id, float value) override;
    virtual void updateStrList(int id, String value) override;
    virtual void onStateChanged(IUdpListener::EState prevState, IUdpListener::EState state) override;
    virtual void onBrReceived(const juce::String addr) override;

    // This is called in the message thread to update the UI after the data has been received.

    //void handleAsyncUpdate() override
    //{
        // Update each slider value from the atomic variables in a thread-safe manner
        //for (int i = 0; i < EKnobId_MAX; ++i)
        //{
        //    knobSliders[i]->setValue(knobValues[i].get(), juce::dontSendNotification);
        //}
    //}
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
