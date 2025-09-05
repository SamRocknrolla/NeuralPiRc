#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    blueLookAndFeel.setColour(juce::Slider::thumbColourId, juce::Colours::aqua);
    redLookAndFeel.setColour(juce::Slider::thumbColourId, juce::Colours::red);

    addAndMakeVisible(modelSelect);
    modelSelect.setColour(juce::Label::textColourId, juce::Colours::black);
    modelSelect.setScrollWheelEnabled(true);
    modelSelect.addListener(this);
    m_cboxMap.assign(NpRpcProto::EComboBoxId::Model, &modelSelect);

    addAndMakeVisible(loadButton);
    loadButton.setButtonText("Import Tone");
    loadButton.setColour(juce::Label::textColourId, juce::Colours::black);
    loadButton.addListener(this);
    loadButton.setEnabled(false);

    addAndMakeVisible(irSelect);
    irSelect.setColour(juce::Label::textColourId, juce::Colours::black);
    irSelect.setScrollWheelEnabled(true);
    irSelect.addListener(this);
    m_cboxMap.assign(NpRpcProto::EComboBoxId::Ir, &irSelect);

    addAndMakeVisible(loadIR);
    loadIR.setButtonText("Import IR");
    loadIR.setColour(juce::Label::textColourId, juce::Colours::black);
    loadIR.addListener(this);
    loadIR.setEnabled(false);

    // Toggle IR
    //addAndMakeVisible(irButton); // Toggle is for testing purposes
    irButton.setToggleState(true, juce::NotificationType::dontSendNotification);
    irButton.onClick = [this] { updateToggleState(&irButton, "IR");   };

    // Toggle LSTM
    //addAndMakeVisible(lstmButton); // Toggle is for testing purposes
    lstmButton.setToggleState(true, juce::NotificationType::dontSendNotification);
    lstmButton.onClick = [this] { updateToggleState(&lstmButton, "LSTM");   };
  

    initKnobSlider(ampGainKnob, constDefSliderVal, this);
    m_sliderMap.assign(NpRpcProto::ESliderId::Gain, &ampGainKnob);

    initKnobSlider(ampMasterKnob, constDefSliderVal, this);
    m_sliderMap.assign(NpRpcProto::ESliderId::Master, &ampMasterKnob);

    initKnobSlider(ampDelayKnob, constDefSliderVal, this);
    m_sliderMap.assign(NpRpcProto::ESliderId::Delay, &ampDelayKnob);

    initKnobSlider(ampReverbKnob, constDefSliderVal, this);
    m_sliderMap.assign(NpRpcProto::ESliderId::Reverb, &ampReverbKnob);

    initKnobSlider(ampBassKnob, constDefSliderVal, this);
    m_sliderMap.assign(NpRpcProto::ESliderId::Bass, &ampBassKnob);

    initKnobSlider(ampMidKnob, constDefSliderVal, this);
    m_sliderMap.assign(NpRpcProto::ESliderId::Mid, &ampMidKnob);

    initKnobSlider(ampTrebleKnob, constDefSliderVal, this);
    m_sliderMap.assign(NpRpcProto::ESliderId::Treble, &ampTrebleKnob);

    initKnobSlider(ampPresenceKnob, constDefSliderVal, this);
    m_sliderMap.assign(NpRpcProto::ESliderId::Presence, &ampPresenceKnob);

    addAndMakeVisible(GainLabel);
    GainLabel.setText("Gain", juce::NotificationType::dontSendNotification);
    GainLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(LevelLabel);
    LevelLabel.setText("Level", juce::NotificationType::dontSendNotification);
    LevelLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(BassLabel);
    BassLabel.setText("Bass", juce::NotificationType::dontSendNotification);
    BassLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(MidLabel);
    MidLabel.setText("Mid", juce::NotificationType::dontSendNotification);
    MidLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(TrebleLabel);
    TrebleLabel.setText("Treble", juce::NotificationType::dontSendNotification);
    TrebleLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(PresenceLabel);
    PresenceLabel.setText("Presence", juce::NotificationType::dontSendNotification);
    PresenceLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(DelayLabel);
    DelayLabel.setText("Delay", juce::NotificationType::dontSendNotification);
    DelayLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(ReverbLabel);
    ReverbLabel.setText("Reverb", juce::NotificationType::dontSendNotification);
    ReverbLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(toneDropDownLabel);
    toneDropDownLabel.setText("Tone", juce::NotificationType::dontSendNotification);
    toneDropDownLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(irDropDownLabel);
    irDropDownLabel.setText("IR", juce::NotificationType::dontSendNotification);
    irDropDownLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(versionLabel);
    versionLabel.setText("v1.3.0", juce::NotificationType::dontSendNotification);
    versionLabel.setJustificationType(juce::Justification::centred);

    auto font = GainLabel.getFont();
    float height = font.getHeight();
    font.setHeight(height); // 0.75);
    GainLabel.setFont(font);
    LevelLabel.setFont(font);
    BassLabel.setFont(font);
    MidLabel.setFont(font);
    TrebleLabel.setFont(font);
    PresenceLabel.setFont(font);
    DelayLabel.setFont(font);
    ReverbLabel.setFont(font);
    toneDropDownLabel.setFont(font);
    irDropDownLabel.setFont(font);
    versionLabel.setFont(font);


    // Name controls:
    addAndMakeVisible(ampNameLabel);
    ampNameField.setEditable(true, true, true);
    addAndMakeVisible(ampNameField);

    addAndMakeVisible(connectButton);
    connectButton.setButtonText("Connect");
    connectButton.setColour(juce::Label::textColourId, juce::Colours::black);
    connectButton.addListener(this);
    connectButton.setEnabled(false);

    addAndMakeVisible(scanButton);
    scanButton.setButtonText("Scan");
    scanButton.setColour(juce::Label::textColourId, juce::Colours::black);
    scanButton.addListener(this);

    addAndMakeVisible(ipCbox);
    ipCbox.setEditableText(false);
    ipCbox.setColour(juce::Label::textColourId, juce::Colours::black);

    // Size of plugin GUI
    setSize(345, 455);

    // Set gain knob color based on conditioned/snapshot model 
    //setParamKnobColor(processor.params);

    m_conn = std::make_unique<UdpSender>(NpRpcProto::NPRPC_CLN_PORT, NpRpcProto::NPRPC_MCAST_ADDR, *this);
    m_conn->startThread();
}

