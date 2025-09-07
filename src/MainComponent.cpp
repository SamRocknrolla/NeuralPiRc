#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
#if JUCE_ANDROID
    openGLContext.attachTo(*this);
#endif
    
    blueLookAndFeel.setColour(juce::Slider::thumbColourId, juce::Colours::aqua);
    redLookAndFeel.setColour(juce::Slider::thumbColourId, juce::Colours::red);

    addAndMakeVisible(modelSelect);
    modelSelect.setColour(juce::Label::textColourId, juce::Colours::black);
    modelSelect.setScrollWheelEnabled(true);
    modelSelect.addListener(this);
    m_cboxMap.assign(NpRpcProto::EComboBoxId::Model, &modelSelect);

    addAndMakeVisible(nextModelButton);
    nextModelButton.setButtonText(">");
    nextModelButton.setColour(juce::Label::textColourId, juce::Colours::black);
    nextModelButton.addListener(this);
    nextModelButton.setEnabled(true);

    addAndMakeVisible(prevModelButton);
    prevModelButton.setButtonText("<");
    prevModelButton.setColour(juce::Label::textColourId, juce::Colours::black);
    prevModelButton.addListener(this);
    prevModelButton.setEnabled(true);

    addAndMakeVisible(irSelect);
    irSelect.setColour(juce::Label::textColourId, juce::Colours::black);
    irSelect.setScrollWheelEnabled(true);
    irSelect.addListener(this);
    m_cboxMap.assign(NpRpcProto::EComboBoxId::Ir, &irSelect);

    addAndMakeVisible(nextIrButton);
    nextIrButton.setColour(juce::Label::textColourId, juce::Colours::black);
    nextIrButton.addListener(this);
    nextIrButton.setButtonText(">");
    nextIrButton.setEnabled(true);

    addAndMakeVisible(prevIrButton);
    prevIrButton.setButtonText("<");
    prevIrButton.setColour(juce::Label::textColourId, juce::Colours::black);
    prevIrButton.addListener(this);
    prevIrButton.setEnabled(true);
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
    GainLabel.setJustificationType(juce::Justification::centredBottom);

    addAndMakeVisible(LevelLabel);
    LevelLabel.setText("Level", juce::NotificationType::dontSendNotification);
    LevelLabel.setJustificationType(juce::Justification::centredBottom);

    addAndMakeVisible(BassLabel);
    BassLabel.setText("Bass", juce::NotificationType::dontSendNotification);
    BassLabel.setJustificationType(juce::Justification::centredBottom);

    addAndMakeVisible(MidLabel);
    MidLabel.setText("Mid", juce::NotificationType::dontSendNotification);
    MidLabel.setJustificationType(juce::Justification::centredBottom);

    addAndMakeVisible(TrebleLabel);
    TrebleLabel.setText("Treble", juce::NotificationType::dontSendNotification);
    TrebleLabel.setJustificationType(juce::Justification::centredBottom);

    addAndMakeVisible(PresenceLabel);
    PresenceLabel.setText("Presence", juce::NotificationType::dontSendNotification);
    PresenceLabel.setJustificationType(juce::Justification::centredBottom);

    addAndMakeVisible(DelayLabel);
    DelayLabel.setText("Delay", juce::NotificationType::dontSendNotification);
    DelayLabel.setJustificationType(juce::Justification::centredBottom);

    addAndMakeVisible(ReverbLabel);
    ReverbLabel.setText("Reverb", juce::NotificationType::dontSendNotification);
    ReverbLabel.setJustificationType(juce::Justification::centredBottom);

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
#if JUCE_ANDROID
    auto display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay();
    Rectangle<int> screenSize = display->userArea;
    setSize(screenSize.getWidth(), screenSize.getHeight());
#else
    setSize(345, 380);
#endif
    setupIU();

    // Set gain knob color based on conditioned/snapshot model
    //setParamKnobColor(processor.params);

    m_conn = std::make_unique<UdpSender>(NpRpcProto::NPRPC_CLN_PORT, NpRpcProto::NPRPC_MCAST_ADDR, *this);
    m_conn->startThread();
}