MainComponent::~MainComponent()
{
}

void MainComponent::initKnobSlider(juce::Slider& slider, float value, SliderListener* listener) {
    addAndMakeVisible(slider);
    slider.setLookAndFeel(&blueLookAndFeel);
    slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 50, 20);
    slider.setNumDecimalPlacesToDisplay(1);
    slider.addListener(this);
    slider.setRange(0.0, 1.0);
    slider.setValue(0.0);
    slider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 50, 20);
    slider.setNumDecimalPlacesToDisplay(2);
    slider.setDoubleClickReturnValue(true, 0.0);

    if (listener != nullptr)
        slider.addListener(listener);

    slider.setValue(value, NotificationType::dontSendNotification);
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    // Workaround for graphics on Windows builds (clipping code doesn't work correctly on Windows)
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    g.drawImageAt(background, 0, 0);  // Debug Line: Redraw entire background image
#else
// Redraw only the clipped part of the background image
    juce::Rectangle<int> ClipRect = g.getClipBounds();
    g.drawImage(background, ClipRect.getX(), ClipRect.getY(), ClipRect.getWidth(), ClipRect.getHeight(), ClipRect.getX(), ClipRect.getY(), ClipRect.getWidth(), ClipRect.getHeight());
#endif
}

void MainComponent::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    modelSelect.setBounds(11, 10, 270, 25);
    loadButton.setBounds(11, 74, 100, 25);

    irSelect.setBounds(11, 42, 270, 25);
    loadIR.setBounds(120, 74, 100, 25);
    irButton.setBounds(248, 42, 257, 25);
    lstmButton.setBounds(248, 10, 257, 25);

    // Amp Widgets
    ampGainKnob.setBounds(10, 120, 75, 95);
    ampMasterKnob.setBounds(95, 120, 75, 95);
    ampBassKnob.setBounds(10, 250, 75, 95);
    ampMidKnob.setBounds(95, 250, 75, 95);
    ampTrebleKnob.setBounds(180, 250, 75, 95);
    ampPresenceKnob.setBounds(265, 250, 75, 95);

    ampDelayKnob.setBounds(180, 120, 75, 95);
    ampReverbKnob.setBounds(265, 120, 75, 95);

    GainLabel.setBounds(6, 108, 80, 10);
    LevelLabel.setBounds(93, 108, 80, 10);
    BassLabel.setBounds(6, 238, 80, 10);
    MidLabel.setBounds(91, 238, 80, 10);
    TrebleLabel.setBounds(178, 238, 80, 10);
    PresenceLabel.setBounds(265, 238, 80, 10);
    DelayLabel.setBounds(178, 108, 80, 10);
    ReverbLabel.setBounds(265, 108, 80, 10);

    toneDropDownLabel.setBounds(267, 16, 80, 10);
    irDropDownLabel.setBounds(261, 48, 80, 10);
    versionLabel.setBounds(268, 431, 80, 10);

    addAndMakeVisible(ampNameLabel);
    ampNameField.setEditable(true, true, true);
    addAndMakeVisible(ampNameField);

    ipCbox.setBounds(15, 365, 270, 25);
    scanButton.setBounds(285, 365, 50, 25);

    connectButton.setBounds(15, 395, 320, 25);

}

void MainComponent::buttonClicked(juce::Button* button)
{
    if (button == &connectButton) {
        if (connectButton.getButtonText().compare("Connect") == 0) {
            onConnectClicker();
        }
        else if (connectButton.getButtonText().compare("Abort") == 0) {
            onAbortClicker();
        }
    }
    else if (button == &scanButton) {
        onScanClicked();
    }
}

void MainComponent::onConnectClicker() {
    m_conn->connect(ipCbox.getText());
    connectButton.setButtonText("Waiting ...");
    connectButton.setEnabled(false);
    scanButton.setEnabled(false);
    ipCbox.setEnabled(false);
}

void MainComponent::onAbortClicker() {
    if (m_conn->disconnect()) {
        connectButton.setButtonText("Aborting ...");
        connectButton.setEnabled(false);
    }
}

void MainComponent::sliderValueChanged(Slider* slider) {
    auto id = m_sliderMap.getId(slider);
    if (id != std::nullopt) {
        m_conn->updateKnob(static_cast<int32_t>(id.value()), static_cast<float>(slider->getValue()));
    }
}

void MainComponent::comboBoxChanged(ComboBox* cbox) {
    auto id = m_cboxMap.getId(cbox);
    if (id != std::nullopt) {
        m_conn->selectModel(static_cast<int32_t>(id.value()), cbox->getSelectedItemIndex());
    }
}

void MainComponent::onScanClicked() {
    ipCbox.clear();
    m_conn->scan();
}

void MainComponent::updateToggleState(juce::Button* /* button */, juce::String name)
{
//    if (name == "IR")
//        processor.ir_state = button->getToggleState();
//    else
//        processor.lstm_state = button->getToggleState();
}

void MainComponent::updateKnob(int id, float value) {
    Slider* slider = m_sliderMap.getPtr(static_cast<NpRpcProto::ESliderId>(id));
    if (slider != nullptr) {
        slider->setValue(value, NotificationType::dontSendNotification);
    }
}

void MainComponent::updateModelIndex(int id, int index) {
    ComboBox* cbox = m_cboxMap.getPtr(static_cast<NpRpcProto::EComboBoxId>(id));
    if (cbox != nullptr) {
        cbox->setSelectedItemIndex(index, NotificationType::dontSendNotification);
    }
}

void MainComponent::addModelItem(int id, String itemValue, int itemIndex) {
    ComboBox* cbox = m_cboxMap.getPtr(static_cast<NpRpcProto::EComboBoxId>(id));
    if (cbox != nullptr) {
        int i = cbox->indexOfItemId(itemIndex);
        if (i == -1)
            cbox->addItem(itemValue, itemIndex);
        else if (cbox->getItemText(i) != itemValue)
            cbox->changeItemText(itemIndex, itemValue);
    }
}

void MainComponent::onStateChanged(IUdpRcListener::EState /* prevState */ , IUdpRcListener::EState state) {
    switch (state) {
    case IUdpRcListener::EState::Disconnecting:
    case IUdpRcListener::EState::Error:
    case IUdpRcListener::EState::Idle:
        m_cboxMap.forEachAssigned([](NpRpcProto::EComboBoxId /* id */, juce::ComboBox* cbox) {
            cbox->clear();
        });
        m_sliderMap.forEachAssigned([](NpRpcProto::ESliderId /* id */, juce::Slider* slider) {
            slider->setValue(constDefSliderVal, NotificationType::dontSendNotification);
        });
        connectButton.setEnabled(ipCbox.getNumItems() != 0);
        connectButton.setButtonText("Connect");
        scanButton.setEnabled(true);
        ipCbox.setEnabled(true);
        break;
    case IUdpRcListener::EState::ReqScan:
    case IUdpRcListener::EState::Scanning:
    case IUdpRcListener::EState::Connecting:
        connectButton.setEnabled(false);
        scanButton.setEnabled(false);
        break;
    case IUdpRcListener::EState::Connected:
        connectButton.setButtonText("Abort");
        connectButton.setEnabled(false);
        connectButton.setEnabled(true);
        scanButton.setEnabled(false);
        break;
    default:
        break;
    }
}

void MainComponent::onBrReceived(const juce::String addr) {
    ipCbox.addItem(addr, ipCbox.getNumItems() + 1);
    if (ipCbox.getNumItems() == 1) {
        ipCbox.setSelectedItemIndex(0);
    }
}


void MainComponent::timerCallback()
{
}

void MainComponent::setParamKnobColor(int params)
{
    // If the knob is used for a parameter, change it to red
    if (params == 0) {
        ampGainKnob.setLookAndFeel(&blueLookAndFeel);
        ampMasterKnob.setLookAndFeel(&blueLookAndFeel);
    }
    else if (params == 1) {
        ampGainKnob.setLookAndFeel(&redLookAndFeel);
        ampMasterKnob.setLookAndFeel(&blueLookAndFeel);
    }
    else if (params == 2) {
        ampGainKnob.setLookAndFeel(&redLookAndFeel);
        ampMasterKnob.setLookAndFeel(&redLookAndFeel);
    }
}