MainComponent::~MainComponent()
{
    stopTimer();
    m_conn->signalThreadShouldExit();
    m_conn->stopThread(1000);
#if JUCE_ANDRIOD
    openGLContext.detach();
#endif
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

void MainComponent::resized() {
    setupIU();
}
void MainComponent::setupIU()
{
    auto area = getLocalBounds().reduced(10); // Add some margin
    const float dpi = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()->dpi;
    const float scale = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()->scale;
    const float mm =  dpi / 25.4f / scale ;
    const int rowH = mm * 8.0f;
    const int butW = mm * 8.0f;
    const int lblH = mm * 4.0f;
    const int spacing = mm / 2;
    FlexItem::Margin lblMargin{mm * 2, 0.0, 0.0, 0.0};
    FlexItem::Margin knobMargin{0.0, 0.0, mm * 2, 0.0};
    const int knobH = area.getWidth() / 4;
    // 1 cm in pixels

    // === ROW 1: Model Select + Load Button ===
    FlexBox row1;
    row1.flexDirection = FlexBox::Direction::row;
    row1.justifyContent = FlexBox::JustifyContent::center;
    row1.items.add(FlexItem(prevModelButton).withWidth(butW).withMargin(spacing));
    row1.items.add(FlexItem(modelSelect).withFlex(4).withMargin(spacing));
    row1.items.add(FlexItem(nextModelButton).withWidth(butW).withMargin(spacing));

    // === ROW 2: IR Select + Load IR + IR Button ===
    FlexBox row2;
    row2.flexDirection = FlexBox::Direction::row;
    row2.justifyContent = FlexBox::JustifyContent::center;
    row2.items.add(FlexItem(prevIrButton).withWidth(butW).withMargin(spacing));
    row2.items.add(FlexItem(irSelect).withFlex(4).withMargin(spacing));
    row2.items.add(FlexItem(nextIrButton).withWidth(butW).withMargin(spacing));

    // === ROW 3: Top Knob Labels (Gain, Master, Delay, Reverb) ===
    FlexBox labelRow1;
    labelRow1.flexDirection = FlexBox::Direction::row;
    labelRow1.justifyContent = FlexBox::JustifyContent::spaceBetween;
    labelRow1.items.add(FlexItem(GainLabel).withFlex(1));
    labelRow1.items.add(FlexItem(LevelLabel).withFlex(1));
    labelRow1.items.add(FlexItem(DelayLabel).withFlex(1));
    labelRow1.items.add(FlexItem(ReverbLabel).withFlex(1));

    // === ROW 4: Top Knob Row (Gain, Master, Delay, Reverb) ===
    FlexBox knobRow1;
    knobRow1.flexDirection = FlexBox::Direction::row;
    knobRow1.justifyContent = FlexBox::JustifyContent::spaceBetween;
    knobRow1.items.add(FlexItem(ampGainKnob).withFlex(1));
    knobRow1.items.add(FlexItem(ampMasterKnob).withFlex(1));
    knobRow1.items.add(FlexItem(ampDelayKnob).withFlex(1));
    knobRow1.items.add(FlexItem(ampReverbKnob).withFlex(1));

    // === ROW 5: Bottom Knob Labels (Bass, Mid, Treble, Presence) ===
    FlexBox labelRow2;
    labelRow2.flexDirection = FlexBox::Direction::row;
    labelRow2.justifyContent = FlexBox::JustifyContent::spaceBetween;
    labelRow2.items.add(FlexItem(BassLabel).withFlex(1));
    labelRow2.items.add(FlexItem(MidLabel).withFlex(1));
    labelRow2.items.add(FlexItem(TrebleLabel).withFlex(1));
    labelRow2.items.add(FlexItem(PresenceLabel).withFlex(1));

    // === ROW 6: Bottom Knob Row (Bass, Mid, Treble, Presence) ===
    FlexBox knobRow2;
    knobRow2.flexDirection = FlexBox::Direction::row;
    knobRow2.justifyContent = FlexBox::JustifyContent::spaceBetween;
    knobRow2.items.add(FlexItem(ampBassKnob).withFlex(1));
    knobRow2.items.add(FlexItem(ampMidKnob).withFlex(1));
    knobRow2.items.add(FlexItem(ampTrebleKnob).withFlex(1));
    knobRow2.items.add(FlexItem(ampPresenceKnob).withFlex(1));

    // === ROW 7: Connection Area ===
    FlexBox connectionRow;
    connectionRow.flexDirection = FlexBox::Direction::row;
    connectionRow.items.add(FlexItem(ipCbox).withFlex(3).withMargin(spacing));
    connectionRow.items.add(FlexItem(scanButton).withFlex(1).withMargin(spacing));

    // === ROW 8: Connect Button ===
    FlexBox connectRow;
    connectRow.items.add(FlexItem(connectButton).withFlex(1).withMargin(spacing));

    // === OUTER FlexBox ===
    FlexBox mainFlex;
    mainFlex.flexDirection = FlexBox::Direction::column;

    mainFlex.items.add(FlexItem().withHeight(lblH).withFlex(0).withMargin(spacing));
    mainFlex.items.add(FlexItem(row1).withHeight(rowH));
    mainFlex.items.add(FlexItem(row2).withHeight(rowH));
    mainFlex.items.add(FlexItem(labelRow1).withHeight(lblH).withMargin(lblMargin));
    mainFlex.items.add(FlexItem(knobRow1).withHeight(knobH).withMargin(knobMargin));
    mainFlex.items.add(FlexItem(labelRow2).withHeight(lblH).withMargin(lblMargin));
    mainFlex.items.add(FlexItem(knobRow2).withHeight(knobH).withMargin(knobMargin));
    mainFlex.items.add(FlexItem(connectionRow).withHeight(rowH).withMargin(spacing));
    mainFlex.items.add(FlexItem(connectRow).withHeight(rowH).withMargin(spacing));

    mainFlex.performLayout(area.toFloat());
}



void MainComponent::timerCallback() {
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
    else if (button == &nextModelButton) {
        setNextComboBoxItem(modelSelect);
    }
    else if (button == &prevModelButton) {
        setPrevComboBoxItem(modelSelect);
    }
    else if (button == &nextIrButton) {
        setNextComboBoxItem(irSelect);
    }
    else if (button == &prevIrButton) {
        setPrevComboBoxItem(irSelect);
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

void MainComponent::setNextComboBoxItem(ComboBox& cbox) {
    if (cbox.getNumItems() > 1) {
        int index = cbox.getSelectedItemIndex() + 1;
        index = (index >= cbox.getNumItems())? 0 : index;
        cbox.setSelectedItemIndex(index);
    }
}
void MainComponent::setPrevComboBoxItem(ComboBox& cbox) {
    if (cbox.getNumItems() > 1) {
        int index = cbox.getSelectedItemIndex() - 1;
        index = (index < 0)? cbox.getNumItems() - 1 : index;
        cbox.setSelectedItemIndex(index);
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
